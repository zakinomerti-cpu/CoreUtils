#ifndef memallocate_header
#define memallocate_header
#include "heapAlloc/heap.h"

#ifdef __DEBUG__

void* memallocate_debug
(
    size_t size,
    const char* file,
    size_t line,
    const char* func
);

void* memreallocate_debug
(
	void* ptr,
	size_t size,
	const char* file,
	size_t line,
	const char* func
);
const char* getFile(void* ptr);
size_t getLine(void* ptr);
const char* getFunc(void* ptr);

int memfree_debug(void* ptr);
void memcheck_dump_leaks(void);

#define memallocate(size) memallocate_debug \
( \
	size, \
	__FILE__, \
	__LINE__, \
	__func__ \
)

#define memreallocate(ptr, size) memreallocate_debug \
( \
	ptr, \
	size, \
	__FILE__, \
	__LINE__, \
	__func__ \
)

#define memfree(ptr) do { \
	int res = memfree_debug(ptr); \
	if (res == -2) { \
			/*assert(0);*/ \
		} \
} while(0)


#else /*__RELEASE__*/

#define memallocate(size) malloc(size)
#define memallocate_debug(size, file, line, func) \
    heapAlloc((size))

#define memreallocate_debug(ptr, size, file, line, func) \
    heapRealloc((ptr), (size))

#define memfree(ptr) \
    heapFree((ptr))

#define memcheck_dump_leaks() \
    ((void)0)

#define getFile(ptr) \
    ((void *)0)

#define getLine(ptr) \
    ((size_t)0)

#define getFunc(ptr) \
    ((void *)0)

#endif
#endif