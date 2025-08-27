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

static int process_tx_chunk(buffer_t *cdata, signing_type_e type, bool first, bool more);
static int process_sign_prepared_transaction(buffer_t *buf);
static int process_sign_transaction_hash(buffer_t *buf, uint8_t out[32]);

int handler_sign_tx(buffer_t *cdata, signing_type_e type, bool first, bool more) {
    int result = process_tx_chunk(cdata, type, first, more);
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
                result = process_sign_prepared_transaction(&buf);
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
    } else if (G_context.state == STATE_EXPECTING_MORE) {
        // More APDUs with transaction parts are expected.
        // Send a SW_OK to signal that we have received the chunk
        return io_send_sw(SW_OK);
    } else {
        // Invalid state
        PRINTF("Invalid state after processing chunk: %d\n", G_context.state);
        return io_send_sw(SW_BAD_STATE);
    }
}

static int process_tx_chunk(buffer_t *cdata, signing_type_e type, bool first, bool more) {
    if (first) {  // first APDU, parse BIP32 path
        explicit_bzero(&G_context, sizeof(G_context));
        PRINTF("Processing first chunk of transaction\n");
        G_context.req_type = CONFIRM_TRANSACTION;
        G_context.signing_type = type;
        G_context.state = STATE_EXPECTING_MORE;

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

        if (more) {
            G_context.state = STATE_EXPECTING_MORE;
        } else {
            G_context.state = STATE_PARSED;
        }
    }
    return 0;
}

static int process_sign_prepared_transaction(buffer_t *buf) {
    parser_status_e status =
        proto_deserialize(buf, SIGN_PREPARED_TRANSACTION, &G_context.tx_info.transaction);

    PRINTF("Parsing status: %d.\n", status);
    if (status != PARSING_OK) {
        return SW_TX_PARSING_FAIL;
    }

    // Calculate the transaction hash
    int res =
        prepared_transaction_hash(&G_context.tx_info.transaction.prepared_tx.prepared_transaction,
                                  G_context.tx_info.m_hash);

    if (res != 0) {
        PRINTF("Failed to compute transaction hash: %d\n", res);
        return SW_TX_HASH_FAIL;
    }

    if (memcmp(G_context.tx_info.m_hash,
               G_context.tx_info.transaction.prepared_tx.prepared_transaction_hash.bytes,
               sizeof(G_context.tx_info.m_hash)) != 0) {
        PRINTF("Transaction hash mismatch: computed %.*H, expected %.*H\n",
               32,
               G_context.tx_info.m_hash,
               32,
               G_context.tx_info.transaction.prepared_tx.prepared_transaction_hash.bytes);

        return SW_TX_HASH_FAIL;
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
