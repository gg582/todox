#include <app/parse.h>
#include <file/format.h>
#include <error/error.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

static void trim(char *s) {
    size_t len = strlen(s);
    while(len > 0 &&
          (s[len - 1] == ' ' || s[len - 1] == '\t' || s[len - 1] == '\r' || s[len - 1] == '\n')) {
        s[len - 1] = '\0';
        len--;
    }
}

static inline int looks_like_time_only(const char *s) {
    return isdigit((unsigned char)s[0]) && isdigit((unsigned char)s[1]) && s[2] == ':' &&
           isdigit((unsigned char)s[3]) && isdigit((unsigned char)s[4]) && s[5] == ':' &&
           isdigit((unsigned char)s[6]) && isdigit((unsigned char)s[7]);
}

/* assume today for time-only timestamps */
static void prepend_today(char *buf, size_t bufsize) {
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    char date_prefix[32] = {0};
    if(tm_now == NULL) {
        return;
    }

    strftime(date_prefix, sizeof(date_prefix), "%Y-%m-%d ", tm_now);
    size_t prefix_len = strlen(date_prefix);
    size_t buf_len = strlen(buf);
    if(prefix_len + buf_len + 1 > bufsize) {
        return;
    }
    
    // WARN: DANGEROUS MEMORY MANAGEMENT VIA memmove+memcpy
    // memmove+memcpy is a rough call; currently I have no idea to refactor it, but when the format goes longer I should fix.
    memmove(buf + prefix_len, buf, buf_len + 1);
    memcpy(buf, date_prefix, prefix_len);
}

bool is_triplet(const char *s) {
    const char *p1 = strstr(s, "%%");
    if(p1 == NULL) {
        return false;
    }
    return strstr(p1 + 2, "%%") != NULL;
}

int parse_triplet(const char *s, todox_format_t *out) {
    char buf[TODOX_ALARM_MAX_LEN];
    strncpy(buf, s, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    trim(buf);

    if(looks_like_time_only(buf)) {
        prepend_today(buf, sizeof(buf));
    }

    char *p1 = strstr(buf, "%%");
    if(p1 == NULL) {
        return -1;
    }
    *p1 = '\0';
    p1 += 2;

    char *p2 = strstr(p1, "%%");
    if(p2 == NULL) {
        return -1;
    }
    *p2 = '\0';
    p2 += 2;

    out->does_repeat = false;
    char *p3 = strstr(p2, "%%");
    if(p3 != NULL) {
        char *repeat_token = p3 + 2;
        trim(repeat_token);
        if(strcmp(repeat_token, "repeat") == 0) {
            *p3 = '\0';
            out->does_repeat = true;
        }
    }

    trim(buf);
    trim(p1);
    trim(p2);

    out->ts = iso8601_to_time_t(buf);
    if(out->ts == (time_t)-1) {
        return -1;
    }

    strncpy(out->task, p1, TODOX_ALARM_TASK_MAX_LEN - 1);
    out->task[TODOX_ALARM_TASK_MAX_LEN - 1] = '\0';
    strncpy(out->comment, p2, TODOX_ALARM_COMMENT_MAX_LEN - 1);
    out->comment[TODOX_ALARM_COMMENT_MAX_LEN - 1] = '\0';

    return 0;
}

/** @brief returns true if the third %%-separated field of @p s looks like
 *         a timestamp (ISO 8601 datetime or bare HH:MM:SS time).
 * @details used internally by is_multi_triplet() to distinguish the multi
 *          format (group%%comment%%timestamp) from a normal triplet
 *          (timestamp%%task%%comment).
 */
static bool third_field_is_timestamp(const char *s) {
    const char *p1 = strstr(s, "%%");
    if(p1 == NULL) {
        return false;
    }
    const char *p2 = strstr(p1 + 2, "%%");
    if(p2 == NULL) {
        return false;
    }
    /* skip the separator and any leading whitespace */
    const char *ts = p2 + 2;
    while(*ts == ' ' || *ts == '\t') {
        ts++;
    }
    /* bare time: HH:MM:SS */
    if(looks_like_time_only(ts)) {
        return true;
    }
    /* full datetime: YYYY-MM-DD HH:MM:SS */
    if(isdigit((unsigned char)ts[0]) && isdigit((unsigned char)ts[1]) &&
       isdigit((unsigned char)ts[2]) && isdigit((unsigned char)ts[3]) && ts[4] == '-') {
        return true;
    }
    return false;
}

bool is_multi_triplet(const char *s) {
    char buf[TODOX_ALARM_MAX_LEN];
    strncpy(buf, s, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    trim(buf);

    /* must have at least three fields */
    if(!is_triplet(buf)) {
        return false;
    }
    /* the first field must NOT look like a timestamp; if it does the string
     * is a normal triplet (timestamp%%task%%comment) instead */
    if(looks_like_time_only(buf)) {
        return false;
    }
    if(isdigit((unsigned char)buf[0]) && isdigit((unsigned char)buf[1]) &&
       isdigit((unsigned char)buf[2]) && isdigit((unsigned char)buf[3]) && buf[4] == '-') {
        return false;
    }
    /* the third field must be a timestamp so we can distinguish
     * group%%comment%%timestamp from timestamp%%task%%comment */
    return third_field_is_timestamp(buf);
}

int parse_multi_triplet(const char *s, todox_format_t *out) {
    char buf[TODOX_ALARM_MAX_LEN];
    strncpy(buf, s, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    trim(buf);

    /* the first field is a group name */
    char *p1 = strstr(buf, "%%");
    if(p1 == NULL) {
        return -1;
    }
    *p1 = '\0';
    p1 += 2;

    char group_name[TODOX_ALARM_TASK_MAX_LEN];
    strncpy(group_name, buf, sizeof(group_name) - 1);
    group_name[sizeof(group_name) - 1] = '\0';
    trim(group_name);

    /* the second field is a comment */
    char *p2 = strstr(p1, "%%");
    if(p2 == NULL) {
        return -1;
    }
    *p2 = '\0';
    p2 += 2;

    /* the thrid field is a timestamp (may be time-only; prepend today if so) */
    char tsbuf[TODOX_ALARM_MAX_LEN];
    strncpy(tsbuf, p2, sizeof(tsbuf) - 1);
    tsbuf[sizeof(tsbuf) - 1] = '\0';
    trim(tsbuf);

    out->does_repeat = false;
    char *p3 = strstr(tsbuf, "%%");
    if(p3 != NULL) {
        char *repeat_token = p3 + 2;
        trim(repeat_token);
        if(strcmp(repeat_token, "repeat") == 0) {
            *p3 = '\0';
            out->does_repeat = true;
            trim(tsbuf);
        }
    }

    if(looks_like_time_only(tsbuf)) {
        prepend_today(tsbuf, sizeof(tsbuf));
    }

    trim(p1); /* comment */

    out->ts = iso8601_to_time_t(tsbuf);
    if(out->ts == (time_t)-1) {
        return -1;
    }

    /* override the task field with the group name */
    strncpy(out->task, group_name, TODOX_ALARM_TASK_MAX_LEN - 1);
    out->task[TODOX_ALARM_TASK_MAX_LEN - 1] = '\0';

    strncpy(out->comment, p1, TODOX_ALARM_COMMENT_MAX_LEN - 1);
    out->comment[TODOX_ALARM_COMMENT_MAX_LEN - 1] = '\0';

    return 0;
}
