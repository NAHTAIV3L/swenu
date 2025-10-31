#ifndef ARRAY_H_
#define ARRAY_H_
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct {
	size_t size;
	size_t capacity;
	char array[];
} array_info;

array_info* array_new_(size_t item_size, size_t size);
void array_print(void* array);
void array_pop(void* array);
void* array_resize_(void* array, size_t item_size, size_t size);

#define CONCAT_INNER(a, b) a ## b
#define CONCAT(a, b) CONCAT_INNER(a, b)

#define array_clear(array) (((array_info*)(array))[-1].size = 0)
#define array_size(array) (((array_info*)(array))[-1].size)
#define array_new(type, size) ((type*)(array_new_(sizeof(type), size)->array))

#define array_free(array) do {        \
	free(((array_info*)(array)) - 1); \
	(array) = NULL;                   \
} while (0)

#define array_erase(array, index) do {                            \
	for (int __i = (index); __i < array_size((array)) - 1; __i++) \
		(array)[__i] = (array)[__i + 1];                          \
	((array_info*)(array))[-1].size--;                            \
} while(0)

#define array_erase_range(array, startinc, endinc) do {                                        \
	for (int __i = (startinc); __i < array_size((array)) - ((endinc) - (startinc) + 1); __i++) \
		(array)[__i] = (array)[__i + ((endinc) - (startinc) + 1)];                             \
	((array_info*)(array))[-1].size -= ((endinc) - (startinc) + 1);                            \
} while(0)

#define array_insert(array, index, value) do {                                      \
	size_t __idx = (index);                                                         \
	void* __temp = array_resize_((array), sizeof(*(array)), array_size(array) + 1); \
	(array) = __temp;                                                               \
	for (int __i = array_size((array)) - 1; __i > (__idx); __i--)                   \
		(array)[__i] = (array)[__i - 1];                                            \
	(array)[(__idx)] = value;                                                       \
} while(0)

#define array_insert_many(array, index, values, len) do {                             \
	size_t __idx = (index);                                                           \
	void* __temp = array_resize_((array), sizeof(*(array)), array_size(array) + len); \
	(array) = __temp;                                                                 \
	for (int __i = array_size((array)) - 1; __i > (__idx) + len - 1; __i--)           \
		(array)[__i] = (array)[__i - len];                                            \
	memcpy(&array[(__idx)], (values), len * sizeof(*(array)));                        \
} while (0)

#define array_insert_str(array, index, values) array_insert_many((array), (index), (values), strlen(values))

#define array_add_many(array, values, len) do {                                         \
	void* __temp = array_resize_((array), sizeof(*(array)), array_size(array) + (len)); \
	(array) = __temp;                                                                   \
	memcpy(&(array[array_size((array)) - (len)]), (values), (len) * sizeof(*(array)));  \
} while (0)

#define array_add_str(array, values) array_add_many((array), (values), strlen(values))

#define array_resize(array, size) do {                             \
	void* __temp = array_resize_((array), sizeof(*(array)), size); \
	(array) = __temp;                                              \
} while (0)

#define array_add(array, value) do {                             \
	void* __temp = array_resize_((array), sizeof(*(array)), -1); \
	(array) = __temp;                                            \
	(array)[array_size((array)) - 1] = value;                    \
} while (0)

#define array_last(array) (array)[array_size(array) - 1]

#define array_for_all(type__, name__, array__)                                                                                                       \
type__* CONCAT(it__, __LINE__) = array__; type__* CONCAT(end__, __LINE__) = array__ ? array__ + array_size(array__) : NULL;                          \
for (type__* name__; (CONCAT(it__, __LINE__) < CONCAT(end__, __LINE__)) ? (name__ = CONCAT(it__, __LINE__), true) : false; CONCAT(it__, __LINE__)++)

#define array_dump(array, type, name, fmt,  ...) do {                                  \
	array_print(array);                                                                \
	fprintf(stderr, "vals: {\n");                                                      \
	type* it__  = (array); type* end__ = (array) ? (array) + array_size(array) : NULL; \
	for (type* name; it__ < end__ ? (name = it__, true) : false; it__++) {             \
		fprintf(stderr, "    "fmt",\n", __VA_ARGS__);                                  \
	}                                                                                  \
	fprintf(stderr, "}\n");                                                            \
} while (0)

#endif // ARRAY_H_
