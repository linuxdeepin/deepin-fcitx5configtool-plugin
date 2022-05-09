#ifndef OSA_STR_OPER_H
#define OSA_STR_OPER_H

#ifdef __cplusplus
extern "C" {
#endif

int osa_is_sub(int start, const char *src, const char *sub);

int osa_split_str_to_token(char *src_str, const char *delim, char **pp_token, int token_count);

int osa_get_sub_count(int start, const char *src, const char *sub);

int osa_calculate_replace_length(const char *src_str, const char *old_str, const char *new_str, int max);

int osa_replace_str(const char *src_str, char *dst_str, const char *old_str, const char *new_str, int max);

int osa_trim_str(char* str);

int osa_take_tag_text_from_file(const char *file_path, const char *before_tag, const char *after_tag, char *text_list, int text_size, int max_num);

int osa_take_tag_text_from_str(const char *text, const char *before_tag, const char *after_tag, char *text_list, int text_size, int max_num);

#ifdef __cplusplus
}
#endif

#endif
