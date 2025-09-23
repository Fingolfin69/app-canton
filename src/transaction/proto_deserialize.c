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
#include "com/daml/ledger/api/v2/interactive/device.pb.h"

#include "pb_decode.h"

#if defined(TEST) || defined(FUZZ)
#include "assert.h"
#define LEDGER_ASSERT(x, y) assert(x)
#else
#include "ledger_assert.h"
#endif

parser_status_e proto_deserialize_daml_tx(buffer_t *buf, transaction_ctx_t *tx_ctx) {
    pb_istream_t stream = pb_istream_from_buffer(buf->ptr, buf->size);

    PRINTF("Decoding Daml transaction from buffer of size %d bytes\n", buf->size);

    if (!pb_decode(&stream,
                   com_daml_ledger_api_v2_interactive_DeviceDamlTransaction_fields,
                   &tx_ctx->tx_parts_ctx.daml_transaction)) {
        PRINTF("Failed to decode Daml transaction: %s\n", PB_GET_ERROR(&stream));
        return VALUE_PARSING_ERROR;
    }

    return PARSING_OK;
}

parser_status_e proto_deserialize_node(buffer_t *buf, transaction_ctx_t *tx_ctx) {
    pb_istream_t stream = pb_istream_from_buffer(buf->ptr, buf->size);

    PRINTF("Decoding Node from buffer of size %d bytes\n", buf->size);

    if (!pb_decode(&stream,
                   com_daml_ledger_api_v2_interactive_DeviceDamlTransaction_Node_fields,
                   &tx_ctx->tx_parts_ctx.node)) {
        PRINTF("Failed to decode Node: %s\n", PB_GET_ERROR(&stream));
        return VALUE_PARSING_ERROR;
    }

    return PARSING_OK;
}

parser_status_e proto_deserialize_metadata(buffer_t *buf, transaction_ctx_t *tx_ctx) {
    pb_istream_t stream = pb_istream_from_buffer(buf->ptr, buf->size);

    PRINTF("Decoding Metadata from buffer of size %d bytes\n", buf->size);

    if (!pb_decode(&stream,
                   com_daml_ledger_api_v2_interactive_DeviceMetadata_fields,
                   &tx_ctx->tx_parts_ctx.metadata)) {
        PRINTF("Failed to decode Metadata: %s\n", PB_GET_ERROR(&stream));
        return VALUE_PARSING_ERROR;
    }

    return PARSING_OK;
}

void release_daml_tx(transaction_ctx_t *tx_ctx) {
    pb_release(com_daml_ledger_api_v2_interactive_DeviceDamlTransaction_fields,
               &tx_ctx->tx_parts_ctx.daml_transaction);
}

void release_node(transaction_ctx_t *tx_ctx) {
    pb_release(com_daml_ledger_api_v2_interactive_DeviceDamlTransaction_Node_fields,
               &tx_ctx->tx_parts_ctx.node);
}

void release_metadata(transaction_ctx_t *tx_ctx) {
    pb_release(com_daml_ledger_api_v2_interactive_DeviceMetadata_fields,
               &tx_ctx->tx_parts_ctx.metadata);
}
