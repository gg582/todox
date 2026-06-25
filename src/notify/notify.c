#include <notify/notify.h>
#include <file/format.h>
#include <error/error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __linux__
#include <unistd.h>
#endif

void todox_msleep(long ms) {
    if(ms <= 0) {
        return;
    }
#ifdef __linux__
    usleep((useconds_t)(ms * 1000));
#else
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
#endif
}

static time_t get_current_time(void) {
    return time(NULL);
}

static long time_diff_ms(time_t future, time_t now) {
    double diff = difftime(future, now);
    if(diff <= 0) {
        return 0;
    }
    return (long)(diff * 1000.0);
}

static size_t find_next_alarm(todox_notify_process_t *proc, time_t now) {
    for(size_t i = proc->current_index; i < proc->alarm_list.len; i++) {
        if(proc->alarm_list.tasks[i].ts > now) {
            return i;
        }
    }
    return proc->alarm_list.len;
}

static void purge_past_alarms(todox_notify_process_t *proc, time_t now) {
    size_t write_idx = 0;
    for(size_t i = 0; i < proc->alarm_list.len; i++) {
        if(proc->alarm_list.tasks[i].ts > now) {
            proc->alarm_list.tasks[write_idx++] = proc->alarm_list.tasks[i];
        }
    }
    if(write_idx != proc->alarm_list.len) {
        proc->alarm_list.len = write_idx;
        proc->current_index = 0;
        todox_write_config(proc->config_path, &proc->alarm_list);
    }
}

todox_notify_process_t *todox_notify_process_create(const char *config_path) {
    todox_notify_process_t *proc = malloc(sizeof(todox_notify_process_t));
    if(proc == NULL) {
        return NULL;
    }
    proc->config_path = config_path;
    proc->alarm_list = todox_parse_config(config_path);
    proc->current_index = 0;
    proc->running = 1;

    time_t now = get_current_time();
    proc->current_index = find_next_alarm(proc, now);
    return proc;
}

void todox_notify_process_destroy(todox_notify_process_t *proc) {
    if(proc == NULL) {
        return;
    }
    if(proc->alarm_list.tasks != NULL) {
        free(proc->alarm_list.tasks);
    }
    free(proc);
}

int todox_notify_process_run(todox_notify_process_t *proc) {
    while(proc->running) {
        time_t now = get_current_time();
        purge_past_alarms(proc, now);
        size_t next_idx = find_next_alarm(proc, now);

        if(next_idx >= proc->alarm_list.len) {
            /* state: no pending future alarms. leave the loop so a service
             * manager can restart the process after the alarm list changes. */
            break;
        }

        proc->current_index = next_idx;
        const todox_format_t *alarm = &proc->alarm_list.tasks[next_idx];
        long sleep_ms = time_diff_ms(alarm->ts, now);

        if(sleep_ms > 0) {
            todox_msleep(sleep_ms);
        }

        now = get_current_time();
        if(alarm->ts <= now) {
            char title[TODOX_ALARM_TASK_MAX_LEN + 16];
            snprintf(title, sizeof(title), "todox: %s", alarm->task);
            todox_send_desktop_notification(title, alarm->comment);
            proc->current_index = next_idx + 1;
        }
    }
    return 0;
}
