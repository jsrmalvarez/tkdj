#ifndef LIB_TAGS_H_
#define LIB_TAGS_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

bool tags_tag_file(const char* path, size_t tagc, char** tags);
bool tags_untag_file(const char* path, size_t tagc, char** tags);
bool tags_list_file(const char* path);


#endif//LIB_TAGS_H_
