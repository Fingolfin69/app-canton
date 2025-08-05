#ifndef HASH_H
#define HASH_H

#include "com/daml/ledger/api/v2/interactive/interactive_submission_service.pb.h"

typedef com_daml_ledger_api_v2_interactive_PreparedTransaction PreparedTransaction;

int prepared_transaction_hash(const PreparedTransaction *pt, uint8_t out[32]);

#endif  // HASH_H