#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "array.h"


array_info* array_new_(size_t item_size, size_t capacity) {
    array_info* info = calloc(1, sizeof(array_info) + item_size * capacity);
    info->capacity = capacity;
    return info;
}

void* array_add_(void* array, size_t item_size) {
    array_info* info = NULL;
    if (!array) {
        info = array_new_(item_size, 8);
    }
    else {
        info = ((array_info*)array) - 1;
    }
    if (info->size == info->capacity) {
        info->capacity <<= 1;
        info = realloc(info, sizeof(array_info) + (info->capacity * item_size));
    }
    info->size++;
    return &(info[1]);
}

void array_print(void* array) {
    array_info* info = &((array_info*)array)[-1];
    printf( "size: %lu\n"
           "capacity: %lu\n"
           "array: %p\n",
           info->size,
           info->capacity,
           info->array);
}

void array_pop(void* array) {
    array_info* info = &((array_info*)array)[-1];
    if (info->size)
        info->size--;
}
