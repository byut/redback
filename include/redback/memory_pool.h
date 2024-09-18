#ifndef REDBACK_MEMORY_POOL_H
#define REDBACK_MEMORY_POOL_H

#include <stddef.h>

/* # */

/// @brief A memory pool structure for managing dynamic memory allocations
///        of fixed-size application resources.
struct redback_memory_pool;

/* # */

///
/// @brief  Allocate and initialize a new memory pool.
///
/// @param  usize:    The size of each individual memory block in bytes.
/// @param  capacity: The total number of memory blocks in the pool.
///
/// @return A pointer to the newly created |redback_memory_pool| structure,
///         or `NULL` if the memory allocation fails.
///
struct redback_memory_pool *redback_memory_pool_new(size_t size, size_t capacity);

///
/// @brief Release the memory allocated for the memory pool.
///
/// @param pool: A pointer to the |redback_memory_pool| structure to be freed.
///
void redback_memory_pool_free(struct redback_memory_pool *pool);

/* # */

///
/// @brief  Allocates a memory block on the memory pool.
///
/// @param  pool: A pointer to the memory pool.
///
/// @return A pointer to the allocated memory block, or NULL if the memory
///         pool has reached its maximum capacity.
///
void *redback_memory_pool_allocate(struct redback_memory_pool *pool);

///
/// @brief  Deallocate a memory block on the memory pool.
///
/// @param  pool: A pointer to the memory pool structure.
/// @param  ptr:  A pointer to the memory block to deallocate.
///
void redback_memory_pool_deallocate(struct redback_memory_pool *pool, const void *ptr);

/* # */

///
/// @brief  Obtain an iterator to the first allocated memory block.
///
/// @param  pool: A pointer to the memory pool structure.
///
void *redback_memory_pool_begin(struct redback_memory_pool *pool);

///
/// @brief  Obtain a constant iterator to the first allocated memory block.
///
/// @param  pool: A pointer to the memory pool structure.
///
const void *redback_memory_pool_cbegin(const struct redback_memory_pool *pool);

///
/// @brief  Obtain a constant iterator to the element following the last allocated memory block.
///
/// @param  pool: A pointer to the memory pool structure.
///
const void *redback_memory_pool_cend(const struct redback_memory_pool *pool);

///
/// @brief   Advance the iterator to the consequent allocated memory block in the memory pool.
///
/// @param   pool: A pointer to the memory pool structure.
/// @param   it:   A memory pool iterator.
///
/// @returns An iterator to the consequent allocated memory block.
///
void *redback_memory_pool_iterator_next(const struct redback_memory_pool *pool, const void *it);

/* # */

///
/// @brief Retrieve the number of allocated memory blocks in the pool.
///
size_t redback_memory_pool_get_size(const struct redback_memory_pool *pool);

///
/// @brief Retrieve the size of each memory block in the pool.
///
size_t redback_memory_pool_get_usize(const struct redback_memory_pool *pool);

///
/// @brief Retrieve the total capacity of memory blocks that can be allocated.
///
size_t redback_memory_pool_get_capacity(const struct redback_memory_pool *pool);

/* # */

#endif
