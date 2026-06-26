#include <list/list.h>
#include <stdlib.h>
#include <string.h>

/** @brief initializes a todox list (implementation). */
void todox_task_init(todox_list *lst) {
    lst->tasks = malloc(sizeof(todox_format_t) * TODOX_ALARM_TABLE_MAX_ROWS);
    lst->len = 0U;
}

/** @brief pushes an item to a todox list, keeping it sorted by timestamp (implementation). */
void todox_task_push(todox_list *lst, todox_format_t itm)
{
    if(lst->len >= TODOX_ALARM_TABLE_MAX_ROWS) {
        return;
    }
    size_t pos = lst->len;
    while(pos > 0 && lst->tasks[pos - 1].ts > itm.ts) {
        lst->tasks[pos] = lst->tasks[pos - 1];
        pos--;
    }
    lst->tasks[pos] = itm;
    lst->len++;
}

/** @brief finds an item from a todox list (implementation). */
unsigned todox_task_find(todox_list *lst, const char *task) {
    for(size_t idx = 0U; idx < lst->len; idx++) {
        if(strncmp(lst->tasks[idx].task, task, TODOX_ALARM_TASK_MAX_LEN) == 0) {
            return (unsigned)idx;
        }
    }
    return (unsigned)-1;
}

/** @brief removes an item from a todox list (implementation). */
const char *todox_task_remove(todox_list *lst, const char *task)
{
    static char removed_task[TODOX_ALARM_TASK_MAX_LEN];
    unsigned idx = todox_task_find(lst, task);
    if(idx == (unsigned)-1) {
        return NULL;
    }
    strncpy(removed_task, lst->tasks[idx].task, TODOX_ALARM_TASK_MAX_LEN - 1);
    removed_task[TODOX_ALARM_TASK_MAX_LEN - 1] = '\0';
    for(size_t i = idx; i + 1 < lst->len; i++) {
        lst->tasks[i] = lst->tasks[i + 1];
    }
    lst->len--;
    return removed_task;
}

/** @brief removes all items matching a task name from a todox list (implementation). */
size_t todox_task_remove_all(todox_list *lst, const char *task)
{
    size_t removed = 0;
    size_t i = 0;
    while(i < lst->len) {
        if(strncmp(lst->tasks[i].task, task, TODOX_ALARM_TASK_MAX_LEN) == 0) {
            for(size_t j = i; j + 1 < lst->len; j++) {
                lst->tasks[j] = lst->tasks[j + 1];
            }
            lst->len--;
            removed++;
        } else {
            i++;
        }
    }
    return removed;
}
