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
#include "buffer.h"

#include "proto_deserialize.h"
#include "utils.h"
#include "types.h"
#include "com/daml/ledger/api/v2/interactive/interactive_submission_service.pb.h"

#include "pb_decode.h"

#if defined(TEST) || defined(FUZZ)
#include "assert.h"
#define LEDGER_ASSERT(x, y) assert(x)
#else
#include "ledger_assert.h"
#endif

parser_status_e proto_deserialize(buffer_t *buf, signing_type_e type, transaction_t *tx) {
    LEDGER_ASSERT(buf != NULL, "NULL buf");
    LEDGER_ASSERT(tx != NULL, "NULL tx");

    if (buf->size > MAX_TX_LEN) {
        return WRONG_LENGTH_ERROR;
    }

    pb_istream_t stream = pb_istream_from_buffer(buf->ptr, buf->size);

    PRINTF("Decoding transaction from buffer of size %d bytes\n", buf->size);

    const pb_msgdesc_t* message_type;

    switch (type) {
        case SIGN_PREPARED_TRANSACTION:
            message_type = com_daml_ledger_api_v2_interactive_PrepareSubmissionResponse_fields;
            break;
        case SIGN_UNTYPED_VERSIONED_MESSAGE:
            // TODO: implement when needed
        case SIGN_HASH:
        default:
            return VALUE_PARSING_ERROR;
    }

    if (!pb_decode(&stream,
                   message_type,
                   &tx->prepared_tx)) {
        PRINTF("Failed to decode transaction: %s\n", PB_GET_ERROR(&stream));
        return VALUE_PARSING_ERROR;
    }

    PRINTF("Decoded transaction successfully.\n");
    // PRINTF("Transaction fields : \n");
    // PRINTF("Prepared TX hash: %.*H\n",
    //        tx->prepared_tx.prepared_transaction_hash.size,
    //        tx->prepared_tx.prepared_transaction_hash.bytes);
    // PRINTF("  Has prepared transaction: %d\n", tx->prepared_tx.has_prepared_transaction);
    // PRINTF("  Has hashing details: %d\n", tx->prepared_tx.hashing_details != NULL);

    return PARSING_OK;
}
