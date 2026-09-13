#ifndef pVoidArray_header
#define pVoidArray_header

#include <stdint.h>
#include <stddef.h>

typedef enum {
	PV_OUTCODE_OK,
	PV_OUTCODE_ALLOC_ERROR,
	PV_OUTCODE_ARG_NULL,
	PV_OUTCODE_EMPTY_ARR,
	PV_OUTCODE_INVALID_ARR,
	PV_OUTCODE_INVALID_INPUT_INDEX,
	PV_OUTCODE_UNKNOWN_ERROR
} PV_OUTCODE;
typedef int8_t (*pVoidArrayEachFn)(void *value, size_t index, void *args);

#define PV_DECLARE_INTERFACE(interfaceName, owner, type)	\
	typedef struct owner owner; \
	typedef struct interfaceName { \
		int8_t (*push)(owner* self, type* in); \
		int8_t (*pop)(owner* self, type** out); \
		int8_t (*get)(const owner*, type** out, size_t index); \
		int8_t (*set)(owner*, type*, size_t index); \
		int8_t (*last)(const owner* self, type** out); \
		int8_t (*size)(const owner* self, size_t* out); \
		int8_t (*capacity)(const owner* self, size_t* out); \
		int8_t (*reserve)(owner* self, size_t newCapacity); \
		int8_t (*remove)(owner*, type** out, size_t index); \
		int8_t (*clear)(owner*); \
		int8_t (*foreach)(owner* array, pVoidArrayEachFn c, void* args); \
	} interfaceName;

#define PV_push(owner, pVoidArr, type) \
	static int8_t (owner##_push)(owner* self, type* data) { \
		if(!self) return PV_OUTCODE_ARG_NULL; \
		if(!self->pVoidArr) return PV_OUTCODE_INVALID_ARR; \
		return self->pVoidArr->ops->push(self->pVoidArr, data); \
	}


#define PV_pop(owner, pVoidArr, type) \
	static int8_t (owner##_pop)(owner* self, type** out) { \
		if(!self || !out) return PV_OUTCODE_ARG_NULL; \
		if(!self->pVoidArr) return PV_OUTCODE_INVALID_ARR; \
		void* result = NULL; \
		int8_t outcode = self->pVoidArr->ops->pop(self->pVoidArr, &result); \
		if(outcode != PV_OUTCODE_OK) return outcode; \
		*out = (type*)result; \
		return outcode; \
 	}

#define PV_get(owner, pVoidArr, type) \
	static int8_t (owner##_get)(const owner* self, type** out, size_t index) { \
		if(!self || !out) return PV_OUTCODE_ARG_NULL; \
		if(!self->pVoidArr) return PV_OUTCODE_INVALID_ARR; \
		void* result = NULL; \
		int8_t outcode = self->pVoidArr->ops->get(self->pVoidArr, &result, index); \
		if(outcode != PV_OUTCODE_OK) return outcode; \
		*out = (type*)result; \
		return outcode; \
	}
#define PV_set(owner, pVoidArr, type) \
	static int8_t (owner##_set)(owner* self, type* data, size_t index) { \
		if(!self) return PV_OUTCODE_ARG_NULL; \
		if(!self->pVoidArr) return PV_OUTCODE_INVALID_ARR; \
		return self->pVoidArr->ops->set(self->pVoidArr, data, index); \
	}
#define PV_last(owner, pVoidArr, type) \
	static int8_t (owner##_last)(const owner* self, type** out) { \
		if(!self || !out) return PV_OUTCODE_ARG_NULL; \
		if(!self->pVoidArr) return PV_OUTCODE_INVALID_ARR; \
		void* result = NULL; \
		int8_t outcode = self->pVoidArr->ops->last(self->pVoidArr, &result); \
		if(outcode != PV_OUTCODE_OK) return outcode; \
		*out = (type*)result; \
		return outcode; \
	}
#define PV_size(owner, pVoidArr) \
	static int8_t (owner##_size)(const owner* self, size_t* out) { \
		if(!self || !out) return PV_OUTCODE_ARG_NULL; \
		if(!self->pVoidArr) return PV_OUTCODE_INVALID_ARR; \
		size_t result = 0; \
		int8_t outcode = self->pVoidArr->ops->size(self->pVoidArr, &result); \
		if(outcode != PV_OUTCODE_OK) return outcode; \
		*out = result; \
		return outcode; \
	}
#define PV_capacity(owner, pVoidArr) \
	static int8_t (owner##_capacity)(const owner* self, size_t* out) { \
		if(!self || !out) return PV_OUTCODE_ARG_NULL; \
		if(!self->pVoidArr) return PV_OUTCODE_INVALID_ARR; \
		size_t result = 0; \
		int8_t outcode = self->pVoidArr->ops->capacity(self->pVoidArr, &result); \
		if(outcode != PV_OUTCODE_OK) return outcode; \
		*out = result; \
		return outcode; \
	}
#define PV_reserve(owner, pVoidArr) \
	static int8_t (owner##_reserve)(owner* self, size_t capacity) { \
		if(!self) return PV_OUTCODE_ARG_NULL; \
		if(!self->pVoidArr) return PV_OUTCODE_INVALID_ARR; \
		return self->pVoidArr->ops->reserve(self->pVoidArr, capacity); \
}

#define PV_remove(owner, pVoidArr, type) \
	static int8_t (owner##_remove)(owner* self, type** out, size_t index) { \
		if(!self || !out) return PV_OUTCODE_ARG_NULL; \
		if(!self->pVoidArr) return PV_OUTCODE_INVALID_ARR; \
		void* result = NULL; \
		int8_t outcode = self->pVoidArr->ops->remove(self->pVoidArr, &result, index); \
		if(outcode != PV_OUTCODE_OK) return outcode; \
		*out = (type*)result; \
		return outcode; \
	}
#define PV_clear(owner, pVoidArr) \
	static int8_t (owner##_clear)(owner* self) { \
		if(!self) return PV_OUTCODE_ARG_NULL; \
		if(!self->pVoidArr) return PV_OUTCODE_INVALID_ARR; \
		return self->pVoidArr->ops->clear(self->pVoidArr); \
	}
#define PV_foreach(owner, pVoidArr) \
	static int8_t (owner##_foreach)(owner* array, pVoidArrayEachFn c, void* args) { \
		if(!array) return PV_OUTCODE_ARG_NULL; \
		if(!array->pVoidArr) return PV_OUTCODE_INVALID_ARR; \
		return array->pVoidArr->ops->foreach(array->pVoidArr, c, args); \
	}

#define PV_REALIZE_INTERFACE(owner, pVoidArr, type) \
	PV_push(owner, pVoidArr, type) \
	PV_pop(owner, pVoidArr, type) \
	PV_get(owner, pVoidArr, type) \
	PV_set(owner, pVoidArr, type) \
	PV_last(owner, pVoidArr, type) \
	PV_size(owner, pVoidArr) \
	PV_capacity(owner, pVoidArr) \
	PV_reserve(owner, pVoidArr) \
	PV_remove(owner, pVoidArr, type) \
	PV_clear(owner, pVoidArr) \
	PV_foreach(owner, pVoidArr) \

#define PV_DECLARE_CONTAINER(name, funcNameThatUsedToCreateYourNewArray, interfaceName, pVoidArr) \
	int8_t funcNameThatUsedToCreateYourNewArray( \
		name** array, \
		const char* file, \
		size_t line, \
		const char* func \
	); \
	struct name { \
		pVoidArray* pVoidArr; \
		const interfaceName* ops; \
	};

#define PV_REALIZE_CONTAINER(name, interfaceName, pVoidArr, funcNameThatUsedToCreateYourNewArray) \
	static const interfaceName name##_ops = { \
		name##_push, name##_pop, name##_get, \
		name##_set, name##_last, name##_size, \
		name##_capacity, name##_reserve, \
		name##_remove, name##_clear, \
		name##_foreach, \
	}; \
	int8_t funcNameThatUsedToCreateYourNewArray( \
		name** array, \
		const char* file, \
		size_t line, \
		const char* func \
	) \
	{ \
	if(!array) return PV_OUTCODE_ARG_NULL; \
	(*array) = (name*)memallocate_debug( \
		sizeof(name), file, line, func); \
	if (!*array) { \
		return PV_OUTCODE_ALLOC_ERROR; \
	} \
	int8_t outcode = pVoidArray_new_hidden(&(*array)->pVoidArr, file, line, func); \
	if(outcode != PV_OUTCODE_OK) { \
		memfree(*array); \
		*array = NULL; \
		return outcode; \
	} \
	(*array)->ops = &name##_ops; \
	return PV_OUTCODE_OK; \
}

#define PV_NEW(funcNameThatUsedToCreateYourNewArray, array) \
	funcNameThatUsedToCreateYourNewArray(array, __FILE__, __LINE__, __func__)

PV_DECLARE_INTERFACE(pVoidArrayInterface, pVoidArray, void)

struct pVoidArray {
	size_t size;
	size_t capacity;
	void** array;
	const pVoidArrayInterface* ops;
};

int8_t pVoidArray_new_hidden(pVoidArray** array,
	const char* file, size_t line, const char* func);
#define pVoidArray_new(array) pVoidArray_new_hidden(array, __FILE__, __LINE__, __func__)
int8_t pVoidArray_delete(pVoidArray** array);



#endif