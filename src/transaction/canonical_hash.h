#pragma once

#include "cx.h"
#include "com/daml/ledger/api/v2/interactive/interactive_submission_service.pb.h"
#include "bytewriter.h"
#include "tx_types.h"

typedef struct {
    cx_sha256_t ctx;
} HashWriter;

int hash_transaction(HashWriter *hw, const DamlTransaction *tx);
int hash_node(HashWriter *hw, const DamlTransaction *tx, const Node *node);
int finalize_hash_transaction(HashWriter *hw, uint8_t out[32]);

int hash_metadata(HashWriter *hw, const Metadata *md);
int finalize_hash_metadata(HashWriter *hw, uint8_t out[32]);

int finalize_hash(const uint8_t tx_hash[32], const uint8_t md_hash[32], uint8_t out[32]);

void hw_init(HashWriter *hw);
void hw_put(HashWriter *hw, const void *p, size_t n);
void hw_finalize(HashWriter *hw, uint8_t out[32]);
void hw_put_byte(HashWriter *hw, uint8_t b);
void hw_put_u32_be(HashWriter *hw, uint32_t v);
void hw_put_u64_be(HashWriter *hw, uint64_t v);
void encode_bool(HashWriter *hw, bool v);
void encode_int32(HashWriter *hw, int32_t v);
void encode_int64(HashWriter *hw, int64_t v);
void encode_int64_by_ptr(HashWriter *hw, int64_t *v);
void encode_bytes(HashWriter *hw, const uint8_t *data, int32_t len);
void encode_string(HashWriter *hw, const char *s);
void encode_hash(HashWriter *hw, const uint8_t h[32]);
void encode_hex_string(HashWriter *hw, const char *hex);
void encode_identifier(HashWriter *hw, const Identifier *id);
void encode_create_cb_start(HashWriter *hw, const Node_CreateCb *c);
void encode_create_cb_end(HashWriter *hw, const Node_CreateCb *c);
