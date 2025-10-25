#include <string.h>
#include "array.h"


array_info* array_new_(size_t item_size, size_t size) {
	// find capacity
	size_t cap = 8;
	if (size > cap) {
		cap = size;

		// round up to next power of 2
		cap--;
		cap |= cap >> 1;
		cap |= cap >> 2;
		cap |= cap >> 4;
		cap |= cap >> 8;
		cap |= cap >> 16;
		cap++;
	}

	// make array
	array_info* info = calloc(1, sizeof(array_info) + item_size * cap);
	info->size = size;
	info->capacity = cap;
	return info;
}

void* array_resize_(void* array, size_t item_size, size_t size) {
	array_info* info = NULL;
	if (!array) {
		info = array_new_(item_size, 0);
	}
	else {
		info = ((array_info*)array) - 1;
	}
	if (size == -1) {
		info->size++;
	}
	else {
		info->size = size;
	}
	if (info->size >= info->capacity) {
		info->capacity = info->size;
		info->capacity--;
		info->capacity |= info->capacity >> 1;
		info->capacity |= info->capacity >> 2;
		info->capacity |= info->capacity >> 4;
		info->capacity |= info->capacity >> 8;
		info->capacity |= info->capacity >> 16;
		info->capacity++;
		info = realloc(info, sizeof(array_info) + (info->capacity * item_size));
	}
	return info->array;
}

void array_print(void* array) {
	array_info* info = &((array_info*)array)[-1];
	fprintf(stderr,
		 "size: %lu\n"
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

