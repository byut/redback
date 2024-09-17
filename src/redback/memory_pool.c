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
    void *array;
    bool *free;
    size_t usize;
    size_t size;
    size_t capacity;
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
