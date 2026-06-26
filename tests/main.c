#include "test.h"

void run_list_tests(void);
void run_parse_tests(void);
void run_time_tests(void);
void run_repeat_tests(void);

void run_all_tests(void) {
    RUN_SUITE(list);
    RUN_SUITE(parse);
    RUN_SUITE(time);
    RUN_SUITE(repeat);
}

TEST_MAIN()
