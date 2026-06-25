#ifndef __TODOX_FORMAT_H__
#define __TODOX_FORMAT_H__

#include <stddef.h>
#include <stdio.h>
#include <time.h>

#define TODOX_TIME_FORMAT "%Y-%m-%d %H:%M:%S %z"
#define TODOX_TIME_COMPAT_FORMAT "%Y-%m-%d %H:%M:%S"

#define TODOX_ALARM_MAX_LEN 1024
#define TODOX_ALARM_TABLE_MAX_ROWS 128
#define TODOX_ALARM_TASK_MAX_LEN 256
#define TODOX_ALARM_COMMENT_MAX_LEN 256

/** @struct todox_format_t
  * @brief  a data format of todolist.
  */
typedef struct __todox_format_t {
    /// unix timestamp
    time_t ts;
    /// a name of a task
    char task[TODOX_ALARM_TASK_MAX_LEN];
    /// a comment section of a task
    char comment[TODOX_ALARM_COMMENT_MAX_LEN];
} todox_format_t;

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

/** @brief converts an ISO 8601 datetime string to time_t.
  * @param[in] ts an ISO 8601 datetime string.
  * @return a unix timestamp, or (time_t)-1 on failure.
  */
time_t iso8601_to_time_t(const char *ts);

/** @brief parses a config file into a todox list.
  * @param[in] config a path to a config file.
  * @return a parsed todox list.
  */
todox_list todox_parse_config(const char *config);

/** @brief writes a todox list to a config file.
  * @param[in] config a path to a config file.
  * @param[in] lst a pointer to a todox list.
  * @return 0 on success, -1 on failure.
  */
int todox_write_config(const char *config, const todox_list *lst);

#endif
