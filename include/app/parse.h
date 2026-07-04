#ifndef TODOX_PARSE_H
#define TODOX_PARSE_H

#include <file/format.h>

/** @brief checks if a string is a ts%%task%%comment triplet.
 * @param[in] s a string to check.
 * @return true if triplet, false otherwise.
 */
bool is_triplet(const char *s);

/** @brief parses a ts%%task%%comment triplet into a todox item.
 * @param[in] s a triplet string.
 * @param[out] out a parsed todox item.
 * @return 0 on success, -1 on failure.
 */
int parse_triplet(const char *s, todox_format_t *out);

/** @brief checks whether a string uses the multi-alarm format.
 * @details the multi format is: group_name%%comment%%timestamp
 *          where timestamp may be a full ISO 8601 datetime or a bare
 *          HH:MM:SS [+|-]HHMM time (today's date is prepended automatically).
 *          Unlike a normal triplet, the first field is treated as a shared
 *          group name rather than a timestamp, so multiple entries can share
 *          the same task name.
 * @param[in] s a string to check.
 * @return true if the string looks like a multi-alarm entry, false otherwise.
 */
bool is_multi_triplet(const char *s);

/** @brief parses a multi-alarm string into a todox item.
 * @details field order: group_name%%comment%%timestamp
 *          if timestamp is time-only (HH:MM:SS [+|-]HHMM) today's date is
 *          prepended.  The task field of @p out is set to @p group_name.
 * @param[in] s a multi-alarm string.
 * @param[out] out a parsed todox item.
 * @return 0 on success, -1 on failure.
 */
int parse_multi_triplet(const char *s, todox_format_t *out);

#endif
