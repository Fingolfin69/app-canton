#pragma once

#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool

#include "types.h"


/**
 * Compute SHA-256 hash with a purpose byte as domain separator.
 *
 * @param[in]  purpose   A single-byte value indicating the hash purpose
 * @param[in]  data      Pointer to input buffer
 * @param[in]  data_len  Length of input buffer
 * @param[out] out       32-byte output buffer for the digest
 */
void sha256_with_purpose(uint8_t purpose, const uint8_t *data, size_t data_len, uint8_t out[32]);