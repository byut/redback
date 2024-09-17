#ifndef REDBACK_MEMORY_POOL_H
#define REDBACK_MEMORY_POOL_H

#include <stddef.h>

/* # */

///
/// @brief ...
///
struct redback_memory_pool;

/* # */

///
/// @brief Allocate resources for an instance of the memory pool.
///
struct redback_memory_pool *redback_memory_pool_new(size_t size, size_t capacity);

///
/// @brief Deallocate the resources utilized by an instance of the memory pool.
///
void redback_memory_pool_free(struct redback_memory_pool *pool);

/* # */

///
/// @brief Allocate resources on the memory pool.
///
void *redback_memory_pool_allocate(struct redback_memory_pool *pool);

///
/// @brief Deallocate the resources on the memory pool.
///
void redback_memory_pool_deallocate(struct redback_memory_pool *pool, const void *ptr);

/* # */

///
/// @brief Get the current size of the memory pool.
///
size_t redback_memory_pool_get_size(const struct redback_memory_pool *pool);

///
/// @brief Get the size of the memory pool's unit.
///
size_t redback_memory_pool_get_usize(const struct redback_memory_pool *pool);

///
/// @brief Get the capacity that the memory pool can hold.
///
size_t redback_memory_pool_get_capacity(const struct redback_memory_pool *pool);

/* # */

#endif
