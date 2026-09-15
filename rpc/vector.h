#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>
#include <stdlib.h>

#define VECTOR_INIT_CAPACITY 4

// Helper methods to simplyfy the use of the Vector
#define VECTOR_ADD(vec, item) \
    vector_add(&vec, (void *) item)

#define VECTOR_SET(vec, id, item) \
    vector_set(&vec, id, (void *) item)

#define VECTOR_GET(vec, type, id) \
    ((type)vector_get(&vec, id))

#define VECTOR_DELETE(vec, id) \
    vector_delete(&vec, id)

#define VECTOR_TOTAL(vec) \
    vector_total(&vec)

#define VECTOR_FREE(vec) \
    vector_free(&vec)

// this is the basically the dynamic thing
typedef struct vector {
    void **items;
    size_t capacity;
    size_t total;
} vector;

int vector_init(vector *);
size_t vector_total(const vector *); // using const for get and total cuz they doesn't modify the vector
int vector_add(vector *, void *);
void vector_set(vector *, size_t, void *);
void *vector_get(const vector *, size_t);
int vector_delete(vector *, size_t);
void vector_free(vector *);

#ifdef VECTOR_IMPLEMENTATION

int vector_init(vector *vec)
{
    vec->capacity = VECTOR_INIT_CAPACITY;
    vec->items = malloc(vec->capacity * sizeof(void *));
    
    if (vec->items == NULL) {
        vec->capacity = 0;
        return 0;
    }
    
    vec->total = 0;
    return 1;
}

size_t vector_total(const vector *v)
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
static int vector_resize(vector *v, int capacity)
{
    void **items = (void **)realloc(v->items, sizeof(void *) * capacity);
    if (items == NULL) {
        return 0;
    }
    v->items = items;
    v->capacity = capacity;

    return 1;
}

int vector_add(vector *v, void *item)
{
    if (v->capacity == v->total) {
        size_t new_capacity = v->capacity * 2;
        if (!vector_resize(v, new_capacity)) {
            return 0;
        }
    }
    v->items[v->total++] = item;
    return 1;
}

void vector_set(vector *v, size_t index, void *item)
{
    if (index < v->total) v->items[index] = item;
}

void* vector_get(const vector *v, size_t index)
{
    if (index < v->total) return v->items[index];
    return NULL;
}

int vector_delete(vector *v, size_t index)
{
    // size_t is unsigned
    // invalid index
    if (index >= v->total)
        return 0;
    // v->items[index] = NULL; this one too

    // Shift elements left to fill the deleted slot.
    // 1 N 3 4 5 6 7
    // 1 3 4 5 6 7 N
    for (size_t i = index; i < v->total - 1; ++i) {
        v->items[i] = v->items[i + 1];
        // v->items[i + 1] = NULL; because we are immediately overwriting that position
    }

    v->total -= 1;
    // 1 2 3 4 
    if (v->capacity > VECTOR_INIT_CAPACITY &&
        v->total <= v->capacity / 4) {
        vector_resize(v, v->capacity / 2);
    }
    return 1;
}

void vector_free(vector *v)
{
    free(v->items);
    v->items = NULL;
    v->capacity = 0;
    v->total = 0;
}

#endif // VECTOR_IMPLEMENTATION
#endif // VECTOR_H
