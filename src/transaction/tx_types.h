#pragma once

#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t

#include "com/daml/ledger/api/v2/interactive/device.pb.h"

typedef com_daml_ledger_api_v2_interactive_DeviceDamlTransaction DamlTransaction;
typedef com_daml_ledger_api_v2_interactive_DeviceDamlTransaction_Node Node;
typedef com_daml_ledger_api_v2_interactive_DeviceMetadata Metadata;
typedef com_daml_ledger_api_v2_interactive_DeviceMetadata_InputContract InputContract;

#define PARTY_ID_LEN 74  // 3 + 2 + 2*34 + 1 = 74 ldg::hex(fingerprint) + null terminator

typedef enum {
    PARSING_OK = 1,
    NONCE_PARSING_ERROR = -1,
    TO_PARSING_ERROR = -2,
    VALUE_PARSING_ERROR = -3,
    MEMO_LENGTH_ERROR = -4,
    MEMO_PARSING_ERROR = -5,
    MEMO_ENCODING_ERROR = -6,
    WRONG_LENGTH_ERROR = -7
} parser_status_e;

/**
 * Structure for transaction parts context.
 */
typedef struct {
    union {
        DamlTransaction daml_transaction;  /// DAML transaction
        Metadata metadata;                 /// metadata of the transaction
    };

    union {
        Node node;                     /// DAML transaction node
        InputContract input_contract;  /// input contract
    };
} transaction_parts_ctx_t;
