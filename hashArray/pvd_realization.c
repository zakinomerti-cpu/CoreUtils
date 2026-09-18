#include "hashArray.h"
#include "pVoidArray.h"
#include "stdlib.h"
#include "string.h"

struct hashArrayElement {
	uint32_t	hash;
	const void*	key;
	size_t		keyLen;
	void*		value;
};

static int8_t fromPv(int8_t code) {
	switch (code){
		case PV_OUTCODE_OK:
			return HA_OUTCODE_OK;
		case PV_OUTCODE_ALLOC_ERROR:
			return HA_OUTCODE_ALLOC_ERROR;
		case PV_OUTCODE_ARG_NULL:
			return HA_OUTCODE_ARG_NULL;
		case PV_OUTCODE_EMPTY_ARR:
			return HA_OUTCODE_EMPTY_TABLE;
		case PV_OUTCODE_INVALID_ARR:
			return HA_OUTCODE_INVALID_TABLE;
		case PV_OUTCODE_INVALID_INPUT_INDEX:
			return HA_OUTCODE_UNKNOWN_ERROR;
		default:
			return HA_OUTCODE_UNKNOWN_ERROR;
	}
}

static uint32_t hash_func(const void* key, size_t len) {
	uint32_t hash = 5381;
	const uint8_t* p = (const uint8_t*)key;
	for (int i = 0; i < len; i += 1) {
		hash = ((hash << 5) + hash) + p[i];
	}
	return hash;
}

static int8_t chainAt(const pVoidArray* buckets, size_t index, pVoidArray **out) {
	void *slot = NULL;
	int8_t outcode;

	if (!buckets || !out) {
		return HA_OUTCODE_ARG_NULL;
	}

	outcode = buckets->ops->get((pVoidArray *)buckets, &slot, index);
	if (outcode != PV_OUTCODE_OK) {
		return fromPv(outcode);
	}

	*out = (pVoidArray *)slot;
	return HA_OUTCODE_OK;
}

static int8_t findInChain
(
	const pVoidArray* chain,
	uint32_t hash,
	const void* key,
	size_t keyLen,
	hashArrayElement** out,
	size_t* index
)
{
	if (!chain || !out) {
		return HA_OUTCODE_ARG_NULL;
	}

	for (size_t i = 0; i < chain->size; i += 1) {
		hashArrayElement* e = chain->array[i];
		if (!e) continue;
		if (e->hash != hash) continue;
		if (e->keyLen != keyLen) continue;
		if (memcmp(e->key, key, keyLen) != 0) continue;

		*out = e;
		if (index) *index = i;
		return HA_OUTCODE_OK;
	}

	return HA_OUTCODE_KEY_NOT_FOUND;
}

static int8_t bucketsNew(pVoidArray** out, size_t count) {

	pVoidArray* buckets = NULL;
	int8_t outcode;

	if (!out || !count) {
		return HA_OUTCODE_ARG_NULL;
	}

	outcode = pVoidArray_new(&buckets);
	if (outcode != PV_OUTCODE_OK) {
		return fromPv(outcode);
	}

	outcode = buckets->ops->reserve(buckets, count);
	if (outcode != PV_OUTCODE_OK) {
		pVoidArray_delete(&buckets);
		return fromPv(outcode);
	}

	for (size_t i = 0; i < count; i += 1) {
		outcode = buckets->ops->push(buckets, NULL);
		if (outcode != PV_OUTCODE_OK) {
			pVoidArray_delete(&buckets);
			return fromPv(outcode);
		}
	}

	*out = buckets;
	return HA_OUTCODE_OK;
}

static void bucketsRelease(pVoidArray** buckets, int dropElements) {
	pVoidArray* array;

	if (!buckets || !*buckets) {
		return;
	}
	array = *buckets;

	for (size_t i = 0; i < array->size; i+=1) {
		pVoidArray* chain = array->array[i];
		if (!chain) continue;

		if (dropElements) {
			for (size_t j = 0; j < chain->size; j+=1) {
				if (chain->array[j]) {
					free(chain->array[j]);
					chain->array[j] = NULL;
				}
			}
		}

		pVoidArray_delete(&chain);
		array->array[i] = NULL;
	}

	pVoidArray_delete(buckets);
}

static int8_t placeEntry(pVoidArray* buckets, size_t capacity, hashArrayElement* e) {

	pVoidArray* chain = NULL;
	size_t index;
	int8_t outcode;

	if (!buckets || !e || !capacity) {
		return HA_OUTCODE_ARG_NULL;
	}

	index = (size_t)(e->hash % (uint32_t)capacity);
	outcode = chainAt(buckets, index, &chain);
	if (outcode != HA_OUTCODE_OK) {
		return outcode;
	}

	if (!chain) {
		outcode = pVoidArray_new(&chain);
		if (outcode != PV_OUTCODE_OK) {
			return fromPv(outcode);
		}

		outcode = buckets->ops->set(buckets, chain, index);
		if (outcode != PV_OUTCODE_OK) {
			pVoidArray_delete(&chain);
			return fromPv(outcode);
		}
	}

	outcode = chain->ops->push(chain, e);
	if (outcode != PV_OUTCODE_OK) {
		return fromPv(outcode);
	}

	return HA_OUTCODE_OK;
}

static int8_t rehash(hashArray* table, size_t newCapacity) {
	pVoidArray* buckets = NULL;
	pVoidArray* old;
	int8_t outcode;

	if (!table) {
		return HA_OUTCODE_ARG_NULL;
	}
	if (!table->buckets) {
		return HA_OUTCODE_INVALID_TABLE;
	}
	if (!newCapacity) {
		newCapacity = HA_DEFAULT_CAPACITY;
	}

	outcode = bucketsNew
	(
		&buckets,
		newCapacity
	);
	if (outcode != HA_OUTCODE_OK) {
		return outcode;
	}

	old = table->buckets;
	for (size_t i = 0; i < old->size; i+=1) {
		pVoidArray* chain = old->array[i];
		if (!chain) continue;

		for (size_t j = 0; j < chain->size; j+=1) {
			hashArrayElement* e = chain->array[j];
			if (!e) continue;

			outcode = placeEntry
			(
				buckets,
				newCapacity,
				e



			);
			if (outcode != HA_OUTCODE_OK) {
				bucketsRelease(&buckets, 0);
				return outcode;
			}
		}
	}

	pVoidArray* buckts = table->buckets;
	bucketsRelease(&buckets, 0);
	table->buckets = buckets;
	table->capacity = newCapacity;
	return HA_OUTCODE_OK;
}

static int8_t growIfNeeded(hashArray* table) {
	if (!table)
		return HA_OUTCODE_ARG_NULL;
	if (!table->capacity)
		return rehash(table, HA_DEFAULT_CAPACITY);
	if ((double)(table->size + 1) <= HA_MAX_LOAD_FACTOR * (double)table->capacity)
		return HA_OUTCODE_OK;
	return rehash(table, table->capacity * 2);
}

static int8_t insert(hashArray* table, const void* key, size_t keyLen,
	void* value, void** oldOut, int replace) {

	hashArrayElement* e = NULL;
	pVoidArray* chain = NULL;
	size_t index;
	int8_t outcode;

	if (!table) {
		return HA_OUTCODE_ARG_NULL;
	}
	if (!table->buckets) {
		return HA_OUTCODE_INVALID_TABLE;
	}
	if (!key || !keyLen) {
		return HA_OUTCODE_INVALID_KEY;
	}

	uint32_t hash = hash_func(key, keyLen);
	index = (size_t)(hash % (uint32_t)table->capacity);

	outcode = chainAt(table->buckets, index, &chain);
	if (outcode != HA_OUTCODE_OK) {
		return outcode;
	}

	if (chain) {
		outcode = findInChain(chain, hash, key, keyLen, &e, NULL);
		if (outcode == HA_OUTCODE_OK) {
			if (!replace) {
				return HA_OUTCODE_DUPLICATE_KEY;
			}
			if (oldOut) *oldOut = e->value;
			e->key = key;
			e->value = value;
			return HA_OUTCODE_OK;
		}
		if (outcode != HA_OUTCODE_KEY_NOT_FOUND) {
			return outcode;
		}
	}

	outcode = growIfNeeded(table);
	if (outcode != HA_OUTCODE_OK) {
		return outcode;
	}

	e = (hashArrayElement *)malloc
	(
	sizeof(hashArrayElement)
	);
	if (!e) {
		return HA_OUTCODE_ALLOC_ERROR;
	}

	e->hash = hash;
	e->key = key;
	e->keyLen = keyLen;
	e->value = value;

	outcode = placeEntry
	(
		table->buckets,
		table->capacity,
		e
	);
	if (outcode != HA_OUTCODE_OK) {
		free(e);
		return outcode;
	}

	table->size += 1;
	if (oldOut) *oldOut = NULL;
	return HA_OUTCODE_OK;
}

static int8_t put(hashArray* table, const void* key, size_t keyLen, void *value) {
	return insert(table, key, keyLen, value, NULL, 0);
}

static int8_t set(hashArray* table, const void* key, size_t keyLen, void *value, void** oldOut) {
	if (oldOut) *oldOut = NULL;
	return insert(table, key, keyLen, value, oldOut, 1);
}

static int8_t get(const hashArray* table, const void* key, size_t keyLen, void **out) {
	hashArrayElement* e = NULL;
	pVoidArray* chain = NULL;
	int8_t outcode;

	if (!table || !out) {
		return HA_OUTCODE_ARG_NULL;
	}
	if (!table->buckets) {
		return HA_OUTCODE_INVALID_TABLE;
	}
	if (!key || !keyLen) {
		return HA_OUTCODE_INVALID_KEY;
	}
	if (!table->size) {
		return HA_OUTCODE_EMPTY_TABLE;
	}

	uint32_t hash = hash_func(key, keyLen);
	outcode = chainAt(table->buckets, hash % (uint32_t)table->capacity, &chain);
	if (outcode != HA_OUTCODE_OK) {
		return outcode;
	}
	if (!chain) {
		return HA_OUTCODE_KEY_NOT_FOUND;
	}

	outcode = findInChain(chain, hash, key, keyLen, &e, NULL);
	if (outcode != HA_OUTCODE_OK) {
		return outcode;
	}

	*out = e->value;
	return HA_OUTCODE_OK;
}

static int8_t premove(hashArray* table, const void* key, size_t keyLen, void **out) {
	hashArrayElement* e = NULL;
	pVoidArray* chain = NULL;
	void* dropped = NULL;
	size_t index = 0;
	int8_t outcode;

	if (!table) {
		return HA_OUTCODE_ARG_NULL;
	}
	if (!table->buckets) {
		return HA_OUTCODE_INVALID_TABLE;
	}
	if (!key || !keyLen) {
		return HA_OUTCODE_INVALID_KEY;
	}
	if (!table->size) {
		return HA_OUTCODE_EMPTY_TABLE;
	}

	uint32_t hash = hash_func(key, keyLen);
	outcode = chainAt(table->buckets, hash % (uint32_t)table->capacity, &chain);
	if (outcode != HA_OUTCODE_OK) {
		return outcode;
	}
	if (!chain) {
		return HA_OUTCODE_KEY_NOT_FOUND;
	}

	outcode = findInChain(chain, hash, key, keyLen, &e, &index);
	if (outcode != HA_OUTCODE_OK) {
		return outcode;
	}

	outcode = chain->ops->remove(chain, &dropped, index);
	if (outcode != PV_OUTCODE_OK) {
		return fromPv(outcode);
	}

	if (out) *out = e->value;
	free(e);
	table->size -= 1;
	return HA_OUTCODE_OK;
}

static int8_t contains(const hashArray* table, const void* key, size_t keyLen) {
	void *value = NULL;
	return get(table, key, keyLen, &value);
}

static int8_t size(const hashArray* table, size_t* out) {
	if (!table || !out) {
		return HA_OUTCODE_ARG_NULL;
	}

	*out = table->size;
	return HA_OUTCODE_OK;
}

static int8_t capacity(const hashArray* table, size_t* out) {
	if (!table || !out) {
		return HA_OUTCODE_ARG_NULL;
	}

	*out = table->capacity;
	return HA_OUTCODE_OK;
}

static int8_t loadFactor(const hashArray* table, double *out) {
	if (!table || !out) {
		return HA_OUTCODE_ARG_NULL;
	}
	if (!table->capacity) {
		*out = 0.0;
		return HA_OUTCODE_OK;
	}

	*out = (double)table->size / (double)table->capacity;
	return HA_OUTCODE_OK;
}

static int8_t reserve(hashArray* table, size_t newCapacity) {
	if (!table) {
		return HA_OUTCODE_ARG_NULL;
	}
	if (!table->buckets) {
		return HA_OUTCODE_INVALID_TABLE;
	}
	if (table->capacity >= newCapacity) {
		return HA_OUTCODE_OK;
	}

	return rehash(table, newCapacity);
}

static int8_t clear(hashArray* table) {
	pVoidArray* buckets;

	if (!table) {
		return HA_OUTCODE_ARG_NULL;
	}
	if (!table->buckets) {
		return HA_OUTCODE_INVALID_TABLE;
	}
	if (!table->size) {
		return HA_OUTCODE_EMPTY_TABLE;
	}

	buckets = table->buckets;
	for (size_t i = 0; i < buckets->size; i += 1) {
		pVoidArray* chain = (pVoidArray*)buckets->array[i];
		if (!chain) continue;

		for (size_t j = 0; j < chain->size; j += 1) {
			if (chain->array[j]) {
				free(chain->array[j]);
				chain->array[j] = NULL;
			}
		}
		chain->ops->clear(chain);
	}

	table->size = 0;
	return HA_OUTCODE_OK;
}

static int8_t for_each(hashArray* table, hashArrayEachFn f, void *args) {
	pVoidArray* buckets;
	int8_t outcode;

	if (!table || !f) {
		return HA_OUTCODE_ARG_NULL;
	}
	if (!table->buckets) {
		return HA_OUTCODE_INVALID_TABLE;
	}
	if (!table->size) {
		return HA_OUTCODE_EMPTY_TABLE;
	}

	buckets = table->buckets;
	for (size_t i = 0; i < buckets->size; i += 1) {
		pVoidArray* chain = (pVoidArray*)buckets->array[i];
		if (!chain) continue;

		for (size_t j = 0; j < chain->size; j += 1) {
			hashArrayElement* e = (hashArrayElement*)chain->array[j];
			if (!e) continue;

			outcode = f(e->key, e->keyLen, e->value, args);
			if (outcode != HA_OUTCODE_OK) {
				return outcode;
			}
		}
	}

	return HA_OUTCODE_OK;
}

static int8_t flatten(const hashArray* table, hashArrayPair** out, size_t* outCount) {
	hashArrayPair* pairs;
	pVoidArray* buckets;
	size_t used = 0;

	if (!table || !out || !outCount) {
		return HA_OUTCODE_ARG_NULL;
	}
	if (!table->buckets) {
		return HA_OUTCODE_INVALID_TABLE;
	}
	if (!table->size) {
		return HA_OUTCODE_EMPTY_TABLE;
	}

	pairs = (hashArrayPair *)malloc(
		sizeof(hashArrayPair) * table->size
	);
	if (!pairs) {
		return HA_OUTCODE_ALLOC_ERROR;
	}
	memset(pairs, 0, sizeof(hashArrayPair) * table->size);

	buckets = table->buckets;
	for (size_t i = 0; i < buckets->size; i += 1) {
		pVoidArray* chain = (pVoidArray *)buckets->array[i];
		if (!chain) continue;

		for (size_t j = 0; j < chain->size; j += 1) {
			hashArrayElement* e = (hashArrayElement *)chain->array[j];
			if (!e) continue;

			if (used >= table->size) {
				free(pairs);
				return HA_OUTCODE_UNKNOWN_ERROR;
			}

			pairs[used].key = e->key;
			pairs[used].keyLen = e->keyLen;
			pairs[used].value = e->value;
			pairs[used].bucket = i;
			pairs[used].hash = e->hash;
			used += 1;
		}
	}

	if (used != table->size) {
		free(pairs);
		return HA_OUTCODE_UNKNOWN_ERROR;
	}

	*out = pairs;
	*outCount = used;
	return HA_OUTCODE_OK;
}

static const hashArrayInterface interface = {
	put, set, get, premove, contains, size,
	capacity, loadFactor, reserve, clear, for_each, flatten
};

int8_t hashArray_new(hashArray** table, size_t initialCapacity) {

	pVoidArray* buckets = NULL;
	int8_t outcode;

	if (!table) return HA_OUTCODE_ARG_NULL;
	if (!initialCapacity) initialCapacity = HA_DEFAULT_CAPACITY;

	(*table) = malloc(sizeof(hashArray));

	if (!*table) {
		return HA_OUTCODE_ALLOC_ERROR;
	}

	outcode = bucketsNew(&buckets, initialCapacity);
	if (outcode != HA_OUTCODE_OK) {
		free(*table);
		*table = NULL;
		return outcode;
	}

	(*table)->size = 0;
	(*table)->capacity = initialCapacity;
	(*table)->buckets = buckets;
	(*table)->ops = &interface;
	return HA_OUTCODE_OK;
}

int8_t hashArray_delete(hashArray** table) {
	if (!table) {
		return HA_OUTCODE_ARG_NULL;
	}
	if (!*table) {
		return HA_OUTCODE_INVALID_TABLE;
	}

	pVoidArray* buckts = (*table)->buckets;
	bucketsRelease(&buckts, 1);
	free(*table);
	*table = NULL;
	return HA_OUTCODE_OK;
}

int8_t hashArray_flat_delete(hashArrayPair** flat) {
	if (!flat) {
		return HA_OUTCODE_ARG_NULL;
	}
	if (!*flat) {
		return HA_OUTCODE_OK;
	}

	free(*flat);
	*flat = NULL;
	return HA_OUTCODE_OK;
}
