#pragma once

#include "cx.h"

#include "tx_types.h"

typedef struct {
    cx_sha256_t ctx;
} HashWriter;

int hash_transaction(HashWriter *hw, const DamlTransaction *tx);
int hash_node(HashWriter *hw, const DamlTransaction *tx, const Node *node);
int finalize_hash_transaction(HashWriter *hw, uint8_t out[32]);

int hash_metadata(HashWriter *hw, const Metadata *md);
int hash_input_contract(HashWriter *hw, const InputContract *c);
int finalize_hash_metadata(HashWriter *hw, uint8_t out[32]);

int finalize_hash(const uint8_t tx_hash[32], const uint8_t md_hash[32], uint8_t out[32]);
