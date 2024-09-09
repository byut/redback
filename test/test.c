#include <unity.h>
#include <unity_internals.h>

void setUp() {}
void tearDown() {}

/* sample test */
void test() { TEST_ASSERT(true); }

int main(int argc, char *argv[]) {
    (void)argc, (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test);
    return UNITY_END();
}
