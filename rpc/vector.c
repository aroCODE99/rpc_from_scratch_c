#include "vector.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>

void vector_init(vector *vec)
{
    log_info("let's see");
    vec->capacity = VECTOR_INIT_CAPACITY;
    log_info("after capacity");
    vec->items = malloc(vec->capacity * sizeof(void *));
    vec->total = 0;
}

int vector_total(vector *v)
{
    return v->total;
}

// what are the void **items ?
// Items points to a memory location containing a void *.
// items
//   |
//   v
// +-------+
// | void* |
// +-------+
//     |
//     v
//   actual data
static void vector_resize(vector *v, int capacity)
{
    // i really don't know how to use this
    #ifdef DEBUG_ON
    printf("vector_resize: %d to %d\n", v->capacity, capacity);
    #endif

    void **items = realloc(v->items, sizeof(void *) * capacity);
    if (items) {
        v->items = items;
        v->capacity = capacity;
    }
}

void vector_add(vector *v, void *item)
{
    if (v->capacity == v->total) vector_resize(v, v->capacity * 2);
    v->items[v->total++] = item;
}

void vector_set(vector *v, int index, void *item)
{
    if (index >= 0 && index < v->total) v->items[index] = item;
}

void* vector_get(vector *v, int index)
{
    if (index >= 0 && index < v->total) return v->items[index];
    return NULL;
}

void vector_delete(vector *v, int index)
{
    // invalid index    
    if (index < 0 || index >= v->total)
        return;
    v->items[index] = NULL;

    // now shifting the elements from the NULL 
    for (int i = index; i < v->total; ++i) {
        v->items[i] = v->items[i + 1];
        v->items[i + 1] = NULL;
    }

    v->total -= 1;
    if (v->total > 0 && v->total == v->capacity / 4)
        vector_resize(v, v->capacity / 2);
}

void vector_free(vector *v)
{
    free(v->items);
}
