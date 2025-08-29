#pragma once

#include "buffer.h"

#include "types.h"

/**
 * Deserialize DamlTransaction protobuf message in structure.
 *
 * @param[in, out] buf
 *   Pointer to buffer with serialized transaction.
 * @param[out]     tx_ctx
 *   Pointer to transaction context structure.
 *
 * @return PARSING_OK if success, error status otherwise.
 *
 */
parser_status_e proto_deserialize_daml_tx(buffer_t *buf, transaction_ctx_t *tx_ctx);

/**
 * Deserialize DamlTransaction.Node protobuf message in structure.
 *
 * @param[in, out] buf
 *   Pointer to buffer with serialized transaction.
 * @param[out]     tx_ctx
 *   Pointer to transaction context structure.
 *
 * @return PARSING_OK if success, error status otherwise.
 *
 */
parser_status_e proto_deserialize_node(buffer_t *buf, transaction_ctx_t *tx_ctx);

/**
 * Deserialize Metadata protobuf message in structure.
 *
 * @param[in, out] buf
 *   Pointer to buffer with serialized transaction.
 * @param[out]     tx_ctx
 *   Pointer to transaction context structure.
 *
 * @return PARSING_OK if success, error status otherwise.
 *
 */
parser_status_e proto_deserialize_metadata(buffer_t *buf, transaction_ctx_t *tx_ctx);

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
parser_status_e proto_deserialize_input_contract(buffer_t *buf, transaction_ctx_t *tx_ctx);

/**
 * Deserialize PreparedSubmissionResponse protobuf message in structure.
 *
 * @param[in, out] buf
 *   Pointer to buffer with serialized transaction.
 * @param[out]     tx_ctx
 *   Pointer to transaction context structure.
 *
 * @return PARSING_OK if success, error status otherwise.
 *
 */
parser_status_e proto_deserialize_prepared_submission_details(buffer_t *buf,
                                                              transaction_ctx_t *tx_ctx);
