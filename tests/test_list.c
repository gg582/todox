#include "test.h"
#include <list/list.h>
#include <stdlib.h>
#include <string.h>

TEST(list_init_empty) {
    todox_list lst;
    todox_task_init(&lst);
    ASSERT_EQ_INT(0U, (unsigned)lst.len);
    free(lst.tasks);
}

TEST(list_push_sorted) {
    todox_list lst;
    todox_task_init(&lst);

    todox_format_t a = {.ts = 100, .task = "task-a", .comment = "comment-a"};
    todox_format_t b = {.ts = 50, .task = "task-b", .comment = "comment-b"};
    todox_format_t c = {.ts = 150, .task = "task-c", .comment = "comment-c"};

    todox_task_push(&lst, a);
    todox_task_push(&lst, b);
    todox_task_push(&lst, c);

    ASSERT_EQ_INT(3U, (unsigned)lst.len);
    ASSERT_EQ_INT(50, (int)lst.tasks[0].ts);
    ASSERT_EQ_INT(100, (int)lst.tasks[1].ts);
    ASSERT_EQ_INT(150, (int)lst.tasks[2].ts);
    ASSERT_EQ_STR("task-b", lst.tasks[0].task);
    ASSERT_EQ_STR("task-a", lst.tasks[1].task);
    ASSERT_EQ_STR("task-c", lst.tasks[2].task);

    free(lst.tasks);
}

TEST(list_find_and_remove) {
    todox_list lst;
    todox_task_init(&lst);

    todox_format_t a = {.ts = 100, .task = "alpha", .comment = ""};
    todox_format_t b = {.ts = 200, .task = "beta", .comment = ""};
    todox_format_t c = {.ts = 300, .task = "gamma", .comment = ""};

    todox_task_push(&lst, a);
    todox_task_push(&lst, b);
    todox_task_push(&lst, c);

    ASSERT_EQ_INT(1U, todox_task_find(&lst, "beta"));
    ASSERT_EQ_INT((unsigned)-1, todox_task_find(&lst, "delta"));

    const char *removed = todox_task_remove(&lst, "beta");
    ASSERT(removed != NULL);
    ASSERT_EQ_STR("beta", removed);
    ASSERT_EQ_INT(2U, (unsigned)lst.len);
    ASSERT_EQ_INT((unsigned)-1, todox_task_find(&lst, "beta"));

    free(lst.tasks);
}

void run_list_tests(void) {
    RUN_TEST(list_init_empty);
    RUN_TEST(list_push_sorted);
    RUN_TEST(list_find_and_remove);
}
