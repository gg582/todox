#include <app/io.h>
#include <file/format.h>
#include <list/list.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const char *get_alarm_file(int argc, char **argv, int idx) {
    if(argc > idx && argv[idx][0] != '\0') {
        return argv[idx];
    }
    const char *env = getenv("TODOX_ALARM_FILE");
    if(env != nullptr && env[0] != '\0') {
        return env;
    }
    return TODOX_DEFAULT_ALARM_FILE;
}

void print_tasks(const todox_list *lst) {
    for(size_t i = 0; i < lst->len; i++) {
        const todox_format_t *itm = &lst->tasks[i];
        char ts_str[32] = {0};

        struct tm *gmt = gmtime(&itm->ts), loc;
        time_t ts = timegm(gmt);
        
        if(localtime_r(&ts, &loc) != nullptr) {
            strftime(ts_str, sizeof(ts_str), TODOX_TIME_FORMAT, &loc);
        }
        printf("%s | %s | %s", ts_str, itm->task, itm->comment);
        if(itm->does_repeat) {
            printf(" | repeat");
        }
        printf("\n");
    }
}
