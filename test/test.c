#include <unity.h>
#include <unity_internals.h>

void setUp() {}
void tearDown() {}

/* # */

void redback_memory_pool_new_should_init_pool_size();
void redback_memory_pool_new_should_copy_pool_unit_size();
void redback_memory_pool_new_should_copy_pool_capacity();
void redback_memory_pool_allocate_should_increment_pool_size();
void redback_memory_pool_allocate_should_reuse_memory();
void redback_memory_pool_allocate_should_not_allocate_beyond_capacity();
void redback_memory_pool_deallocate_should_decrement_pool_size();

/* # */

/* sample test */
void test() { TEST_ASSERT(true); }

int main(int argc, char *argv[]) {
    (void)argc, (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test);

    RUN_TEST(redback_memory_pool_new_should_init_pool_size);
    RUN_TEST(redback_memory_pool_new_should_copy_pool_unit_size);
    RUN_TEST(redback_memory_pool_new_should_copy_pool_capacity);
    RUN_TEST(redback_memory_pool_allocate_should_increment_pool_size);
    RUN_TEST(redback_memory_pool_allocate_should_reuse_memory);
    RUN_TEST(redback_memory_pool_allocate_should_not_allocate_beyond_capacity);
    RUN_TEST(redback_memory_pool_deallocate_should_decrement_pool_size);

    return UNITY_END();
}
