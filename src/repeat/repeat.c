#define _XOPEN_SOURCE 700

#include <repeat/repeat.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#define TODOX_REPEAT_DEFAULT_HOUR 9
#define TODOX_REPEAT_DEFAULT_MIN  0
#define TODOX_REPEAT_DEFAULT_SEC  0

static void trim(char *s) {
    size_t len = strlen(s);
    while(len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t' || s[len - 1] == '\r' || s[len - 1] == '\n')) {
        s[len - 1] = '\0';
        len--;
    }
}

/** @brief converts a weekday abbreviation to a tm_wday value. */
static int parse_single_wday(const char *s) {
    if(strcasecmp(s, "mon") == 0) return 1;
    if(strcasecmp(s, "tue") == 0) return 2;
    if(strcasecmp(s, "wed") == 0) return 3;
    if(strcasecmp(s, "thu") == 0) return 4;
    if(strcasecmp(s, "fri") == 0) return 5;
    if(strcasecmp(s, "sat") == 0) return 6;
    if(strcasecmp(s, "sun") == 0) return 0;
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

/** @brief computes the next occurrence of a weekday at a fixed local time. */
static time_t next_weekday_timestamp(int wday, int hour, int min, int sec) {
    time_t now = time(NULL);
    struct tm tm_now;
    localtime_r(&now, &tm_now);

    struct tm target = tm_now;
    target.tm_hour = hour;
    target.tm_min = min;
    target.tm_sec = sec;
    target.tm_isdst = -1;

    int delta = (wday - tm_now.tm_wday + 7) % 7;
    if(delta == 0 && (tm_now.tm_hour > hour ||
                      (tm_now.tm_hour == hour && tm_now.tm_min > min) ||
                      (tm_now.tm_hour == hour && tm_now.tm_min == min && tm_now.tm_sec >= sec))) {
        delta = 7;
    }
    target.tm_mday += delta;
    return mktime(&target);
}

int is_weekday_expr(const char *s) {
    char buf[TODOX_ALARM_MAX_LEN];
    strncpy(buf, s, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    trim(buf);

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

    char *sep = strstr(buf, "%%");
    if(sep == NULL) {
        return -1;
    }
    *sep = '\0';
    const char *expr = buf;
    char *rest = sep + 2;

    char *task_sep = strstr(rest, "%%");
    if(task_sep == NULL) {
        return -1;
    }
    *task_sep = '\0';
    char *task = rest;
    char *comment = task_sep + 2;

    trim(task);
    trim(comment);

    int wdays[7];
    size_t count = 0;
    if(parse_weekday_expr(expr, wdays, &count) != 0 || count == 0 || count > 7) {
        return -1;
    }

    for(size_t i = 0; i < count; i++) {
        out[i].ts = next_weekday_timestamp(wdays[i], TODOX_REPEAT_DEFAULT_HOUR, TODOX_REPEAT_DEFAULT_MIN, TODOX_REPEAT_DEFAULT_SEC);
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
