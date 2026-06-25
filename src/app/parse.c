#include <app/parse.h>
#include <file/format.h>
#include <error/error.h>
#include <string.h>

static void trim(char *s) {
    size_t len = strlen(s);
    while(len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t' || s[len - 1] == '\r' || s[len - 1] == '\n')) {
        s[len - 1] = '\0';
        len--;
    }
}

int is_triplet(const char *s) {
    const char *p1 = strstr(s, "%%");
    if(p1 == NULL) {
        return 0;
    }
    return strstr(p1 + 2, "%%") != NULL;
}

int parse_triplet(const char *s, todox_format_t *out) {
    char buf[TODOX_ALARM_MAX_LEN];
    strncpy(buf, s, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    trim(buf);

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
