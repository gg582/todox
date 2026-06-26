#include "test.h"
#include <app/parse.h>
#include <string.h>
#include <time.h>

TEST(parse_full_timestamp_with_tz) {
    todox_format_t itm = {0};
    int ret = parse_triplet("2026-06-25 21:00:00 +0900%%task%%comment", &itm);
    ASSERT_EQ_INT(0, ret);
    ASSERT_EQ_STR("task", itm.task);
    ASSERT_EQ_STR("comment", itm.comment);
}

TEST(parse_full_timestamp_without_tz) {
    todox_format_t itm = {0};
    int ret = parse_triplet("2026-06-25 21:00:00%%task%%comment", &itm);
    ASSERT_EQ_INT(0, ret);
    ASSERT_EQ_STR("task", itm.task);
    ASSERT_EQ_STR("comment", itm.comment);
}

TEST(parse_time_only) {
    todox_format_t itm = {0};
    int ret = parse_triplet("21:30:00 +0900%%task%%comment", &itm);
    ASSERT_EQ_INT(0, ret);
    ASSERT_EQ_STR("task", itm.task);
    ASSERT_EQ_STR("comment", itm.comment);
}

TEST(parse_invalid_missing_task) {
    todox_format_t itm = {0};
    int ret = parse_triplet("2026-06-25 21:00:00 +0900%%comment", &itm);
    ASSERT_EQ_INT(-1, ret);
}

TEST(parse_full_timestamp_with_repeat) {
    todox_format_t itm = {0};
    int ret = parse_triplet("2026-06-25 21:00:00 +0900%%task%%comment%%repeat", &itm);
    ASSERT_EQ_INT(0, ret);
    ASSERT_EQ_STR("task", itm.task);
    ASSERT_EQ_STR("comment", itm.comment);
    ASSERT_EQ_INT(1, itm.repeat);
}

TEST(is_triplet_positive) {
    ASSERT(is_triplet("a%%b%%c"));
}

TEST(is_triplet_negative) {
    ASSERT(!is_triplet("a%%b"));
}

void run_parse_tests(void) {
    RUN_TEST(parse_full_timestamp_with_tz);
    RUN_TEST(parse_full_timestamp_without_tz);
    RUN_TEST(parse_time_only);
    RUN_TEST(parse_invalid_missing_task);
    RUN_TEST(parse_full_timestamp_with_repeat);
    RUN_TEST(is_triplet_positive);
    RUN_TEST(is_triplet_negative);
}
