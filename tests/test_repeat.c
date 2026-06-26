#include "test.h"
#include <repeat/repeat.h>
#include <string.h>
#include <time.h>

TEST(weekday_expr_single) {
    ASSERT(is_weekday_expr("mon%%task%%comment"));
    ASSERT(is_weekday_expr("tue%%task%%comment"));
    ASSERT(is_weekday_expr("sun%%task%%comment"));
}

TEST(weekday_expr_range) {
    ASSERT(is_weekday_expr("mon-fri%%task%%comment"));
}

TEST(weekday_expr_list) {
    ASSERT(is_weekday_expr("mon:wed:fri%%task%%comment"));
}

TEST(weekday_expr_negative) {
    ASSERT(!is_weekday_expr("2026-06-25 21:00:00 +0900%%task%%comment"));
    ASSERT(!is_weekday_expr("not%%task%%comment"));
}

TEST(weekday_expand_single) {
    todox_format_t items[7] = {0};
    int n = expand_weekday_triplet("mon%%task%%comment", items);
    ASSERT_EQ_INT(1, n);
    ASSERT_EQ_STR("task", items[0].task);
    ASSERT_EQ_STR("comment", items[0].comment);
    ASSERT_EQ_INT(1, items[0].repeat);
    ASSERT(items[0].ts != (time_t)-1);
}

TEST(weekday_expand_range) {
    todox_format_t items[7] = {0};
    int n = expand_weekday_triplet("mon-fri%%task%%comment", items);
    ASSERT_EQ_INT(5, n);
    for(int i = 0; i < n; i++) {
        ASSERT_EQ_STR("task", items[i].task);
        ASSERT_EQ_INT(1, items[i].repeat);
    }
}

TEST(weekday_expand_list) {
    todox_format_t items[7] = {0};
    int n = expand_weekday_triplet("mon:wed:fri%%task%%comment", items);
    ASSERT_EQ_INT(3, n);
}

TEST(next_weekly_occurrence) {
    time_t base = iso8601_to_time_t("2026-06-25 21:00:00 +0900");
    ASSERT(base != (time_t)-1);
    time_t next = todox_next_weekly_occurrence(base);
    ASSERT(next != (time_t)-1);
    ASSERT(next > base);
}

void run_repeat_tests(void) {
    RUN_TEST(weekday_expr_single);
    RUN_TEST(weekday_expr_range);
    RUN_TEST(weekday_expr_list);
    RUN_TEST(weekday_expr_negative);
    RUN_TEST(weekday_expand_single);
    RUN_TEST(weekday_expand_range);
    RUN_TEST(weekday_expand_list);
    RUN_TEST(next_weekly_occurrence);
}
