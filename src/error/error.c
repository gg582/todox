#include <error/error.h>
#include <stdlib.h>

/** @brief make an error. */
todox_error_t TODOX_ERROR(const char *msg, enum TODOX_ERROR_LEVEL level, int code) {
    todox_error_t err;
    err.msg = msg;
    err.level = level;
    err.code = code;
    return err;
}

/** @brief notify errors. exit with error code when error level is CRITICAL. */
void todox_notify(const todox_error_t err) {
    if(err.level == DEBUG && TODOX_DEBUG_PRINT == 0) {
        return;
    }

    switch(err.level) {
        case DEBUG:
            fprintf(stdout, "[DEBUG] msg: %s(code: %d)\n", err.msg, err.code);
            break;
        case CRITICAL:
            fprintf(stderr, "[ERROR] msg: %s(exit code: %d)\n", err.msg, err.code);
            exit(err.code);
        case WARN:
            fprintf(stderr, "[WARN] msg: %s(code: %d)\n", err.msg, err.code);
            break;
        case INFO:
            fprintf(stdout, "[INFO] msg: %s(code: %d)\n", err.msg, err.code);
            break;
        default:
            fprintf(stdout, "[UNK] msg with unknown loglevel: %s(code: %d)\n", err.msg, err.code);
    }
}
