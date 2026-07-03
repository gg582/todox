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
    ASSERT_EQ_INT(1, itm.does_repeat);
}

TEST(is_triplet_positive) { ASSERT(is_triplet("a%%b%%c")); }

TEST(is_triplet_negative) { ASSERT(!is_triplet("a%%b")); }

TEST(parse_multi_triplet_valid) {
    todox_format_t itm = {0};
    ASSERT(is_multi_triplet("group_name%%my comment%%21:30:00 +0900"));
    int ret = parse_multi_triplet("group_name%%my comment%%21:30:00 +0900", &itm);
    ASSERT_EQ_INT(0, ret);
    ASSERT_EQ_STR("group_name", itm.task);
    ASSERT_EQ_STR("my comment", itm.comment);
    ASSERT_EQ_INT(0, itm.does_repeat);

    todox_format_t itm2 = {0};
    int ret2 = parse_multi_triplet("group_name%%my comment%%21:30:00 +0900%%repeat", &itm2);
    ASSERT_EQ_INT(0, ret2);
    ASSERT_EQ_STR("group_name", itm2.task);
    ASSERT_EQ_STR("my comment", itm2.comment);
    ASSERT_EQ_INT(1, itm2.does_repeat);
}

TEST(parse_multi_triplet_negative) {
    ASSERT(!is_multi_triplet("2026-06-25 21:00:00 +0900%%task%%comment"));
}

void run_parse_tests(void) {
    RUN_TEST(parse_full_timestamp_with_tz);
    RUN_TEST(parse_full_timestamp_without_tz);
    RUN_TEST(parse_time_only);
    RUN_TEST(parse_invalid_missing_task);
    RUN_TEST(parse_full_timestamp_with_repeat);
    RUN_TEST(is_triplet_positive);
    RUN_TEST(is_triplet_negative);
    RUN_TEST(parse_multi_triplet_valid);
    RUN_TEST(parse_multi_triplet_negative);
}
