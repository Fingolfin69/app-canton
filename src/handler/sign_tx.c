/*****************************************************************************
 *   Ledger App Boilerplate.
 *   (c) 2020 Ledger SAS.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

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
#include "types.h"
#include "proto_deserialize.h"
#include "validate.h"
#include "canonical_hash.h"

static int process_tx_chunk(buffer_t *cdata,
                            signing_type_e type,
                            bool first,
                            bool more,
                            bool msg_end);
static int process_sign_prepared_transaction(transaction_ctx_t *tx_info);
static int process_sign_transaction_hash(buffer_t *buf, uint8_t out[32]);
static int process_prepared_tx_part(buffer_t *buf);

int handler_sign_tx(buffer_t *cdata, signing_type_e type, bool first, bool more, bool msg_end) {
    int result = process_tx_chunk(cdata, type, first, more, msg_end);
    if (result != 0) {
        return io_send_sw(result);  // Send the error code via io_send_sw
    }

    // Check if we're in the correct state to proceed with signing
    if (G_context.state == STATE_PARSED) {
        // Transaction is complete and ready to be signed
        buffer_t buf = {.ptr = G_context.tx_info.raw_tx,
                        .size = G_context.tx_info.raw_tx_len,
                        .offset = 0};

        if (G_context.signing_type != type) {
            PRINTF("Signing type mismatch: expected %d, got %d\n", G_context.signing_type, type);
            return io_send_sw(SW_BAD_STATE);
        }

        switch (G_context.signing_type) {
            case SIGN_PREPARED_TRANSACTION:
                result = process_sign_prepared_transaction(&G_context.tx_info);
                break;
            case SIGN_HASH:
                result = process_sign_transaction_hash(&buf, G_context.tx_info.m_hash);
                break;
            case SIGN_UNTYPED_VERSIONED_MESSAGE:
                // Not implemented yet
                break;
            default:
                PRINTF("Unsupported signing type: %d\n", G_context.signing_type);
                return io_send_sw(SW_BAD_STATE);
        }

        if (result != 0) {
            return io_send_sw(result);  // Send the error code via io_send_sw
        }

        PRINTF("Hash: %.*H\n", sizeof(G_context.tx_info.m_hash), G_context.tx_info.m_hash);

        return ui_display_blind_signed_transaction();
    } else if (G_context.state >= STATE_EXPECTING_MORE &&
               G_context.state <= STATE_RECEIVING_PREPARED_SUBMISSION_DETAILS) {
        // More APDUs with transaction parts are expected.
        // Send a SW_OK to signal that we have received the chunk
        return io_send_sw(SW_OK);
    } else {
        // Invalid state
        PRINTF("Invalid state after processing chunk: %d\n", G_context.state);
        return io_send_sw(SW_BAD_STATE);
    }
}

static int process_tx_chunk(buffer_t *cdata,
                            signing_type_e type,
                            bool first,
                            bool more,
                            bool msg_end) {
    if (first) {  // first APDU, parse BIP32 path
        explicit_bzero(&G_context, sizeof(G_context));
        PRINTF("Processing first chunk of transaction\n");
        G_context.req_type = CONFIRM_TRANSACTION;
        G_context.signing_type = type;
        G_context.state =
            type == SIGN_PREPARED_TRANSACTION ? STATE_RECEIVING_DAML_TX_PART : STATE_EXPECTING_MORE;

        if (!buffer_read_u8(cdata, &G_context.bip32_path_len) ||
            !buffer_read_bip32_path(cdata,
                                    G_context.bip32_path,
                                    (size_t) G_context.bip32_path_len)) {
            return SW_WRONG_DATA_LENGTH;
        }

    } else {  // parse transaction
        if (G_context.req_type != CONFIRM_TRANSACTION) {
            PRINTF("Request type mismatch: expected CONFIRM_TRANSACTION, got %d\n",
                   G_context.req_type);
            return SW_BAD_STATE;
        }

        if (G_context.tx_info.raw_tx_len + cdata->size > MAX_TRANSACTION_LEN) {
            return SW_WRONG_TX_LENGTH;
        }

        if (!buffer_move(cdata,
                         G_context.tx_info.raw_tx + G_context.tx_info.raw_tx_len,
                         cdata->size)) {
            PRINTF("Failed to copy transaction chunk\n");
            return SW_TX_PARSING_FAIL;
        }

        G_context.tx_info.raw_tx_len += cdata->size;

        if (G_context.signing_type == SIGN_PREPARED_TRANSACTION) {
            if (msg_end) {
                buffer_t buf = {.ptr = G_context.tx_info.raw_tx,
                                .size = G_context.tx_info.raw_tx_len,
                                .offset = 0};

                // Reset for next message (transaction part)
                G_context.tx_info.raw_tx_len = 0;

                // Deserialize and hash received message (transaction part)
                return process_prepared_tx_part(&buf);
            }
        } else {
            if (more) {
                G_context.state = STATE_EXPECTING_MORE;
            } else {
                G_context.state = STATE_PARSED;
            }
        }
    }
    return 0;
}

static int process_sign_prepared_transaction(transaction_ctx_t *tx_info) {
    int res = finalize_hash(tx_info->partial_tx_hash, tx_info->partial_md_hash, tx_info->m_hash);

    if (res != 0) {
        PRINTF("Failed to compute transaction hash: %d\n", res);
        return SW_TX_HASH_FAIL;
    }

    if (memcmp(tx_info->m_hash,
               tx_info->tx_parts_ctx.prepared_submission_details.prepared_transaction_hash.bytes,
               sizeof(G_context.tx_info.m_hash)) != 0) {
        PRINTF("Transaction hash mismatch: computed %.*H, expected %.*H\n",
               32,
               tx_info->m_hash,
               32,
               tx_info->tx_parts_ctx.prepared_submission_details.prepared_transaction_hash.bytes);

        return SW_TX_HASH_FAIL;
    }

    return 0;
}

static int process_prepared_tx_part(buffer_t *buf) {
    switch (G_context.state) {
        case STATE_RECEIVING_DAML_TX_PART: {
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
            G_context.state = G_context.tx_info.tx_parts_ctx.daml_transaction.nodes_count == 0
                                  ? STATE_RECEIVING_METADATA
                                  : STATE_RECEIVING_DAML_NODES;
        } break;
        case STATE_RECEIVING_DAML_NODES: {
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
                G_context.state = STATE_RECEIVING_METADATA;
            }
        } break;
        case STATE_RECEIVING_METADATA: {
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

            G_context.state = G_context.tx_info.tx_parts_ctx.metadata.input_contracts_count == 0
                                  ? STATE_RECEIVING_PREPARED_SUBMISSION_DETAILS
                                  : STATE_RECEIVING_METADATA_INPUT_CONTRACTS;
        } break;
        case STATE_RECEIVING_METADATA_INPUT_CONTRACTS: {
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
                G_context.state = STATE_RECEIVING_PREPARED_SUBMISSION_DETAILS;
            }
        } break;
        case STATE_RECEIVING_PREPARED_SUBMISSION_DETAILS: {
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

            G_context.state = STATE_PARSED;

        } break;
        default:
            PRINTF("Invalid state during processing prepared tx part: %d\n", G_context.state);
            return SW_BAD_STATE;
    }

    return 0;
}

static int process_sign_transaction_hash(buffer_t *buf, uint8_t out[32]) {
    if (buf->size != 32) {
        PRINTF("Invalid hash length: expected 32, got %d\n", buf->size);
        return SW_WRONG_DATA_LENGTH;
    }

    memcpy(out, buf->ptr, 32);

    return 0;
}
