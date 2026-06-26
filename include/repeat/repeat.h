#ifndef __TODOX_REPEAT_H__
#define __TODOX_REPEAT_H__

#include <file/format.h>
#include <stddef.h>

/** @brief checks whether the leading token of an alarm string is a weekday expression.
  * @param[in] s an alarm string.
  * @return non-zero if the string starts with a weekday expression, zero otherwise.
  */
int is_weekday_expr(const char *s);

/** @brief expands a weekday triplet into one or more concrete repeated alarms.
  * @param[in] s an alarm string starting with a weekday expression.
  * @param[out] out an array of at least 7 items to fill.
  * @return number of alarms produced, or -1 on failure.
  */
int expand_weekday_triplet(const char *s, todox_format_t *out);

/** @brief advances a timestamp by exactly one calendar week.
  * @param[in] t a unix timestamp.
  * @return the timestamp one calendar week later.
  */
time_t todox_next_weekly_occurrence(time_t t);

#endif
