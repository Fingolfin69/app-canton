#include "pb_input_contract_parser.h"

#include "buffer.h"
#include "canonical_hash.h"
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

static HashWriter node_hw;
static uint8_t node_hash[32];
static int32_t value_elem_count;

static bool decode_value_variant(pb_istream_t *stream, const pb_field_t *field, void **arg);

static bool count_record_field(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    (void) field;
    (void) arg;

    cbRecordField rf = com_daml_ledger_api_v2_cb_RecordField_init_zero;

    if (!pb_decode(stream, com_daml_ledger_api_v2_cb_RecordField_fields, &rf)) {
        PRINTF("Failed to decode Record field: %s\n", PB_GET_ERROR(stream));
        return false;
    }

    pb_release(com_daml_ledger_api_v2_cb_RecordField_fields, &rf);

    value_elem_count++;

    return true;
}

static bool count_list_elem(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    (void) field;
    (void) arg;

    cbValue v = com_daml_ledger_api_v2_cb_Value_init_zero;

    if (!pb_decode(stream, com_daml_ledger_api_v2_cb_Value_fields, &v)) {
        PRINTF("Failed to decode List: %s\n", PB_GET_ERROR(stream));
        return false;
    }

    pb_release(com_daml_ledger_api_v2_cb_Value_fields, &v);

    value_elem_count++;

    return true;
}

static bool count_gen_map_entry(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    (void) field;
    (void) arg;

    cbGenMapEntry e = com_daml_ledger_api_v2_cb_GenMap_Entry_init_zero;

    if (!pb_decode(stream, com_daml_ledger_api_v2_cb_GenMap_Entry_fields, &e)) {
        PRINTF("Failed to decode GenMap entry: %s\n", PB_GET_ERROR(stream));
        return false;
    }

    pb_release(com_daml_ledger_api_v2_cb_GenMap_Entry_fields, &e);

    value_elem_count++;

    return true;
}

static bool count_value(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    (void) stream;
    (void) field;
    (void) arg;

    value_elem_count = 0;

    switch (field->tag) {
        case com_daml_ledger_api_v2_cb_Value_unit_tag:
        case com_daml_ledger_api_v2_cb_Value_bool__tag:
        case com_daml_ledger_api_v2_cb_Value_int64_tag:
        case com_daml_ledger_api_v2_cb_Value_numeric_tag:
        case com_daml_ledger_api_v2_cb_Value_timestamp_tag:
        case com_daml_ledger_api_v2_cb_Value_date_tag:
        case com_daml_ledger_api_v2_cb_Value_party_tag:
        case com_daml_ledger_api_v2_cb_Value_text_tag:
        case com_daml_ledger_api_v2_cb_Value_contract_id_tag:
        case com_daml_ledger_api_v2_cb_Value_optional_tag: {
            cbOptional *msg = field->pData;
            msg->value.funcs.decode = &count_list_elem;
        } break;
        case com_daml_ledger_api_v2_cb_Value_list_tag: {
            cbList *msg = field->pData;
            msg->elements.funcs.decode = &count_list_elem;
        } break;
        case com_daml_ledger_api_v2_cb_Value_text_map_tag: {
            LEDGER_ASSERT(false, "TextMap not implemented");
        } break;
        case com_daml_ledger_api_v2_cb_Value_gen_map_tag: {
            cbGenMap *msg = field->pData;
            msg->entries.funcs.decode = &count_gen_map_entry;
        } break;
        case com_daml_ledger_api_v2_cb_Value_record_tag: {
            cbRecord *msg = field->pData;
            msg->fields.funcs.decode = &count_record_field;
        } break;
        case com_daml_ledger_api_v2_cb_Value_variant_tag: {
            LEDGER_ASSERT(false, "Variant not implemented");
        } break;
        case com_daml_ledger_api_v2_cb_Value_enum__tag: {
            LEDGER_ASSERT(false, "Enum not implemented");
        } break;
        default:
            LEDGER_ASSERT(false, "Unknown Value type %d", field->tag);
    }

    return true;
}

static bool count_value_helper(pb_istream_t *stream) {
    cbValue c = com_daml_ledger_api_v2_cb_Value_init_zero;
    pb_istream_t saved_stream = *stream;

    c.cb_sum.funcs.decode = &count_value;

    if (!pb_decode(stream, com_daml_ledger_api_v2_cb_Value_fields, &c)) {
        PRINTF("Failed to count Input contract argument: %s\n", PB_GET_ERROR(stream));
        return false;
    }

    pb_release(com_daml_ledger_api_v2_cb_Value_fields, &c);

    // Restore stream state
    *stream = saved_stream;

    return true;
}

static bool count_record_field_helper(pb_istream_t *stream) {
    cbRecordField rf = com_daml_ledger_api_v2_cb_RecordField_init_zero;
    rf.value.cb_sum.funcs.decode = &count_value;

    pb_istream_t saved_stream = *stream;

    if (!pb_decode(stream, com_daml_ledger_api_v2_cb_RecordField_fields, &rf)) {
        PRINTF("Failed to decode Record field for counting: %s\n", PB_GET_ERROR(stream));
        return false;
    }

    // TODO: shouldn't be here
    // hw_put_byte(&node_hw, rf.label != NULL);  // encode optional field
    // encode_string(&node_hw, rf.label);

    pb_release(com_daml_ledger_api_v2_cb_RecordField_fields, &rf);

    *stream = saved_stream;  // restore stream position
    return true;
}

static void decode_value_primitive_variants(cbValue *v) {
    switch (v->which_sum) {
        case com_daml_ledger_api_v2_cb_Value_unit_tag: {
            PRINTF("Decoding unit\n");
            hw_put_byte(&node_hw, 0x00);
        } break;
        case com_daml_ledger_api_v2_cb_Value_bool__tag: {
            PRINTF("Decoding bool: %s\n", v->bool_ ? "true" : "false");
            hw_put_byte(&node_hw, 0x01);
            encode_bool(&node_hw, v->bool_);
        } break;
        case com_daml_ledger_api_v2_cb_Value_int64_tag: {
            PRINTF("Decoding int64: %lld\n", v->int64);
            hw_put_byte(&node_hw, 0x02);
            encode_int64(&node_hw, v->int64);
        } break;
        case com_daml_ledger_api_v2_cb_Value_date_tag: {
            PRINTF("Decoding date: %lld\n", v->date);
            hw_put_byte(&node_hw, 0x05);
            encode_int32(&node_hw, v->date);
        } break;
        case com_daml_ledger_api_v2_cb_Value_timestamp_tag: {
            PRINTF("Decoding timestamp: %lld\n", v->timestamp);
            hw_put_byte(&node_hw, 0x04);
            encode_int64(&node_hw, v->timestamp);
        } break;
        case com_daml_ledger_api_v2_cb_Value_numeric_tag: {
            PRINTF("Decoding numeric: %s\n", v->numeric);
            hw_put_byte(&node_hw, 0x03);
            encode_string(&node_hw, v->numeric);
        } break;
        case com_daml_ledger_api_v2_cb_Value_party_tag: {
            PRINTF("Decoding party: %s\n", v->party);
            hw_put_byte(&node_hw, 0x06);
            encode_string(&node_hw, v->party);
        } break;
        case com_daml_ledger_api_v2_cb_Value_text_tag: {
            PRINTF("Decoding text: %s\n", v->text);
            hw_put_byte(&node_hw, 0x07);
            encode_string(&node_hw, v->text);
        } break;
        case com_daml_ledger_api_v2_cb_Value_contract_id_tag: {
            PRINTF("Decoding contract_id: %s\n", v->contract_id);
            hw_put_byte(&node_hw, 0x08);
            encode_hex_string(&node_hw, v->contract_id);
        } break;
    }
}

static bool decode_record_field_label(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    (void) field;
    (void) arg;

    // Read string from stream
    char label_buffer[64] = {0};
    size_t len =
        stream->bytes_left < sizeof(label_buffer) ? stream->bytes_left : sizeof(label_buffer) - 1;

    if (!pb_read(stream, (pb_byte_t *) label_buffer, len)) {
        PRINTF("Failed to read string from stream\n");
        return false;
    }

    PRINTF("Decoded Record field label: %s\n", label_buffer);

    // Encode the label
    PRINTF(">>>>>>>>Encoded Record field label: %s\n", label_buffer);
    hw_put_byte(&node_hw, 0x01);  // encode optional field
    encode_string(&node_hw, label_buffer);

    return true;
}

static bool decode_record_field(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    (void) field;
    (void) arg;

    PRINTF("Decoding Record fields\n");

    // Encode number of record fields previously counted
    if (value_elem_count != 0) {
        encode_int32(&node_hw, value_elem_count);
        value_elem_count = 0;
    }

    if (!count_record_field_helper(stream)) {
        return false;
    }

    cbRecordField rf = com_daml_ledger_api_v2_cb_RecordField_init_zero;
    rf.value.cb_sum.funcs.decode = &decode_value_variant;
    rf.label.funcs.decode = &decode_record_field_label;

    if (!pb_decode(stream, com_daml_ledger_api_v2_cb_RecordField_fields, &rf)) {
        PRINTF("Failed to decode Record field: %s\n", PB_GET_ERROR(stream));
        return false;
    }

    decode_value_primitive_variants(&rf.value);

    // PRINTF("/Decoded Record field with label: %s\n", rf.label);

    pb_release(com_daml_ledger_api_v2_cb_RecordField_fields, &rf);

    return true;
}

static bool decode_list_elem(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    (void) field;
    (void) arg;

    PRINTF("Decoding List elements\n");
    if (!count_value_helper(stream)) {
        return false;
    }

    cbValue v = com_daml_ledger_api_v2_cb_Value_init_zero;
    v.cb_sum.funcs.decode = &decode_value_variant;

    if (!pb_decode(stream, com_daml_ledger_api_v2_cb_Value_fields, &v)) {
        PRINTF("Failed to decode List: %s\n", PB_GET_ERROR(stream));
        return false;
    }

    decode_value_primitive_variants(&v);

    PRINTF("/Decoding List elements\n");
    pb_release(com_daml_ledger_api_v2_cb_Value_fields, &v);

    return true;
}

static bool decode_value_opt(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    (void) field;
    (void) arg;

    PRINTF("Decoding Optional value\n");
    if (!count_value_helper(stream)) {
        return false;
    }

    cbValue v = com_daml_ledger_api_v2_cb_Value_init_zero;
    v.cb_sum.funcs.decode = &decode_value_variant;

    if (!pb_decode(stream, com_daml_ledger_api_v2_cb_Value_fields, &v)) {
        PRINTF("Failed to decode Optional value: %s\n", PB_GET_ERROR(stream));
        return false;
    }

    decode_value_primitive_variants(&v);

    PRINTF("/Decoding Optional value\n");

    pb_release(com_daml_ledger_api_v2_cb_Value_fields, &v);

    return true;
}

static bool decode_gen_map_entry(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    (void) field;
    (void) arg;

    PRINTF("Decoding GenMap entry\n");

    cbGenMapEntry entry = com_daml_ledger_api_v2_cb_GenMap_Entry_init_zero;

    if (!pb_decode(stream, com_daml_ledger_api_v2_cb_GenMap_Entry_fields, &entry)) {
        PRINTF("Failed to decode GenMap entry: %s\n", PB_GET_ERROR(stream));
        return false;
    }

    decode_value_primitive_variants(&entry.key);
    decode_value_primitive_variants(&entry.value);

    PRINTF("/Decoding GenMap entry\n");
    pb_release(com_daml_ledger_api_v2_cb_GenMap_Entry_fields, &entry);

    return true;
}

static bool decode_identifier(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    (void) field;
    (void) arg;

    PRINTF("Decoding Identifier\n");

    com_daml_ledger_api_v2_cb_Identifier id = com_daml_ledger_api_v2_cb_Identifier_init_zero;

    if (!pb_decode(stream, com_daml_ledger_api_v2_cb_Identifier_fields, &id)) {
        PRINTF("Failed to decode Identifier: %s\n", PB_GET_ERROR(stream));
        return false;
    }

    encode_identifier(&node_hw, (const com_daml_ledger_api_v2_Identifier *) &id);

    PRINTF("/Decoding Identifier\n");

    pb_release(com_daml_ledger_api_v2_cb_Identifier_fields, &id);

    return true;
}

static bool decode_identifier_opt(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    (void) field;
    (void) arg;

    PRINTF("Decoding optional Identifier\n");
    hw_put_byte(&node_hw, 0x01);  // encode optional field presence
    return decode_identifier(stream, field, arg);
}

static bool decode_value_variant(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    (void) stream;
    (void) field;
    (void) arg;

    PRINTF("Decoding value variant\n");

    switch (field->tag) {
        case com_daml_ledger_api_v2_cb_Value_unit_tag:
        case com_daml_ledger_api_v2_cb_Value_bool__tag:
        case com_daml_ledger_api_v2_cb_Value_int64_tag:
        case com_daml_ledger_api_v2_cb_Value_numeric_tag:
        case com_daml_ledger_api_v2_cb_Value_timestamp_tag:
        case com_daml_ledger_api_v2_cb_Value_date_tag:
        case com_daml_ledger_api_v2_cb_Value_party_tag:
        case com_daml_ledger_api_v2_cb_Value_text_tag:
        case com_daml_ledger_api_v2_cb_Value_contract_id_tag:
        case com_daml_ledger_api_v2_cb_Value_optional_tag: {
            cbOptional *msg = field->pData;
            msg->value.funcs.decode = &decode_value_opt;
            hw_put_byte(&node_hw, 0x09);
            // Encode optional field presence
            hw_put_byte(&node_hw, value_elem_count == 0 ? 0x00 : 0x01);
            value_elem_count = 0;
        } break;
        case com_daml_ledger_api_v2_cb_Value_list_tag: {
            cbList *msg = field->pData;
            msg->elements.funcs.decode = &decode_list_elem;
            hw_put_byte(&node_hw, 0x0A);
            encode_int32(&node_hw, value_elem_count);
            value_elem_count = 0;
        } break;
        case com_daml_ledger_api_v2_cb_Value_text_map_tag: {
            hw_put_byte(&node_hw, 0x0B);
            LEDGER_ASSERT(false, "TextMap not implemented");
        } break;
        case com_daml_ledger_api_v2_cb_Value_gen_map_tag: {
            cbGenMap *msg = field->pData;
            msg->entries.funcs.decode = &decode_gen_map_entry;
            hw_put_byte(&node_hw, 0x0F);
            encode_int32(&node_hw, value_elem_count);
            value_elem_count = 0;
        } break;
        case com_daml_ledger_api_v2_cb_Value_record_tag: {
            cbRecord *msg = field->pData;
            msg->record_id.funcs.decode = &decode_identifier_opt;
            msg->fields.funcs.decode = &decode_record_field;
            hw_put_byte(&node_hw, 0x0C);
        } break;
        case com_daml_ledger_api_v2_cb_Value_variant_tag: {
            hw_put_byte(&node_hw, 0x0D);
            LEDGER_ASSERT(false, "Variant not implemented");
        } break;
        case com_daml_ledger_api_v2_cb_Value_enum__tag: {
            hw_put_byte(&node_hw, 0x0E);
            LEDGER_ASSERT(false, "Enum not implemented");
        } break;
        default:
            LEDGER_ASSERT(false, "Unknown Value type %d", field->tag);
    }

    return true;
}

static bool decode_input_contract_argument(pb_istream_t *stream,
                                           const pb_field_t *field,
                                           void **arg) {
    (void) field;
    (void) arg;

    PRINTF("Decoding Input contract argument\n");
    if (!count_value_helper(stream)) {
        return false;
    }

    cbValue v = com_daml_ledger_api_v2_cb_Value_init_zero;

    v.cb_sum.funcs.decode = &decode_value_variant;

    if (!pb_decode(stream, com_daml_ledger_api_v2_cb_Value_fields, &v)) {
        PRINTF("Failed to decode Input contract argument: %s\n", PB_GET_ERROR(stream));
        return false;
    }

    decode_value_primitive_variants(&v);

    pb_release(com_daml_ledger_api_v2_cb_Value_fields, &v);

    return true;
}

static bool decode_tx_v1_create(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    (void) field;
    (void) arg;

    com_daml_ledger_api_v2_interactive_transaction_v1_cb_Create c_cb =
        com_daml_ledger_api_v2_interactive_transaction_v1_cb_Create_init_zero;

    PRINTF("Decode Create node\n");
    size_t stream_bytes_left = stream->bytes_left;
    void *stream_state = stream->state;

    // Decoding Create node's plain fields
    if (!pb_decode(stream,
                   com_daml_ledger_api_v2_interactive_transaction_v1_cb_Create_fields,
                   &c_cb)) {
        PRINTF("Failed to decode Create node: %s\n", PB_GET_ERROR(stream));
        return false;
    }

    // Hashing fields up to `argument` field
    hw_init(&node_hw);
    encode_create_cb_start(&node_hw, &c_cb);

    pb_release(com_daml_ledger_api_v2_interactive_transaction_v1_cb_Create_fields, &c_cb);

    // Rewind stream to the beginning of Create node CB message
    stream->bytes_left = stream_bytes_left;
    stream->state = stream_state;

    // Decoding Create node CB recursive field `argument` and hashing it inside callbacks
    c_cb.argument.funcs.decode = &decode_input_contract_argument;
    if (!pb_decode(stream,
                   com_daml_ledger_api_v2_interactive_transaction_v1_cb_Create_fields,
                   &c_cb)) {
        PRINTF("Failed to decode Create node (CB): %s\n", PB_GET_ERROR(stream));
        return false;
    }

    // Finishing hashing Create node
    encode_create_cb_end(&node_hw, &c_cb);
    hw_finalize(&node_hw, node_hash);

    pb_release(com_daml_ledger_api_v2_interactive_transaction_v1_cb_Create_fields, &c_cb);

    PRINTF("/Decode Create node\n");

    return true;
}

parser_status_e proto_deserialize_cb_input_contract(buffer_t *buf, transaction_ctx_t *tx_ctx) {
    pb_istream_t stream = pb_istream_from_buffer(buf->ptr, buf->size);

    PRINTF("Decoding Input contract from buffer of size %d bytes\n", buf->size);

    tx_ctx->tx_parts_ctx.input_contract.cb_contract.funcs.decode = &decode_tx_v1_create;

    if (!pb_decode(&stream,
                   com_daml_ledger_api_v2_interactive_DeviceMetadata_InputContract_fields,
                   &tx_ctx->tx_parts_ctx.input_contract)) {
        PRINTF("Failed to decode Input contract: %s\n", PB_GET_ERROR(&stream));
        return VALUE_PARSING_ERROR;
    }

    encode_int64(&tx_ctx->hasher, tx_ctx->tx_parts_ctx.input_contract.created_at);
    PRINTF("Contract hash: %.*H\n", 32, node_hash);
    encode_hash(&tx_ctx->hasher, node_hash);

    return PARSING_OK;
}

void release_cb_input_contract(transaction_ctx_t *tx_ctx) {
    pb_release(com_daml_ledger_api_v2_interactive_DeviceMetadata_InputContract_fields,
               &tx_ctx->tx_parts_ctx.input_contract);
}
