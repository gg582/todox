#ifndef TODOX_COMPAT_BSD_TIME_H
#define TODOX_COMPAT_BSD_TIME_H

#include <time.h>

/** @brief BSD-compatible replacement for GNU timegm().
 * @details converts a UTC struct tm to a time_t without relying on GNU
 *          extensions or timezone environment variables.
 * @param[in,out] tm a broken-down time in UTC.
 * @return seconds since the Unix epoch, or (time_t)-1 on failure.
 */
time_t bsd_timegm(struct tm *tm);

#endif
