#include <app/app.h>
#include <app/io.h>
#include <app/parse.h>
#include <file/config.h>
#include <file/format.h>
#include <list/list.h>
#include <notify/notify.h>
#include <repeat/repeat.h>
#include <error/error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/** @brief detaches the process into a background daemon.
  * @details performs the standard double-fork, setsid, umask(0), and
  *          redirects stdin/stdout/stderr to /dev/null. chdir("/") is
  *          intentionally omitted so that relative alarm file paths remain
  *          valid after daemonization.
  * @return 0 on success, -1 on failure.
  */
static int daemonize(void) {
    pid_t pid = fork();
    if(pid < 0) {
        return -1;
    }
    if(pid > 0) {
        exit(0);
    }

    if(setsid() < 0) {
        return -1;
    }

    pid = fork();
    if(pid < 0) {
        return -1;
    }
    if(pid > 0) {
        exit(0);
    }

    umask(0);

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    open("/dev/null", O_RDONLY);
    open("/dev/null", O_WRONLY);
    open("/dev/null", O_WRONLY);

    return 0;
}

/** @brief prints help text with usage and examples. */
static int cmd_help(const char *progname) {
    printf("usage: %s [command] [file] [args]\n\n", progname);
    printf("commands:\n");
    printf("  (none)                      parse and list alarms\n");
    printf("  add [file] \"TS%%%%TASK%%%%COMMENT\"  add an alarm\n");
    printf("  add [file] \"TS%%%%TASK%%%%COMMENT%%%%repeat\"\n");
    printf("                              add a weekly repeating alarm\n");
    printf("  add [file] \"DAY%%%%TIME%%%%TASK%%%%COMMENT\"  add a weekly alarm on DAY at TIME\n");
    printf("  remove [file] \"TASK\"            remove an alarm\n");
    printf("  --daemonize [file]            run notification daemon in background\n");
    printf("\n");
    printf("notes:\n");
    printf("  if TODOX_ALARM_FILE is set, the [file] argument can be omitted.\n");
    printf("  install.sh sets TODOX_ALARM_FILE in the service, so after installation\n");
    printf("  most commands do not need a file path.\n");
    printf("  date can be omitted from TS; time-only input is treated as today.\n");
    printf("    e.g. \"21:00:00 +0900%%%%task%%%%comment\" uses today's date.\n");
    printf("  DAY is a weekday expression: mon, tue, wed, thu, fri, sat, sun,\n");
    printf("    a range such as mon-fri, or a list such as mon:wed:fri.\n");
    printf("  TIME is a time of day with an optional timezone: HH:MM:SS [+|-]HHMM.\n");
    printf("  alarms created with DAY or the %%%%repeat token fire every week.\n");
    printf("  the daemon advances repeating alarms by one week after they fire.\n");
    printf("\n");
    printf("examples:\n");
    printf("  %s alarm.txt\n", progname);
    printf("  TODOX_ALARM_FILE=alarm.txt %s\n", progname);
    printf("  %s add alarm.txt \"1970-01-01 00:00:00 +0000%%%%test title%%%%test comment\"\n", progname);
    printf("  TODOX_ALARM_FILE=alarm.txt %s add \"1970-01-01 00:00:00 +0000%%%%test title%%%%test comment\"\n", progname);
    printf("  %s add \"21:00:00 +0900%%%%task%%%%comment\"       (today's date)\n", progname);
    printf("  %s add \"mon-fri%%%%09:00:00 +0900%%%%standup%%%%daily sync\"  (weekday alarm)\n", progname);
    printf("  %s add \"mon:wed:fri%%%%21:00:00 +0900%%%%gym%%%%workout\"        (selected weekdays)\n", progname);
    printf("  %s add \"09:00:00 +0900%%%%report%%%%weekly%%%%repeat\"  (weekly repeat)\n", progname);
    printf("  %s remove alarm.txt \"test title\"\n", progname);
    printf("  TODOX_ALARM_FILE=alarm.txt %s remove \"test title\"\n", progname);
    printf("  %s --daemonize alarm.txt\n", progname);
    printf("  TODOX_ALARM_FILE=alarm.txt %s --daemonize\n", progname);
    return 0;
}

/** @brief loads alarms and runs the notification daemon loop. */
static int run_daemon(int argc, char **argv) {
    const char *file = get_alarm_file(argc, argv, 2);
    todox_notify_process_t *proc = todox_notify_process_create(file);
    if(proc == NULL) {
        fprintf(stderr, "failed to create notify process\n");
        return 1;
    }
    int ret = todox_notify_process_run(proc);
    todox_notify_process_destroy(proc);
    return ret;
}

/** @brief checks whether a task name already exists in the alarm list. */
static int is_duplicate(const todox_list *lst, const char *task) {
    return todox_task_find((todox_list *)lst, task) != (unsigned)-1;
}

static int cmd_add(int argc, char **argv) {
    const char *file;
    const char *triple;
    if(argc == 3) {
        file = get_alarm_file(0, NULL, 0);
        triple = argv[2];
    } else if(argc == 4) {
        file = argv[2];
        triple = argv[3];
    } else {
        fprintf(stderr, "usage: %s add [<file>] \"<ts>%%%%<task>%%%%<comment>\"\n", argv[0]);
        return 1;
    }

    todox_list lst = todox_parse_config(file);
    if(is_weekday_expr(triple)) {
        todox_format_t items[7] = {0};
        int n = expand_weekday_triplet(triple, items);
        if(n < 0) {
            fprintf(stderr, "invalid weekday format\n");
            free(lst.tasks);
            return 1;
        }
        if(is_duplicate(&lst, items[0].task)) {
            fprintf(stderr, "duplicate task name: %s\n", items[0].task);
            free(lst.tasks);
            return 1;
        }
        for(int i = 0; i < n; i++) {
            todox_task_push(&lst, items[i]);
        }
        int ret = todox_write_config(file, &lst);
        free(lst.tasks);
        return ret;
    }

    todox_format_t itm = {0};
    if(parse_triplet(triple, &itm) != 0) {
        fprintf(stderr, "invalid alarm format\n");
        free(lst.tasks);
        return 1;
    }
    if(is_duplicate(&lst, itm.task)) {
        fprintf(stderr, "duplicate task name: %s\n", itm.task);
        free(lst.tasks);
        return 1;
    }
    todox_task_push(&lst, itm);
    int ret = todox_write_config(file, &lst);
    free(lst.tasks);
    return ret;
}

static int cmd_remove(int argc, char **argv) {
    const char *file;
    const char *task;
    if(argc == 3) {
        file = get_alarm_file(0, NULL, 0);
        task = argv[2];
    } else if(argc == 4) {
        file = argv[2];
        task = argv[3];
    } else {
        fprintf(stderr, "usage: %s remove [<file>] \"<task>\"\n", argv[0]);
        return 1;
    }

    todox_list lst = todox_parse_config(file);
    if(todox_task_remove(&lst, task) == NULL) {
        fprintf(stderr, "task not found: %s\n", task);
        free(lst.tasks);
        return 1;
    }
    int ret = todox_write_config(file, &lst);
    free(lst.tasks);
    return ret;
}

static int cmd_list(int argc, char **argv) {
    const char *file = get_alarm_file(argc, argv, 1);
    todox_list lst = todox_parse_config(file);
    print_tasks(&lst);
    free(lst.tasks);
    return 0;
}

int app_run(int argc, char **argv) {
    if(argc >= 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        return cmd_help(argv[0]);
    }

    if(argc >= 2 && strcmp(argv[1], "--daemonize") == 0) {
        if(daemonize() != 0) {
            fprintf(stderr, "failed to daemonize\n");
            return 1;
        }
        return run_daemon(argc, argv);
    }

    if(argc >= 2 && strcmp(argv[1], "add") == 0) {
        return cmd_add(argc, argv);
    }

    if(argc >= 2 && strcmp(argv[1], "remove") == 0) {
        return cmd_remove(argc, argv);
    }

    return cmd_list(argc, argv);
}
