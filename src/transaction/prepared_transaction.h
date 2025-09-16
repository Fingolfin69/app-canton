#pragma once

#include <stdint.h>  // uint*_t

typedef struct {
    const char *item_name;
    size_t field_index;
} field_display_t;

void process_prepared_tx_init();
int process_prepared_tx_part(buffer_t *buf);
bool init_transaction_pairs(transaction_ctx_t *tx_info, size_t count);
