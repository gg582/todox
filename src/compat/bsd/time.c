/* Modern BSD libc provides timegm(). Expose BSD extensions so the
 * declaration is visible (required by OpenBSD and harmless elsewhere).
 */
#ifndef _BSD_SOURCE
#define _BSD_SOURCE 1
#endif

#include <compat/bsd/time.h>

/** @brief BSD-compatible wrapper around the standard libc timegm().
  * @details On current FreeBSD, NetBSD, OpenBSD and DragonFly systems
  *          timegm() is part of libc, so the manual leap-year arithmetic
  *          is no longer necessary.
  */
time_t bsd_timegm(struct tm *tm) {
    if(tm == NULL) {
        return (time_t)-1;
    }
    return timegm(tm);
}
