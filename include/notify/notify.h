#ifndef __TODOX_NOTIFY_H__
#define __TODOX_NOTIFY_H__

#include <file/format.h>
#include <list/list.h>

/** @struct todox_notify_process_t
  * @brief a daemon process that wakes up for future alarms.
  */
typedef struct __todox_notify_process_t {
    /// path to the alarm config file
    const char *config_path;
    /// parsed and sorted alarm list
    todox_list alarm_list;
    /// index of the next alarm to fire
    size_t current_index;
    /// non-zero while the daemon should keep running
    int running;
} todox_notify_process_t;

/** @brief creates a notify process.
  * @param[in] config_path path to the alarm config file.
  * @return a new notify process, or NULL on failure.
  */
todox_notify_process_t *todox_notify_process_create(const char *config_path);

/** @brief destroys a notify process.
  * @param[in] proc a pointer to a notify process.
  */
void todox_notify_process_destroy(todox_notify_process_t *proc);

/** @brief runs the notify daemon loop.
  * @param[in] proc a pointer to a notify process.
  * @return 0 on normal stop, -1 on failure.
  */
int todox_notify_process_run(todox_notify_process_t *proc);

/** @brief sends a desktop notification via libdbus.
  * @param[in] title notification title.
  * @param[in] body notification body.
  * @return 0 on success, -1 on failure.
  */
int todox_send_desktop_notification(const char *title, const char *body);

/** @brief sleeps for a given number of milliseconds.
  * @param[in] ms milliseconds to sleep.
  */
void todox_msleep(long ms);

#endif
