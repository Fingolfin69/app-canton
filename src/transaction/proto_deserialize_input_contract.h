#pragma once

#include "buffer.h"

#include "types.h"

/**
 * Deserialize Metadata.InputContract protobuf message in structure.
 *
 * @param[in, out] buf
 *   Pointer to buffer with serialized transaction.
 * @param[out]     tx_ctx
 *   Pointer to transaction context structure.
 *
 * @return PARSING_OK if success, error status otherwise.
 *
 */
parser_status_e proto_deserialize_cb_input_contract(buffer_t *buf, transaction_ctx_t *tx_ctx);

/**
 * Release dynamically allocated memory for Metadata.InputContract protobuf message.
 *
 * @param[in, out]     tx_ctx
 *   Pointer to transaction context structure.
 *
 */
void release_cb_input_contract(transaction_ctx_t *tx_ctx);
