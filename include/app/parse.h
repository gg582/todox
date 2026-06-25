#ifndef __TODOX_PARSE_H__
#define __TODOX_PARSE_H__

#include <file/format.h>

/** @brief checks if a string is a ts%%task%%comment triplet.
  * @param[in] s a string to check.
  * @return non-zero if triplet, zero otherwise.
  */
int is_triplet(const char *s);

/** @brief parses a ts%%task%%comment triplet into a todox item.
  * @param[in] s a triplet string.
  * @param[out] out a parsed todox item.
  * @return 0 on success, -1 on failure.
  */
int parse_triplet(const char *s, todox_format_t *out);

#endif
