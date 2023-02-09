// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMELOG_H
#define IMELOG_H

#ifdef __cplusplus
extern "C" {
#endif

#define COMPILE_ROOT_DIR    NULL

#ifdef _WIN32
#define OSA_LOG_DIR          "X:\\kcm_log\\cfgtool"
#else
#define OSA_LOG_DIR          "/home/uos/kcm_log/cfgtool"
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

int osaLogIsInit();

int osaLogCreateDir(char* path_name);

int osaLogInit(int log_level, const char *error_file_prefix);
int osaLogClose();
int osaLogForceClose();

int osaLogPrintf(int log_level, const char *file, const char *function,
                   long line, const char *expanded_name, int expanded_num,
                   const char *fmt, ...);

int osaLogPrintfEx(int log_level, const char *file, const char *function,
                      long line, const char *expanded_name, int expanded_num,
                      const char *log_text, int log_size);

#ifndef NO_USE_OSA_LOG
#define osaLogCritical(expanded_name, expanded_num, ...)                                                      \
	osaLogPrintf(OSA_LOG_CRITICAL, __FILE__, __FUNCTION__  , __LINE__, expanded_name, expanded_num, __VA_ARGS__);
#define osaLogError(expanded_name, expanded_num, ...)                                                         \
	osaLogPrintf(OSA_LOG_ERROR, __FILE__, __FUNCTION__ , __LINE__, expanded_name, expanded_num, __VA_ARGS__); \
	osaLogPrintf(OSA_LOG_ERROR, __FILE__, __FUNCTION__, __LINE__, LOG_ERROR_NAME, LOG_ERROR_NUM, __VA_ARGS__)
#define osaLogWarning(expanded_name, expanded_num, ...)                                                       \
	osaLogPrintf(OSA_LOG_WARNING, __FILE__, __FUNCTION__ , __LINE__, expanded_name, expanded_num, __VA_ARGS__);
#define osaLogInfo(expanded_name, expanded_num, ...)                                                          \
	osaLogPrintf(OSA_LOG_INFO, __FILE__, __FUNCTION__ , __LINE__, expanded_name, expanded_num, __VA_ARGS__);
#define osaLogDebug(expanded_name, expanded_num, ...)                                                         \
	osaLogPrintf(OSA_LOG_DEBUG, __FILE__, __FUNCTION__ , __LINE__, expanded_name, expanded_num, __VA_ARGS__);

#define osaLogInfoEx(expanded_name, expanded_num, log_text, log_size) \
	osaLogPrintfEx(OSA_LOG_DEBUG, __FILE__, __FUNCTION__ , __LINE__, expanded_name, expanded_num, log_text, log_size)
#else
#define osaLogCritical(...)
#define osaLogError(...)
#define osaLogWarning(...)
#define osaLogInfo(...)
#define osaLogDebug(...)
#endif

#ifdef __cplusplus
}
#endif

#endif
