#include "mem_string_pool.h"

#include <string.h>

#include "os_print.h"
#include "mem.h"

#define STRING_POOL_SIZE 128
#define MIN_STR_LEN      0

inline static void* offset_to_ptr(uint16_t offst) {
    if (offst == (uint16_t) -1) {
        return NULL;
    }
    return (void*) ((uintptr_t) app_mem_base() + (uintptr_t) offst);
}

inline static uint16_t ptr_to_offset(const void* ptr) {
    if (ptr == NULL) {
        return -1;
    }

    return (uint16_t) ((uintptr_t) ptr - (uintptr_t) app_mem_base());
}

typedef struct {
    uint16_t str_offset;
    uint16_t ref_cnt;
} string_pool_entry_t;

static string_pool_entry_t string_pool[STRING_POOL_SIZE] = {0};
static size_t string_pool_size = 0;

void mem_string_pool_init() {
    for (size_t i = 0; i < STRING_POOL_SIZE; ++i) {
        string_pool[i].str_offset = ptr_to_offset(NULL);
        string_pool[i].ref_cnt = 0;
    }
    string_pool_size = 0;
}

void* mem_string_intern(const char* str, size_t len) {
    if (len >= MIN_STR_LEN) {
        // Check if already interned
        for (size_t i = 0; i < STRING_POOL_SIZE; ++i) {
            const char* interned_str = offset_to_ptr(string_pool[i].str_offset);
            if (interned_str != NULL && strlen(interned_str) == len &&
                strncmp(interned_str, str, len) == 0) {
                string_pool[i].ref_cnt++;
                return (void*) interned_str;
            }
        }
    }

    // Allocate new string
    char* new_str = (char*) app_mem_alloc(len + 1);
    if (new_str == NULL) {
        return NULL;
    }

    memcpy(new_str, str, len);
    new_str[len] = '\0';

    // Store in intern buffer if long enough
    if (len >= MIN_STR_LEN && string_pool_size < STRING_POOL_SIZE) {
        for (size_t i = 0; i < STRING_POOL_SIZE; ++i) {
            if (string_pool[i].str_offset == (uint16_t) -1) {
                string_pool[i].str_offset = ptr_to_offset(new_str);
                string_pool[i].ref_cnt = 1;
                string_pool_size++;
                break;
            }
        }
    }

    return new_str;
}

// On removal of a string, decrease its ref count
// Returns true if the string is not receferenced anymore and can be freed otherwise false
bool mem_string_pool_remove(const char* str) {
    for (size_t i = 0; i < STRING_POOL_SIZE; ++i) {
        const char* interned_str = offset_to_ptr(string_pool[i].str_offset);
        if (interned_str != NULL && strcmp(interned_str, str) == 0) {
            // Found in string pool
            if (string_pool[i].ref_cnt > 0) {
                string_pool[i].ref_cnt--;
                if (string_pool[i].ref_cnt == 0) {
                    // Free the string
                    string_pool[i].str_offset = ptr_to_offset(NULL);
                    string_pool_size--;
                    // String reference count is zero, it can be freed
                    return true;
                } else {
                    // String still referenced
                    return false;
                }
            }
        }
    }

    // String not found, it can be freed
    return true;
}

void mem_string_pool_stat() {
    PRINTF("MEM Interned strings (%u/%u):\n", (unsigned) string_pool_size, STRING_POOL_SIZE);

    int size8 = 0;
    int cnt8 = 0;
    int size16 = 0;
    int cnt16 = 0;
    int size24 = 0;
    int cnt24 = 0;
    int size32 = 0;
    int cnt32 = 0;
    int size48 = 0;
    int cnt48 = 0;
    int size64 = 0;
    int cnt64 = 0;
    int size_max = 0;
    int cnt_max = 0;

    for (size_t i = 0; i < STRING_POOL_SIZE; ++i) {
        const char* str = offset_to_ptr(string_pool[i].str_offset);
        if (str != NULL && string_pool[i].ref_cnt > 1) {
            size_t cnt = string_pool[i].ref_cnt;
            size_t slen = strlen(str);
            if (slen <= 8) {
                cnt8 += cnt;
                size8 += slen * cnt;
            } else if (slen <= 16) {
                cnt16 += cnt;
                size16 += slen * cnt;
            } else if (slen <= 24) {
                cnt24 += cnt;
                size24 += slen * cnt;
            } else if (slen <= 32) {
                cnt32 += cnt;
                size32 += slen * cnt;
            } else if (slen <= 48) {
                cnt48 += cnt;
                size48 += slen * cnt;
            } else if (slen <= 64) {
                cnt64 += cnt;
                size64 += slen * cnt;
            } else {
                cnt_max += cnt;
                size_max += slen * cnt;
            }
        }
    }

    PRINTF("MEM  String pool saved space distribution:\n");
    PRINTF("MEM    <=  8:   %u, cnt: %u\n", size8, cnt8);
    PRINTF("MEM    <= 16:  %u, cnt: %u\n", size16, cnt16);
    PRINTF("MEM    <= 24:  %u, cnt: %u\n", size24, cnt24);
    PRINTF("MEM    <= 32:  %u, cnt: %u\n", size32, cnt32);
    PRINTF("MEM    <= 48:  %u, cnt: %u\n", size48, cnt48);
    PRINTF("MEM    <= 64:  %u, cnt: %u\n", size64, cnt64);
    PRINTF("MEM    >  64: %u, cnt: %u\n", size_max, cnt_max);

    (void) size8;
    (void) cnt8;
    (void) size16;
    (void) cnt16;
    (void) size24;
    (void) cnt24;
    (void) size32;
    (void) cnt32;
    (void) size48;
    (void) cnt48;
    (void) size64;
    (void) cnt64;
    (void) size_max;
    (void) cnt_max;
}
