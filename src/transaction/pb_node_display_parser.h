#pragma once

#include "utils.h"

MUST_CHECK bool init_transaction_pairs(transaction_ctx_t *tx_info, size_t count);
MUST_CHECK int parse_node_for_display(buffer_t *buf);
