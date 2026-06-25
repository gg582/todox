#ifndef __TODOX_TEST_H__
#define __TODOX_TEST_H__

#include <stdio.h>
#include <string.h>

static int g_tests_run = 0;
static int g_tests_failed = 0;

#define TEST(name) static void test_##name(void)

#define RUN_TEST(name) do { \
    g_tests_run++; \
    printf("  running %s ... ", #name); \
    test_##name(); \
    printf("OK\n"); \
} while(0)

#define ASSERT(cond) do { \
    if(!(cond)) { \
        g_tests_failed++; \
        printf("FAILED\n    assertion: %s\n    at %s:%d\n", #cond, __FILE__, __LINE__); \
        return; \
    } \
} while(0)

#define ASSERT_EQ_INT(expected, actual) \
    ASSERT((expected) == (actual))

#define ASSERT_EQ_STR(expected, actual) \
    ASSERT(strcmp((expected), (actual)) == 0)

#define RUN_SUITE(name) do { \
    printf("\n%s\n", #name); \
    run_##name##_tests(); \
} while(0)

#define TEST_MAIN() int main(void) { \
    run_all_tests(); \
    printf("\n%d tests run, %d failed\n", g_tests_run, g_tests_failed); \
    return g_tests_failed > 0 ? 1 : 0; \
}

#endif
