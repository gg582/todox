#ifndef __TODOX_LIST_H__
#define __TODOX_LIST_H__

#include <file/format.h>
#include <stddef.h>

/** @struct todox_list
  * @brief a sorted list of todox items.
  */
typedef struct __todox_list {
    todox_format_t *tasks;
    size_t len;
} todox_list;

/** @brief initializes a todox list.
  * @param[in] lst a pointer to a todox list.
  */
void todox_task_init(todox_list *lst);

/** @brief pushes an item to a todox list and keeps it sorted by timestamp.
  * @param[in] lst a pointer to a todox list.
  * @param[in] itm an item to push.
  */
void todox_task_push(todox_list *lst, todox_format_t itm);

/** @brief finds an item from a todox list.
  * @param[in] lst a pointer to a todox list.
  * @param[in] task a task name to find.
  * @return an index of the item, or (unsigned)-1 if not found.
  */
unsigned todox_task_find(todox_list *lst, const char *task);

/** @brief removes an item from a todox list.
  * @param[in] lst a pointer to a todox list.
  * @param[in] task a task name to remove.
  * @return a removed task name, or NULL if not found.
  */
const char *todox_task_remove(todox_list *lst, const char *task);

/** @brief removes all items matching a task name from a todox list.
  * @param[in] lst a pointer to a todox list.
  * @param[in] task a task name to remove.
  * @return the number of removed items.
  */
size_t todox_task_remove_all(todox_list *lst, const char *task);

#endif
