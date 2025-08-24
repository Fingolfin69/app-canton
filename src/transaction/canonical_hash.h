#ifndef HASH_H
#define HASH_H

#include "com/daml/ledger/api/v2/interactive/interactive_submission_service.pb.h"

typedef com_daml_ledger_api_v2_interactive_DamlTransaction DamlTransaction;
typedef com_daml_ledger_api_v2_interactive_DamlTransaction_Node Node;
typedef com_daml_ledger_api_v2_interactive_Metadata Metadata;
typedef com_daml_ledger_api_v2_interactive_Metadata_InputContract InputContract;
typedef com_daml_ledger_api_v2_interactive_PreparedTransaction PreparedTransaction;

typedef struct {
    uint8_t *base, *ptr, *end;
    bool overflow;
} ByteWriter;

int hash_transaction(ByteWriter *bw, const DamlTransaction *tx);
int hash_node(ByteWriter *bw, const DamlTransaction *tx, const Node *node);
int finalize_hash_transaction(ByteWriter *bw, uint8_t out[32]);

int hash_metadata(ByteWriter *bw, const Metadata *md);
int hash_input_contract(ByteWriter *bw, const InputContract *c);
int finalize_hash_metadata(ByteWriter *bw, uint8_t out[32]);

int finalize_hash(const uint8_t tx_hash[32], const uint8_t md_hash[32], uint8_t out[32]);

#endif  // HASH_H
