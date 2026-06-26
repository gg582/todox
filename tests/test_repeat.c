#include "test.h"
#include <repeat/repeat.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

TEST(weekday_expr_single) {
    ASSERT(is_weekday_expr("mon%%21:00:00 +0900%%task%%comment"));
    ASSERT(is_weekday_expr("tue%%21:00:00 +0900%%task%%comment"));
    ASSERT(is_weekday_expr("sun%%21:00:00 +0900%%task%%comment"));
}

TEST(weekday_expr_range) { ASSERT(is_weekday_expr("mon-fri%%21:00:00 +0900%%task%%comment")); }

TEST(weekday_expr_list) { ASSERT(is_weekday_expr("mon:wed:fri%%21:00:00 +0900%%task%%comment")); }

TEST(weekday_expr_negative) {
    ASSERT(!is_weekday_expr("mon%%task%%comment"));
    ASSERT(!is_weekday_expr("2026-06-25 21:00:00 +0900%%task%%comment"));
    ASSERT(!is_weekday_expr("not%%task%%comment"));
}

TEST(weekday_expand_single) {
    todox_format_t items[7] = {0};
    int n = expand_weekday_triplet("mon%%21:00:00 +0900%%task%%comment", items);
    ASSERT_EQ_INT(1, n);
    ASSERT_EQ_STR("task", items[0].task);
    ASSERT_EQ_STR("comment", items[0].comment);
    ASSERT_EQ_INT(1, items[0].repeat);
    ASSERT(items[0].ts != (time_t)-1);
}

TEST(weekday_expand_range) {
    todox_format_t items[7] = {0};
    int n = expand_weekday_triplet("mon-fri%%21:00:00 +0900%%task%%comment", items);
    ASSERT_EQ_INT(5, n);
    for(int i = 0; i < n; i++) {
        ASSERT_EQ_STR("task", items[i].task);
        ASSERT_EQ_INT(1, items[i].repeat);
    }
}

TEST(weekday_expand_list) {
    todox_format_t items[7] = {0};
    int n = expand_weekday_triplet("mon:wed:fri%%21:00:00 +0900%%task%%comment", items);
    ASSERT_EQ_INT(3, n);
}

TEST(weekday_expand_no_tz_matches_explicit) {
    const char *orig_tz = getenv("TZ");
    setenv("TZ", "KST-9", 1);
    tzset();

    todox_format_t with_tz[7] = {0};
    todox_format_t without_tz[7] = {0};

    int n1 = expand_weekday_triplet("mon%%21:00:00 +0900%%task%%comment", with_tz);
    int n2 = expand_weekday_triplet("mon%%21:00:00%%task%%comment", without_tz);

    if(orig_tz != NULL) {
        setenv("TZ", orig_tz, 1);
    } else {
        unsetenv("TZ");
    }
    tzset();

    ASSERT_EQ_INT(n1, n2);
    ASSERT_EQ_INT(1, n1);
    ASSERT_EQ_INT(with_tz[0].ts, without_tz[0].ts);
}

TEST(weekday_expand_missing_time) {
    todox_format_t items[7] = {0};
    int n = expand_weekday_triplet("mon%%task%%comment", items);
    ASSERT_EQ_INT(-1, n);
}

TEST(next_weekly_occurrence) {
    time_t base = iso8601_to_time_t("2026-06-25 21:00:00 +0900");
    ASSERT(base != (time_t)-1);
    time_t next = todox_next_weekly_occurrence(base);
    ASSERT(next != (time_t)-1);
    ASSERT(next > base);
}

TEST(advance_repeat_to_future_once) {
    time_t base = iso8601_to_time_t("2026-06-25 21:00:00 +0900");
    time_t now = iso8601_to_time_t("2026-06-26 21:00:00 +0900");
    ASSERT(base != (time_t)-1);
    ASSERT(now != (time_t)-1);
    time_t next = todox_advance_repeat_to_future(base, now);
    ASSERT(next > now);
    time_t expected = todox_next_weekly_occurrence(base);
    ASSERT_EQ_INT((int)expected, (int)next);
}

TEST(advance_repeat_to_future_multiple_weeks) {
    time_t base = iso8601_to_time_t("2026-06-25 21:00:00 +0900");
    time_t now = iso8601_to_time_t("2026-07-20 21:00:00 +0900");
    ASSERT(base != (time_t)-1);
    ASSERT(now != (time_t)-1);
    time_t next = todox_advance_repeat_to_future(base, now);
    ASSERT(next > now);
}

void run_repeat_tests(void) {
    RUN_TEST(weekday_expr_single);
    RUN_TEST(weekday_expr_range);
    RUN_TEST(weekday_expr_list);
    RUN_TEST(weekday_expr_negative);
    RUN_TEST(weekday_expand_single);
    RUN_TEST(weekday_expand_range);
    RUN_TEST(weekday_expand_list);
    RUN_TEST(weekday_expand_no_tz_matches_explicit);
    RUN_TEST(weekday_expand_missing_time);
    RUN_TEST(next_weekly_occurrence);
    RUN_TEST(advance_repeat_to_future_once);
    RUN_TEST(advance_repeat_to_future_multiple_weeks);
}
