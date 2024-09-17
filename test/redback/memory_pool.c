#include <unity.h>
#include <unity_internals.h>

#include <redback/memory_pool.h>

#include <stdint.h>

void redback_memory_pool_new_should_init_pool_size() {
    struct redback_memory_pool *pool;
    pool = redback_memory_pool_new(sizeof(uint8_t), 1);

    TEST_ASSERT_NOT_NULL(pool);
    TEST_ASSERT_EQUAL(redback_memory_pool_get_size(pool), 0);

    redback_memory_pool_free(pool);
}

void redback_memory_pool_new_should_copy_pool_unit_size() {
    struct redback_memory_pool *pool;
    pool = redback_memory_pool_new(sizeof(uint8_t), 1);

    TEST_ASSERT_NOT_NULL(pool);
    TEST_ASSERT_EQUAL(redback_memory_pool_get_usize(pool), sizeof(uint8_t));

    redback_memory_pool_free(pool);
}

void redback_memory_pool_new_should_copy_pool_capacity() {
    struct redback_memory_pool *pool;
    pool = redback_memory_pool_new(sizeof(uint8_t), 1);

    TEST_ASSERT_NOT_NULL(pool);
    TEST_ASSERT_EQUAL(redback_memory_pool_get_capacity(pool), 1);

    redback_memory_pool_free(pool);
}

void redback_memory_pool_allocate_should_increment_pool_size() {
    struct redback_memory_pool *pool;
    pool = redback_memory_pool_new(sizeof(uint8_t), 1);

    TEST_ASSERT_NOT_NULL(pool);

    [[maybe_unused]] const void *ptr = redback_memory_pool_allocate(pool);

    TEST_ASSERT_EQUAL(redback_memory_pool_get_size(pool), 1);

    redback_memory_pool_deallocate(pool, ptr);
    redback_memory_pool_free(pool);
}

void redback_memory_pool_allocate_should_reuse_memory() {
    struct redback_memory_pool *pool;
    pool = redback_memory_pool_new(sizeof(uint8_t), 2);

    TEST_ASSERT_NOT_NULL(pool);

    [[maybe_unused]] const void *ptr1 = redback_memory_pool_allocate(pool);
    const void *ptr2 = redback_memory_pool_allocate(pool);

    redback_memory_pool_deallocate(pool, ptr2);

    const void *ptr3 = redback_memory_pool_allocate(pool);

    TEST_ASSERT_EQUAL(ptr2, ptr3);

    redback_memory_pool_free(pool);
}

void redback_memory_pool_allocate_should_not_allocate_beyond_capacity() {
    struct redback_memory_pool *pool;
    pool = redback_memory_pool_new(sizeof(uint8_t), 8);

    TEST_ASSERT_NOT_NULL(pool);

    for (size_t i = 0, capacity = redback_memory_pool_get_capacity(pool); i < capacity; ++i)
        (void)redback_memory_pool_allocate(pool);

    [[maybe_unused]] const void *ptr = redback_memory_pool_allocate(pool);

    TEST_ASSERT_NULL(ptr);

    redback_memory_pool_free(pool);
}

void redback_memory_pool_deallocate_should_decrement_pool_size() {
    struct redback_memory_pool *pool;
    pool = redback_memory_pool_new(sizeof(uint8_t), 1);

    TEST_ASSERT_NOT_NULL(pool);

    [[maybe_unused]] const void *ptr = redback_memory_pool_allocate(pool);
    redback_memory_pool_deallocate(pool, ptr);

    TEST_ASSERT_EQUAL(redback_memory_pool_get_size(pool), 0);

    redback_memory_pool_free(pool);
}
