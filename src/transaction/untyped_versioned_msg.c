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
#include <stddef.h>   // size_t
#include <stdbool.h>  // bool
#include <string.h>   // memmove
#include <stdlib.h>   // qsort

#include "mem.h" 
#include "os.h"
#include "cx.h"
#include "ledger_assert.h"
#include "globals.h"

#include "party_id.h"
#include "utils.h"
#include "buffer.h"
#include "tx_types.h"
#include "sw.h"
#include "bytewriter.h"

#define HASH_LEN 34
#define HEX_LEN (HASH_LEN * 2 + 1)
#define MAX_HASHES 3  // Adjust as needed
#define PURPOSE_TOPOLOGY_TRANSACTION_SIGNATURE ((uint8_t) 11)
#define PURPOSE_MULTI_TOPOLOGY_TRANSACTION_SIGNATURE ((uint8_t) 55)

static uint8_t (*tx_hashes)[HASH_LEN] = NULL;
static size_t hash_count = 0;

static void init_hash_storage(void) {
    if (tx_hashes) {
        app_mem_free(tx_hashes);
    }
    tx_hashes = app_mem_alloc(MAX_HASHES * sizeof(uint8_t[HASH_LEN]));
    hash_count = 0;
}

static void cleanup_hash_storage() {
    if (tx_hashes) {
        app_mem_free(tx_hashes);
        tx_hashes = NULL;
        hash_count = 0;
    }
}

static void add_hash(const uint8_t hash[HASH_LEN]) {
    LEDGER_ASSERT(hash != NULL, "NULL hash");
    LEDGER_ASSERT(tx_hashes != NULL, "Hash storage not initialized");
    LEDGER_ASSERT(hash_count < MAX_HASHES, "Hash storage full");
    memcpy(tx_hashes[hash_count], hash, HASH_LEN);
    hash_count++;
}

static int compare_hashes_hex(const void *a, const void *b) {
    const uint8_t *hash_a = (const uint8_t *)a;
    const uint8_t *hash_b = (const uint8_t *)b;

    char hex_a[HEX_LEN];
    char hex_b[HEX_LEN];

#pragma GCC diagnostic ignored "-Wformat"
    snprintf(hex_a, sizeof(hex_a), "%.*h", HASH_LEN, hash_a);
    snprintf(hex_b, sizeof(hex_b), "%.*h", HASH_LEN, hash_b);   
    return strcmp(hex_a, hex_b);
}

void process_untyped_versioned_msg_tx_init(void){
    init_hash_storage();
}

int process_untyped_versioned_msg_tx(buffer_t *buf) {
    UNUSED(buf);
    uint8_t h[HASH_LEN] = {0};

    canton_hash(PURPOSE_TOPOLOGY_TRANSACTION_SIGNATURE, buf->ptr, buf->size, h);
    add_hash(h);

    if(G_context.state == STATE_PARSED) {
        // Allocate storage for a concatenated string of all hashes + their lengths
        size_t len = hash_count * HASH_LEN + hash_count * 4 + 4; // Each hash prefixed by its length (4 bytes) + 4 bytes for count
        uint8_t* concat = app_mem_alloc(len);
        ByteWriter bw;
        bw_init(&bw, concat,len);
        bw_put_u32_be(&bw, hash_count); // Prefix with number of hashes
        // Sort hashes
        qsort(tx_hashes, hash_count, HASH_LEN, compare_hashes_hex);
        for (size_t i = 0; i < hash_count; i++) {
            // Concatenate each hash, prefixed them with their length (always 34)
            bw_put_u32_be(&bw, HASH_LEN);
            bw_put(&bw, tx_hashes[i], HASH_LEN);
        }
        // Compute final hash
        canton_hash(PURPOSE_MULTI_TOPOLOGY_TRANSACTION_SIGNATURE, concat, len, G_context.tx_info.m_hash);
        PRINTF("Final untyped versioned message hash: %.*H\n", HASH_LEN, G_context.tx_info.m_hash);
        app_mem_free(concat);
        cleanup_hash_storage();
    }
    return 0;
}
