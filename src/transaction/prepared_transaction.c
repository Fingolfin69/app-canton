#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool
#include <stddef.h>   // size_t
#include <string.h>   // memset, explicit_bzero

#include "os.h"
#include "cx.h"
#include "buffer.h"
#include "mem.h"  // for app_mem_alloc

#include "sign_tx.h"
#include "sw.h"
#include "globals.h"
#include "display.h"
#include "tx_types.h"
#include "proto_deserialize.h"
#include "proto_deserialize_input_contract.h"
#include "prepared_transaction.h"
#include "validate.h"
#include "canonical_hash.h"

#define TRANSFER_CMD_DISPLAY_FIELDS 4

// Configuration structures
typedef struct {
    const char *package_id;
    const char *module_name;
    const char *entity_name;
} identifier_config_t;

// Review titles
static const char *TOKEN_TRANSFER_REVIEW_TITLE = "Review transaction to send tokens";
static const char *TOKEN_TRANSFER_REVIEW_FINISH = "Sign transaction to send tokens?";
static const char *NATIVE_COIN_TRANSFER_REVIEW_TITLE = "Review transaction to send Canton Coin";
static const char *NATIVE_COIN_TRANSFER_REVIEW_FINISH = "Sign transaction to send Canton Coin?";
static const char *PREAPPROVAL_PROPOSAL_REVIEW_TITLE =
    "Review transaction to pre-approve incoming transfers";
static const char *PREAPPROVAL_PROPOSAL_REVIEW_FINISH =
    "Sign transaction to pre-approve incoming transfers?";

// Configuration constants
static const identifier_config_t EXTERNAL_PARTY_AMULET_RULES_TEMPLATE = {
    .package_id = "95a88ff9ffd509e097802ecf3bbd58c83a5dff408e439cca4e2105ebd2cd0760",
    .module_name = "Splice.ExternalPartyAmuletRules",
    .entity_name = "ExternalPartyAmuletRules"};

static const identifier_config_t PREAPPROVAL_PREAPPROVAL_TEMPLATE = {
    .package_id = "b30bb727552cf6b624dbc9a5ff95f6c158e0a654e2e9c5c27bcfe3f5d0f9ada2",
    .module_name = "Splice.Wallet.TransferPreapproval",
    .entity_name = "TransferPreapprovalProposal"};

static const identifier_config_t TOKEN_TRANSFER_RECORD = {
    .package_id = "55ba4deb0ad4662c4168b39859738a0e91388d252286480c7331b3f71a517281",
    .module_name = "Splice.Api.Token.TransferInstructionV1",
    .entity_name = "TransferFactory_Transfer"};

static const identifier_config_t NATIVE_COIN_TRANSFER_RECORD = {
    .package_id = "95a88ff9ffd509e097802ecf3bbd58c83a5dff408e439cca4e2105ebd2cd0760",
    .module_name = "Splice.ExternalPartyAmuletRules",
    .entity_name = "ExternalPartyAmuletRules_CreateTransferCommand"};

// Field display configuration
// Transfer commands fields
const field_display_t SENDER_FIELD = {"Sender", 0};
const field_display_t RECEIVER_FIELD = {"Receiver", 1};
const field_display_t AMOUNT_FIELD = {"Amount", 2};
const field_display_t INSTRUMENT_ID_FIELD = {"Instrument ID", 3};
// Pre-approval proposal fields
const field_display_t PREAPPROVAL_RECEIVER_FIELD = {"Receiver", 0};
const field_display_t PROVIDER_FIELD = {"Provider", 1};

#define TOKEN_TRANSFER_CMD_DISPLAY_FIELDS       4
#define NATIVE_TRANSFER_CMD_DISPLAY_FIELDS      3
#define PREAPPROVAL_PROPOSAL_CMD_DISPLAY_FIELDS 2

typedef enum {
    RECEIVING_DAML_TX_PART,              /// Receiving part of DAML transaction
    RECEIVING_DAML_NODES,                /// Receiving DAML nodes
    RECEIVING_METADATA,                  /// Receiving metadata
    RECEIVING_METADATA_INPUT_CONTRACTS,  /// Receiving input contracts
} prepared_tx_receiving_state_e;

static prepared_tx_receiving_state_e tx_state = RECEIVING_DAML_TX_PART;
static int process_prepared_tx_finalize();
static void parse_node_for_display(const Node *node, transaction_ctx_t *tx_info);

void process_prepared_tx_init() {
    tx_state = RECEIVING_DAML_TX_PART;
}

int process_prepared_tx_part(buffer_t *buf) {
    switch (tx_state) {
        case RECEIVING_DAML_TX_PART: {
            parser_status_e status = proto_deserialize_daml_tx(buf, &G_context.tx_info);

            if (status != PARSING_OK) {
                PRINTF("Failed to parse DAML transaction part: %d\n", status);
                return SW_TX_PARSING_FAIL;
            }

            int res = hash_transaction(&G_context.tx_info.hasher,
                                       &G_context.tx_info.tx_parts_ctx.daml_transaction);

            if (res != 0) {
                PRINTF("Failed to hash DAML transaction part: %d\n", res);
                return SW_TX_HASH_FAIL;
            }

            G_context.tx_info.recv_node_idx = 0;
            tx_state = G_context.tx_info.tx_parts_ctx.daml_transaction.nodes_count == 0
                           ? RECEIVING_METADATA
                           : RECEIVING_DAML_NODES;
        } break;
        case RECEIVING_DAML_NODES: {
            parser_status_e status = proto_deserialize_node(buf, &G_context.tx_info);

            if (status != PARSING_OK) {
                PRINTF("Failed to parse DAML Node part: %d\n", status);
                return SW_TX_PARSING_FAIL;
            }

            int res = hash_node(&G_context.tx_info.hasher,
                                &G_context.tx_info.tx_parts_ctx.daml_transaction,
                                &G_context.tx_info.tx_parts_ctx.node);

            // Extract displayable fields from the node
            parse_node_for_display(&G_context.tx_info.tx_parts_ctx.node, &G_context.tx_info);

            release_node(&G_context.tx_info);

            if (res != 0) {
                PRINTF("Failed to hash DAML Node part: %d\n", res);
                return SW_TX_HASH_FAIL;
            }

            G_context.tx_info.recv_node_idx++;

            if (G_context.tx_info.recv_node_idx ==
                G_context.tx_info.tx_parts_ctx.daml_transaction.nodes_count) {
                tx_state = RECEIVING_METADATA;
            }
        } break;
        case RECEIVING_METADATA: {
            int res = finalize_hash_transaction(&G_context.tx_info.hasher,
                                                G_context.tx_info.partial_tx_hash);
            release_daml_tx(&G_context.tx_info);

            if (res != 0) {
                PRINTF("Failed to finalize DAML transaction hash: %d\n", res);
                return SW_TX_HASH_FAIL;
            }

            parser_status_e status = proto_deserialize_metadata(buf, &G_context.tx_info);

            if (status != PARSING_OK) {
                PRINTF("Failed to parse Metadata part: %d\n", status);
                return SW_TX_PARSING_FAIL;
            }

            res =
                hash_metadata(&G_context.tx_info.hasher, &G_context.tx_info.tx_parts_ctx.metadata);

            release_metadata(&G_context.tx_info);

            if (res != 0) {
                PRINTF("Failed to hash Metadata part: %d\n", res);
                return SW_TX_HASH_FAIL;
            }

            G_context.tx_info.recv_node_idx = 0;

            if (G_context.tx_info.tx_parts_ctx.metadata.input_contracts_count == 0) {
                return process_prepared_tx_finalize();
            } else {
                tx_state = RECEIVING_METADATA_INPUT_CONTRACTS;
            }

        } break;
        case RECEIVING_METADATA_INPUT_CONTRACTS: {
            parser_status_e status = proto_deserialize_cb_input_contract(buf, &G_context.tx_info);

            if (status != PARSING_OK) {
                PRINTF("Failed to parse Input Contract part: %d\n", status);
                return SW_TX_PARSING_FAIL;
            }

            release_cb_input_contract(&G_context.tx_info);

            G_context.tx_info.recv_node_idx++;

            if (G_context.tx_info.recv_node_idx ==
                G_context.tx_info.tx_parts_ctx.metadata.input_contracts_count) {
                return process_prepared_tx_finalize();
            }
        } break;
        default:
            PRINTF("Invalid state during processing prepared tx part: %d\n", tx_state);
            return SW_BAD_STATE;
    }

    return 0;
}

static int process_prepared_tx_finalize() {
    if (G_context.state != STATE_PARSED) {
        PRINTF("Invalid state: expected STATE_PARSED, got %d\n", G_context.state);
        return SW_BAD_STATE;
    }

    int res = finalize_hash_metadata(&G_context.tx_info.hasher, G_context.tx_info.partial_md_hash);

    if (res != 0) {
        PRINTF("Failed to finalize Metadata hash: %d\n", res);
        return SW_TX_HASH_FAIL;
    }

    // Finalize hash
    res = finalize_hash(G_context.tx_info.partial_tx_hash,
                        G_context.tx_info.partial_md_hash,
                        G_context.tx_info.m_hash);

    G_context.tx_info.m_hash_len = 32;

    if (res != 0) {
        PRINTF("Failed to compute transaction hash: %d\n", res);
        return SW_TX_HASH_FAIL;
    }

    return 0;
}

// Helper function to match identifiers
static bool match_identifier(const Identifier *id, const identifier_config_t *config) {
    return strcmp(id->package_id, (char *) PIC(config->package_id)) == 0 &&
           strcmp(id->module_name, (char *) PIC(config->module_name)) == 0 &&
           strcmp(id->entity_name, (char *) PIC(config->entity_name)) == 0;
}

// Helper function to initialize transaction pairs
bool init_transaction_pairs(transaction_ctx_t *tx_info, size_t count) {
    // Free existing pairs if any
    if (tx_info->pairs != NULL) {
        app_mem_free(tx_info->pairs);
    }

    // Free existing allocated items strings if any
    if (tx_info->display_items_strings != NULL) {
        // Free individual strings first
        for (size_t i = 0; i < tx_info->pairs_count; i++) {
            if (tx_info->display_items_strings[i] != NULL) {
                app_mem_free(tx_info->display_items_strings[i]);
            }
        }
        app_mem_free(tx_info->display_items_strings);
    }

    // Allocate new arrays
    tx_info->pairs_count = count;
    tx_info->pairs =
        (nbgl_contentTagValue_t *) app_mem_alloc(count * sizeof(nbgl_contentTagValue_t));
    tx_info->display_items_strings = (char **) app_mem_alloc(count * sizeof(char *));

    if (tx_info->pairs == NULL || tx_info->display_items_strings == NULL) {
        tx_info->pairs_count = 0;
        return false;
    }

    memset(tx_info->pairs, 0, count * sizeof(nbgl_contentTagValue_t));
    memset(tx_info->display_items_strings, 0, count * sizeof(char *));

    return true;
}

// Helper function to set field value safely with context-managed memory
static void set_field_value(transaction_ctx_t *tx_info,
                            const field_display_t *field_config,
                            const char *value) {
    if (field_config->field_index < TRANSFER_CMD_DISPLAY_FIELDS && value != NULL) {
        size_t value_len = strlen(value) + 1;

        // Allocate memory in the context's display_items_strings array
        tx_info->display_items_strings[field_config->field_index] =
            (char *) app_mem_alloc(value_len);
        if (tx_info->display_items_strings[field_config->field_index] != NULL) {
            memcpy(tx_info->display_items_strings[field_config->field_index], value, value_len);
            tx_info->pairs[field_config->field_index].item = (char *) PIC(field_config->item_name);
            tx_info->pairs[field_config->field_index].value =
                tx_info->display_items_strings[field_config->field_index];
        }
    }
}

// Process token transfer fields
static void process_token_transfer_fields(const com_daml_ledger_api_v2_Record *rec,
                                          transaction_ctx_t *tx_info) {
    // Single pass through top-level fields
    for (size_t i = 0; i < rec->fields_count; i++) {
        const RecordField *field = &rec->fields[i];

        if (strcmp(field->label, "transfer") == 0) {
            const com_daml_ledger_api_v2_Record *transfer_rec = &field->value->record;
            for (size_t j = 0; j < transfer_rec->fields_count; j++) {
                const RecordField *transfer_field = &transfer_rec->fields[j];

                if (strcmp(transfer_field->label, "sender") == 0) {
                    set_field_value(tx_info, &SENDER_FIELD, transfer_field->value->party);
                } else if (strcmp(transfer_field->label, "receiver") == 0) {
                    set_field_value(tx_info, &RECEIVER_FIELD, transfer_field->value->party);
                } else if (strcmp(transfer_field->label, "amount") == 0) {
                    set_field_value(tx_info, &AMOUNT_FIELD, transfer_field->value->numeric);
                } else if (strcmp(transfer_field->label, "instrumentId") == 0) {
                    const com_daml_ledger_api_v2_Record *inst_id_rec =
                        &transfer_field->value->record;
                    // Single pass through instrument ID fields
                    for (size_t k = 0; k < inst_id_rec->fields_count; k++) {
                        const RecordField *inst_id_field = &inst_id_rec->fields[k];
                        if (strcmp(inst_id_field->label, "id") == 0) {
                            set_field_value(tx_info,
                                            &INSTRUMENT_ID_FIELD,
                                            inst_id_field->value->text);
                        }
                    }
                }
            }
        }
    }
}

// Process token transfer fields
static void process_native_transfer_fields(const com_daml_ledger_api_v2_Record *rec,
                                           transaction_ctx_t *tx_info) {
    // Single pass through top-level fields
    for (size_t i = 0; i < rec->fields_count; i++) {
        const RecordField *field = &rec->fields[i];
        if (strcmp(field->label, "sender") == 0) {
            set_field_value(tx_info, &SENDER_FIELD, field->value->party);
        } else if (strcmp(field->label, "receiver") == 0) {
            set_field_value(tx_info, &RECEIVER_FIELD, field->value->party);
        } else if (strcmp(field->label, "amount") == 0) {
            set_field_value(tx_info, &AMOUNT_FIELD, field->value->numeric);
        }
    }
}

// Process token transfer fields
static void process_preapproval_proposal_fields(const com_daml_ledger_api_v2_Record *rec,
                                                transaction_ctx_t *tx_info) {
    // Single pass through top-level fields
    for (size_t i = 0; i < rec->fields_count; i++) {
        const RecordField *field = &rec->fields[i];
        if (strcmp(field->label, "receiver") == 0) {
            set_field_value(tx_info, &PREAPPROVAL_RECEIVER_FIELD, field->value->party);
        } else if (strcmp(field->label, "provider") == 0) {
            set_field_value(tx_info, &PROVIDER_FIELD, field->value->party);
        }
    }
}

// Helper function to process transfer command
static void process_token_transfer_command(const com_daml_ledger_api_v2_Record *rec,
                                           transaction_ctx_t *tx_info) {
    if (!init_transaction_pairs(tx_info, TRANSFER_CMD_DISPLAY_FIELDS)) {
        return;
    }
    process_token_transfer_fields(rec, tx_info);
    tx_info->clear_signing_available = true;
    tx_info->review_title = TOKEN_TRANSFER_REVIEW_TITLE;
    tx_info->review_finish = TOKEN_TRANSFER_REVIEW_FINISH;
}

static void process_native_transfer_command(const com_daml_ledger_api_v2_Record *rec,
                                            transaction_ctx_t *tx_info) {
    if (!init_transaction_pairs(tx_info, NATIVE_TRANSFER_CMD_DISPLAY_FIELDS)) {
        return;
    }
    process_native_transfer_fields(rec, tx_info);
    tx_info->clear_signing_available = true;
    tx_info->review_title = NATIVE_COIN_TRANSFER_REVIEW_TITLE;
    tx_info->review_finish = NATIVE_COIN_TRANSFER_REVIEW_FINISH;
}

// Helper function to process exercise node
static void process_exercise_node(const Node_Exercise *exercise, transaction_ctx_t *tx_info) {
    if (!exercise->has_template_id) {
        return;
    }
    // Check if this is a transfer command
    if (match_identifier(&exercise->template_id, &EXTERNAL_PARTY_AMULET_RULES_TEMPLATE)) {
        if (match_identifier(&exercise->chosen_value.record.record_id, &TOKEN_TRANSFER_RECORD)) {
            process_token_transfer_command(&exercise->chosen_value.record, tx_info);
        } else if (match_identifier(&exercise->chosen_value.record.record_id,
                                    &NATIVE_COIN_TRANSFER_RECORD)) {
            process_native_transfer_command(&exercise->chosen_value.record, tx_info);
        }
    }
}

static void process_preapproval_proposal_command(const com_daml_ledger_api_v2_Record *rec,
                                                 transaction_ctx_t *tx_info) {
    if (!init_transaction_pairs(tx_info, PREAPPROVAL_PROPOSAL_CMD_DISPLAY_FIELDS)) {
        return;
    }
    process_preapproval_proposal_fields(rec, tx_info);
    tx_info->clear_signing_available = true;
    tx_info->review_title = PREAPPROVAL_PROPOSAL_REVIEW_TITLE;
    tx_info->review_finish = PREAPPROVAL_PROPOSAL_REVIEW_FINISH;
}

static void process_create_node(const Node_Create *create, transaction_ctx_t *tx_info) {
    if (!create->has_template_id) {
        return;
    }
    // Check if this is a pre-approval proposal command
    if (match_identifier(&create->template_id, &PREAPPROVAL_PREAPPROVAL_TEMPLATE)) {
        process_preapproval_proposal_command(&create->argument.record, tx_info);
    }
}

// Main parsing function
static void parse_node_for_display(const Node *node, transaction_ctx_t *tx_info) {
    if (node->NODE_VERSION_ONEOF_FIELD != NODE_V1_TAG) {
        return;
    }

    const Node_V1 *v = &node->v1;
    if (v->NODE_V1_KIND_ONEOF_FIELD == NODE_V1_EXERCISE_TAG) {
        process_exercise_node(&v->exercise, tx_info);
    } else if (v->NODE_V1_KIND_ONEOF_FIELD == NODE_V1_CREATE_TAG) {
        process_create_node(&v->create, tx_info);
    }
}
