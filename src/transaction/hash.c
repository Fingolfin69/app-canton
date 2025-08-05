#include "hash.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include "ledger_assert.h"
#include "lcx_sha256.h"

#include "pb.h"         // Contains PB_LTYPE macros and basic pb_field_t definitions
#include "pb_common.h"  // Contains pb_field_

// Protocol codegen headers
#include "com/daml/ledger/api/v2/interactive/interactive_submission_service.pb.h"
#include "com/daml/ledger/api/v2/value.pb.h"

#include "constants.h"
#include "mem.h"

/* -------------------------------------------------------------------------- */
/*  Spec constants                                                             */
/* -------------------------------------------------------------------------- */

static const uint8_t PREPARED_TRANSACTION_HASH_PURPOSE[4] = {0x00, 0x00, 0x00, 0x30};
#define HASHING_SCHEME_VERSION ((uint8_t) 2) /* 0x02 */
#define NODE_ENCODING_VERSION  ((uint8_t) 1) /* 0x01 */

/* -------------------------------------------------------------------------- */
/*  Adaption layer for nanopb oneof names                                      */
/* -------------------------------------------------------------------------- */
#define VALUE_ONEOF_FIELD     which_sum
#define VALUE_UNIT_TAG        com_daml_ledger_api_v2_Value_unit_tag
#define VALUE_BOOL_TAG        com_daml_ledger_api_v2_Value_bool__tag
#define VALUE_INT64_TAG       com_daml_ledger_api_v2_Value_int64_tag
#define VALUE_NUMERIC_TAG     com_daml_ledger_api_v2_Value_numeric_tag
#define VALUE_TIMESTAMP_TAG   com_daml_ledger_api_v2_Value_timestamp_tag
#define VALUE_DATE_TAG        com_daml_ledger_api_v2_Value_date_tag
#define VALUE_PARTY_TAG       com_daml_ledger_api_v2_Value_party_tag
#define VALUE_TEXT_TAG        com_daml_ledger_api_v2_Value_text_tag
#define VALUE_CONTRACT_ID_TAG com_daml_ledger_api_v2_Value_contract_id_tag
#define VALUE_OPTIONAL_TAG    com_daml_ledger_api_v2_Value_optional_tag
#define VALUE_LIST_TAG        com_daml_ledger_api_v2_Value_list_tag
#define VALUE_TEXT_MAP_TAG    com_daml_ledger_api_v2_Value_text_map_tag
#define VALUE_RECORD_TAG      com_daml_ledger_api_v2_Value_record_tag
#define VALUE_VARIANT_TAG     com_daml_ledger_api_v2_Value_variant_tag
#define VALUE_ENUM_TAG        com_daml_ledger_api_v2_Value_enum__tag
#define VALUE_GEN_MAP_TAG     com_daml_ledger_api_v2_Value_gen_map_tag

// Node version and kind oneofs
#define NODE_VERSION_ONEOF_FIELD which_versioned_node
#define NODE_V1_TAG              com_daml_ledger_api_v2_interactive_DamlTransaction_Node_v1_tag
#define NODE_V1_KIND_ONEOF_FIELD which_node_type
#define NODE_V1_CREATE_TAG       com_daml_ledger_api_v2_interactive_transaction_v1_Node_create_tag
#define NODE_V1_EXERCISE_TAG     com_daml_ledger_api_v2_interactive_transaction_v1_Node_exercise_tag
#define NODE_V1_FETCH_TAG        com_daml_ledger_api_v2_interactive_transaction_v1_Node_fetch_tag
#define NODE_V1_ROLLBACK_TAG     com_daml_ledger_api_v2_interactive_transaction_v1_Node_rollback_tag

typedef com_daml_ledger_api_v2_interactive_DamlTransaction DamlTransaction;
typedef com_daml_ledger_api_v2_interactive_DamlTransaction_Node Node;
typedef com_daml_ledger_api_v2_interactive_transaction_v1_Node Node_V1;
typedef com_daml_ledger_api_v2_interactive_DamlTransaction_NodeSeed NodeSeed;
typedef com_daml_ledger_api_v2_interactive_Metadata Metadata;

typedef com_daml_ledger_api_v2_interactive_transaction_v1_Create Node_Create;
typedef com_daml_ledger_api_v2_interactive_transaction_v1_Exercise Node_Exercise;
typedef com_daml_ledger_api_v2_interactive_transaction_v1_Fetch Node_Fetch;
typedef com_daml_ledger_api_v2_interactive_transaction_v1_Rollback Node_Rollback;

typedef com_daml_ledger_api_v2_interactive_Metadata_InputContract InputContract;

typedef com_daml_ledger_api_v2_Value Value;
typedef com_daml_ledger_api_v2_RecordField RecordField;
typedef com_daml_ledger_api_v2_GenMap_Entry GenMapEntry;
typedef com_daml_ledger_api_v2_TextMap_Entry TextMapEntry;
typedef com_daml_ledger_api_v2_Identifier Identifier;

/* -------------------------------------------------------------------------- */
/*  Error handling                                                            */
/* -------------------------------------------------------------------------- */

typedef enum {
    HASH_OK = 0,
    HASH_ERROR_BUFFER_OVERFLOW = 1,
    HASH_ERROR_INVALID_HASH_STRING = 2,
    HASH_ERROR_UNSUPPORTED_VALUE = 3,
    HASH_ERROR_NODE_ID_NOT_FOUND = 4,
    HASH_ERROR_UNKNOWN_NODE_VERSION = 5,
    HASH_ERROR_UNKNOWN_NODE_TYPE = 6,
} HashError;

typedef struct {
    uint8_t err_msg[32];
    int err_code;
} HashErrorInfo;

static HashErrorInfo HASH_ERR_INFO = {
    .err_msg = {0},
    .err_code = HASH_OK,
};

static void clear_hash_error() {
    HASH_ERR_INFO.err_code = HASH_OK;
    memset(HASH_ERR_INFO.err_msg, 0, sizeof(HASH_ERR_INFO.err_msg));
}

static bool is_hash_error() {
    return HASH_ERR_INFO.err_code != HASH_OK;
}

static void set_hash_error(HashError err, const char *msg) {
    // Don't overwrite existing error
    if (HASH_ERR_INFO.err_code != HASH_OK) {
        return;
    }

    HASH_ERR_INFO.err_code = err;
    if (msg) {
        strncpy((char *) HASH_ERR_INFO.err_msg, msg, sizeof(HASH_ERR_INFO.err_msg) - 1);
        HASH_ERR_INFO.err_msg[sizeof(HASH_ERR_INFO.err_msg) - 1] = '\0';  // Ensure null termination
    } else {
        HASH_ERR_INFO.err_msg[0] = '\0';  // Clear message if none provided
    }
}

/* -------------------------------------------------------------------------- */
/*  BuffWriter helper                                                         */
/* -------------------------------------------------------------------------- */

typedef struct {
    uint8_t *base, *ptr, *end;
    bool overflow;
} ByteWriter;

static inline void bw_init(ByteWriter *bw, void *buf, size_t cap) {
    bw->base = bw->ptr = (uint8_t *) buf;
    bw->end = bw->base + cap;
    bw->overflow = false;
}
static inline size_t bw_size(const ByteWriter *bw) {
    return (size_t) (bw->ptr - bw->base);
}
static inline void bw_put(ByteWriter *bw, const void *p, size_t n) {
    if (bw->overflow || bw->ptr + n > bw->end) {
        bw->overflow = true;
        set_hash_error(HASH_ERROR_BUFFER_OVERFLOW, "Buffer overflow in ByteWriter");
        return;
    }
    memcpy(bw->ptr, p, n);
    bw->ptr += n;
}
static inline void bw_put_byte(ByteWriter *bw, uint8_t b) {
    bw_put(bw, &b, 1);
}

// Big‑endian helpers
static inline void bw_put_u32_be(ByteWriter *bw, uint32_t v) {
    uint8_t t[4] = {(uint8_t) (v >> 24), (uint8_t) (v >> 16), (uint8_t) (v >> 8), (uint8_t) v};
    bw_put(bw, t, 4);
}
static inline void bw_put_u64_be(ByteWriter *bw, uint64_t v) {
    uint8_t t[8] = {(uint8_t) (v >> 56),
                    (uint8_t) (v >> 48),
                    (uint8_t) (v >> 40),
                    (uint8_t) (v >> 32),
                    (uint8_t) (v >> 24),
                    (uint8_t) (v >> 16),
                    (uint8_t) (v >> 8),
                    (uint8_t) v};
    bw_put(bw, t, 8);
}

/* -------------------------------------------------------------------------- */
/*  Encoders                                                                  */
/* -------------------------------------------------------------------------- */

static inline void encode_bool(ByteWriter *bw, bool v) {
    bw_put_byte(bw, v ? 1 : 0);
}
static inline void encode_int32(ByteWriter *bw, int32_t v) {
    bw_put_u32_be(bw, (uint32_t) v);
}
static inline void encode_int64(ByteWriter *bw, int64_t v) {
    bw_put_u64_be(bw, (uint64_t) v);
}

static void encode_bytes(ByteWriter *bw, const uint8_t *data, int32_t len) {
    encode_int32(bw, len);
    bw_put(bw, data, (size_t) len);
}
static void encode_string(ByteWriter *bw, const char *s) {
    encode_bytes(bw, (const uint8_t *) s, (int32_t) strlen(s));
}
static void encode_hash(ByteWriter *bw, const uint8_t h[32]) {
    bw_put(bw, h, 32);
}

// hex‑decode helper
static uint8_t hex_val(char c) {
    return (uint8_t) ((c >= '0' && c <= '9')   ? c - '0'
                      : (c >= 'a' && c <= 'f') ? 10 + c - 'a'
                                               : 10 + c - 'A');
}
static void encode_hex_string(ByteWriter *bw, const char *hex) {
    size_t len = strlen(hex);
    if (len % 2 != 0) {
        set_hash_error(HASH_ERROR_INVALID_HASH_STRING, "Hex string must have even length");
        return;
    }

    encode_int32(bw, (int32_t) (len / 2));
    for (size_t i = 0; i < len; i += 2) {
        uint8_t b = (hex_val(hex[i]) << 4) | hex_val(hex[i + 1]);
        bw_put_byte(bw, b);
    }
}

// Generic optional encoder
typedef void (*EncodeFn)(ByteWriter *, const void *ctx);
static void encode_optional(ByteWriter *bw, bool present, EncodeFn fn, const void *ctx) {
    bw_put_byte(bw, present ? 1 : 0);
    if (present) fn(bw, ctx);
}

// Generic repeated encoder (contiguous array)
static void encode_repeated(ByteWriter *bw,
                            size_t count,
                            const void *array,
                            size_t elem_sz,
                            EncodeFn fn) {
    encode_int32(bw, (int32_t) count);
    const uint8_t *p = (const uint8_t *) array;
    for (size_t i = 0; i < count; ++i) fn(bw, p + i * elem_sz);
}

static void encode_identifier(ByteWriter *, const Identifier *);
static void encode_value(ByteWriter *, const Value *);

// Helper wrappers invoked by encode_repeated
static void wrap_encode_string(ByteWriter *bw, const void *ctx) {
    encode_string(bw, *(char *const *) ctx);
}
static void wrap_encode_value(ByteWriter *bw, const void *ctx) {
    encode_value(bw, (const Value *) ctx);
}
static void wrap_encode_identifier(ByteWriter *bw, const void *ctx) {
    encode_identifier(bw, (const Identifier *) ctx);
}

static void encode_text_map_entry(ByteWriter *bw, const TextMapEntry *e) {
    encode_string(bw, e->key);
    encode_value(bw, e->value);
}
static void wrap_encode_text_map_entry(ByteWriter *bw, const void *ctx) {
    encode_text_map_entry(bw, (const TextMapEntry *) ctx);
}

static void encode_record_field(ByteWriter *bw, const RecordField *f) {
    encode_optional(bw, f->label != NULL, (EncodeFn) wrap_encode_string, &f->label);
    encode_value(bw, f->value);
}
static void wrap_encode_record_field(ByteWriter *bw, const void *ctx) {
    encode_record_field(bw, (const RecordField *) ctx);
}

static void encode_gen_map_entry(ByteWriter *bw, const GenMapEntry *e) {
    encode_value(bw, e->key);
    encode_value(bw, e->value);
}
static void wrap_encode_gen_map_entry(ByteWriter *bw, const void *ctx) {
    encode_gen_map_entry(bw, (const GenMapEntry *) ctx);
}

static void encode_value(ByteWriter *bw, const Value *v) {
    switch (v->VALUE_ONEOF_FIELD) {
        case VALUE_UNIT_TAG:
            bw_put_byte(bw, 0x00);
            return;
        case VALUE_BOOL_TAG:
            bw_put_byte(bw, 0x01);
            encode_bool(bw, v->bool_);
            return;
        case VALUE_INT64_TAG:
            bw_put_byte(bw, 0x02);
            encode_int64(bw, v->int64);
            return;
        case VALUE_NUMERIC_TAG:
            bw_put_byte(bw, 0x03);
            encode_string(bw, v->numeric);
            return;
        case VALUE_TIMESTAMP_TAG:
            bw_put_byte(bw, 0x04);
            encode_int64(bw, v->timestamp);
            return;
        case VALUE_DATE_TAG:
            bw_put_byte(bw, 0x05);
            encode_int32(bw, v->date);
            return;
        case VALUE_PARTY_TAG:
            bw_put_byte(bw, 0x06);
            encode_string(bw, v->party);
            return;
        case VALUE_TEXT_TAG:
            bw_put_byte(bw, 0x07);
            encode_string(bw, v->text);
            return;
        case VALUE_CONTRACT_ID_TAG:
            bw_put_byte(bw, 0x08);
            encode_hex_string(bw, v->contract_id);
            return;
        case VALUE_OPTIONAL_TAG:
            bw_put_byte(bw, 0x09);
            encode_optional(bw,
                            v->optional.value != NULL,
                            (EncodeFn) encode_value,
                            &v->optional.value);
            return;
        case VALUE_LIST_TAG:
            bw_put_byte(bw, 0x0A);
            encode_repeated(bw,
                            v->list.elements_count,
                            v->list.elements,
                            sizeof(Value),
                            wrap_encode_value);
            return;
        case VALUE_TEXT_MAP_TAG:
            bw_put_byte(bw, 0x0B);
            encode_repeated(bw,
                            v->text_map.entries_count,
                            v->text_map.entries,
                            sizeof(TextMapEntry),
                            wrap_encode_text_map_entry);
            return;
        case VALUE_RECORD_TAG:
            bw_put_byte(bw, 0x0C);
            encode_optional(bw,
                            v->record.has_record_id,
                            wrap_encode_identifier,
                            &v->record.record_id);
            encode_repeated(bw,
                            v->record.fields_count,
                            v->record.fields,
                            sizeof(RecordField),
                            wrap_encode_record_field);
            return;
        case VALUE_VARIANT_TAG:
            bw_put_byte(bw, 0x0D);
            encode_optional(bw,
                            v->variant.has_variant_id,
                            wrap_encode_identifier,
                            &v->variant.variant_id);
            encode_string(bw, v->variant.constructor);
            encode_value(bw, v->variant.value);
            return;
        case VALUE_ENUM_TAG:
            bw_put_byte(bw, 0x0E);
            encode_optional(bw, v->enum_.has_enum_id, wrap_encode_identifier, &v->enum_.enum_id);
            encode_string(bw, v->enum_.constructor);
            return;
        case VALUE_GEN_MAP_TAG:
            bw_put_byte(bw, 0x0F);
            encode_repeated(bw,
                            v->gen_map.entries_count,
                            v->gen_map.entries,
                            sizeof(GenMapEntry),
                            wrap_encode_gen_map_entry);
            return;
        default:
            set_hash_error(HASH_ERROR_UNSUPPORTED_VALUE, "Unsupported Value which_* tag");
    }
}

static void split_dot_and_encode(ByteWriter *bw, const char *dotstr) {
    size_t parts = 1;
    for (const char *p = dotstr; *p; ++p)
        if (*p == '.') ++parts;
    encode_int32(bw, (int32_t) parts);
    const char *start = dotstr;
    while (true) {
        const char *dot = strchr(start, '.');
        size_t len = dot ? (size_t) (dot - start) : strlen(start);
        encode_int32(bw, (int32_t) len);
        bw_put(bw, start, len);
        if (!dot) break;
        start = dot + 1;
    }
}
static void encode_identifier(ByteWriter *bw, const Identifier *id) {
    encode_string(bw, id->package_id);
    split_dot_and_encode(bw, id->module_name);
    split_dot_and_encode(bw, id->entity_name);
}

static const uint8_t *find_seed(const char *node_id, const NodeSeed *seeds, size_t n) {
    for (size_t i = 0; i < n; ++i)
        // ATTENTION, FIXME
        // if (strcmp(seeds[i].node_id, node_id) == 0) return seeds[i].seed->bytes;
        if (true) return seeds[i].seed->bytes;
    return NULL;
}

static void encode_node(ByteWriter *,
                        const Node *,
                        const DamlTransaction *,
                        const NodeSeed *,
                        size_t);

// Hash a referenced node‑id and write the 32‑byte digest
static void encode_node_id_hashed(ByteWriter *bw,
                                  const char *node_id,
                                  const DamlTransaction *tx,
                                  const NodeSeed *seeds,
                                  size_t n_seeds) {
    const Node *node = NULL;
    for (size_t i = 0; i < tx->nodes_count; ++i)
        if (strcmp(tx->nodes[i].node_id, node_id) == 0) {
            node = &tx->nodes[i];
            break;
        }

    if (node == NULL) {
        set_hash_error(HASH_ERROR_NODE_ID_NOT_FOUND, "Node id not found in transaction nodes");
        return;
    }

    uint8_t *scratch = app_mem_alloc(MAX_ENCODED_NODE_LEN);
    LEDGER_ASSERT(scratch != NULL, "Failed to allocate scratch buf for node id");

    ByteWriter n_bw;
    bw_init(&n_bw, scratch, MAX_ENCODED_NODE_LEN);
    encode_node(&n_bw, node, tx, seeds, n_seeds);

    uint8_t h[32];
    cx_sha256_hash(scratch, bw_size(&n_bw), h);
    bw_put(bw, h, 32);
    PRINTF("Node id hash: %.*H\n", 32, h);
}

static void encode_repeated_node_ids(ByteWriter *bw,
                                     size_t count,
                                     char *const *ids,
                                     const DamlTransaction *tx,
                                     const NodeSeed *seeds,
                                     size_t n_seeds) {
    encode_int32(bw, (int32_t) count);

    for (size_t i = 0; i < count; ++i) {
        encode_node_id_hashed(bw, ids[i], tx, seeds, n_seeds);
    }
}

static void encode_create(ByteWriter *bw,
                          const Node_Create *c,
                          const char *node_id,
                          const NodeSeed *seeds,
                          size_t n_seeds) {
    bw_put_byte(bw, NODE_ENCODING_VERSION);
    encode_string(bw, c->lf_version);
    bw_put_byte(bw, 0x00);
    const uint8_t *seed = find_seed(node_id, seeds, n_seeds);
    encode_optional(bw, seed != NULL, (EncodeFn) encode_hash, seed);
    encode_hex_string(bw, c->contract_id);
    encode_string(bw, c->package_name);
    encode_identifier(bw, &c->template_id);
    encode_value(bw, &c->argument);
    encode_repeated(bw, c->signatories_count, c->signatories, sizeof(char *), wrap_encode_string);
    encode_repeated(bw, c->stakeholders_count, c->stakeholders, sizeof(char *), wrap_encode_string);
}

static void encode_exercise(ByteWriter *bw,
                            const Node_Exercise *e,
                            const char *node_id,
                            const DamlTransaction *tx,
                            const NodeSeed *seeds,
                            size_t n_seeds) {
    (void) bw;
    (void) e;
    (void) node_id;
    (void) tx;
    (void) seeds;
    (void) n_seeds;

    LEDGER_ASSERT(false, "TODO");
}

static void encode_fetch(ByteWriter *bw, const Node_Fetch *f) {
    (void) bw;
    (void) f;

    LEDGER_ASSERT(false, "TODO");
}

static void encode_rollback(ByteWriter *bw,
                            const Node_Rollback *r,
                            const DamlTransaction *tx,
                            const NodeSeed *seeds,
                            size_t n_seeds) {
    (void) bw;
    (void) r;
    (void) tx;
    (void) seeds;
    (void) n_seeds;

    LEDGER_ASSERT(false, "TODO");
}

static void encode_node(ByteWriter *bw,
                        const Node *node,
                        const DamlTransaction *tx,
                        const NodeSeed *seeds,
                        size_t n_seeds) {
    if (node->NODE_VERSION_ONEOF_FIELD != NODE_V1_TAG) {
        set_hash_error(HASH_ERROR_UNKNOWN_NODE_VERSION, "Unsupported Node version");
        return;
    }

    const Node_V1 *v = &node->v1;
    switch (v->NODE_V1_KIND_ONEOF_FIELD) {
        case NODE_V1_CREATE_TAG:
            encode_create(bw, &v->create, node->node_id, seeds, n_seeds);
            break;
        case NODE_V1_EXERCISE_TAG:
            encode_exercise(bw, &v->exercise, node->node_id, tx, seeds, n_seeds);
            break;
        case NODE_V1_FETCH_TAG:
            encode_fetch(bw, &v->fetch);
            break;
        case NODE_V1_ROLLBACK_TAG:
            encode_rollback(bw, &v->rollback, tx, seeds, n_seeds);
            break;
        default:
            set_hash_error(HASH_ERROR_UNKNOWN_NODE_TYPE, "Unknown Node.V1 which_* tag");
    }
}

static void encode_transaction(ByteWriter *bw, const DamlTransaction *tx) {
    encode_string(bw, tx->version);
    encode_repeated_node_ids(bw,
                             tx->roots_count,
                             tx->roots,
                             tx,
                             tx->node_seeds,
                             tx->node_seeds_count);
}
static void hash_transaction(const DamlTransaction *tx, uint8_t out[32]) {
    uint8_t *scratch = app_mem_alloc(MAX_ENCODED_TX_LEN);
    LEDGER_ASSERT(scratch != NULL, "Failed to allocate scratch buf for transaction");

    ByteWriter bw;
    bw_init(&bw, scratch, MAX_ENCODED_TX_LEN);
    bw_put(&bw, PREPARED_TRANSACTION_HASH_PURPOSE, 4);
    encode_transaction(&bw, tx);

    cx_sha256_hash(scratch, bw_size(&bw), out);

    PRINTF("TX hash: %.*H\n", 32, out);
}

static void encode_input_contract(ByteWriter *bw, const InputContract *c) {
    encode_int64(bw, c->created_at);
    // FIXME:
    // encode_hash(bw, c->v1_hash);
}
static void wrap_encode_input_contract(ByteWriter *bw, const void *ctx) {
    encode_input_contract(bw, (const InputContract *) ctx);
}

static void encode_metadata(ByteWriter *bw, const Metadata *m) {
    bw_put_byte(bw, 0x01);
    encode_repeated(bw,
                    m->submitter_info.act_as_count,
                    m->submitter_info.act_as,
                    sizeof(char *),
                    wrap_encode_string);
    encode_string(bw, m->submitter_info.command_id);
    encode_string(bw, m->transaction_uuid);
    encode_int32(bw, m->mediator_group);
    encode_string(bw, m->synchronizer_id);
    encode_optional(bw,
                    m->has_min_ledger_effective_time,
                    (EncodeFn) encode_int64,
                    &m->min_ledger_effective_time);
    encode_optional(bw,
                    m->has_max_ledger_effective_time,
                    (EncodeFn) encode_int64,
                    &m->max_ledger_effective_time);
    encode_int64(bw, m->preparation_time);
    encode_repeated(bw,
                    m->input_contracts_count,
                    m->input_contracts,
                    sizeof(InputContract),
                    wrap_encode_input_contract);
}

static void hash_metadata(const Metadata *md, uint8_t out[32]) {
    uint8_t *scratch = app_mem_alloc(MAX_ENCODED_METADATA_LEN);
    LEDGER_ASSERT(scratch != NULL, "Failed to allocate buf for metadata hash");

    ByteWriter bw;
    bw_init(&bw, scratch, MAX_ENCODED_METADATA_LEN);
    bw_put(&bw, PREPARED_TRANSACTION_HASH_PURPOSE, 4);
    encode_metadata(&bw, md);

    cx_sha256_hash(scratch, bw_size(&bw), out);

    PRINTF("Metadata hash: %.*H\n", 32, out);
}

int prepared_transaction_hash(const PreparedTransaction *pt, uint8_t out[32]) {
    uint8_t tx_hash[32], md_hash[32];

    clear_hash_error();

    hash_transaction(&pt->transaction, tx_hash);

    if (is_hash_error()) {
        PRINTF("Error hashing transaction: '%s', code: %d\n",
               HASH_ERR_INFO.err_msg,
               HASH_ERR_INFO.err_code);
        return HASH_ERR_INFO.err_code;
    }

    hash_metadata(&pt->metadata, md_hash);

    if (is_hash_error()) {
        PRINTF("Error hashing metadata: '%s', code: %d\n",
               HASH_ERR_INFO.err_msg,
               HASH_ERR_INFO.err_code);
        return HASH_ERR_INFO.err_code;
    }

    uint8_t buf[4 + 1 + 32 + 32];
    ByteWriter bw;
    bw_init(&bw, buf, sizeof(buf));
    bw_put(&bw, PREPARED_TRANSACTION_HASH_PURPOSE, 4);
    bw_put_byte(&bw, HASHING_SCHEME_VERSION);
    bw_put(&bw, tx_hash, 32);
    bw_put(&bw, md_hash, 32);

    cx_sha256_hash(buf, sizeof(buf), out);

    return 0;
}
