#ifndef __TODOX_ERROR_H__
#define __TODOX_ERROR_H__

#include <stdio.h>

/** @struct todox_error_t
  * @brief an error struct of todox.
  */

#define TODOX_WRONG_TIMESTAMP 100
#define TODOX_NO_CONFIG_FILE  110

#ifndef TODOX_DEBUG_PRINT
#define TODOX_DEBUG_PRINT 0
#endif

enum TODOX_ERROR_LEVEL {
    DEFAULT,
    ERROR,
    WARN,
    INFO,
    DEBUG
};

typedef struct __todox_error_t {
    enum TODOX_ERROR_LEVEL level;
    int code;
    char *msg;
} todox_error_t;

/** @brief notifies an error message according to its level.
  * @param[in] err an error to notify.
  */
void todox_notify(const todox_error_t err);

/** @brief creates an error struct.
  * @param[in] msg an error message.
  * @param[in] level an error level.
  * @param[in] code an error code.
  * @return an error struct.
  */
todox_error_t TODOX_ERROR(const char *msg, enum TODOX_ERROR_LEVEL level, int code);

#endif
