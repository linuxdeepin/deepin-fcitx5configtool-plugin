#ifndef IME_LOG_H
#define IME_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#define COMPILE_ROOT_DIR    NULL

#ifdef _WIN32
#define OSA_LOG_DIR          "X:\\kcm_log\\cfgtool"
#else
#define OSA_LOG_DIR          "/home/wangtz/kcm_log/cfgtool"
#endif

#define DEBUG_LOG_VERSIN     "0.1.0_220413_14_10"
#define LOG_EXPANDED_NAME    (char *)"[main]"
#define LOG_EXPANDED_NUM     0

#define LOG_CFGTOOL_NAME    (char *)"[cfgtool]"
#define LOG_CFGTOOL_NUM     1

#define LOG_ERROR_NAME      (char *)"[error]"
#define LOG_ERROR_NUM       2

#define LOG_LIVE_NAME       (char *)"[live]"
#define LOG_LIVE_NUM        3

#define LOG_TEST_NAME       (char *)"[test]"
#define LOG_TEST_NUM        4

#define LOG_DBUS_NAME       (char *)"[dbus]"
#define LOG_DBUS_NUM        5

/*定义日志级别*/
#define OSA_LOG_CRITICAL    0
#define OSA_LOG_ERROR       1
#define OSA_LOG_WARNING     2
#define OSA_LOG_INFO        3
#define OSA_LOG_DEBUG       4

int osa_log_is_init();

int osa_log_create_dir(char* path_name);

int osa_log_init(int log_level, const char *error_file_prefix);
int osa_log_close();
int osa_log_force_close();

int osa_log_printf(int log_level, const char *file, const char *function,
                   long line, const char *expanded_name, int expanded_num,
                   const char *fmt, ...);

int osa_log_printf_ex(int log_level, const char *file, const char *function,
                      long line, const char *expanded_name, int expanded_num,
                      const char *log_text, int log_size);

/*
#define osa_log_critical(expanded_name, expanded_num, ...)                                                      \
	osa_log_printf(OSA_LOG_CRITICAL, __FILE__, __FUNCTION__  , __LINE__, expanded_name, expanded_num, __VA_ARGS__); \
	log_critical(0, __VA_ARGS__)
#define osa_log_error(expanded_name, expanded_num, ...)                                                     \
	osa_log_printf(OSA_LOG_ERROR, __FILE__, __FUNCTION__ , __LINE__, expanded_name, expanded_num, __VA_ARGS__); \
	log_error(0, __VA_ARGS__)
#define osa_log_warning(expanded_name, expanded_num, ...)                                                     \
	osa_log_printf(OSA_LOG_WARNING, __FILE__, __FUNCTION__ , __LINE__, expanded_name, expanded_num, __VA_ARGS__); \
	log_warning(0, __VA_ARGS__)
#define osa_log_info(expanded_name, expanded_num, ...)                                                     \
	osa_log_printf(OSA_LOG_INFO, __FILE__, __FUNCTION__ , __LINE__, expanded_name, expanded_num, __VA_ARGS__); \
	log_info(0, __VA_ARGS__)
#define osa_log_debug(expanded_name, expanded_num, ...)                                                     \
	osa_log_printf(OSA_LOG_DEBUG, __FILE__, __FUNCTION__ , __LINE__, expanded_name, expanded_num, __VA_ARGS__); \
	log_debug(0, __VA_ARGS__)
*/

#define osa_log_critical(expanded_name, expanded_num, ...)                                                      \
	osa_log_printf(OSA_LOG_CRITICAL, __FILE__, __FUNCTION__  , __LINE__, expanded_name, expanded_num, __VA_ARGS__);
#define osa_log_error(expanded_name, expanded_num, ...)                                                         \
	osa_log_printf(OSA_LOG_ERROR, __FILE__, __FUNCTION__ , __LINE__, expanded_name, expanded_num, __VA_ARGS__); \
	osa_log_printf(OSA_LOG_ERROR, __FILE__, __FUNCTION__, __LINE__, LOG_ERROR_NAME, LOG_ERROR_NUM, __VA_ARGS__)
#define osa_log_warning(expanded_name, expanded_num, ...)                                                       \
	osa_log_printf(OSA_LOG_WARNING, __FILE__, __FUNCTION__ , __LINE__, expanded_name, expanded_num, __VA_ARGS__);
#define osa_log_info(expanded_name, expanded_num, ...)                                                          \
	osa_log_printf(OSA_LOG_INFO, __FILE__, __FUNCTION__ , __LINE__, expanded_name, expanded_num, __VA_ARGS__);
#define osa_log_debug(expanded_name, expanded_num, ...)                                                         \
	osa_log_printf(OSA_LOG_DEBUG, __FILE__, __FUNCTION__ , __LINE__, expanded_name, expanded_num, __VA_ARGS__);

#define osa_log_info_ex(expanded_name, expanded_num, log_text, log_size) \
	osa_log_printf_ex(OSA_LOG_DEBUG, __FILE__, __FUNCTION__ , __LINE__, expanded_name, expanded_num, log_text, log_size)

#ifdef __cplusplus
}
#endif

#endif
