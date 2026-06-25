#define _GNU_SOURCE
#define _XOPEN_SOURCE 700

#include <file/format.h>
#include <error/error.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

/**
  * @brief parses an ISO 8601 datetime string into a UTC time_t.
  * @details accepts "YYYY-MM-DD HH:MM:SS" (interpreted in the system timezone) or
  *          "YYYY-MM-DD HH:MM:SS +HHMM" (converted to UTC).
  * @param[in] ts a null-terminated datetime string.
  * @return seconds since the Unix epoch, or (time_t)-1 on failure.
  */

static inline char *try_convert_without_tz(const char *s, struct tm *tm_time)
{
    return strptime(s, TODOX_TIME_COMPAT_FORMAT, tm_time);
}

static int parse_tz_offset(const char *s) {
    size_t len = strlen(s);
    if(len < 5) {
        return 0;
    }
    const char *tz = s + len - 5;
    if((tz[0] == '+' || tz[0] == '-') &&
       isdigit((unsigned char)tz[1]) && isdigit((unsigned char)tz[2]) &&
       isdigit((unsigned char)tz[3]) && isdigit((unsigned char)tz[4])) {
        int sign = (tz[0] == '+') ? 1 : -1;
        int hours = (tz[1] - '0') * 10 + (tz[2] - '0');
        int mins  = (tz[3] - '0') * 10 + (tz[4] - '0');
        return sign * (hours * 3600 + mins * 60);
    }
    return 0;
}

time_t iso8601_to_time_t(const char *ts)
{
    struct tm tm_time = {0};
    time_t t = (time_t)-1;
    int has_tz = 0;

    if(strptime(ts, TODOX_TIME_FORMAT, &tm_time) != NULL) {
        has_tz = 1;
    } else if(try_convert_without_tz(ts, &tm_time) == NULL) {
        todox_notify(TODOX_ERROR("wrong time format", WARN, TODOX_WRONG_TIMESTAMP));
        return t;
    }

    if(has_tz) {
        t = timegm(&tm_time);
        if(t != (time_t)-1) {
            t -= parse_tz_offset(ts);
        }
    } else {
        t = mktime(&tm_time);
    }

    if(t == (time_t)-1) {
        todox_notify(TODOX_ERROR("failed to convert into posix timestamp", WARN, TODOX_WRONG_TIMESTAMP));
    }
    return t;
}
