#ifndef hashArray_header
#define hashArray_header

#include <stdint.h>
#include <stddef.h>

typedef enum {
	HA_OUTCODE_OK,
	HA_OUTCODE_ALLOC_ERROR,
	HA_OUTCODE_ARG_NULL,
	HA_OUTCODE_EMPTY_TABLE,
	HA_OUTCODE_INVALID_TABLE,
	HA_OUTCODE_INVALID_KEY,
	HA_OUTCODE_KEY_NOT_FOUND,
	HA_OUTCODE_DUPLICATE_KEY,
	HA_OUTCODE_UNKNOWN_ERROR
} HA_OUTCODE;

#define HA_MAX_LOAD_FACTOR 0.75
#define HA_DEFAULT_CAPACITY 8

typedef int8_t (*hashArrayEachFn)(const void* key, size_t keyLen, void* value, void* args);
typedef struct hashArrayPair {
	const void*	key;
	size_t		keyLen;
	void*		value;
	size_t		bucket;
	int		hash;
} hashArrayPair;

#define HA_DECLARE_INTERFACE(interfaceName, owner, type)	\
	typedef struct owner owner; \
	typedef struct interfaceName { \
		int8_t (*put)(owner* self, const void* key, size_t keyLen, type* value); \
		int8_t (*set)(owner* self, const void* key, size_t keyLen, type* value, type** oldOut); \
		int8_t (*get)(const owner* self, const void* key, size_t keyLen, type** out); \
		int8_t (*remove)(owner* self, const void* key, size_t keyLen, type** out); \
		int8_t (*contains)(const owner* self, const void* key, size_t keyLen); \
		int8_t (*size)(const owner* self, size_t* out); \
		int8_t (*capacity)(const owner* self, size_t* out); \
		int8_t (*loadFactor)(const owner* self, double* out); \
		int8_t (*reserve)(owner* self, size_t newCapacity); \
		int8_t (*clear)(owner* self); \
		int8_t (*foreach)(owner* self, hashArrayEachFn c, void* args); \
		int8_t (*flatten)(const owner* self, hashArrayPair** out, size_t* outCount); \
	} interfaceName; \
	int8_t owner##_delete(owner** array);


#define HA_delete(owner) \
	int8_t owner##_delete(owner** array) { \
		if (!array) { \
			return HA_OUTCODE_ARG_NULL; \
		} \
		if (!*array) { \
			return HA_OUTCODE_INVALID_TABLE; \
		} \
		\
		int8_t r = hashArray_delete(&(*array)->hashArr); \
		if(r != HA_OUTCODE_OK) { \
			return r; \
		} \
		free(*array); \
		*array = NULL; \
		return r; \
	}





#define HA_put(owner, hashArr, type) \
	static int8_t (owner##_put)(owner* self, const void* key, size_t keyLen, type* value) { \
		if(!self) return HA_OUTCODE_ARG_NULL; \
		if(!self->hashArr) return HA_OUTCODE_INVALID_TABLE; \
		return self->hashArr->ops->put(self->hashArr, key, keyLen, value); \
	}

#define HA_set(owner, hashArr, type) \
	static int8_t (owner##_set)(owner* self, const void* key, size_t keyLen, type* value, type** oldOut) { \
		if(!self) return HA_OUTCODE_ARG_NULL; \
		if(!self->hashArr) return HA_OUTCODE_INVALID_TABLE; \
		void* old = NULL; \
		int8_t outcode = self->hashArr->ops->set(self->hashArr, key, keyLen, value, &old); \
		if(outcode != HA_OUTCODE_OK) return outcode; \
		if(oldOut) *oldOut = (type*)old; \
		return outcode; \
	}

#define HA_get(owner, hashArr, type) \
	static int8_t (owner##_get)(const owner* self, const void* key, size_t keyLen, type** out) { \
		if(!self || !out) return HA_OUTCODE_ARG_NULL; \
		if(!self->hashArr) return HA_OUTCODE_INVALID_TABLE; \
		void* result = NULL; \
		int8_t outcode = self->hashArr->ops->get(self->hashArr, key, keyLen, &result); \
		if(outcode != HA_OUTCODE_OK) return outcode; \
		*out = (type*)result; \
		return outcode; \
	}

#define HA_remove(owner, hashArr, type) \
	static int8_t (owner##_remove)(owner* self, const void* key, size_t keyLen, type** out) { \
		if(!self) return HA_OUTCODE_ARG_NULL; \
		if(!self->hashArr) return HA_OUTCODE_INVALID_TABLE; \
		void* result = NULL; \
		int8_t outcode = self->hashArr->ops->remove(self->hashArr, key, keyLen, &result); \
		if(outcode != HA_OUTCODE_OK) return outcode; \
		if(out) *out = (type*)result; \
		return outcode; \
	}

#define HA_contains(owner, hashArr) \
	static int8_t (owner##_contains)(const owner* self, const void* key, size_t keyLen) { \
		if(!self) return HA_OUTCODE_ARG_NULL; \
		if(!self->hashArr) return HA_OUTCODE_INVALID_TABLE; \
		return self->hashArr->ops->contains(self->hashArr, key, keyLen); \
	}

#define HA_size(owner, hashArr) \
	static int8_t (owner##_size)(const owner* self, size_t* out) { \
		if(!self || !out) return HA_OUTCODE_ARG_NULL; \
		if(!self->hashArr) return HA_OUTCODE_INVALID_TABLE; \
		size_t result = 0; \
		int8_t outcode = self->hashArr->ops->size(self->hashArr, &result); \
		if(outcode != HA_OUTCODE_OK) return outcode; \
		*out = result; \
		return outcode; \
	}

#define HA_capacity(owner, hashArr) \
	static int8_t (owner##_capacity)(const owner* self, size_t* out) { \
		if(!self || !out) return HA_OUTCODE_ARG_NULL; \
		if(!self->hashArr) return HA_OUTCODE_INVALID_TABLE; \
		size_t result = 0; \
		int8_t outcode = self->hashArr->ops->capacity(self->hashArr, &result); \
		if(outcode != HA_OUTCODE_OK) return outcode; \
		*out = result; \
		return outcode; \
	}

#define HA_loadFactor(owner, hashArr) \
	static int8_t (owner##_loadFactor)(const owner* self, double* out) { \
		if(!self || !out) return HA_OUTCODE_ARG_NULL; \
		if(!self->hashArr) return HA_OUTCODE_INVALID_TABLE; \
		double result = 0.0; \
		int8_t outcode = self->hashArr->ops->loadFactor(self->hashArr, &result); \
		if(outcode != HA_OUTCODE_OK) return outcode; \
		*out = result; \
		return outcode; \
	}

#define HA_reserve(owner, hashArr) \
	static int8_t (owner##_reserve)(owner* self, size_t newCapacity) { \
		if(!self) return HA_OUTCODE_ARG_NULL; \
		if(!self->hashArr) return HA_OUTCODE_INVALID_TABLE; \
		return self->hashArr->ops->reserve(self->hashArr, newCapacity); \
	}

#define HA_clear(owner, hashArr) \
	static int8_t (owner##_clear)(owner* self) { \
		if(!self) return HA_OUTCODE_ARG_NULL; \
		if(!self->hashArr) return HA_OUTCODE_INVALID_TABLE; \
		return self->hashArr->ops->clear(self->hashArr); \
	}

#define HA_foreach(owner, hashArr) \
	static int8_t (owner##_foreach)(owner* table, hashArrayEachFn c, void* args) { \
		if(!table) return HA_OUTCODE_ARG_NULL; \
		if(!table->hashArr) return HA_OUTCODE_INVALID_TABLE; \
		return table->hashArr->ops->foreach(table->hashArr, c, args); \
	}

/* тяжёлая операция, не стоит делать ее в цикле */
#define HA_flatten(owner, hashArr) \
	static int8_t (owner##_flatten)(const owner* self, hashArrayPair** out, size_t* outCount) { \
		if(!self || !out || !outCount) return HA_OUTCODE_ARG_NULL; \
		if(!self->hashArr) return HA_OUTCODE_INVALID_TABLE; \
		return self->hashArr->ops->flatten(self->hashArr, out, outCount); \
	}

#define HA_REALIZE_INTERFACE(owner, hashArr, type) \
	HA_delete(owner) \
	HA_put(owner, hashArr, type) \
	HA_set(owner, hashArr, type) \
	HA_get(owner, hashArr, type) \
	HA_remove(owner, hashArr, type) \
	HA_contains(owner, hashArr) \
	HA_size(owner, hashArr) \
	HA_capacity(owner, hashArr) \
	HA_loadFactor(owner, hashArr) \
	HA_reserve(owner, hashArr) \
	HA_clear(owner, hashArr) \
	HA_foreach(owner, hashArr) \
	HA_flatten(owner, hashArr) \

#define HA_DECLARE_CONTAINER(name, funcNameThatUsedToCreateYourNewTable, interfaceName, hashArr) \
	int8_t funcNameThatUsedToCreateYourNewTable( \
		name** table, \
		size_t initialCapacity \
	); \
	struct name { \
		hashArray* hashArr; \
		const interfaceName* ops; \
	};

#define HA_REALIZE_CONTAINER(name, interfaceName, hashArr, funcNameThatUsedToCreateYourNewTable) \
	static const interfaceName name##_ops = { \
		name##_put, name##_set, name##_get, \
		name##_remove, name##_contains, name##_size, \
		name##_capacity, name##_loadFactor, name##_reserve, \
		name##_clear, name##_foreach, name##_flatten, \
	}; \
	int8_t funcNameThatUsedToCreateYourNewTable( \
		name** table, \
		size_t initialCapacity \
	) \
	{ \
	if(!table) return HA_OUTCODE_ARG_NULL; \
	(*table) = (name*)malloc( \
		sizeof(name)); \
	if (!*table) { \
		return HA_OUTCODE_ALLOC_ERROR; \
	} \
	int8_t outcode = hashArray_new(&(*table)->hashArr, initialCapacity); \
	if(outcode != HA_OUTCODE_OK) { \
		free(*table); \
		*table = NULL; \
		return outcode; \
	} \
	(*table)->ops = &name##_ops; \
	return HA_OUTCODE_OK; \
}

#define HA_NEW(funcNameThatUsedToCreateYourNewTable, table, initialCapacity) \
	funcNameThatUsedToCreateYourNewTable(table, initialCapacity)

HA_DECLARE_INTERFACE(hashArrayInterface, hashArray, void)

typedef struct hashArrayElement hashArrayElement;

struct hashArray {
	size_t size;
	size_t capacity;
	void* buckets;
	const hashArrayInterface* ops;
};

int8_t hashArray_new(hashArray** table, size_t initialCapacity);
int8_t hashArray_delete(hashArray** table);
int8_t hashArray_flat_delete(hashArrayPair** flat);

#endif
