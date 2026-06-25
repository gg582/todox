#include <app/io.h>
#include <file/format.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const char *get_alarm_file(int argc, char **argv, int idx) {
    if(argc > idx && argv[idx][0] != '\0') {
        return argv[idx];
    }
    const char *env = getenv("TODOX_ALARM_FILE");
    if(env != NULL && env[0] != '\0') {
        return env;
    }
    return "alarm.txt";
}

void print_tasks(const todox_list *lst) {
    for(size_t i = 0; i < lst->len; i++) {
        const todox_format_t *itm = &lst->tasks[i];
        char ts_str[32] = {0};
        struct tm *gmt = gmtime(&itm->ts);
        if(gmt != NULL) {
            strftime(ts_str, sizeof(ts_str), TODOX_TIME_FORMAT, gmt);
        }
        printf("%s | %s | %s\n", ts_str, itm->task, itm->comment);
    }
}
