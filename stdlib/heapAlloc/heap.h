#ifndef UNTITLED_HEAPALLOC_H
#define UNTITLED_HEAPALLOC_H
#include <stddef.h>

void*   heapAlloc(size_t size);
void*   heapRealloc(void* ptr, size_t size);
void    heapFree(void* ptr);

#endif //UNTITLED_HEAPALLOC_H
