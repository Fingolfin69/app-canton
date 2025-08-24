#pragma once

#include "buffer.h"

#include "types.h"

/**
 * Deserialize raw transaction in structure.
 *
 * @param[in, out] buf
 *   Pointer to buffer with serialized transaction.
 * @param[out]     tx_ctx
 *   Pointer to transaction context structure.
 *
 * @return PARSING_OK if success, error status otherwise.
 *
 */
parser_status_e deserialize_transaction_part_daml_tx(buffer_t *buf, transaction_ctx_t *tx_ctx);

/**
 * Deserialize raw transaction in structure.
 *
 * @param[in, out] buf
 *   Pointer to buffer with serialized transaction.
 * @param[out]     tx_ctx
 *   Pointer to transaction context structure.
 *
 * @return PARSING_OK if success, error status otherwise.
 *
 */
parser_status_e deserialize_transaction_part_node(buffer_t *buf, transaction_ctx_t *tx_ctx);

/**
 * Deserialize raw transaction in structure.
 *
 * @param[in, out] buf
 *   Pointer to buffer with serialized transaction.
 * @param[out]     tx_ctx
 *   Pointer to transaction context structure.
 *
 * @return PARSING_OK if success, error status otherwise.
 *
 */
parser_status_e deserialize_transaction_part_metadata(buffer_t *buf, transaction_ctx_t *tx_ctx);

/**
 * Deserialize raw transaction in structure.
 *
 * @param[in, out] buf
 *   Pointer to buffer with serialized transaction.
 * @param[out]     tx_ctx
 *   Pointer to transaction context structure.
 *
 * @return PARSING_OK if success, error status otherwise.
 *
 */
parser_status_e deserialize_transaction_part_input_contract(buffer_t *buf,
                                                            transaction_ctx_t *tx_ctx);

/**
 * Deserialize raw transaction in structure.
 *
 * @param[in, out] buf
 *   Pointer to buffer with serialized transaction.
 * @param[out]     tx_ctx
 *   Pointer to transaction context structure.
 *
 * @return PARSING_OK if success, error status otherwise.
 *
 */
parser_status_e deserialize_transaction_part_prepared_submission_details(buffer_t *buf,
                                                                         transaction_ctx_t *tx_ctx);
