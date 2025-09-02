/*****************************************************************************
 *   Ledger App Boilerplate.
 *   (c) 2020 Ledger SAS.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool
#include <string.h>   // memmove

#include "types.h"

#include "os.h"
#include "cx.h"

void canton_fingerprint(uint8_t purpose, const uint8_t *data, size_t data_len, uint8_t out[32]) {
    cx_sha256_t ctx;
    uint8_t purpose_be[] = {0, 0, 0, purpose};
    CX_ASSERT(cx_sha256_init_no_throw(&ctx));
    CX_ASSERT(cx_hash_update((cx_hash_t *) &ctx, purpose_be, sizeof(purpose_be)));
    if (data_len > 0) {
        CX_ASSERT(cx_hash_update((cx_hash_t *) &ctx, data, data_len));
    }
    CX_ASSERT(cx_hash_final((cx_hash_t *) &ctx, out));
}
