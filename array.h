#ifndef ARRAY_H_
#define ARRAY_H_
#include <stdlib.h>

typedef struct {
    size_t size;
    size_t capacity;
    char array[];
} array_info;

array_info* array_new_(size_t item_size, size_t size);
void array_print(void* array);
void array_pop(void* array);
void* array_add_(void* array, size_t item_size);


#define array_clear(array) (((array_info*)(array))[-1].size = 0)
#define array_size(array) (((array_info*)(array))[-1].size)
#define array_new(type, size) ((type*)(array_new_(sizeof(type), size)->array))

#define array_free(array) do {          \
    free(((array_info*)(array)) - 1);   \
    (array) = NULL;                     \
} while (0)

#define array_add(array, value) do {                        \
    void* __temp = array_add_((array), sizeof(*(array)));   \
    (array) = __temp;                                       \
    (array)[array_size((array)) - 1] = value;               \
} while (0)

#define array_last(array) (array)[array_size(array) - 1]

#define array_for_all(type__, it__, array__) \
type__* it__ = array__; type__* end__ = array__ ? array__ + array_size(array__) : NULL; \
for (; it__ < end__; it__++)

#endif // ARRAY_H_
