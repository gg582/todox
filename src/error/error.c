#include <error/error.h>
#include <stdlib.h>
#include <string.h>

todox_error_t TODOX_ERROR(const char *msg, enum TODOX_ERROR_LEVEL level, int code) {
    todox_error_t err;
    err.msg = strdup(msg);
    err.level = level;
    err.code = code;
    return err;
}

void todox_notify(const todox_error_t err) {
    if(err.level == DEBUG && TODOX_DEBUG_PRINT == 0) {
        free(err.msg);
        return;
    }

    switch(err.level) {
        case WARN:
            fprintf(stderr, "[WARN] msg: %s(code: %d)\n", err.msg, err.code);
            break;
        case ERROR:
            fprintf(stderr, "[ERROR] msg: %s(exit code: %d)\n", err.msg, err.code);
            free(err.msg);
            exit(err.code);
        case INFO:
            fprintf(stdout, "[INFO] msg: %s(code: %d)\n", err.msg, err.code);
            break;
        case DEBUG:
            fprintf(stdout, "[DEBUG] msg: %s(code: %d)\n", err.msg, err.code);
            break;
        default:
            fprintf(stdout, "[UNK] msg with unknown loglevel: %s(code: %d)\n", err.msg, err.code);
    }
    free(err.msg);
}
