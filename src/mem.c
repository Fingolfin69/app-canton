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

#define SIZE_MEM_BUFFER (1024 * 3)

static uint8_t mem_buffer[SIZE_MEM_BUFFER] __attribute__((aligned(sizeof(intmax_t))));
static mem_ctx_t mem_ctx = NULL;

#ifdef HAVE_MEMORY_PROFILING
#define MP_LOG_PREFIX "==MP "
#endif

MUST_CHECK bool app_mem_init(void) {
    void *buf = mem_buffer;
    size_t buf_size = sizeof(mem_buffer);

    mem_ctx = mem_init(buf, buf_size);
#ifdef HAVE_MEMORY_PROFILING
    PRINTF(MP_LOG_PREFIX "init;0x%p;%u\n", buf, buf_size);
#endif
    return mem_ctx != NULL;
}

MUST_CHECK void *app_mem_realloc_impl(void *ptr, size_t size, const char *file, int line) {
    void *new_ptr;
    new_ptr = mem_realloc(mem_ctx, ptr, size);

#ifdef HAVE_MEMORY_PROFILING
    if (new_ptr != NULL) {
        // Log the realloc event (ptr might be different now)
        if (ptr != NULL && ptr != new_ptr) {
            PRINTF(MP_LOG_PREFIX "free;0x%p;%s:%u\n", ptr, file, line);
        }
        PRINTF(MP_LOG_PREFIX "alloc;%u;0x%p;%s:%u\n", size, new_ptr, file, line);
    }
#else
    (void) file;
    (void) line;
#endif
    return new_ptr;
}

MUST_CHECK void *app_mem_alloc_impl(size_t size, const char *file, int line) {
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
    if (ptr == NULL) {
        return;
    }
#ifdef HAVE_MEMORY_PROFILING
    PRINTF(MP_LOG_PREFIX "free;0x%p;%s:%u\n", ptr, file, line);
#else
    (void) file;
    (void) line;
#endif

    mem_free(mem_ctx, ptr);
}

void app_mem_stat() {
    mem_stat_t stat = {0};
    mem_stat(mem_ctx, &stat);

    PRINTF("MEM: total %u, free %u, allocated %u, chunks %u, allocated chunks %u\n",
           stat.total_size,
           stat.free_size,
           stat.allocated_size,
           stat.nb_chunks,
           stat.nb_allocated);
}
