#include <notify/notify.h>
#include <file/config.h>
#include <file/format.h>
#include <list/list.h>
#include <repeat/repeat.h>
#include <error/error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* polling interval when no future alarm is pending */
#define TODOX_NO_ALARM_POLL_INTERVAL_MS 10000

void todox_msleep(long ms) {
    if(ms <= 0) {
        return;
    }
    /* nanosleep is used on all platforms. It is POSIX.1-2008 compliant,
     * handles signal interruption correctly, and avoids the deprecated
     * usleep() workaround that was previously Linux-specific. */
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    while(nanosleep(&ts, &ts) == -1) {
        /* retry if interrupted by a signal */
    }
}

static time_t get_current_time(void) { return time(NULL); }

static long time_diff_ms(time_t future, time_t now) {
    double diff = difftime(future, now);
    if(diff <= 0) {
        return 0;
    }
    return (long)(diff * 1000.0);
}

/** @brief advances a repeat alarm by one week and keeps the list sorted. */
static void advance_repeat_alarm(todox_notify_process_t *proc, size_t idx) {
    proc->alarm_list.tasks[idx].ts = todox_next_weekly_occurrence(proc->alarm_list.tasks[idx].ts);

    todox_format_t moved = proc->alarm_list.tasks[idx];
    size_t j = idx;
    while(j + 1 < proc->alarm_list.len && proc->alarm_list.tasks[j + 1].ts < moved.ts) {
        proc->alarm_list.tasks[j] = proc->alarm_list.tasks[j + 1];
        j++;
    }
    proc->alarm_list.tasks[j] = moved;
}

static size_t find_next_alarm(todox_notify_process_t *proc, time_t now) {
    for(size_t i = proc->current_index; i < proc->alarm_list.len; i++) {
        if(proc->alarm_list.tasks[i].ts > now) {
            return i;
        }
    }
    return proc->alarm_list.len;
}

static void reload_alarms(todox_notify_process_t *proc) {
    if(proc->alarm_list.tasks != NULL) {
        free(proc->alarm_list.tasks);
    }
    proc->alarm_list = todox_parse_config(proc->config_path);
    proc->current_index = 0;
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
            /* no pending future alarms. wait a short interval, then reload
             * the config so newly added alarms are picked up. */
            todox_msleep(TODOX_NO_ALARM_POLL_INTERVAL_MS);
            reload_alarms(proc);
            continue;
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
            if(alarm->repeat) {
                advance_repeat_alarm(proc, next_idx);
                proc->current_index = next_idx;
                todox_write_config(proc->config_path, &proc->alarm_list);
            } else {
                proc->current_index = next_idx + 1;
            }
        }
    }
    return 0;
}
