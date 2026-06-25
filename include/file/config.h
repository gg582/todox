#ifndef __TODOX_CONFIG_H__
#define __TODOX_CONFIG_H__

#include <file/format.h>
#include <list/list.h>

/** @brief parses a config file into a todox list.
  * @param[in] config a path to a config file.
  * @return a parsed todox list.
  */
todox_list todox_parse_config(const char *config);

/** @brief writes a todox list to a config file.
  * @param[in] config a path to a config file.
  * @param[in] lst a pointer to a todox list.
  * @return 0 on success, -1 on failure.
  */
int todox_write_config(const char *config, const todox_list *lst);

#endif
