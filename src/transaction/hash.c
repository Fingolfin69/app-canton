#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t
#include <stdbool.h>  // bool
#include <string.h> 
#include "com/daml/ledger/api/v2/interactive/interactive_submission_service.pb.h"
#include "com/daml/ledger/api/v2/value.pb.h"
#include "ledger_assert.h"
#include "mem.h"

#include <pb.h>         // Contains PB_LTYPE macros and basic pb_field_t definitions
#include <pb_common.h>  // Contains pb_field_


// static uint8_t HASHING_SCHEME_VERSION = com_daml_ledger_api_v2_interactive_HashingSchemeVersion_HASHING_SCHEME_VERSION_V2;
static uint8_t NODE_ENCODING_VERSION = 0x01;

uint32_t encode_text_map_entry(const char *entry, uint8_t *out);

// Define encode_fn as a function pointer type
typedef uint32_t (*encode_fn_t)(const char *value, uint8_t *out);

// Writes a big-endian int32 to `out`, returns number of bytes written (4)
static uint32_t encode_int32(int32_t value, uint8_t *out) {
    // Value is already constrained to int32_t range by type
    out[0] = (value >> 24) & 0xFF;
    out[1] = (value >> 16) & 0xFF;
    out[2] = (value >> 8) & 0xFF;
    out[3] = value & 0xFF;
    return 4;
}

static uint32_t encode_int64(int64_t value, uint8_t *out) {
    // Value is already constrained to int64_t range by type
    out[0] = (value >> 56) & 0xFF;
    out[1] = (value >> 48) & 0xFF;
    out[2] = (value >> 40) & 0xFF;
    out[3] = (value >> 32) & 0xFF;
    out[4] = (value >> 24) & 0xFF;
    out[5] = (value >> 16) & 0xFF;
    out[6] = (value >> 8) & 0xFF;
    out[7] = value & 0xFF;
    return 8;
}

static uint32_t encode_bytes(const uint8_t *value, size_t length, uint8_t *out) {
    LEDGER_ASSERT(length <= UINT32_MAX - 4, "Length exceeds maximum allowed size");
    uint32_t len_bytes = encode_int32((int32_t)length, out);
    memcpy(out + len_bytes, value, length);
    return len_bytes + length;
}

static uint32_t encode_string(const char *value, uint8_t *out) {
    if (value == NULL) {
        return 0;  // Null string
    }
    size_t length = strlen(value);
    return encode_bytes((const uint8_t *)value, length, out);
}

static uint32_t encode_bool(bool value, uint8_t *out) {
    out[0] = value ? 1 : 0;
    return 1;  // Boolean is always 1 byte
}

static uint32_t encode_hash(const uint8_t *hash, size_t hash_len, uint8_t *out) {
    if (hash_len != 32) {
        return 0;  // Invalid hash length
    }
    memcpy(out, hash, hash_len);
    return hash_len;  // SHA-256 hash is always 32 bytes
}

// Converts a single hex character to its 4-bit value (0-15), or returns -1 on error
static int8_t hex_char_to_nibble(char c) {
    if ('0' <= c && c <= '9') {
        return c - '0';
    } else if ('a' <= c && c <= 'f') {
        return c - 'a' + 10;
    } else if ('A' <= c && c <= 'F') {
        return c - 'A' + 10;
    } else {
        LEDGER_ASSERT(false, "Invalid hex character");
    }
}

// Parses a hex string into bytes, returns number of bytes written or -1 on error
static uint32_t hex_to_bytes(const char *hex_str, uint8_t *out, size_t out_size) {
    size_t len = strlen(hex_str);
    LEDGER_ASSERT(len <= out_size * 2, "Hex string too long for output buffer");
    size_t byte_len = len / 2;
    LEDGER_ASSERT(byte_len <= out_size, "Output buffer too small for hex string");

    for (size_t i = 0; i < byte_len; i++) {
        int8_t high = hex_char_to_nibble(hex_str[2 * i]);
        int8_t low  = hex_char_to_nibble(hex_str[2 * i + 1]);
        LEDGER_ASSERT(high >= 0 && low >= 0, "Invalid hex character in string");
        out[i] = (high << 4) | low;
    }
    return byte_len;
}

static uint32_t encode_hex_string(const char *hex_str, uint8_t *out) {
    uint8_t bytes[256];
    int byte_len = hex_to_bytes(hex_str, bytes, sizeof(bytes));
    LEDGER_ASSERT(byte_len >= 0, "Invalid hex string");
    return encode_bytes(bytes, byte_len, out);
}

static uint32_t encode_optional(const char *value, uint8_t *out, encode_fn_t encode_fn) {
    if (value != NULL) {
        out[0] = 1;  // Present
        return encode_fn(value, out + 1);
    } else {
        out[0] = 0;  // Not present
        return 1;  // Only the presence byte
    }
}

static bool has_field_by_name(const void *p_struct, const pb_field_t *fields, const char *field_name) {
    // TODO for encode_proto_optional
}


static uint32_t encode_proto_optional(const void *parent_value, const char *field_name, const char *value, encode_fn_t encode_fn, uint8_t *out) {
    // TODO for encode_metadata
}

static uint32_t encode_repeated(const char **values, size_t count, uint8_t *out, encode_fn_t encode_fn) {
    LEDGER_ASSERT(count <= UINT32_MAX / 4, "Count too large for encoding");
    
    uint32_t len_bytes = encode_int32((int32_t)count, out);
    uint32_t total_size = len_bytes;

    for (size_t i = 0; i < count; i++) {
        uint32_t size = encode_fn(values[i], out + total_size);
        LEDGER_ASSERT(size > 0, "Encoding failed for value");
        total_size += size;
    }
    return total_size;
}



static size_t split_string(const char *str, char delimiter, char **parts, size_t max_parts) {
    size_t count = 0;
    const char *start = str;
    const char *end;

    while ((end = strchr(start, delimiter)) != NULL && count < max_parts) {
        size_t len = end - start;
        parts[count] = (char *)app_mem_alloc(len + 1);  // Allocate memory for part
        LEDGER_ASSERT(parts[count] != NULL, "Memory allocation failed");
        strncpy(parts[count], start, len);
        parts[count][len] = '\0';  // Null-terminate the string
        count++;
        start = end + 1;  // Move past the delimiter
    }

    if (*start != '\0' && count < max_parts) {
        parts[count] = (char *)app_mem_alloc(strlen(start) + 1);
        LEDGER_ASSERT(parts[count] != NULL, "Memory allocation failed");
        strcpy(parts[count], start);
        count++;
    }

    return count;
}

static uint32_t encode_identifier(const com_daml_ledger_api_v2_Identifier *identifier, uint8_t *out) {
    LEDGER_ASSERT(identifier != NULL, "NULL identifier");
    LEDGER_ASSERT(out != NULL, "NULL output buffer");

    uint32_t total_size = 0;

    // Encode package_id as a single string
    total_size += encode_string(identifier->package_id, out + total_size);

    // Split module_name by '.' and encode each part as a repeated string
    char *module_parts[256];  // Assuming a maximum of 256 parts
    size_t module_count = split_string(identifier->module_name, '.', module_parts, 256);
    total_size += encode_repeated((const char **)module_parts, module_count, out + total_size, encode_string);

    // Split entity_name by '.' and encode each part as a repeated string
    char *entity_parts[256];  // Assuming a maximum of 256 parts
    size_t entity_count = split_string(identifier->entity_name, '.', entity_parts, 256);
    total_size += encode_repeated((const char **)entity_parts, entity_count, out + total_size, encode_string);

    // Free allocated memory for parts
    for (size_t i = 0; i < module_count; i++) {
        app_mem_free(module_parts[i]);  // Free each allocated part
    }
    for (size_t i = 0; i < entity_count; i++) {
        app_mem_free(entity_parts[i]);  // Free each allocated part
    }

    return total_size;
}

static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

int atoi(const char* str) {
    int res = 0;

    for (int i = 0; str[i] != '\0'; i++) {
        if (!is_digit(str[i])) {
            return 0;
        }
        res = res * 10 + str[i] - '0';
    }

    return res;
}

static uint32_t find_seed(
    const char *node_id,
    const com_daml_ledger_api_v2_interactive_DamlTransaction_NodeSeed *node_seeds,
    size_t num_seeds,
    uint8_t *out
) {
    LEDGER_ASSERT(node_id != NULL, "NULL node_id");
    LEDGER_ASSERT(node_seeds != NULL, "NULL node_seeds");
    LEDGER_ASSERT(out != NULL, "NULL output buffer");

    for (size_t i = 0; i < num_seeds; i++) {
        if (node_seeds[i].node_id == atoi(node_id)) {
            return encode_bytes((const uint8_t *)node_seeds[i].seed.bytes, node_seeds[i].seed.size, out);
        }
    }
    return 0;
}

static uint32_t encode_value(const char *value, uint8_t *out) {
    LEDGER_ASSERT(value != NULL, "NULL value");
    LEDGER_ASSERT(out != NULL, "NULL output buffer");

    const com_daml_ledger_api_v2_Value* value_pb = (const com_daml_ledger_api_v2_Value *)value;  // Cast to the correct type

    switch (value_pb->which_sum) {
        case com_daml_ledger_api_v2_Value_unit_tag:
            return 0;
        case com_daml_ledger_api_v2_Value_bool__tag:
            out[0] = 0x01;
            return encode_bool(value_pb->sum.bool_, out + 1) + 1;  // +1 for the tag byte
        case com_daml_ledger_api_v2_Value_int64_tag:
            out[0] = 0x02;
            return encode_int64(value_pb->sum.int64, out + 1) + 1;  // +1 for the tag byte
        case com_daml_ledger_api_v2_Value_numeric_tag:
            out[0] = 0x03;
            return encode_string(value_pb->sum.numeric, out + 1) + 1;
        case com_daml_ledger_api_v2_Value_timestamp_tag:
            out[0] = 0x04;
            return encode_int64(value_pb->sum.timestamp, out + 1) + 1;
        case com_daml_ledger_api_v2_Value_date_tag:
            out[0] = 0x05;
            return encode_int32(value_pb->sum.date, out + 1) + 1;
        case com_daml_ledger_api_v2_Value_party_tag:
            out[0] = 0x06;
            return encode_string(value_pb->sum.party, out + 1) + 1;
        case com_daml_ledger_api_v2_Value_text_tag:
            out[0] = 0x07;
            return encode_string(value_pb->sum.text, out + 1) + 1;
        case com_daml_ledger_api_v2_Value_contract_id_tag:
            out[0] = 0x08;
            return encode_hex_string(value_pb->sum.contract_id, out + 1) + 1;
        // case com_daml_ledger_api_v2_Value_optional_tag:
        //     out[0] = 0x09;
        //     return encode_proto_optional(&value_pb->sum.optional, "value", value_pb->sum.optional.value, encode_value) + 1;
        case com_daml_ledger_api_v2_Value_list_tag:
            out[0] = 0x0A;
            // Encode the repeated elements
            // Assuming that encode_repeated returns the size of the encoded elements
            return encode_repeated((const char **)value_pb->sum.list.elements, value_pb->sum.list.elements_count, out + 1, encode_value) + 1;
        // case com_daml_ledger_api_v2_Value_text_map_tag:
        //     out[0] = 0x0B;
        //     return encode_repeated((const char **)value_pb->sum.text_map.entries, value_pb->sum.text_map.entries_count, out + 1, encode_text_map_entry) + 1;
        // case com_daml_ledger_api_v2_Value_record_tag:
        //     return encode_proto_optional(&value->sum.record, "record_id", value->sum.record.record_id, encode_identifier) +
        //            encode_repeated((const char **)value->sum.record.fields, value->sum.record.fields_count, out + 1 + encode_proto_optional(&value->sum.record, "record_id", value->sum.record.record_id, encode_identifier), encode_record_field) + 1;
        // case com_daml_ledger_api_v2_Value_variant_tag:
        //     return encode_proto_optional(&value->sum.variant, "variant_id", value->sum.variant.variant_id, encode_identifier) +
        //            encode_string(value->sum.variant.constructor, out + 1 + encode_proto_optional(&value->sum.variant, "variant_id", value->sum.variant.variant_id, encode_identifier)) +
        //            encode_value(value->sum.variant.value, out + 1 + encode_proto_optional(&value->sum.variant, "variant_id", value->sum.variant.variant_id, encode_identifier) + encode_string(value->sum.variant.constructor, out + 1 + encode_proto_optional(&value->sum.variant, "variant_id", value->sum.variant.variant_id, encode_identifier))) + 1;
        // case com_daml_ledger_api_v2_Value_enum_tag:
        //     return encode_proto_optional(&value->sum.enum_, "enum_id", value->sum.enum_.enum_id, encode_identifier) +
        //            encode_string(value->sum.enum_.constructor, out + 1 + encode_proto_optional(&value->sum.enum_, "enum_id", value->sum.enum_.enum_id, encode_identifier)) + 1;
        // case com_daml_ledger_api_v2_Value_gen_map_tag:
        //     return encode_repeated((const char **)value->sum.gen_map.entries, value->sum.gen_map.count, out + 1, encode_gen_map_entry) +
        //            1;  // Assuming gen_map entries are encoded as strings
        default:
            LEDGER_ASSERT(false, "Unsupported value type");
            return 0;  // Should never reach here
    }
}


static uint32_t encode_create_node(
    const com_daml_ledger_api_v2_interactive_transaction_v1_Create *create,
    const char *node_id,
    const com_daml_ledger_api_v2_interactive_DamlTransaction_NodeSeed *node_seeds,
    size_t num_seeds,
    uint8_t *out
) {
    LEDGER_ASSERT(create != NULL, "NULL create node");
    LEDGER_ASSERT(node_id != NULL, "NULL node_id");
    LEDGER_ASSERT(node_seeds != NULL, "NULL node_seeds");
    LEDGER_ASSERT(out != NULL, "NULL output buffer");

    uint32_t total_size = 0;

    // Encode version
    out[total_size++] = NODE_ENCODING_VERSION;

    // Encode LF version
    total_size += encode_string(create->lf_version, out + total_size);

    // Create node tag
    out[total_size++] = 0x00;  // Create node tag

    // Encode seed if present
    uint32_t seed_size = find_seed(node_id, node_seeds, num_seeds, out + total_size);
    if (seed_size > 0) {
        total_size += seed_size;
    } else {
        out[total_size++] = 0;  // No seed present
    }

    // Encode contract_id as hex string
    total_size += encode_hex_string(create->contract_id, out + total_size);

    // Encode package_name
    total_size += encode_string(create->package_name, out + total_size);

    // Encode template_id
    total_size += encode_identifier(&create->template_id, out + total_size);

    // Encode argument if present
    if (create->has_argument) {
        total_size += encode_optional((const char*) &create->argument, out + total_size, encode_value);
    } else {
        out[total_size++] = 0;  // No argument present
    }

    // Encode signatories
    total_size += encode_repeated((const char **)create->signatories, create->signatories_count, out + total_size, encode_string);

    // Encode stakeholders
    total_size += encode_repeated((const char **)create->stakeholders, create->stakeholders_count, out + total_size, encode_string);

    return total_size;
}
