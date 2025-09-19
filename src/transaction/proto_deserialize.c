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

typedef com_daml_ledger_api_v2_Value Value;
typedef com_daml_ledger_api_v2_RecordField RecordField;
typedef com_daml_ledger_api_v2_List List;

bool decode_value_var(pb_istream_t *stream, const pb_field_t *field, void **arg);

//!!!!! WORKS CORRECTLY RECURSIVELY
bool decode_record_field(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    PRINTF("GOOSE Decoding Record fields\n");
    RecordField rf = {};
    rf.value.cb_sum.funcs.decode = &decode_value_var;

    if (!pb_decode(stream, com_daml_ledger_api_v2_RecordField_fields, &rf)) {
        PRINTF("GOOSE Failed to decode Record field: %s\n", PB_GET_ERROR(stream));
        return false;
    }

    PRINTF("GOOSE Decoded Record field with label: %s\n", rf.label);

    pb_release(com_daml_ledger_api_v2_RecordField_fields, &rf);

    return true;
}

//!!!!! DOES NOT WORK RECURSIVELY (callback on nested value in List is not called)
bool decode_list(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    PRINTF("GOOSE Decoding List elements\n");
    List lst = com_daml_ledger_api_v2_List_init_zero;
    lst.elements.funcs.decode = &decode_value_var;

    if (!pb_decode(stream, com_daml_ledger_api_v2_List_fields, &lst)) {
        PRINTF("GOOSE Failed to decode List: %s\n", PB_GET_ERROR(stream));
        return false;
    }

    PRINTF("/GOOSE Decoding List elements\n");
    pb_release(com_daml_ledger_api_v2_List_fields, &lst);

    return true;
}

bool decode_value_var(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    PRINTF("GOOSE Decoding Value\n");

    Value *topmsg = field->message;
    (void) topmsg;

    if (field->tag == com_daml_ledger_api_v2_Value_record_tag) {
        com_daml_ledger_api_v2_Record *msg = field->pData;
        msg->fields.funcs.decode = &decode_record_field;
    } else if (field->tag == com_daml_ledger_api_v2_Value_list_tag) {
        com_daml_ledger_api_v2_List *msg = field->pData;
        msg->elements.funcs.decode = &decode_list;
    }

    return true;
}

bool decode_input_contract_argument(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    PRINTF("GOOSE Decoding Input contract argument\n");

    Value v;
    v.cb_sum.funcs.decode = &decode_value_var;

    if (!pb_decode(stream, com_daml_ledger_api_v2_Value_fields, &v)) {
        PRINTF("GOOSE Failed to decode Input contract argument: %s\n", PB_GET_ERROR(stream));
        return false;
    }

    pb_release(com_daml_ledger_api_v2_Value_fields, &v);


    return true;
}

bool decode_tx_v1_create(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    PRINTF("GOOSE Decode Create node\n");

    com_daml_ledger_api_v2_interactive_transaction_v1_Create c;
    c.argument.funcs.decode = &decode_input_contract_argument;

    if (!pb_decode(stream, com_daml_ledger_api_v2_interactive_transaction_v1_Create_fields, &c)) {
        PRINTF("GOOSE Failed to decode Create node: %s\n", PB_GET_ERROR(stream));
        return false;
    }

    pb_release(com_daml_ledger_api_v2_interactive_transaction_v1_Create_fields, &c);


    return true;
}

parser_status_e proto_deserialize_input_contract(buffer_t *buf, transaction_ctx_t *tx_ctx) {
    pb_istream_t stream = pb_istream_from_buffer(buf->ptr, buf->size);

    PRINTF("GOOSE Decoding Input contract from buffer of size %d bytes\n", buf->size);

    tx_ctx->tx_parts_ctx.input_contract.cb_contract.funcs.decode = &decode_tx_v1_create;

    if (!pb_decode(&stream,
                   com_daml_ledger_api_v2_interactive_DeviceMetadata_InputContract_fields,
                   &tx_ctx->tx_parts_ctx.input_contract)) {
        PRINTF("Failed to decode Input contract: %s\n", PB_GET_ERROR(&stream));
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

void release_input_contract(transaction_ctx_t *tx_ctx) {
    pb_release(com_daml_ledger_api_v2_interactive_DeviceMetadata_InputContract_fields,
               &tx_ctx->tx_parts_ctx.input_contract);
}
