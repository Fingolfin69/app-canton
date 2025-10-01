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
#include "pb_parser.h"
#include "prepared_transaction.h"
#include "validate.h"
#include "canonical_hash.h"
#include "pb_decode.h"

/* -------------------------------------------------------------------------- */
/* Constants                                                                  */
/* -------------------------------------------------------------------------- */

#define TRANSFER_CMD_DISPLAY_FIELDS 4

#define TOKEN_TRANSFER_FIELDS_NB       4
#define NATIVE_TRANSFER_FIELDS_NB      4
#define PREAPPROVAL_PROPOSAL_FIELDS_NB 3

#define MAX_FIELD_PATH_LEN 128

#define MAX_HASH_TABLE_SIZE 53

#define TOKEN_TRANSFER_INSTRUMENT_ID_FIELD_INDEX 3

#define NATIVE_COIN_TICKER        "CC"
#define NATIVE_COIN_INSTRUMENT_ID "Amulet"

/* -------------------------------------------------------------------------- */
/* Function pointers type                                                     */
/* -------------------------------------------------------------------------- */

typedef struct pb_callback_context_t pb_callback_context_t;  // forward declaration
typedef void (*field_format_callback_t)(pb_callback_context_t *ctx, char *value);

/* -------------------------------------------------------------------------- */
/* Structures                                                                 */
/* -------------------------------------------------------------------------- */
typedef struct {
    const char *module_name;
    const char *entity_name;
} identifier_config_t;

typedef struct {
    const char *path;
    const char *item_name;
    size_t field_index;
    field_format_callback_t format_callback;
} field_display_t;

typedef struct {
    const identifier_config_t *identifier;
    const field_display_t *const *fields;
    size_t fields_count;
    const char *review_title;
    const char *review_finish;
} display_config_t;

typedef struct {
    field_display_t **table;
    size_t table_size;
} simple_hash_map_t;

struct pb_callback_context_t {
    char *field_path;
    transaction_ctx_t *tx_info;
    const field_display_t *const *display_config;
    uint8_t nb_fields;
    field_display_t **hash_table;
    simple_hash_map_t field_map;
    uint8_t found_fields_count;
    bool clear_signing_available;
    const char *review_title;
    const char *review_finish;
};

/* -------------------------------------------------------------------------- */
/* Static functions declarations                                              */
/* -------------------------------------------------------------------------- */

static bool decode_value_var(pb_istream_t *stream, const pb_field_t *field, void **arg);
static void format_amount_field(pb_callback_context_t *ctx, char *value);
static void format_token_amount_field(pb_callback_context_t *ctx, char *value);
static void format_native_amount_field(pb_callback_context_t *ctx, char *value);

/* -------------------------------------------------------------------------- */
/* Review titles for different transaction types                              */
/* -------------------------------------------------------------------------- */

#define TOKEN_TRANSFER_REVIEW_TITLE        "Review transaction to send tokens"
#define TOKEN_TRANSFER_REVIEW_FINISH       "Sign transaction to send tokens?"
#define NATIVE_COIN_TRANSFER_REVIEW_TITLE  "Review transaction to send Canton Coin"
#define NATIVE_COIN_TRANSFER_REVIEW_FINISH "Sign transaction to send Canton Coin?"
#define PREAPPROVAL_PROPOSAL_REVIEW_TITLE  "Review transaction to pre-approve incoming transfers"
#define PREAPPROVAL_PROPOSAL_REVIEW_FINISH "Sign transaction to pre-approve incoming transfers?"
#define PREAPPROVAL_ASSET_FIELD_VALUE      "Canton Coin (CC)"

/* -------------------------------------------------------------------------- */
/* Known identifiers for matching transaction types                           */
/* -------------------------------------------------------------------------- */

static const identifier_config_t PREAPPROVAL_PREAPPROVAL_TEMPLATE = {
    .module_name = "Splice.Wallet.TransferPreapproval",
    .entity_name = "TransferPreapprovalProposal"};

static const identifier_config_t TOKEN_TRANSFER_RECORD = {
    .module_name = "Splice.Api.Token.TransferInstructionV1",
    .entity_name = "TransferFactory_Transfer"};

static const identifier_config_t NATIVE_COIN_TRANSFER_RECORD = {
    .module_name = "Splice.ExternalPartyAmuletRules",
    .entity_name = "ExternalPartyAmuletRules_CreateTransferCommand"};

/* -------------------------------------------------------------------------- */
/* Known instrument id to ticker mapping                                      */
/* -------------------------------------------------------------------------- */

static const char *INSTRUMENT_ID_TO_TICKER_MAPPING[] = {
    "Amulet",
    NATIVE_COIN_TICKER,  // Canton Coin
};

/* -------------------------------------------------------------------------- */
/* Field display configuration                                                */
/* -------------------------------------------------------------------------- */

// Transfer commands fields
const field_display_t SENDER_FIELD = {"transfer.sender", "From", 0, NULL};
const field_display_t AMOUNT_FIELD = {"transfer.amount", "Amount", 1, format_token_amount_field};
const field_display_t RECEIVER_FIELD = {"transfer.receiver", "To", 2, NULL};
const field_display_t INSTRUMENT_ID_FIELD = {"transfer.instrumentId.id", "Token", 3, NULL};
// Native coin transfer fields
const field_display_t NATIVE_SENDER_FIELD = {"sender", "From", 0, NULL};
const field_display_t NATIVE_AMOUNT_FIELD = {"amount", "Amount", 1, format_native_amount_field};
const field_display_t NATIVE_RECEIVER_FIELD = {"receiver", "To", 2, NULL};
const field_display_t NATIVE_MEMO_FIELD = {"description", "Memo", 3, NULL};
// Pre-approval proposal fields
const field_display_t PREAPPROVAL_RECEIVER_FIELD = {"receiver", "Pre-approve for account", 0, NULL};
// Static field not parsed from tx but added manually in set_display_config
const field_display_t PREAPPROVAL_ASSET_FIELD = {"asset", "For asset", 1, NULL};
const field_display_t PROVIDER_FIELD = {"provider", "By validator", 2, NULL};

static const field_display_t *const TOKEN_TRANSFER_FIELDS[TOKEN_TRANSFER_FIELDS_NB] = {
    &SENDER_FIELD,
    &AMOUNT_FIELD,
    &RECEIVER_FIELD,
    &INSTRUMENT_ID_FIELD};

static const field_display_t *const NATIVE_COIN_TRANSFER_FIELDS[NATIVE_TRANSFER_FIELDS_NB] = {
    &NATIVE_SENDER_FIELD,
    &NATIVE_AMOUNT_FIELD,
    &NATIVE_RECEIVER_FIELD,
    &NATIVE_MEMO_FIELD};

static const field_display_t *const PREAPPROVAL_PROPOSAL_FIELDS[PREAPPROVAL_PROPOSAL_FIELDS_NB] = {
    &PREAPPROVAL_RECEIVER_FIELD,
    &PREAPPROVAL_ASSET_FIELD,
    &PROVIDER_FIELD};

const display_config_t DISPLAY_CONFIGS[] = {{
                                                .identifier = &TOKEN_TRANSFER_RECORD,
                                                .fields = TOKEN_TRANSFER_FIELDS,
                                                .fields_count = TOKEN_TRANSFER_FIELDS_NB,
                                                .review_title = TOKEN_TRANSFER_REVIEW_TITLE,
                                                .review_finish = TOKEN_TRANSFER_REVIEW_FINISH,
                                            },
                                            {
                                                .identifier = &NATIVE_COIN_TRANSFER_RECORD,
                                                .fields = NATIVE_COIN_TRANSFER_FIELDS,
                                                .fields_count = NATIVE_TRANSFER_FIELDS_NB,
                                                .review_title = NATIVE_COIN_TRANSFER_REVIEW_TITLE,
                                                .review_finish = NATIVE_COIN_TRANSFER_REVIEW_FINISH,
                                            },
                                            {
                                                .identifier = &PREAPPROVAL_PREAPPROVAL_TEMPLATE,
                                                .fields = PREAPPROVAL_PROPOSAL_FIELDS,
                                                .fields_count = PREAPPROVAL_PROPOSAL_FIELDS_NB,
                                                .review_title = PREAPPROVAL_PROPOSAL_REVIEW_TITLE,
                                                .review_finish = PREAPPROVAL_PROPOSAL_REVIEW_FINISH,
                                            }};

const field_display_t *g_hash_table[MAX_HASH_TABLE_SIZE] = {0};

/* -------------------------------------------------------------------------- */
/*  Hashing utilities for field paths lookup during parsing                   */
/* -------------------------------------------------------------------------- */

// FNV-1a hashing function (32-bit version)
static uint32_t fnv1a32(const char *s) {
    uint32_t h = 2166136261u;
    while (*s) {
        h ^= (uint8_t) (*s++);
        h *= 16777619u;
    }
    return h;
}

// Populate hash table for quick lookup
static int populate_hash_map(simple_hash_map_t *map,
                             const field_display_t *const *fields,
                             size_t nfields,
                             field_display_t **hash_table,
                             size_t table_size) {
    if (nfields > table_size / 2) return -1;

    for (size_t i = 0; i < table_size; i++) hash_table[i] = NULL;

    for (size_t i = 0; i < nfields; i++) {
        // Now access the field through the pointer array
        const field_display_t *field = fields[i];
        if (field == NULL) {
            continue;
        }

        const char *field_path = (const char *) PIC(field->path);
        if (field_path == NULL) {
            PRINTF("Warning: field_path for field[%zu] is NULL\n", i);
            continue;
        }

        PRINTF("Building hash for field path: %s\n", field_path);

        // Compute hash
        uint32_t hash = fnv1a32(field_path);
        // Compute index
        uint32_t idx = hash % table_size;
        // Linear probing for collision resolution
        while (hash_table[idx] != NULL) {
            idx = (idx + 1) % table_size;
        }
        // Store the field pointer in the hash table
        hash_table[idx] = (field_display_t *) field;
    }

    map->table = hash_table;
    map->table_size = table_size;
    return 0;
}

// Lookup = no loop except probe (very rare with low load)
static const field_display_t *simple_hash_lookup(const simple_hash_map_t *map, const char *key) {
    uint32_t h = fnv1a32(key);
    uint32_t idx = h % map->table_size;

    // probe until we find match or empty slot
    while (1) {
        const field_display_t *fd = map->table[idx];
        // print the field path being checked
        if (fd == NULL) return NULL;
        if (strcmp((char *) PIC(fd->path), key) == 0) {
            return fd;
        }
        idx = (idx + 1) % map->table_size;
    }
}

/* -------------------------------------------------------------------------- */
/*  Field formatting callbacks                                                */
/* -------------------------------------------------------------------------- */

// Remove all trailing zeros and possible dot if integer for amount fields
static void format_amount_field(pb_callback_context_t *ctx, char *value) {
    UNUSED(ctx);
    PRINTF("Formatting amount field with value: %s\n", value);
    size_t len = strlen(value);

    if (value == NULL || len == 0) {
        PRINTF("Value is NULL or empty, skipping formatting\n");
        return;
    }

    char *dot = strchr(value, '.');
    if (dot != NULL) {
        char *end = value + len - 1;
        while (end > dot && *end == '0') {
            *end-- = '\0';
        }
        if (end == dot) {
            *end = '\0';  // Remove the dot if it's the last character
        }
    }

    PRINTF("Formatted amount: %s\n", value);
}

static void format_token_amount_field(pb_callback_context_t *ctx, char *value) {
    // Call the generic amount formatter first
    format_amount_field(ctx, value);
    // Check if instrument id to ticker mapping is needed
    if (strcmp(value, "0") != 0 && strchr(value, '.') == NULL) {
        // Look for the instrument id in the mapping
        for (size_t i = 0; i < sizeof(INSTRUMENT_ID_TO_TICKER_MAPPING) / (2 * sizeof(char *));
             i++) {
            const char *instrument_id = (const char *) PIC(INSTRUMENT_ID_TO_TICKER_MAPPING[2 * i]);
            const char *ticker = (const char *) PIC(INSTRUMENT_ID_TO_TICKER_MAPPING[2 * i + 1]);
            // Check if stored instrument id in available display items matches
            if (ctx->tx_info->pairs != NULL &&
                ctx->tx_info->pairs[TOKEN_TRANSFER_INSTRUMENT_ID_FIELD_INDEX].value != NULL &&
                strcmp(ctx->tx_info->pairs[TOKEN_TRANSFER_INSTRUMENT_ID_FIELD_INDEX].value,
                       instrument_id) == 0) {
                // Append ticker to value
                size_t new_len = strlen(value) + 1 + strlen(ticker) + 1;
                if (new_len < 64) {  // Assuming value buffer is at least 64 bytes
                    strcat(value, " ");
                    strcat(value, ticker);
                }

                // If matched no need to display the instrument id field, update nb_fields
                ctx->tx_info->pairs_count--;

                // If matched "Amulet" update review title and finish to mention Canton Coin
                if (strcmp(instrument_id, NATIVE_COIN_INSTRUMENT_ID) == 0) {
                    ctx->review_title = NATIVE_COIN_TRANSFER_REVIEW_TITLE;
                    ctx->review_finish = NATIVE_COIN_TRANSFER_REVIEW_FINISH;
                }

                break;
            }
        }
    }
}

static void format_native_amount_field(pb_callback_context_t *ctx, char *value) {
    // Call the generic amount formatter first
    format_amount_field(ctx, value);
    // Append "CC" ticker for Canton Coin
    size_t new_len = strlen(value) + 1 + strlen(NATIVE_COIN_TICKER) + 1;
    if (new_len < 64) {  // Assuming value buffer is at least 64 bytes
        strcat(value, " ");
        strcat(value, NATIVE_COIN_TICKER);
    }
}

/* -------------------------------------------------------------------------- */
/*  Helper functions for transaction display management                       */
/* -------------------------------------------------------------------------- */

// Helper function to initialize transaction pairs used for display
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
static void set_field_value(pb_callback_context_t *ctx,
                            const field_display_t *field_config,
                            const char *value) {
    if (field_config->field_index < ctx->nb_fields && value != NULL) {
        size_t value_len = strlen(value) + 1;
        // Allocate memory in the context's display_items_strings array
        ctx->tx_info->display_items_strings[field_config->field_index] =
            (char *) app_mem_alloc(value_len);
        if (ctx->tx_info->display_items_strings[field_config->field_index] != NULL) {
            memcpy(ctx->tx_info->display_items_strings[field_config->field_index],
                   value,
                   value_len);

            ctx->tx_info->pairs[field_config->field_index].item =
                (char *) PIC(field_config->item_name);
            ctx->tx_info->pairs[field_config->field_index].value =
                ctx->tx_info->display_items_strings[field_config->field_index];
        }
    }
}

// Helper function to set display configuration from a const array.
static void set_display_config(pb_callback_context_t *ctx, const display_config_t *config_source) {
    if (ctx->display_config != NULL) {
        app_mem_free((void *) ctx->display_config);
    }

    uint8_t count = config_source->fields_count;

    PRINTF("Setting display config with %d fields\n", count);

    // Allocate display_config array
    ctx->display_config =
        (const field_display_t **) app_mem_alloc(count * sizeof(field_display_t *));

    LEDGER_ASSERT(ctx->display_config != NULL, "Memory full");

    const field_display_t *const *source =
        (const field_display_t *const *) PIC(config_source->fields);

    // Populate from the const array
    for (size_t i = 0; i < count; i++) {
        ((field_display_t **) ctx->display_config)[i] = (field_display_t *) PIC(source[i]);
    }

    ctx->nb_fields = config_source->fields_count;
    ctx->review_title = config_source->review_title;
    ctx->review_finish = config_source->review_finish;

    if (populate_hash_map(&ctx->field_map,
                          ctx->display_config,
                          ctx->nb_fields,
                          ctx->hash_table,
                          MAX_HASH_TABLE_SIZE) != 0) {
        PRINTF("Error populating hash map\n");
    }
    init_transaction_pairs(ctx->tx_info, ctx->nb_fields);

    // If pre-approval proposal, add static field for asset
    if (strcmp((const char *) PIC(ctx->review_title), PREAPPROVAL_PROPOSAL_REVIEW_TITLE) == 0) {
        set_field_value(ctx, &PREAPPROVAL_ASSET_FIELD, PREAPPROVAL_ASSET_FIELD_VALUE);
        ctx->found_fields_count++;  // Increment pairs count for static field
    }

    return;
}

// Helper function to match identifiers
static bool match_identifier(const Identifier *id, const identifier_config_t *config) {
    return strcmp(id->module_name, (char *) PIC(config->module_name)) == 0 &&
           strcmp(id->entity_name, (char *) PIC(config->entity_name)) == 0;
}

// Identify transaction type and set display configuration accordingly
static void find_tx_type_and_config(pb_callback_context_t *ctx, const Identifier *id) {
    if (ctx->display_config != NULL) {
        // Already set, no need to find again
        return;
    }

    for (size_t i = 0; i < sizeof(DISPLAY_CONFIGS) / sizeof(DISPLAY_CONFIGS[0]); i++) {
        const display_config_t *config = (const display_config_t *) PIC(&DISPLAY_CONFIGS[i]);
        if (match_identifier(id, (const identifier_config_t *) PIC(config->identifier))) {
            set_display_config(ctx, config);
            return;
        }
    }
    return;
}

// Lookup field in hash map and set value for display if found. Discriminate field types if needed.
static void find_tx_field(pb_callback_context_t *ctx, cbValue *value) {
    if (ctx->display_config != NULL && !ctx->clear_signing_available) {
        PRINTF("Looking up field path: %s\n", ctx->field_path);
        const field_display_t *fd = simple_hash_lookup(&ctx->field_map, ctx->field_path);
        if (fd != NULL && value != NULL) {
            PRINTF("Found matching field for path: %s\n", ctx->field_path);
            // Set the field value
            switch (value->which_sum) {
                case com_daml_ledger_api_v2_Value_party_tag:
                    PRINTF("Setting party value: %s\n", value->party);
                    set_field_value(ctx, fd, value->party);
                    ctx->found_fields_count++;
                    break;
                case com_daml_ledger_api_v2_Value_numeric_tag:
                    PRINTF("Setting numeric value: %s\n", value->numeric);
                    set_field_value(ctx, fd, value->numeric);
                    ctx->found_fields_count++;
                    break;
                case com_daml_ledger_api_v2_Value_text_tag:
                    PRINTF("Setting text value: %s\n", value->text);
                    set_field_value(ctx, fd, value->text);
                    ctx->found_fields_count++;
                    break;
                default:
                    PRINTF("Field type not handled for display: %d\n", value->which_sum);
                    break;
            }

            // If all fields found, mark clear signing available to skip further processing
            if (ctx->found_fields_count == ctx->nb_fields) {
                PRINTF("All fields found, skipping further processing\n");
                ctx->clear_signing_available = true;  // To skip further processing

                // Apply formatting callbacks if any
                for (size_t i = 0; i < ctx->nb_fields; i++) {
                    field_format_callback_t callback =
                        (field_format_callback_t) PIC(ctx->display_config[i]->format_callback);
                    if (callback != NULL) {
                        callback(ctx, ctx->tx_info->display_items_strings[i]);
                    }
                }
            }
        }
    }
}

/* -------------------------------------------------------------------------- */
/*  Field path management                                                     */
/* -------------------------------------------------------------------------- */

// Function to allocate memory for the display field path
static void init_field_path(pb_callback_context_t *ctx) {
    if (ctx->field_path != NULL) {
        app_mem_free(ctx->field_path);
    }
    ctx->field_path = (char *) app_mem_alloc(MAX_FIELD_PATH_LEN);
    memset(ctx->field_path, 0, MAX_FIELD_PATH_LEN);
}

// Function to free memory for the display field path
static void free_field_path(pb_callback_context_t *ctx) {
    if (ctx->field_path != NULL) {
        app_mem_free(ctx->field_path);
        ctx->field_path = NULL;
    }
}

// Push a new segment onto the field path
static void push_path(pb_callback_context_t *ctx, const char *new_segment) {
    if (ctx->display_config != NULL && ctx->field_path != NULL && new_segment != NULL) {
        size_t current_len = strlen(ctx->field_path);
        size_t new_segment_len = strlen(new_segment);
        if (current_len + 1 + new_segment_len < MAX_FIELD_PATH_LEN) {
            if (current_len > 0) {
                ctx->field_path[current_len] = '.';  // Add dot separator
                current_len++;
            }
            memcpy(ctx->field_path + current_len,
                   new_segment,
                   new_segment_len + 1);  // +1 to include null terminator
        }
    }
}

// Pop the last segment from the field path
static void pop_path(pb_callback_context_t *ctx) {
    if (ctx->display_config != NULL && ctx->field_path != NULL) {
        char *last_dot = strrchr(ctx->field_path, '.');
        if (last_dot != NULL) {
            *last_dot = '\0';  // truncate at last dot
        } else {
            ctx->field_path[0] = '\0';  // reset to empty
        }
    }
}

/* -------------------------------------------------------------------------- */
/*  Protobuf decoding callbacks                                               */
/* -------------------------------------------------------------------------- */

// Decode the label field of a record field
static bool decode_record_field_label(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    UNUSED(field);
    pb_callback_context_t *ctx = (pb_callback_context_t *) (*arg);
    // Read string from stream
    char label_buffer[64] = {0};
    if (!pb_read(stream, (pb_byte_t *) label_buffer, stream->bytes_left)) {
        PRINTF("Failed to read string from stream\n");
        return false;
    }
    // Push the label onto the field path
    push_path(ctx, label_buffer);
    return true;
}

// Decode the record ID field to identify the record type
static bool decode_record_id_field(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    UNUSED(field);
    pb_callback_context_t *ctx = (pb_callback_context_t *) (*arg);
    com_daml_ledger_api_v2_Identifier id = {};

    if (!pb_decode(stream, com_daml_ledger_api_v2_Identifier_fields, &id)) {
        PRINTF("Failed to decode Record ID field\n");
        return false;
    }

    // Identify the kind of record with match_identifier to know if we should
    // process fields for display.
    find_tx_type_and_config(ctx, &id);

    pb_release(com_daml_ledger_api_v2_Identifier_fields, &id);

    return true;
}

// Decode a record field, pushing and popping the field path as needed
static bool decode_record_field(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    UNUSED(field);
    PRINTF("Decoding Record field\n");
    cbRecordField rf = {};
    pb_callback_context_t *ctx = (pb_callback_context_t *) (*arg);

    rf.label.funcs.decode = &decode_record_field_label;
    rf.label.arg = ctx;

    rf.value.cb_sum.funcs.decode = &decode_value_var;
    rf.value.cb_sum.arg = ctx;

    if (!pb_decode(stream, com_daml_ledger_api_v2_cb_RecordField_fields, &rf)) {
        PRINTF("Failed to decode Record field: %s\n", PB_GET_ERROR(stream));
        return false;
    }

    // Check if current field path matches any display configured field
    find_tx_field(ctx, &rf.value);

    pop_path(ctx);

    pb_release(com_daml_ledger_api_v2_cb_RecordField_fields, &rf);

    return true;
}

// Decode an Optional value, which may contain another Value
static bool decode_value_opt(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    UNUSED(field);
    pb_callback_context_t *ctx = (pb_callback_context_t *) (*arg);

    PRINTF("Decoding Optional value\n");

    cbValue v = com_daml_ledger_api_v2_cb_Value_init_zero;
    v.cb_sum.funcs.decode = &decode_value_var;

    if (!pb_decode(stream, com_daml_ledger_api_v2_cb_Value_fields, &v)) {
        PRINTF("Failed to decode Optional value: %s\n", PB_GET_ERROR(stream));
        return false;
    }

    find_tx_field(ctx, &v);

    pb_release(com_daml_ledger_api_v2_cb_Value_fields, &v);

    return true;
}

// Decode a value variant, handling Record types specifically
static bool decode_value_var(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    cbValue *topmsg = field->message;
    (void) topmsg;

    switch (field->tag) {
        case com_daml_ledger_api_v2_cb_Value_record_tag: {
            pb_callback_context_t *ctx = (pb_callback_context_t *) (*arg);
            cbRecord *msg = field->pData;
            msg->fields.funcs.decode = &decode_record_field;
            msg->fields.arg = ctx;
            msg->record_id.funcs.decode = &decode_record_id_field;
            msg->record_id.arg = ctx;
            // decode the record, which will invoke the callbacks
            if (!pb_decode(stream, com_daml_ledger_api_v2_cb_Record_fields, msg)) {
                PRINTF("Failed to decode Record in Value: %s\n", PB_GET_ERROR(stream));
                return false;
            }
            pop_path(ctx);
            ctx->display_config = NULL;
            pb_release(com_daml_ledger_api_v2_cb_Record_fields, msg);
            break;
        }
        case com_daml_ledger_api_v2_cb_Value_optional_tag: {
            pb_callback_context_t *ctx = (pb_callback_context_t *) (*arg);
            cbOptional *msg = field->pData;
            msg->value.funcs.decode = &decode_value_opt;
            msg->value.arg = ctx;
            break;
        }
        default: {
            // For other types, no special handling needed here
            break;
        }
    }
    return true;
}

// Callback to decode Node messages, focusing on Exercise and Create nodes
static bool node_decode_callback(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    UNUSED(stream);
    pb_callback_context_t *ctx = (pb_callback_context_t *) (*arg);
    com_daml_ledger_api_v2_interactive_transaction_v1_cb_NodeDisplay *node = field->message;

    // Only handle Exercise nodes
    if (field->tag == NODE_V1_EXERCISE_TAG) {
        PRINTF("Decoding Exercise node\n");
        node->exercise.chosen_value.cb_sum.funcs.decode = &decode_value_var;
        node->exercise.chosen_value.cb_sum.arg = ctx;
    } else if (field->tag == NODE_V1_CREATE_TAG) {
        PRINTF("Decoding Create node\n");
        node->create.argument.cb_sum.funcs.decode = &decode_value_var;
        node->create.argument.cb_sum.arg = ctx;
    }

    return true;
}

// Callback to decode versioned node messages, attaching Node-level callback
static bool versioned_node_decode_callback(pb_istream_t *stream,
                                           const pb_field_t *field,
                                           void **arg) {
    UNUSED(stream);
    pb_callback_context_t *ctx = (pb_callback_context_t *) (*arg);
    com_daml_ledger_api_v2_interactive_DeviceDamlTransactionDisplay_Node *node = field->message;

    // Attach Node-level callback dynamically
    if (node->NODE_VERSION_ONEOF_FIELD == NODE_V1_TAG) {
        node->v1.cb_node_type.funcs.decode = &node_decode_callback;
        node->v1.cb_node_type.arg = ctx;
    }

    return true;
}

/* -------------------------------------------------------------------------- */
/*  Entry point for parsing transaction display information                   */
/* -------------------------------------------------------------------------- */

void parse_node_for_display(buffer_t *buf) {
    // Only parse if we haven't already found all fields
    if (G_context.tx_info.clear_signing_available) {
        return;
    }

    pb_callback_context_t ctx = {0};
    init_field_path(&ctx);
    ctx.tx_info = &G_context.tx_info;
    ctx.hash_table = (field_display_t **) g_hash_table;
    ctx.display_config = NULL;

    G_context.tx_info.tx_parts_ctx.node.cb_versioned_node.funcs.decode =
        &versioned_node_decode_callback;
    G_context.tx_info.tx_parts_ctx.node.cb_versioned_node.arg = &ctx;

    pb_istream_t stream = pb_istream_from_buffer(buf->ptr, buf->size);

    if (!pb_decode(&stream,
                   com_daml_ledger_api_v2_interactive_DeviceDamlTransactionDisplay_Node_fields,
                   &G_context.tx_info.tx_parts_ctx.node)) {
        PRINTF("Decode failed: %s\n", PB_GET_ERROR(&stream));
    }

    pb_release(com_daml_ledger_api_v2_interactive_DeviceDamlTransactionDisplay_Node_fields,
               &G_context.tx_info.tx_parts_ctx.node);

    free_field_path(&ctx);

    G_context.tx_info.tx_parts_ctx.node.cb_versioned_node.funcs.decode = NULL;
    G_context.tx_info.tx_parts_ctx.node.cb_versioned_node.arg = NULL;

    if (!G_context.tx_info.clear_signing_available && ctx.clear_signing_available) {
        G_context.tx_info.clear_signing_available = ctx.clear_signing_available;
        G_context.tx_info.review_title = ctx.review_title;
        G_context.tx_info.review_finish = ctx.review_finish;
    }
}
