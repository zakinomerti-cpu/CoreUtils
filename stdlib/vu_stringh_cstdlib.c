//
// Created by xinitrix on 12.09.2026.
//

#include "vu_string.h"
#include "string.h"
void*   vu_memcpy(void* dest, const void* src, size_t n) {
    return memcpy(dest, src, n);
}

void*   vu_memset(void* dest, int val, size_t n) {
    return memset(dest, val, n);
}

int     vu_memcmp(const void *s1, const void *s2, size_t n) {
    return memcmp(s1, s2, n);
}
