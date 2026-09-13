//
// Created by xinitrix on 12.09.2026.
//
#include "heap.h"
#include <stdlib.h>

void* heapAlloc(size_t size) {
    return malloc(size);
}

void* heapRealloc(void* ptr, size_t size) {
    return realloc(ptr, size);
}

void heapFree(void* ptr) {
    free(ptr);
}
