/*****************************************************************************
 *   Ledger App Boilerplate.
 *   (c) 2020 Ledger SAS.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

#include <stdint.h>   // uint*_t
#include <stddef.h>   // size_t
#include <stdbool.h>  // bool
#include <string.h>   // memmove
#include <stdlib.h>   // qsort

#include "mem.h"
#include "os.h"
#include "cx.h"
#include "ledger_assert.h"
#include "globals.h"

#include "party_id.h"
#include "utils.h"
#include "buffer.h"
#include "tx_types.h"
#include "sw.h"
#include "bytewriter.h"
#include "pb_parser.h"
#include "pb_node_display_parser.h"

#define HASH_LEN                                     34
#define HEX_LEN                                      (HASH_LEN * 2 + 1)
#define MAX_HASHES                                   3  // Adjust as needed
#define PURPOSE_TOPOLOGY_TRANSACTION_SIGNATURE       ((uint8_t) 11)
#define PURPOSE_MULTI_TOPOLOGY_TRANSACTION_SIGNATURE ((uint8_t) 55)
#define ONBOARDING_FLOW_DISPLAY_FIELDS               4

typedef struct {
    const char *item_name;
    size_t field_index;
} field_display_t;

// Namespace Delegation fields
const field_display_t NAMESPACE_FIELD = {"Namespace", 0};
const field_display_t PARTY_KEY_FIELD = {"Party Key", 1};
// Party to Key Mapping fields
const field_display_t PARTY_FIELD = {"Party ID", 2};
// Party to Participant fields
const field_display_t PARTICIPANT_UID_FIELD = {"Participant ID", 3};

static uint8_t (*tx_hashes)[HASH_LEN] = NULL;
static size_t hash_count = 0;
static bool has_parsed_namespace_delegation = false;
static bool has_parsed_party_to_participant = false;
static bool has_parsed_party_to_key_mapping = false;

static const char *REVIEW_TITLE = "Review transaction to add account";
static const char *REVIEW_FINISH = "Sign transaction to add account?";

static int parse_topology_transaction_for_display(buffer_t *buf);

static void init_hash_storage(void) {
    if (tx_hashes) {
        app_mem_free(tx_hashes);
    }
    tx_hashes = app_mem_alloc(MAX_HASHES * sizeof(uint8_t[HASH_LEN]));
    hash_count = 0;
}

static void cleanup_hash_storage() {
    if (tx_hashes) {
        app_mem_free(tx_hashes);
        tx_hashes = NULL;
        hash_count = 0;
    }
}

static void add_hash(const uint8_t hash[HASH_LEN]) {
    LEDGER_ASSERT(hash != NULL, "NULL hash");
    LEDGER_ASSERT(tx_hashes != NULL, "Hash storage not initialized");
    LEDGER_ASSERT(hash_count < MAX_HASHES, "Hash storage full");
    memcpy(tx_hashes[hash_count], hash, HASH_LEN);
    hash_count++;
}

static int compare_hashes_hex(const void *a, const void *b) {
    const uint8_t *hash_a = (const uint8_t *) a;
    const uint8_t *hash_b = (const uint8_t *) b;

    char hex_a[HEX_LEN];
    char hex_b[HEX_LEN];

    SNPRINTF(hex_a, sizeof(hex_a), "%.*h", HASH_LEN, hash_a);
    SNPRINTF(hex_b, sizeof(hex_b), "%.*h", HASH_LEN, hash_b);
    return strcmp(hex_a, hex_b);
}

void process_untyped_versioned_msg_tx_init(void) {
    init_hash_storage();
    init_transaction_pairs(&G_context.tx_info, ONBOARDING_FLOW_DISPLAY_FIELDS);
    has_parsed_namespace_delegation = false;
    has_parsed_party_to_participant = false;
    has_parsed_party_to_key_mapping = false;
}

static void compute_multi_hash(void) {
    // Allocate storage for a concatenated string of all hashes + their lengths
    size_t len = hash_count * HASH_LEN + hash_count * 4 +
                 4;  // Each hash prefixed by its length (4 bytes) + 4 bytes for count
    uint8_t *concat = app_mem_alloc(len);
    ByteWriter bw;
    bw_init(&bw, concat, len);
    bw_put_u32_be(&bw, hash_count);  // Prefix with number of hashes
    // Sort hashes lexicographically in hex format
    qsort(tx_hashes, hash_count, HASH_LEN, compare_hashes_hex);
    for (size_t i = 0; i < hash_count; i++) {
        // Concatenate each hash, prefixed them with their length (always 34)
        bw_put_u32_be(&bw, HASH_LEN);
        bw_put(&bw, tx_hashes[i], HASH_LEN);
    }
    // Compute final hash
    canton_hash(PURPOSE_MULTI_TOPOLOGY_TRANSACTION_SIGNATURE,
                concat,
                len,
                G_context.tx_info.m_hash);
    G_context.tx_info.m_hash_len = HASH_LEN;
    PRINTF("Final untyped versioned message hash: %.*H\n", HASH_LEN, G_context.tx_info.m_hash);
    app_mem_free(concat);
}

int process_untyped_versioned_msg_tx(buffer_t *buf) {
    UNUSED(buf);
    uint8_t h[HASH_LEN] = {0};

    canton_hash(PURPOSE_TOPOLOGY_TRANSACTION_SIGNATURE, buf->ptr, buf->size, h);
    add_hash(h);

    int ret = parse_topology_transaction_for_display(buf);
    if (ret != 0) {
        return ret;
    }

    if (G_context.state == STATE_PARSED) {
        compute_multi_hash();
        cleanup_hash_storage();
        if (has_parsed_namespace_delegation && has_parsed_party_to_key_mapping &&
            has_parsed_party_to_participant) {
            G_context.tx_info.clear_signing_available = true;
            // Allocate review title and finish strings
            G_context.tx_info.review_title = REVIEW_TITLE;
            G_context.tx_info.review_finish = REVIEW_FINISH;
        }
    }
    return 0;
}

// Helper function to set field value
static bool set_field_value(transaction_ctx_t *tx_info,
                            const field_display_t *field_config,
                            const char *value,
                            bool check_existing) {
    // Input validation
    if (field_config->field_index >= tx_info->pairs_count || value == NULL) {
        return false;
    }

    size_t value_len = strlen(value) + 1;

    // Check existing value if requested
    if (check_existing && tx_info->pairs[field_config->field_index].value != NULL) {
        PRINTF("Checking field number %d, %s against existing value %s\n",
               field_config->field_index,
               tx_info->pairs[field_config->field_index].item,
               tx_info->pairs[field_config->field_index].value);
        PRINTF("New value: %s\n", value);

        // Check for conflict
        if (strcmp(tx_info->pairs[field_config->field_index].value, value) != 0) {
            return false;  // Conflict detected
        }

        // Value matches existing, no need to reallocate
        return true;
    }

    // Allocate and set new value
    tx_info->display_items_strings[field_config->field_index] = (char *) app_mem_alloc(value_len);
    if (tx_info->display_items_strings[field_config->field_index] == NULL) {
        return false;  // Memory allocation failed
    }

    memcpy(tx_info->display_items_strings[field_config->field_index], value, value_len);
    tx_info->pairs[field_config->field_index].item = (char *) PIC(field_config->item_name);
    tx_info->pairs[field_config->field_index].value =
        tx_info->display_items_strings[field_config->field_index];

    return true;
}

static bool set_field_value_bytes_to_hex(transaction_ctx_t *tx_info,
                                         const field_display_t *field_config,
                                         const uint8_t *bytes,
                                         size_t byte_len,
                                         bool check_existing) {
    // Input validation
    if (field_config->field_index >= tx_info->pairs_count || bytes == NULL || byte_len == 0) {
        return false;
    }

    size_t hex_len = byte_len * 2 + 1;

    // Check existing value if requested
    if (check_existing && tx_info->pairs[field_config->field_index].value != NULL) {
        PRINTF("Checking field number %d, %s against existing value %s\n",
               field_config->field_index,
               tx_info->pairs[field_config->field_index].item,
               tx_info->pairs[field_config->field_index].value);
        PRINTF("New value: %.*H\n", byte_len, bytes);

        char *new_value_hex = (char *) app_mem_alloc(hex_len);
        if (new_value_hex == NULL) {
            return false;  // Memory allocation failed
        }

        SNPRINTF(new_value_hex, hex_len, "%.*h", byte_len, bytes);

        // Check for conflict
        bool conflict =
            (strlen(tx_info->pairs[field_config->field_index].value) != hex_len - 1) ||
            (memcmp(tx_info->pairs[field_config->field_index].value, new_value_hex, hex_len - 1) !=
             0);

        app_mem_free(new_value_hex);

        if (conflict) {
            return false;  // Conflict detected
        }

        // Value matches existing, no need to reallocate
        return true;
    }

    // Allocate and set new value
    tx_info->display_items_strings[field_config->field_index] = (char *) app_mem_alloc(hex_len);
    if (tx_info->display_items_strings[field_config->field_index] == NULL) {
        return false;  // Memory allocation failed
    }

    // Convert bytes to hex string
    SNPRINTF(tx_info->display_items_strings[field_config->field_index],
             hex_len,
             "%.*h",
             byte_len,
             bytes);

    tx_info->pairs[field_config->field_index].item = (char *) PIC(field_config->item_name);
    tx_info->pairs[field_config->field_index].value =
        tx_info->display_items_strings[field_config->field_index];

    PRINTF("Set field number %d, %s to hex value %s\n",
           field_config->field_index,
           tx_info->pairs[field_config->field_index].item,
           tx_info->pairs[field_config->field_index].value);

    return true;
}

// Process namespace delegation mapping
static int process_namespace_delegation(const NamespaceDelegation *delegation,
                                        transaction_ctx_t *tx_info) {
    LEDGER_ASSERT(!has_parsed_namespace_delegation, "Multiple namespace delegations found");
    LEDGER_ASSERT(delegation != NULL, "NULL namespace delegation");

    // Set namespace delegation specific fields
    if (delegation->namespace != NULL) {
        set_field_value(tx_info, &NAMESPACE_FIELD, delegation->namespace, false);
    } else {
        return -1;  // Namespace is required
    }

    if (delegation->has_target_key) {
        if (!set_field_value_bytes_to_hex(tx_info,
                                          &PARTY_KEY_FIELD,
                                          delegation->target_key.public_key.bytes,
                                          delegation->target_key.public_key.size,
                                          true)) {
            return -1;  // Conflict detected
        }
    } else {
        return -1;  // Target key is required
    }

    has_parsed_namespace_delegation = true;
    return 0;
}

// Process party to key mapping
static int process_party_to_key_mapping(const PartyToKeyMapping *mapping,
                                        transaction_ctx_t *tx_info) {
    // Set party to key mapping specific fields
    if (mapping->party != NULL) {
        if (!set_field_value(tx_info, &PARTY_FIELD, mapping->party, true)) {
            return -1;  // Conflict detected
        }
    } else {
        return -1;  // Party is required
    }

    // For signing keys, we'll show the first one or count if multiple
    if (mapping->signing_keys_count > 0) {
        com_digitalasset_canton_crypto_v30_SigningPublicKey *key = &mapping->signing_keys[0];
        // Check key value against existing value if already set
        if (!set_field_value_bytes_to_hex(tx_info,
                                          &PARTY_KEY_FIELD,
                                          key->public_key.bytes,
                                          key->public_key.size,
                                          true)) {
            return -1;  // Conflict detected
        }
    } else {
        return -1;  // 1 signing key is required
    }

    has_parsed_party_to_key_mapping = true;
    return 0;
}

// Process party to participant mapping
static int process_party_to_participant(const PartyToParticipant *mapping,
                                        transaction_ctx_t *tx_info) {
    // Set party to participant specific fields
    if (mapping->party != NULL) {
        set_field_value(tx_info, &PARTY_FIELD, mapping->party, true);
    } else {
        return -1;  // Party is required
    }

    if (mapping->participants_count > 0 && mapping->participants[0].participant_uid != NULL) {
        set_field_value(tx_info,
                        &PARTICIPANT_UID_FIELD,
                        mapping->participants[0].participant_uid,
                        false);
    } else {
        return -1;  // 1 participant is required
    }

    has_parsed_party_to_participant = true;
    return 0;
}

static int parse_topology_transaction_for_display(buffer_t *buf) {
    parser_status_e status = proto_deserialize_topology_transaction(buf, &G_context.tx_info);
    if (status != PARSING_OK) {
        return status;
    }

    if (G_context.tx_info.tx_parts_ctx.topology_transaction.has_mapping == false) {
        return 0;
    }

    if (G_context.tx_info.tx_parts_ctx.topology_transaction.operation !=
        com_digitalasset_canton_protocol_v30_Enums_TopologyChangeOp_TOPOLOGY_CHANGE_OP_ADD_REPLACE) {
        return -1;  // TODO: add relevant error code
    }

    switch (G_context.tx_info.tx_parts_ctx.topology_transaction.mapping.which_mapping) {
        case TOPOLOGY_MAPPING_NAMESPACE_DELEGATION_TAG:
            process_namespace_delegation(&G_context.tx_info.tx_parts_ctx.topology_transaction
                                              .mapping.mapping.namespace_delegation,
                                         &G_context.tx_info);
            break;
        case TOPOLOGY_MAPPING_PARTY_TO_PARTICIPANT_TAG:
            process_party_to_participant(&G_context.tx_info.tx_parts_ctx.topology_transaction
                                              .mapping.mapping.party_to_participant,
                                         &G_context.tx_info);
            break;
        case TOPOLOGY_MAPPING_PARTY_TO_KEY_MAPPING_TAG:
            process_party_to_key_mapping(&G_context.tx_info.tx_parts_ctx.topology_transaction
                                              .mapping.mapping.party_to_key_mapping,
                                         &G_context.tx_info);
            break;
        default:
            PRINTF("Unknown mapping type in topology transaction: %d\n",
                   G_context.tx_info.tx_parts_ctx.topology_transaction.mapping.which_mapping);
            return 0;
    }

    return 0;
}
