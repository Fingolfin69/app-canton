#pragma once

#include <stdint.h>   // uint*_t
#include <stddef.h>   // size_t
#include <stdbool.h>  // bool
#include <string.h>   // memmove

#include "os.h"
#include "cx.h"
#include "ledger_assert.h"

#include "party_id.h"
#include "utils.h"

#include "tx_types.h"

void process_untyped_versioned_msg_tx_init(void);
int process_untyped_versioned_msg_tx(buffer_t *buf);
