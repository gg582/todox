#ifndef __TODOX_IO_H__
#define __TODOX_IO_H__

#include <file/format.h>
#include <list/list.h>

/** @brief determines the alarm file path.
 * @param[in] argc argument count.
 * @param[in] argv argument vector.
 * @param[in] idx index to check in argv.
 * @return a path from argv, TODOX_ALARM_FILE, or "alarm.txt".
 */
const char *get_alarm_file(int argc, char **argv, int idx);

/** @brief prints tasks to stdout.
 * @param[in] lst a pointer to a todox list.
 */
void print_tasks(const todox_list *lst);

#endif
