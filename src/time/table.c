#include <file/config.h>
#include <file/format.h>
#include <list/list.h>
#include <app/parse.h>
#include <error/error.h>
#include <stdlib.h>
#include <string.h>

/** @brief parse a config file.
 * @param config path to a config file.
 * @return todox_list
 */
todox_list todox_parse_config(const char *config) {
    todox_list lst;
    todox_task_init(&lst);

    FILE *config_file = fopen(config, "rt");
    if(config_file == NULL) {
        todox_notify(TODOX_ERROR("config file not found, starting with empty alarm list", DEBUG,
                                 TODOX_NO_CONFIG_FILE));
        return lst;
    }

    char buffer[TODOX_ALARM_MAX_LEN * TODOX_ALARM_TABLE_MAX_ROWS];
    while(fgets(buffer, sizeof(buffer), config_file) != NULL) {
        todox_format_t todo_itm = {0};
        char line[TODOX_ALARM_MAX_LEN];
        strncpy(line, buffer, sizeof(line) - 1);
        line[sizeof(line) - 1] = '\0';

        if(parse_triplet(line, &todo_itm) != 0) {
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
        todox_notify(
            TODOX_ERROR("failed to open config file for writing", ERROR, TODOX_NO_CONFIG_FILE));
        return -1;
    }

    for(size_t i = 0; i < lst->len; i++) {
        const todox_format_t *itm = &lst->tasks[i];
        char ts_str[32] = {0};
        struct tm *gmt = gmtime(&itm->ts);
        if(gmt != NULL) {
            strftime(ts_str, sizeof(ts_str), TODOX_TIME_FORMAT, gmt);
        }
        if(itm->repeat) {
            fprintf(config_file, "%s%%%%%s%%%%%s%%%%repeat\n", ts_str, itm->task, itm->comment);
        } else {
            fprintf(config_file, "%s%%%%%s%%%%%s\n", ts_str, itm->task, itm->comment);
        }
    }

    fclose(config_file);
    return 0;
}
