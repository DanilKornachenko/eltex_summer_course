#ifndef STAT_H
#define STAT_H

#include <sys/stat.h>

__mode_t parse_rights(const char* str, __mode_t base_mode);

__mode_t view_file_rights(const char* path);

#endif
