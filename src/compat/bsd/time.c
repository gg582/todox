#include <compat/bsd/time.h>

static int is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static const int month_days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/** @brief BSD-compatible replacement for GNU timegm().
  * @details computes UTC seconds since the Unix epoch without relying on
  *          timezone environment variables or GNU extensions.
  */
time_t bsd_timegm(struct tm *tm) {
    if(tm == NULL) {
        return (time_t)-1;
    }

    int year = tm->tm_year + 1900;
    int mon = tm->tm_mon;
    int mday = tm->tm_mday;
    int hour = tm->tm_hour;
    int min = tm->tm_min;
    int sec = tm->tm_sec;

    if(year < 1970 || mon < 0 || mon > 11 || mday < 1 || hour < 0 || min < 0 || sec < 0 ||
       hour > 23 || min > 59 || sec > 60) {
        return (time_t)-1;
    }

    long long days = 0;
    for(int y = 1970; y < year; y++) {
        days += is_leap_year(y) ? 366 : 365;
    }

    for(int m = 0; m < mon; m++) {
        days += month_days[m];
        if(m == 1 && is_leap_year(year)) {
            days += 1;
        }
    }

    int dim = month_days[mon];
    if(mon == 1 && is_leap_year(year)) {
        dim += 1;
    }
    if(mday > dim) {
        return (time_t)-1;
    }

    days += mday - 1;

    return (time_t)(days * 86400LL + hour * 3600LL + min * 60LL + sec);
}
