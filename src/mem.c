/**
 * Dynamic allocator that uses a fixed-length buffer that is hopefully big enough
 *
 * The two functions alloc & dealloc use the buffer as a simple stack.
 * Especially useful when an unpredictable amount of data will be received and have to be stored
 * during the transaction but discarded right after.
 */

#include <stdint.h>
#include <string.h>
#include "mem.h"
#include "mem_alloc.h"
#include "os_print.h"
#include "ledger_assert.h"

#define SIZE_MEM_BUFFER (1024 * 16)

static uint8_t mem_buffer[SIZE_MEM_BUFFER] __attribute__((aligned(sizeof(intmax_t))));
static mem_ctx_t mem_ctx = NULL;

typedef struct {
    void *ptr;
    size_t size;
} last_allocated_t;

static last_allocated_t last_allocated[10];
static int last_allocated_index = 0;

#ifdef HAVE_MEMORY_PROFILING
#define MP_LOG_PREFIX "==MP "
#endif

bool app_mem_init(void) {
    void *buf = mem_buffer;
    size_t buf_size = sizeof(mem_buffer);

    mem_ctx = mem_init(buf, buf_size);
#ifdef HAVE_MEMORY_PROFILING
    PRINTF(MP_LOG_PREFIX "init;0x%p;%u\n", buf, buf_size);
#endif
    return mem_ctx != NULL;
}

void *app_mem_realloc_impl(void *ptr, size_t size, const char *file, int line) {
    void *new_ptr;
    int index;
    bool found;

    if (ptr != NULL) {
        found = false;
        index = last_allocated_index;

        // Look up the last allocated pointer and size
        for (size_t cnt = 0; cnt < sizeof(last_allocated) / sizeof(last_allocated[0]); ++cnt) {
            if (last_allocated[index].ptr == ptr) {
                found = true;
                break;
            }
            index--;
            if (index < 0) {
                index = sizeof(last_allocated) / sizeof(last_allocated[0]) - 1;
            }
        }
        LEDGER_ASSERT(found, "Pointer not found in last allocated list");

        // Reallocate memory
        new_ptr = mem_alloc(mem_ctx, size);
        memcpy(new_ptr, ptr, last_allocated[index].size);
        mem_free(mem_ctx, ptr);

        // Clear the cached pointer
        last_allocated[index].ptr = NULL;
        last_allocated[index].size = 0;
    } else {
        new_ptr = mem_alloc(mem_ctx, size);
    }

    // Cache the last allocated pointer and size
    if (new_ptr != NULL) {
        last_allocated[last_allocated_index].ptr = new_ptr;
        last_allocated[last_allocated_index].size = size;
        last_allocated_index++;
        last_allocated_index %= sizeof(last_allocated) / sizeof(last_allocated[0]);
    }

#ifdef HAVE_MEMORY_PROFILING
    PRINTF(MP_LOG_PREFIX "realloc;%u;0x%p;0x%p;%s:%u\n", size, ptr, new_ptr, file, line);
#else
    (void) file;
    (void) line;
#endif
    return new_ptr;
}

void *app_mem_alloc_impl(size_t size, const char *file, int line) {
    void *ptr;
    ptr = mem_alloc(mem_ctx, size);
#ifdef HAVE_MEMORY_PROFILING
    PRINTF(MP_LOG_PREFIX "alloc;%u;0x%p;%s:%u\n", size, ptr, file, line);
#else
    (void) file;
    (void) line;
#endif
    return ptr;
}

void app_mem_free_impl(void *ptr, const char *file, int line) {
#ifdef HAVE_MEMORY_PROFILING
    PRINTF(MP_LOG_PREFIX "free;0x%p;%s:%u\n", ptr, file, line);
#else
    (void) file;
    (void) line;
#endif
    if (ptr == NULL) {
        return;
    }

    mem_free(mem_ctx, ptr);
}
