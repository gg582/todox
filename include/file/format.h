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
    /// non-zero when the alarm repeats weekly
    int repeat;
} todox_format_t;

/** @brief converts an ISO 8601 datetime string to time_t.
  * @param[in] ts an ISO 8601 datetime string.
  * @return a unix timestamp, or (time_t)-1 on failure.
  */
time_t iso8601_to_time_t(const char *ts);

#endif
