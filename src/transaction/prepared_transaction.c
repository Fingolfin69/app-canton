#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool
#include <stddef.h>   // size_t
#include <string.h>   // memset, explicit_bzero

#include "os.h"
#include "cx.h"
#include "buffer.h"

#include "sign_tx.h"
#include "sw.h"
#include "globals.h"
#include "display.h"
#include "tx_types.h"
// #include "types.h"
#include "proto_deserialize.h"
#include "validate.h"
#include "canonical_hash.h"

typedef enum {
    RECEIVING_DAML_TX_PART,                 /// Receiving part of DAML transaction
    RECEIVING_DAML_NODES,                   /// Receiving DAML nodes
    RECEIVING_METADATA,                     /// Receiving metadata
    RECEIVING_METADATA_INPUT_CONTRACTS,     /// Receiving input contracts
    RECEIVING_PREPARED_SUBMISSION_DETAILS,  /// Receiving prepared submission details
} prepared_tx_receiving_state_e;

static prepared_tx_receiving_state_e tx_state = RECEIVING_DAML_TX_PART;

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

            int res = hash_transaction(&G_context.tx_info.hash_buf,
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

            int res = hash_node(&G_context.tx_info.hash_buf,
                                &G_context.tx_info.tx_parts_ctx.daml_transaction,
                                &G_context.tx_info.tx_parts_ctx.node);

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
            int res = finalize_hash_transaction(&G_context.tx_info.hash_buf,
                                                G_context.tx_info.partial_tx_hash);

            if (res != 0) {
                PRINTF("Failed to finalize DAML transaction hash: %d\n", res);
                return SW_TX_HASH_FAIL;
            }

            parser_status_e status = proto_deserialize_metadata(buf, &G_context.tx_info);

            if (status != PARSING_OK) {
                PRINTF("Failed to parse Metadata part: %d\n", status);
                return SW_TX_PARSING_FAIL;
            }

            res = hash_metadata(&G_context.tx_info.hash_buf,
                                &G_context.tx_info.tx_parts_ctx.metadata);

            if (res != 0) {
                PRINTF("Failed to hash Metadata part: %d\n", res);
                return SW_TX_HASH_FAIL;
            }

            G_context.tx_info.recv_node_idx = 0;

            tx_state = G_context.tx_info.tx_parts_ctx.metadata.input_contracts_count == 0
                           ? RECEIVING_PREPARED_SUBMISSION_DETAILS
                           : RECEIVING_METADATA_INPUT_CONTRACTS;
        } break;
        case RECEIVING_METADATA_INPUT_CONTRACTS: {
            parser_status_e status = proto_deserialize_input_contract(buf, &G_context.tx_info);

            if (status != PARSING_OK) {
                PRINTF("Failed to parse Input Contract part: %d\n", status);
                return SW_TX_PARSING_FAIL;
            }

            int res = hash_input_contract(&G_context.tx_info.hash_buf,
                                          &G_context.tx_info.tx_parts_ctx.input_contract);

            if (res != 0) {
                PRINTF("Failed to hash Input Contract part: %d\n", res);
                return SW_TX_HASH_FAIL;
            }

            G_context.tx_info.recv_node_idx++;

            if (G_context.tx_info.recv_node_idx ==
                G_context.tx_info.tx_parts_ctx.metadata.input_contracts_count) {
                tx_state = RECEIVING_PREPARED_SUBMISSION_DETAILS;
            }
        } break;
        case RECEIVING_PREPARED_SUBMISSION_DETAILS: {
            int res = finalize_hash_metadata(&G_context.tx_info.hash_buf,
                                             G_context.tx_info.partial_md_hash);

            if (res != 0) {
                PRINTF("Failed to finalize Metadata hash: %d\n", res);
                return SW_TX_HASH_FAIL;
            }

            parser_status_e status =
                proto_deserialize_prepared_submission_details(buf, &G_context.tx_info);

            if (status != PARSING_OK) {
                PRINTF("Failed to parse Prepared Submission Details part: %d\n", status);
                return SW_TX_PARSING_FAIL;
            }

            if (G_context.state == STATE_PARSED) {
                // Finalize hash
                res = finalize_hash(G_context.tx_info.partial_tx_hash,
                                    G_context.tx_info.partial_md_hash,
                                    G_context.tx_info.m_hash);
                if (res != 0) {
                    PRINTF("Failed to compute transaction hash: %d\n", res);
                    return SW_TX_HASH_FAIL;
                }

                if (memcmp(G_context.tx_info.m_hash,
                           G_context.tx_info.tx_parts_ctx.prepared_submission_details
                               .prepared_transaction_hash.bytes,
                           sizeof(G_context.tx_info.m_hash)) != 0) {
                    PRINTF("Transaction hash mismatch: computed %.*H, expected %.*H\n",
                           32,
                           G_context.tx_info.m_hash,
                           32,
                           G_context.tx_info.tx_parts_ctx.prepared_submission_details
                               .prepared_transaction_hash.bytes);
                    return SW_TX_HASH_FAIL;
                }
            }
        } break;
        default:
            PRINTF("Invalid state during processing prepared tx part: %d\n", tx_state);
            return SW_BAD_STATE;
    }

    return 0;
}
