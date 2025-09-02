#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

void mem_string_pool_init(void);
void* mem_string_intern(const char* str, size_t len);
bool mem_string_pool_remove(const char* str);
void mem_string_pool_stat(void);
