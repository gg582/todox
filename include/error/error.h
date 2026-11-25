#ifndef TODOX_ERROR_H
#define TODOX_ERROR_H

#include <stdio.h>

/** @struct todox_error_t
 * @brief an error struct of todox.
 */

#define TODOX_WRONG_TIMESTAMP 100
#define TODOX_NO_CONFIG_FILE 110

#ifndef TODOX_DEBUG_PRINT
#define TODOX_DEBUG_PRINT 0
#endif

enum TODOX_ERROR_LEVEL { DEFAULT, INFO, WARN, CRITICAL, DEBUG };

typedef struct prim__todox_error_t {
    enum TODOX_ERROR_LEVEL level;
    int code;
} todox_error_t;

/** @brief notifies an error message according to its level.
 * @param[in] msg an error message.
 * @param[in] err an error to notify.
 */
void todox_notify(const char *msg, const todox_error_t err);

/** @brief creates an error struct.
 * @param[in] level an error level.
 * @param[in] code an error code.
 * @return an error struct.
 */
todox_error_t TODOX_ERROR(enum TODOX_ERROR_LEVEL level, int code);

#endif
