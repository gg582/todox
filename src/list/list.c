#include <list/list.h>
#include <stdlib.h>
#include <string.h>

/** @brief initializes a todox list.
  * @param[in] lst a pointer to a todox list.
  */
void todox_task_init(todox_list *lst) {
    lst->tasks = malloc(sizeof(todox_format_t) * TODOX_ALARM_TABLE_MAX_ROWS);
    lst->len = 0U;
}

/** @brief pushes an item to a todox list, keeping it sorted by timestamp.
  * @param[in] lst a pointer to a todox list.
  * @param[in] itm an item to push
  */
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

/** @brief finds an item from a todox list.
  * @param[in] lst a pointer to a todox list.
  * @param[in] task a task name to find.
  * @return an index of the item, or (unsigned)-1 if not found.
  */
unsigned todox_task_find(todox_list *lst, const char *task) {
    for(size_t idx = 0U; idx < lst->len; idx++) {
        if(strncmp(lst->tasks[idx].task, task, TODOX_ALARM_TASK_MAX_LEN) == 0) {
            return (unsigned)idx;
        }
    }
    return (unsigned)-1;
}

/** @brief removes an item from a todox list.
  * @param[in] lst a pointer to a todox list.
  * @param[in] task a task name to remove.
  * @return a string that indicates an item to remove, or NULL if not found.
  */
const char *todox_task_remove(todox_list *lst, const char *task)
{
    unsigned idx = todox_task_find(lst, task);
    if(idx == (unsigned)-1) {
        return NULL;
    }
    const char *removed = lst->tasks[idx].task;
    for(size_t i = idx; i + 1 < lst->len; i++) {
        lst->tasks[i] = lst->tasks[i + 1];
    }
    lst->len--;
    return removed;
}
