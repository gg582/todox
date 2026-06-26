#define _XOPEN_SOURCE 700
#ifdef __linux__
#define _GNU_SOURCE
#endif

#include <repeat/repeat.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <ctype.h>

#ifndef __linux__
#include <compat/bsd/time.h>
#endif

static void trim(char *s) {
    char *start = s;

    while(*start != '\0' && isspace((unsigned char)*start)) {
        start++;
    }

    if(start != s) {
        memmove(s, start, strlen(start) + 1);
    }

    size_t len = strlen(s);
    while(len > 0 && isspace((unsigned char)s[len - 1])) {
        s[--len] = '\0';
    }
}

/** @brief converts a weekday abbreviation to a tm_wday value. */
static int parse_single_wday(const char *s) {
    if(strcasecmp(s, "mon") == 0)
        return 1;
    if(strcasecmp(s, "tue") == 0)
        return 2;
    if(strcasecmp(s, "wed") == 0)
        return 3;
    if(strcasecmp(s, "thu") == 0)
        return 4;
    if(strcasecmp(s, "fri") == 0)
        return 5;
    if(strcasecmp(s, "sat") == 0)
        return 6;
    if(strcasecmp(s, "sun") == 0)
        return 0;
    return -1;
}

/** @brief expands a weekday expression into a list of tm_wday values. */
static int parse_weekday_expr(const char *expr, int *wdays, size_t *count) {
    char buf[32];
    strncpy(buf, expr, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    *count = 0;

    char *dash = strchr(buf, '-');
    if(dash != NULL) {
        *dash = '\0';
        int start = parse_single_wday(buf);
        int end = parse_single_wday(dash + 1);
        if(start < 0 || end < 0) {
            return -1;
        }
        int i = start;
        while(1) {
            wdays[(*count)++] = i;
            if(i == end) {
                break;
            }
            i = (i + 1) % 7;
            if(*count > 7) {
                return -1;
            }
        }
        return 0;
    }

    char *colon = strchr(buf, ':');
    if(colon != NULL) {
        char *saveptr = NULL;
        char *token = strtok_r(buf, ":", &saveptr);
        while(token != NULL) {
            int w = parse_single_wday(token);
            if(w < 0) {
                return -1;
            }
            wdays[(*count)++] = w;
            if(*count > 7) {
                return -1;
            }
            token = strtok_r(NULL, ":", &saveptr);
        }
        return 0;
    }

    int w = parse_single_wday(buf);
    if(w < 0) {
        return -1;
    }
    wdays[(*count)++] = w;
    return 0;
}

/** @brief parses a timezone offset of the form +HHMM or -HHMM into seconds. */
static int parse_tz_offset(const char *s) {
    size_t len = strlen(s);
    if(len < 5) {
        return 0;
    }
    const char *tz = s + len - 5;
    if((tz[0] == '+' || tz[0] == '-') && isdigit((unsigned char)tz[1]) &&
       isdigit((unsigned char)tz[2]) && isdigit((unsigned char)tz[3]) &&
       isdigit((unsigned char)tz[4])) {
        int sign = (tz[0] == '+') ? 1 : -1;
        int hours = (tz[1] - '0') * 10 + (tz[2] - '0');
        int mins = (tz[3] - '0') * 10 + (tz[4] - '0');
        return sign * (hours * 3600 + mins * 60);
    }
    return 0;
}

/** @brief parses a time field as HH:MM:SS with an optional +HHMM/-HHMM offset. */
static int parse_time_field(const char *s, int *hour, int *min, int *sec, int *tz_offset) {
    char buf[64];
    strncpy(buf, s, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    trim(buf);

    if(strlen(buf) == 0) {
        return -1;
    }

    struct tm tm_time = {0};
    if(strptime(buf, "%H:%M:%S %z", &tm_time) != NULL) {
        *hour = tm_time.tm_hour;
        *min = tm_time.tm_min;
        *sec = tm_time.tm_sec;
        *tz_offset = parse_tz_offset(buf);
        return 0;
    }
    if(strptime(buf, "%H:%M:%S", &tm_time) != NULL) {
        *hour = tm_time.tm_hour;
        *min = tm_time.tm_min;
        *sec = tm_time.tm_sec;
        *tz_offset = 0;
        return 0;
    }
    return -1;
}

/** @brief computes the next occurrence of a weekday at the given time in a timezone-aware way. */
static time_t next_weekday_timestamp(int wday, int hour, int min, int sec, int tz_offset) {
    time_t now = time(NULL);
    time_t now_in_tz = now + tz_offset;
    struct tm tm_now;
    gmtime_r(&now_in_tz, &tm_now);

    struct tm target = tm_now;
    target.tm_hour = hour;
    target.tm_min = min;
    target.tm_sec = sec;
    target.tm_isdst = 0;

    int delta = (wday - tm_now.tm_wday + 7) % 7;
    if(delta == 0 && (tm_now.tm_hour > hour || (tm_now.tm_hour == hour && tm_now.tm_min > min) ||
                      (tm_now.tm_hour == hour && tm_now.tm_min == min && tm_now.tm_sec >= sec))) {
        delta = 7;
    }
    target.tm_mday += delta;
    return timegm(&target) - tz_offset;
}

/** @brief counts how many '%%' separated fields are in a string. */
static size_t count_fields(const char *s) {
    size_t count = 1;
    const char *p = s;
    while((p = strstr(p, "%%")) != NULL) {
        count++;
        p += 2;
    }
    return count;
}

int is_weekday_expr(const char *s) {
    char buf[TODOX_ALARM_MAX_LEN];
    strncpy(buf, s, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    trim(buf);

    if(count_fields(buf) < 4) {
        return 0;
    }

    char *sep = strstr(buf, "%%");
    if(sep != NULL) {
        *sep = '\0';
    }

    int wdays[7];
    size_t count = 0;
    if(parse_weekday_expr(buf, wdays, &count) != 0 || count == 0) {
        return 0;
    }
    return 1;
}

int expand_weekday_triplet(const char *s, todox_format_t *out) {
    char buf[TODOX_ALARM_MAX_LEN];
    strncpy(buf, s, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    trim(buf);

    if(count_fields(buf) < 4) {
        return -1;
    }

    char *sep = strstr(buf, "%%");
    if(sep == NULL) {
        return -1;
    }
    *sep = '\0';
    const char *expr = buf;
    char *rest = sep + 2;

    char *time_sep = strstr(rest, "%%");
    if(time_sep == NULL) {
        return -1;
    }
    *time_sep = '\0';
    char *time_field = rest;
    char *task_comment = time_sep + 2;

    char *task_sep = strstr(task_comment, "%%");
    if(task_sep == NULL) {
        return -1;
    }
    *task_sep = '\0';
    char *task = task_comment;
    char *comment = task_sep + 2;

    trim(time_field);
    trim(task);
    trim(comment);

    int hour, min, sec, tz_offset;
    if(parse_time_field(time_field, &hour, &min, &sec, &tz_offset) != 0) {
        return -1;
    }

    int wdays[7];
    size_t count = 0;
    if(parse_weekday_expr(expr, wdays, &count) != 0 || count == 0 || count > 7) {
        return -1;
    }

    for(size_t i = 0; i < count; i++) {
        out[i].ts = next_weekday_timestamp(wdays[i], hour, min, sec, tz_offset);
        out[i].repeat = 1;
        strncpy(out[i].task, task, TODOX_ALARM_TASK_MAX_LEN - 1);
        out[i].task[TODOX_ALARM_TASK_MAX_LEN - 1] = '\0';
        strncpy(out[i].comment, comment, TODOX_ALARM_COMMENT_MAX_LEN - 1);
        out[i].comment[TODOX_ALARM_COMMENT_MAX_LEN - 1] = '\0';
    }
    return (int)count;
}

time_t todox_next_weekly_occurrence(time_t t) {
    struct tm tm_time;
    localtime_r(&t, &tm_time);
    tm_time.tm_mday += 7;
    return mktime(&tm_time);
}
