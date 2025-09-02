#pragma once

#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t

#include "com/daml/ledger/api/v2/interactive/interactive_submission_service.pb.h"

#define MAX_TX_LEN   2048
#define PARTY_ID_LEN 131  // 2*32 + 2 + 2*32 + 1 hex(pubkey)::hex(fingerprint) + null terminator
#define MAX_MEMO_LEN 465  // 510 - ADDRESS_LEN - 2*SIZE(U64) - SIZE(MAX_VARINT)

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
 * Structure for transaction hashing context.
 */
typedef struct {
    union {
        com_daml_ledger_api_v2_interactive_DamlTransaction daml_transaction;  /// DAML transaction
        com_daml_ledger_api_v2_interactive_Metadata metadata;  /// metadata of the transaction
        com_daml_ledger_api_v2_interactive_PrepareSubmissionResponse
            prepared_submission_details;  /// prepared transaction
    };

    union {
        com_daml_ledger_api_v2_interactive_DamlTransaction_Node node;  /// DAML transaction node
        com_daml_ledger_api_v2_interactive_Metadata_InputContract
            input_contract;  /// input contract
    };
} transaction_parts_ctx_t;
