#include <redback/connection.h>
#include <redback/memory_pool.h>

#include <event2/bufferevent.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* # */

struct redback_memory_pool {
    void *array;     ///< Pointer to the memory block array.
    bool *free;      ///< Boolean array to track which blocks are free (true) or used (false).
    size_t usize;    ///< Size of each memory block (unit) in bytes.
    size_t size;     ///< Current number of used memory blocks.
    size_t capacity; ///< Total capacity of memory blocks that can be allocated.
};

/* # */

struct redback_memory_pool *redback_memory_pool_new(size_t usize, size_t capacity) {
    const size_t array_capacity = (usize * capacity);
    const size_t array_offset = sizeof(struct redback_memory_pool);
    const size_t free_capacity = (sizeof(bool) * capacity);
    const size_t free_offset = array_offset + array_capacity;

    struct redback_memory_pool *pool;
    pool = (struct redback_memory_pool *)malloc(
        sizeof(struct redback_memory_pool) + array_capacity + free_capacity);
    if (!pool)
        return NULL;

    pool->usize = usize;
    pool->size = 0;
    pool->capacity = capacity;

    pool->array = ((uint8_t *)pool + array_offset);
    pool->free = (bool *)((uint8_t *)pool + free_offset);

    memset(pool->array, 0x00, array_capacity);
    memset(pool->free, 0x01, free_capacity);

    return pool;
}

void redback_memory_pool_free(struct redback_memory_pool *pool) {
    free(pool);
}

/* # */

void *redback_memory_pool_allocate(struct redback_memory_pool *pool) {
    if (pool->size == pool->capacity)
        return NULL;
    size_t i;
    for (i = 0; i < pool->capacity && !pool->free[i]; ++i)
        ;
    pool->size++;
    pool->free[i] = false;
    return (uint8_t *)pool->array + (i * pool->usize);
}

void redback_memory_pool_deallocate(struct redback_memory_pool *pool, const void *ptr) {
    pool->size--;
    pool->free[((uint8_t *)ptr - (uint8_t *)pool->array) / pool->usize] = true;
}

/* # */

void *redback_memory_pool_begin(struct redback_memory_pool *pool) {
    return (void *)redback_memory_pool_cbegin(pool);
}

const void *redback_memory_pool_cbegin(const struct redback_memory_pool *pool) {
    if (0 == pool->size)
        return NULL;
    size_t i;
    for (i = 0; i < pool->capacity && (int)pool->free[i]; ++i)
        ;
    return (uint8_t *)pool->array + (i * pool->usize);
}

/* # */

const void *redback_memory_pool_cend(const struct redback_memory_pool *pool) {
    return pool->free;
}

/* # */

void *redback_memory_pool_iterator_next(const struct redback_memory_pool *pool, const void *it) {
    size_t i = (((uint8_t *)it - (uint8_t *)pool->array) / pool->usize) + 1;
    while (i < pool->capacity && (int)pool->free[i])
        i++;
    if (i == pool->capacity)
        return (void *)redback_memory_pool_cend(pool);
    return (uint8_t *)pool->array + (i * pool->usize);
}

/* # */

size_t redback_memory_pool_get_size(const struct redback_memory_pool *pool) {
    return pool->size;
}

size_t redback_memory_pool_get_usize(const struct redback_memory_pool *pool) {
    return pool->usize;
}

size_t redback_memory_pool_get_capacity(const struct redback_memory_pool *pool) {
    return pool->capacity;
}

/* # */
