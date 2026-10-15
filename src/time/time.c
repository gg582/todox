#define _DEFAULT_SOURCE
#ifdef __linux__
#define _GNU_SOURCE
#endif

#include <file/format.h>
#include <error/error.h>
#include <repeat/repeat.h>

#include <time.h>
#include <string.h>

#if defined(__OpenBSD__)
/* FIXME: it is deprecated, but somehow build fails on OpenBSD when removing this. */
#define _XOPEN_SOURCE 700
#define _BSD_SOURCE
#include <sys/types.h>
#endif 

#include <ctype.h>

/** @brief tries parsing a datetime string without timezone. */
static inline char *try_convert_without_tz(const char *s, struct tm *tm_time) {
    return strptime(s, TODOX_TIME_COMPAT_FORMAT, tm_time);
}

/** @brief parses an ISO 8601 datetime string into a time_t. */
time_t iso8601_to_time_t(const char *ts) {
    struct tm tm_time = {0};
    time_t t = (time_t)-1;
    bool has_tz = false;

    // First, parse a timestamp with an explicit timezone.
    if(strptime(ts, TODOX_TIME_FORMAT, &tm_time) != nullptr) {
        has_tz = true;
    // If there is NO timestamp, retry conversion without explicit timezone.
    } else if(try_convert_without_tz(ts, &tm_time) == nullptr) {
        todox_notify(TODOX_ERROR("wrong time format", WARN, TODOX_WRONG_TIMESTAMP));
        return t;
    }

    // If a system has a timezone, apply its offset.
    if(has_tz) {
        t = timegm(&tm_time);
        if(t != (time_t)-1) {
            t -= parse_tz_offset(ts);
        }
    } else {
        t = mktime(&tm_time);
    }

    // If time == -1, it means we failed to parse any information.
    // Display WARN without killing a program.
    if(t == (time_t)-1) {
        todox_notify(
            TODOX_ERROR("failed to convert into posix timestamp", WARN, TODOX_WRONG_TIMESTAMP));
    }
    return t;
}
