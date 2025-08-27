#pragma once

#include "buffer.h"

#include "types.h"

// Include for uint32

/**
 * Deserialize raw transaction in structure.
 *
 * @param[in, out] buf
 *   Pointer to buffer with serialized transaction.
 * @param[out]     tx
 *   Pointer to transaction structure.
 *
 * @return PARSING_OK if success, error status otherwise.
 *
 */
parser_status_e proto_deserialize(buffer_t *buf, signing_type_e type, transaction_t *tx);
