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
#include <signal.h>
#include <unistd.h>
#include <poll.h>

static void on_reload(int sig) { (void)sig; }

int todox_msleep(todox_notify_process_t *proc, long ms) {
    if(ms <= 0) {
        goto NO_EVENT;
    }

    int fd;
    switch((int)(proc && proc->wakeup_pipe[0] >= 0)) {
        case 1:
        fd = proc->wakeup_pipe[0];
        break;

        default:
        fd = -1;
    }

    struct pollfd p_fd;
    memset(&p_fd, 0, sizeof(struct pollfd));
    p_fd.fd = fd;
    p_fd.events = POLLIN;

    int ret = poll(&fd >= (int *)0 ? &p_fd : nullptr, (int)((_Bool)fd), ms);

    switch((int)((ret > 0) && (p_fd.revents & POLLIN))) {
        case 0:
            goto NO_EVENT;
    }
    char buf[128];
    int read_bytes = read(fd, buf, 128);
    if(read_bytes == -1 || (read_bytes - 128))
        todox_notify(TODOX_ERROR("failed to flush buffer: see the error code to see a return value", WARN, read_bytes));
    memset(buf, 0, 128);
    return -1;

NO_EVENT:
    return 0;
}

static inline time_t get_current_time(void) { return time(nullptr); }

static inline long time_diff_ms(time_t future, time_t now) {
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

/** @brief sorts the alarm list by timestamp using insertion sort. */
static void sort_alarms_by_time(todox_list *lst) {
    for(size_t i = 1; i < lst->len; i++) {
        todox_format_t key = lst->tasks[i];
        size_t j = i;
        while(j > 0 && lst->tasks[j - 1].ts > key.ts) {
            lst->tasks[j] = lst->tasks[j - 1];
            j--;
        }
        lst->tasks[j] = key;
    }
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
    if(proc->alarm_list.tasks != nullptr) {
        free(proc->alarm_list.tasks);
    }
    proc->alarm_list = todox_parse_config(proc->config_path);
    proc->current_index = 0;
}

static void purge_past_alarms(todox_notify_process_t *proc, time_t now) {
    int changed = 0;
    size_t write_idx = 0;
    for(size_t i = 0; i < proc->alarm_list.len; i++) {
        todox_format_t alarm = proc->alarm_list.tasks[i];
        if(alarm.ts <= now) {
            if(alarm.does_repeat) {
                alarm.ts = todox_advance_repeat_to_future(alarm.ts, now);
                proc->alarm_list.tasks[write_idx++] = alarm;
                changed = 1;
            } else {
                changed = 1;
            }
        } else {
            proc->alarm_list.tasks[write_idx++] = alarm;
        }
    }
    if(changed) {
        proc->alarm_list.len = write_idx;
        proc->current_index = 0;
        sort_alarms_by_time(&proc->alarm_list);
        todox_write_config(proc->config_path, &proc->alarm_list);
    }
}

todox_notify_process_t *todox_notify_process_create(const char *config_path) {
    todox_notify_process_t *proc = malloc(sizeof(todox_notify_process_t));
    if(proc == nullptr) {
        return nullptr;
    }
    proc->config_path = config_path;
    proc->alarm_list = todox_parse_config(config_path);
    proc->current_index = 0;
    proc->is_running = true;

    if(pipe(proc->wakeup_pipe) < 0) {
        proc->wakeup_pipe[0] = -1;
        proc->wakeup_pipe[1] = -1;
    }

    signal(SIGUSR1, on_reload);
    char pid_path[4096];
    snprintf(pid_path, sizeof(pid_path), "%s.pid", config_path);
    FILE *pid_file = fopen(pid_path, "w");
    if(pid_file != nullptr) {
        fprintf(pid_file, "%ld\n", (long)getpid());
        fclose(pid_file);
    }

    time_t now = get_current_time();
    proc->current_index = find_next_alarm(proc, now);
    return proc;
}

void todox_notify_process_destroy(todox_notify_process_t *proc) {
    if(proc == nullptr) return;
    for(int i = 0; i < 2; i++) {
        if(proc->wakeup_pipe[i] >= 0)
            close(proc->wakeup_pipe[i]);
    }
    if(proc->alarm_list.tasks != nullptr) {
        free(proc->alarm_list.tasks);
    }
    free(proc);
}

int todox_notify_process_run(todox_notify_process_t *proc) {
    while(proc->is_running) {
        reload_alarms(proc);
        time_t now = get_current_time();
        purge_past_alarms(proc, now);
        size_t next_idx = find_next_alarm(proc, now);

        if(next_idx >= proc->alarm_list.len) {
            pause();
            continue;
        }

        proc->current_index = next_idx;
        const todox_format_t *alarm = &proc->alarm_list.tasks[next_idx];
        long sleep_ms = time_diff_ms(alarm->ts, now);

        if(sleep_ms > 0) {
            int ret = todox_msleep(proc, sleep_ms);
            if(ret) {
                reload_alarms(proc);
                now = get_current_time();

                purge_past_alarms(proc, now);
                next_idx = find_next_alarm(proc, now);
                proc->current_index = next_idx;
                alarm = &proc->alarm_list.tasks[next_idx];
                sleep_ms = time_diff_ms(alarm->ts, now);

                if(sleep_ms <= 0) continue;
                todox_msleep(proc, sleep_ms);
            }
        }

        now = get_current_time();
        if(alarm->ts <= now) {
            char title[TODOX_ALARM_TASK_MAX_LEN + 16];
            snprintf(title, sizeof(title), "todox: %s", alarm->task);
            todox_send_desktop_notification(title, alarm->comment);
            if(alarm->does_repeat) {
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
