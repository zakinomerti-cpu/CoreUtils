#include "memallocate.h"

#ifdef __DEBUG__
#define MAGICNUMBERMEMALLOCATE 0xABCDABCDABCD1111

static HANDLE procHeap = NULL;

typedef struct AllocationInfo {
	uint64_t	magic;
	size_t 		requested_size;

	const char* file;
	size_t		line;
	const char* func;

	struct AllocationInfo* prev;
	struct AllocationInfo* next;

	uint8_t padding[sizeof(void*) == 4 ? 0 : 8];

} AllocationInfo;

static AllocationInfo* last_alloc_info = NULL;

void* memallocate_debug
(
	size_t size,
	const char* file,
	size_t line,
	const char* func
)
{
	if (!procHeap) procHeap = GetProcessHeap();
	size_t allocInfoHeader_size = sizeof(AllocationInfo);
	void* memory = HeapAlloc(procHeap, 0, size+allocInfoHeader_size);
	if(!memory) return NULL;

	AllocationInfo* info = (AllocationInfo*)memory;
	info->magic = MAGICNUMBERMEMALLOCATE;
	info->requested_size = size;

	info->file = file;
	info->line = line;
	info->func = func;

	info->prev = last_alloc_info;
	info->next = NULL;

	if(last_alloc_info) {
		last_alloc_info->next = info;
	}

	last_alloc_info = info;

	uint8_t* memory_pointer = (uint8_t*)memory;
	memory_pointer+=allocInfoHeader_size;
	return (void*)memory_pointer;
}

void* memreallocate_debug
(
	void* ptr,
	size_t size,
	const char* file,
	size_t line,
	const char* func
)
{
	AllocationInfo* info;
	if (!procHeap) procHeap = GetProcessHeap();
	size_t allocInfoHeader_size = sizeof(AllocationInfo);
	if(ptr) {
		info = (AllocationInfo*)((uint8_t*)ptr-allocInfoHeader_size);
		if (info->magic != MAGICNUMBERMEMALLOCATE) return NULL;
	}
	else {
		return memallocate_debug(size, file, line, func);
	}

	AllocationInfo* tmp = HeapReAlloc(procHeap, 0, info, size+allocInfoHeader_size);
	if (!tmp) return NULL;

	if (tmp->prev) {
		tmp->prev->next = tmp;
	}

	if (tmp->next) {
		tmp->next->prev = tmp;
	}

	if (last_alloc_info == info) {
		last_alloc_info = tmp;
	}

	tmp->requested_size = size;
	return (uint8_t *)tmp + allocInfoHeader_size;
}

int memfree_debug(void* ptr) {
	if(!ptr) return -1;
	if (!procHeap) procHeap = GetProcessHeap();
	size_t allocInfoHeader_size = sizeof(AllocationInfo);
	uint8_t* mem_ptr = (uint8_t*)ptr;

	mem_ptr -= allocInfoHeader_size;
	AllocationInfo* info = (AllocationInfo*)mem_ptr;


	if(info->magic != MAGICNUMBERMEMALLOCATE)
		return -2;

	if(info->prev)
		info->prev->next = info->next;
	if(info->next)
		info->next->prev = info->prev;
	else
		last_alloc_info = info->prev;

	HeapFree(procHeap, 0, info);
	return 0;
}

void memcheck_dump_leaks(void) {
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	char outstr1[] = "==================================================\n";
	char outstr2[] = "[MEMCHECK] No memory leaks detected. Great job!\n";
	char outstr3[] = "[MEMCHECK] WARNING! Memory leaks detected:\n";


	//WriteConsoleA(h, "[CRITICAL ERROR] Попытка ", 5, NULL, NULL);
	if (!last_alloc_info) {
		WriteConsoleA(h, outstr1, sizeof(outstr1)-1, NULL, NULL);
		WriteConsoleA(h, outstr2, sizeof(outstr1)-1, NULL, NULL);
		WriteConsoleA(h, outstr1, sizeof(outstr1)-1, NULL, NULL);
		return;
	}

	WriteConsoleA(h, outstr1, sizeof(outstr1)-1, NULL, NULL);
	WriteConsoleA(h, outstr3, sizeof(outstr1)-1, NULL, NULL);
	WriteConsoleA(h, outstr1, sizeof(outstr1)-1, NULL, NULL);

	AllocationInfo* current = last_alloc_info;
	size_t total_leaked_bytes = 0;
	size_t total_leaked_blocks = 0;

	while (current) {

		char currentPointer[16];
		char currentSize[16];
		char currentFile[128];
		char currentLine[128];
		char currentFunc[128];
		wsprintfA(currentPointer, "0x%Ix\r\0", (uintptr_t)current);
		wsprintfA(currentSize, "0x%Ix\r\0", current->requested_size);
		wsprintfA(currentFile, "%s\0", current->file);
		wsprintfA(currentLine, "%s\0", current->line);
		wsprintfA(currentFunc, "%s\0", current->func);

		char currentOut1[128];
		wsprintfA(currentOut1, " -> Block: %s | Size: %s bytes\n\0", currentPointer, currentSize);

		char currentOut2[256];
		wsprintfA(currentOut2, "	Allocated in: %s() -> %s:%s\n\0", currentFunc, currentFile, currentLine);

		WriteConsoleA(h, currentOut1, sizeof(currentOut1)-1, NULL, NULL);
		WriteConsoleA(h, currentOut2, sizeof(currentOut2)-1, NULL, NULL);


		total_leaked_bytes += current->requested_size;
		total_leaked_blocks++;

		current = current->prev;
	}

	char outstr4[128];
	wsprintfA(outstr4, "[MEMCHECK] Summary: Found %d leak(s), total lost: %d bytes.\n", total_leaked_blocks, total_leaked_bytes);
	WriteConsoleA(h, outstr4, sizeof(outstr4)-1, NULL, NULL);
	WriteConsoleA(h, outstr1, sizeof(outstr1)-1, NULL, NULL);
}

const char* getFile(void* ptr) {
	if(!ptr) return NULL;
	size_t allocInfoHeader_size = sizeof(AllocationInfo);
	uint8_t* mem_ptr = (uint8_t*)ptr;
	mem_ptr -= allocInfoHeader_size;
	AllocationInfo* info = (AllocationInfo*)mem_ptr;
	if (info->magic != MAGICNUMBERMEMALLOCATE) return NULL;
	return info->file;
}

size_t getLine(void* ptr) {
	size_t allocInfoHeader_size = sizeof(AllocationInfo);
	uint8_t* mem_ptr = (uint8_t*)ptr;
	mem_ptr -= allocInfoHeader_size;
	AllocationInfo* info = (AllocationInfo*)mem_ptr;
	if (info->magic != MAGICNUMBERMEMALLOCATE) return 0;
	return info->line;
}

const char* getFunc(void* ptr) {
	size_t allocInfoHeader_size = sizeof(AllocationInfo);
	uint8_t* mem_ptr = (uint8_t*)ptr;
	mem_ptr -= allocInfoHeader_size;
	AllocationInfo* info = (AllocationInfo*)mem_ptr;
	if (info->magic != MAGICNUMBERMEMALLOCATE) return NULL;
	return info->func;
}

#else


#endif