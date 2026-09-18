#include "pVoidArray.h"
#include "stdlib.h"

static int8_t push(pVoidArray* array, void* element) {
    if (!array) return PV_OUTCODE_ARG_NULL;
    if (array->capacity <= array->size) {
        size_t nCap = array->capacity == 0 ? 4 : array->capacity * 2;

        void** tmp = realloc(
            array->array, nCap * sizeof(void*)
        );
        if (!tmp) return PV_OUTCODE_ALLOC_ERROR;
        array->capacity = nCap;
        array->array = tmp;
    }
    array->array[array->size] = element;
    array->size+=1;
    return PV_OUTCODE_OK;
}

static int8_t last(const pVoidArray* array, void** out) {
    if (!array || !out) {
        return PV_OUTCODE_ARG_NULL;
    }
    if (!array->size) {
        return PV_OUTCODE_EMPTY_ARR;
    }

    *out = array->array[array->size-1];
    return PV_OUTCODE_OK;
}

static int8_t pop(pVoidArray* array, void** out) {
    if (!array || !out) {
        return PV_OUTCODE_ARG_NULL;
    }
    if (!array->size) {
        return PV_OUTCODE_EMPTY_ARR;
    }
    *out = array->array[array->size-1];
    array->size-=1;
    return PV_OUTCODE_OK;
}
static int8_t get(const pVoidArray* array, void** out, size_t index) {
    if (!array || !out) {
        return PV_OUTCODE_ARG_NULL;
    }
    if (!array->size) {
        return PV_OUTCODE_EMPTY_ARR;
    }
    if (index >= array->size) {
        return PV_OUTCODE_INVALID_INPUT_INDEX;
    }
    *out = array->array[index];
    return PV_OUTCODE_OK;
}

static int8_t set(pVoidArray* array, void* element, size_t index) {
    if (!array) {
        return PV_OUTCODE_ARG_NULL;
    }
    if (!array->size) {
        return PV_OUTCODE_EMPTY_ARR;
    }
    if (index >= array->size) {
        return PV_OUTCODE_INVALID_INPUT_INDEX;
    }
    array->array[index] = element;
    return PV_OUTCODE_OK;
}

static int8_t size(const pVoidArray* array, size_t* out) {
    if (!array || !out) {
        return PV_OUTCODE_ARG_NULL;
    }

    *out = array->size;
    return PV_OUTCODE_OK;
}

static int8_t capacity(const pVoidArray* array, size_t* out) {
    if (!array || !out) {
        return PV_OUTCODE_ARG_NULL;
    }

    *out = array->capacity;
    return PV_OUTCODE_OK;
}

static int8_t reserve(pVoidArray* array, size_t newCapacity) {
    if (!array) {
        return PV_OUTCODE_ARG_NULL;
    }
    if (array->capacity >= newCapacity) {
        return PV_OUTCODE_OK;
    }
    void** tmp = realloc(
        array->array, sizeof(void*)*newCapacity);
    if (!tmp) return PV_OUTCODE_ALLOC_ERROR;
    array->capacity = newCapacity;
    array->array = tmp;
    return PV_OUTCODE_OK;
}

static int8_t premove(pVoidArray* array, void** out, size_t index) {
    if (!array || !out) {
        return PV_OUTCODE_ARG_NULL;
    }
    if (!array->size) {
        return PV_OUTCODE_EMPTY_ARR;
    }
    if (index >= array->size) {
        return PV_OUTCODE_INVALID_INPUT_INDEX;
    }

    *out = array->array[index];
    for (size_t i = index; i < array->size-1; i+=1) {
        array->array[i] = array->array[i+1];
    }
    array->size-=1;
    return PV_OUTCODE_OK;
}

static int8_t clear(pVoidArray* array) {
    if (!array) {
        return PV_OUTCODE_ARG_NULL;
    }
    if (!array->size) {
        return PV_OUTCODE_EMPTY_ARR;
    }
    array->size = 0;
    return PV_OUTCODE_OK;
}

static int8_t for_each(pVoidArray* array, pVoidArrayEachFn f, void* args) {
    if (!array) {
        return PV_OUTCODE_ARG_NULL;
    }
    if (!array->size) {
        return PV_OUTCODE_EMPTY_ARR;
    }
    for (size_t i = 0; i < array->size; i+=1) {
        void* value = array->array[i];
        f(value, i, args);
    }

    return PV_OUTCODE_OK;
}

static const pVoidArrayInterface interface = {
    push, pop, get, set, last, size, capacity, reserve, premove, clear, for_each
};

int8_t pVoidArray_new_hidden(pVoidArray** array,
    const char* file, size_t line, const char* func) {

    if(!array) return PV_OUTCODE_ARG_NULL;
    (*array) = (pVoidArray*) malloc(
        sizeof(pVoidArray));

    if (!*array) {
        return PV_OUTCODE_ALLOC_ERROR;
    }

    (*array)->array = NULL;
    (*array)->size = 0;
    (*array)->capacity = 0;
    (*array)->ops = &interface;
    return PV_OUTCODE_OK;
}

int8_t pVoidArray_delete(pVoidArray** array) {
    if (!array) {
        return PV_OUTCODE_ARG_NULL;
    }
    if(!*array) {
        return PV_OUTCODE_INVALID_ARR;
    }
    free((*array)->array);
    free(*array);
    *array = NULL;
    return PV_OUTCODE_OK;
}
