#ifndef OSASTROPER_H
#define OSASTROPER_H

#ifdef __cplusplus
extern "C" {
#endif

int osaIsSub(int start, const char *src, const char *sub);

int osaSplitStrToToken(char *src_str, const char *delim, char **pp_token, int token_count);

int osaGetSubCount(int start, const char *src, const char *sub);

int osaCalculateReplaceLength(const char *src_str, const char *old_str, const char *new_str, int max);

int osaReplaceStr(const char *src_str, char *dst_str, const char *old_str, const char *new_str, int max);

int osaTrimStr(char* str);

int osaTakeTagTextFromFile(const char *file_path, const char *before_tag, const char *after_tag, char *text_list, int text_size, int max_num);

int osaTakeTagTextFromStr(const char *text, const char *before_tag, const char *after_tag, char *text_list, int text_size, int max_num);

#ifdef __cplusplus
}
#endif

#endif
