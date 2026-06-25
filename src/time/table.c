#include <file/format.h>
#include <error/error.h>
#include <stdlib.h>
#include <string.h>

static void trim_right(char *s) {
    size_t len = strlen(s);
    while(len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t' || s[len - 1] == '\r' || s[len - 1] == '\n')) {
        s[len - 1] = '\0';
        len--;
    }
}

static int parse_alarm_line(char *line, todox_format_t *out) {
    trim_right(line);

    char *sep1 = strstr(line, "%%");
    if(sep1 == NULL) {
        return -1;
    }
    *sep1 = '\0';
    sep1 += 2;

    char *sep2 = strstr(sep1, "%%");
    if(sep2 == NULL) {
        return -1;
    }
    *sep2 = '\0';
    sep2 += 2;

    trim_right(line);
    trim_right(sep1);
    trim_right(sep2);

    out->ts = iso8601_to_time_t(line);
    if(out->ts == (time_t)-1) {
        return -1;
    }

    strncpy(out->task, sep1, TODOX_ALARM_TASK_MAX_LEN - 1);
    out->task[TODOX_ALARM_TASK_MAX_LEN - 1] = '\0';
    strncpy(out->comment, sep2, TODOX_ALARM_COMMENT_MAX_LEN - 1);
    out->comment[TODOX_ALARM_COMMENT_MAX_LEN - 1] = '\0';

    return 0;
}

/** @brief parse a config file.
  * @param config path to a config file.
  * @return todox_list
  */
todox_list todox_parse_config(const char *config) {
    todox_list lst;
    todox_task_init(&lst);

    FILE *config_file = fopen(config, "rt");
    if(config_file == NULL)
    {
        todox_notify(TODOX_ERROR("failed to open config file", ERROR, TODOX_NO_CONFIG_FILE));
        return lst;
    }

    char buffer[TODOX_ALARM_MAX_LEN * TODOX_ALARM_TABLE_MAX_ROWS];
    while(fgets(buffer, sizeof(buffer), config_file) != NULL) {
        todox_format_t todo_itm = {0};
        char line[TODOX_ALARM_MAX_LEN];
        strncpy(line, buffer, sizeof(line) - 1);
        line[sizeof(line) - 1] = '\0';

        if(parse_alarm_line(line, &todo_itm) != 0) {
            continue;
        }

        todox_task_push(&lst, todo_itm);
    }

    fclose(config_file);
    return lst;
}

/** @brief write a todox list to a config file.
  * @param config path to a config file.
  * @param lst a pointer to a todox list.
  * @return 0 on success, -1 on failure.
  */
int todox_write_config(const char *config, const todox_list *lst) {
    FILE *config_file = fopen(config, "wt");
    if(config_file == NULL) {
        todox_notify(TODOX_ERROR("failed to open config file for writing", ERROR, TODOX_NO_CONFIG_FILE));
        return -1;
    }

    for(size_t i = 0; i < lst->len; i++) {
        const todox_format_t *itm = &lst->tasks[i];
        char ts_str[32] = {0};
        struct tm *gmt = gmtime(&itm->ts);
        if(gmt != NULL) {
            strftime(ts_str, sizeof(ts_str), TODOX_TIME_FORMAT, gmt);
        }
        fprintf(config_file, "%s%%%%%s%%%%%s\n", ts_str, itm->task, itm->comment);
    }

    fclose(config_file);
    return 0;
}
