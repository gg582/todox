#include "test.h"
#include <file/format.h>
#include <time.h>

TEST(time_with_tz) {
    time_t t = iso8601_to_time_t("2026-06-25 21:00:00 +0900");
    ASSERT(t != (time_t)-1);
    struct tm *gmt = gmtime(&t);
    ASSERT(gmt != NULL);
    ASSERT_EQ_INT(2026, gmt->tm_year + 1900);
    ASSERT_EQ_INT(6, gmt->tm_mon + 1);
    ASSERT_EQ_INT(25, gmt->tm_mday);
    ASSERT_EQ_INT(12, gmt->tm_hour);
    ASSERT_EQ_INT(0, gmt->tm_min);
    ASSERT_EQ_INT(0, gmt->tm_sec);
}

TEST(time_without_tz) {
    time_t t = iso8601_to_time_t("2026-06-25 12:00:00");
    ASSERT(t != (time_t)-1);
    struct tm *local = localtime(&t);
    ASSERT(local != NULL);
    ASSERT_EQ_INT(2026, local->tm_year + 1900);
    ASSERT_EQ_INT(6, local->tm_mon + 1);
    ASSERT_EQ_INT(25, local->tm_mday);
    ASSERT_EQ_INT(12, local->tm_hour);
}

TEST(time_invalid) {
    time_t t = iso8601_to_time_t("not a date");
    ASSERT(t == (time_t)-1);
}

void run_time_tests(void) {
    RUN_TEST(time_with_tz);
    RUN_TEST(time_without_tz);
    RUN_TEST(time_invalid);
}
