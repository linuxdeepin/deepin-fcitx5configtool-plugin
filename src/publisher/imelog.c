#include "imelog.h"

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#endif

#ifndef WIN32
#include <dirent.h>
#include <unistd.h>

#include<sys/stat.h>
#include<sys/types.h>
#endif

#ifdef WIN32
#define strncpy(s1,s2,n)              strncpy_s(s1, sizeof(s1), s2, n)
#endif

static const int ERRNO_ESUCCESS      = 0;
static const int ERRNO_EOPENFILE     = 1;
static const int ERRNO_ENOMEMORY     = 12;
static const int ERRNO_EINVALUE      = 22;

static const int ERRNO_EINIT         = -18;
static const int ERRNO_ENOTFOUND     = -23;
static const int ERRNO_EINVALIDPARAM = -24;
static const int ERRNO_EORDER        = -25;
static const int ERRNO_EFORMAT       = -26;
static const int ERRNO_ESYSERROR     = -27;
static const int ERRNO_EFAILED       = -28;
static const int ERRNO_EFORCEJUMP    = -29;

#ifdef WIN32
typedef void *HANDLE;
typedef HANDLE sem_t;
typedef struct __osa_sem_t
{
	sem_t sem;
	char obj_name[32];
}osa_sem_t;

typedef int clockid_t;

#define MAX_SEM_VALUE     99999999
#define CLOCK_REALTIME    1

static int osaClockGettime(clockid_t clk_id, struct timespec *tp)
{
	SYSTEMTIME st;
	struct tm stm;

	GetLocalTime(&st);

	memset(&stm, 0, sizeof(stm));
	stm.tm_year = st.wYear - 1900;
	stm.tm_mon = st.wMonth - 1;
	stm.tm_mday = st.wDay;
	stm.tm_hour = st.wHour;
	stm.tm_min = st.wMinute;
	stm.tm_sec = st.wSecond;

	tp->tv_sec = mktime(&stm);
	tp->tv_nsec = st.wMilliseconds * 1000 * 1000;

	return 0;
}

static int osaSemInit(osa_sem_t *sem, int pshared, unsigned int value)
{
	sem->sem = CreateSemaphore(NULL, value, MAX_SEM_VALUE, NULL);
	if (sem->sem == NULL)
		return -1;
	return 0;
}

static int osaSemDestroy(osa_sem_t *sem)
{
	CloseHandle(sem->sem);
	return 0;
}

static int osaSemWait(osa_sem_t *sem)
{
	DWORD dwWaitResult;

	dwWaitResult = WaitForSingleObject(sem->sem, INFINITE);

	if (dwWaitResult == WAIT_OBJECT_0)
		return 0;

	return -1;
}

static int osaSemPost(osa_sem_t *sem)
{
	ReleaseSemaphore(sem->sem, 1, NULL);
	return 0;
}
#else
#include <unistd.h>
#include <semaphore.h>

typedef struct __osa_sem_t
{
	sem_t sem;
	char obj_name[32];
}osa_sem_t;

static int osaSemInit(osa_sem_t *sem_st, int pshared, unsigned int value)
{
#ifdef __MAC__
	char sem_name[36] = { 0 };
	/* MacOS X doesn't support anonymous semaphore */
	pj_create_random_string(sem_name, 23);
	sem_name[23] = '\0';

	/* Create semaphore */
	sem_st->sem = sem_open(sem_name, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, value);
	if (sem_st->sem == SEM_FAILED)
	{
		return -1;
	}
	/* And immediately release the name as we don't need it */
	sem_unlink(sem_name);

#else
	if (sem_init(&sem_st->sem, pshared, value) != 0)
		return -1;
#endif
	return 0;
}

static int osaSemWait(osa_sem_t *sem_st)
{
	int result;

#ifdef __MAC__
	result = sem_wait(sem_st->sem);
#else
	result = sem_wait(&sem_st->sem);
#endif

	if (result == 0)
		return 0;

	/*printf("osaSemWait error [%d]\n", errno);*/
	return -1;
}

static int osaSemDestroy(osa_sem_t *sem_st)
{
	int result;
#ifdef __MAC__
	result = sem_close(sem_st->sem);
#else
	result = sem_destroy(&sem_st->sem);
#endif
	if (result == 0)
		return 0;

	/*printf("osa_sem_destroy error [%d]\n", errno);*/
	return -1;
}

static int osaSemPost(osa_sem_t *sem_st)
{
	int result;

#ifdef __MAC__
	result = sem_post(sem_st->sem);
#else
	result = sem_post(&sem_st->sem);
#endif

	if (result == 0)
		return 0;

	/*printf("osaSemPost error [%d]\n", errno);*/
	return -1;
}

#endif

/*
 * <函数功能：目录不存在，创建目录。存在，不做操作。支持多层目录创建
 * <	函数参数：path_name，路径名。路径名要处理特殊字符，以保证目录正确性
 * <	返回值：0成功，<  0  失败
 */
int osaLogCreateDir(char* path_name)
{
	int i = 0;
	int len = 0;
	char dir_path[256] = { 0 };

	if (NULL == path_name) {
		return -1;
	}
	len = strlen(path_name);
	dir_path[len] = '\0';
	strncpy(dir_path, path_name, len);

	for (i = 0; i < len; i++) {
		if (dir_path[i] == '/' && i > 0) {
			dir_path[i] = '\0';
			if (access(dir_path, 0) < 0) {
				if (mkdir(dir_path, 0755) < 0) {
					/*printf("mkdir=%s:msg=%s\n", dir_path, strerror(errno));*/
					printf("ERROR: mkdir [%s]\n", dir_path);
					return -1;
				}
			}
			dir_path[i] = '/';
		}
	}
	printf("osa_log_create_dir [%s] \n", path_name);
	return 0;
}

typedef int      BOOL;
#ifndef FALSE
#define FALSE    0
#endif

#ifndef TRUE
#define TRUE     1
#endif

static int g_osa_log_init_flag = 0;

static char g_osa_log_prefix[5][32] = {
	"debuglog.critical",
	"debuglog.error",
	"debuglog.warning",
	"debuglog.info",
	"debuglog.debug"
};

static int         g_osa_log_file_no[100];
static char        g_osa_log_file_prefix[128];
static FILE       *g_osa_log_fp[100];
static BOOL        g_osa_log_open_flag[100];
static int         g_osa_log_level;

static osa_sem_t g_osa_log_lock;
static char      g_osa_log_module_name[32];

static char      g_osa_log_path[256]  = { 0 };
static char      g_osa_log_module[32] = { 0 };

#define MAX_DATA_NAME_LEN    64
#define MAX_DATA_LEN         1024
#define MAX_FILE_SIZE        64 * 1024 * 1024

static int osaOpenLogFile(const char *expanded_name, int expanded_num)
{
	int  file_len;
	char log_file_name[128];

	g_osa_log_file_no[expanded_num] = g_osa_log_file_no[expanded_num] % 3 + 1;
	sprintf(log_file_name, "%s_%s_%03d.log", g_osa_log_file_prefix, expanded_name, g_osa_log_file_no[expanded_num]);

	g_osa_log_fp[expanded_num] = fopen(log_file_name, "ab");
	g_osa_log_open_flag[expanded_num] = (g_osa_log_fp[expanded_num] != NULL);
	if (!g_osa_log_open_flag[expanded_num]) {
		printf("open log file : [%s] failed.\n", log_file_name);
		return ERRNO_EOPENFILE;
	}

	fseek(g_osa_log_fp[expanded_num], 0, SEEK_END);
	file_len = ftell(g_osa_log_fp[expanded_num]);
	if (file_len >= MAX_FILE_SIZE)
	{
		fclose(g_osa_log_fp[expanded_num]);
		g_osa_log_fp[expanded_num] = NULL;
		g_osa_log_open_flag[expanded_num] = FALSE;

		g_osa_log_fp[expanded_num] = fopen(log_file_name, "wb");
		if (!g_osa_log_open_flag[expanded_num]) {
			printf("open log file : [%s] failed.\n", log_file_name);
			return ERRNO_EOPENFILE;
		}
	}

	return ERRNO_ESUCCESS;
}

static int osaWriteErrorFile(const char *data_name, char *data, const char *expanded_name, int expanded_num)
{
#if 1
	int file_len;

	osaSemWait(&g_osa_log_lock);

	if (!g_osa_log_open_flag[expanded_num])
		osaOpenLogFile(expanded_name, expanded_num);

	if (!g_osa_log_open_flag[expanded_num])
	{
		osaSemPost(&g_osa_log_lock);
		return ERRNO_ESUCCESS;
	}

	fprintf(g_osa_log_fp[expanded_num], "[%s]%s", data_name, data);
	fflush(g_osa_log_fp[expanded_num]);

	file_len = (int)ftell(g_osa_log_fp[expanded_num]);
	if (file_len >= MAX_FILE_SIZE)
	{
		fclose(g_osa_log_fp[expanded_num]);
		g_osa_log_fp[expanded_num] = NULL;
		g_osa_log_open_flag[expanded_num] = FALSE;
	}

	osaSemPost(&g_osa_log_lock);
#endif

	return ERRNO_ESUCCESS;
}

static int osaWriteErrorFileEx(char *data_name, const char *data, int data_size, const char *expanded_name, int expanded_num)
{
#if 1
	static char buf[128];
	int file_len;

	osaSemWait(&g_osa_log_lock);

	if (!g_osa_log_open_flag[expanded_num])
		osaOpenLogFile(expanded_name, expanded_num);

	if (!g_osa_log_open_flag[expanded_num])
	{
		osaSemPost(&g_osa_log_lock);
		return ERRNO_ESUCCESS;
	}

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "[%s]", data_name);
	/*buf_len = strlen(buf);*/
	fprintf(g_osa_log_fp[expanded_num], buf);
	fflush(g_osa_log_fp[expanded_num]);

	fwrite((const void*)data, data_size, 1, g_osa_log_fp[expanded_num]);
	fflush(g_osa_log_fp[expanded_num]);

	file_len = (int)ftell(g_osa_log_fp[expanded_num]);
	if (file_len >= MAX_FILE_SIZE)
	{
		fclose(g_osa_log_fp[expanded_num]);
		g_osa_log_open_flag[expanded_num] = FALSE;
	}

	osaSemPost(&g_osa_log_lock);
#endif

	return ERRNO_ESUCCESS;
}

BOOL isLeapYear(int year)
{
	return (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
}

void nolocksLocaltime(struct tm *tmp, time_t t, time_t tz, int dst) {
	const time_t secs_min  = 60;
	const time_t secs_hour = 3600;
	const time_t secs_day  = 3600 * 24;

	t -= tz;                      /* Adjust for timezone.      */
	t += 3600 * dst;              /* Adjust for daylight time. */
	time_t days = t / secs_day;   /* Days passed since epoch.  */
	time_t seconds = t % secs_day;/* Remaining seconds.        */

	tmp->tm_isdst = dst;
	tmp->tm_hour = seconds / secs_hour;
	tmp->tm_min = (seconds % secs_hour) / secs_min;
	tmp->tm_sec = (seconds % secs_hour) % secs_min;

	/* 1/1/1970 was a Thursday, that is, day 4 from the POV of the tm structure * where sunday = 0,
	so to calculate the day of the week we have to add 4 * and take the modulo by 7. */
	tmp->tm_wday = (days + 4) % 7;
	/* Calculate the current year. */
	tmp->tm_year = 1970;
	while (1) {
		/* Leap years have one day more. */
		time_t days_this_year = 365 + isLeapYear(tmp->tm_year);
		if (days_this_year > days) break;
		days -= days_this_year;
		tmp->tm_year++;
	}
	tmp->tm_yday = days;/* Number of day of the current year. */

	/* We need to calculate in which month and day of the month we are.
	To do * so we need to skip days according to how many days there are in each * month,
	and adjust for the leap year that has one more day in February. */
	int mdays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	mdays[1] += isLeapYear(tmp->tm_year);

	tmp->tm_mon = 0;
	while (days >= mdays[tmp->tm_mon]) {
		days -= mdays[tmp->tm_mon];
		tmp->tm_mon++;
	}

	tmp->tm_mday = days + 1;  /* Add 1 since our 'days' is zero-based. */
	tmp->tm_year -= 1900;   /* Surprisingly tm_year is year-1900. */
}

#ifndef WIN32
/*
#define GET_PTHREAD     pthread_self()
#define GET_PID         getpid()
*/
#define GET_PTHREAD     0
#define GET_PID         0
#else
/*
#define GET_PTHREAD     getCurrentThreadId()
*/
#define GET_PTHREAD     0
#define GET_PID         0
#endif

#define USE_SYSTEM_TIME
#ifndef USE_SYSTEM_TIME
int g_osa_time_millis_code = 0;
DateTime g_osa_dt = { 0 };
void osaGetTimeStr(char *time_str)
{
	int64 timestamp;
	DateTime dt;

	DateTime_getSystemTime(&dt);
	DateTime_addTimeZone(&dt, 8);

	if (g_osa_dt.m_year == dt.m_year && g_osa_dt.m_month == dt.m_month && g_osa_dt.m_day == dt.m_day &&
		g_osa_dt.m_hours == dt.m_hours && g_osa_dt.m_minutes == dt.m_minutes && g_osa_dt.m_seconds == dt.m_seconds) {
		g_sys_navigation_debug_time_millis_code++;
	}
	else {
		g_osa_time_millis_code = 0;
		g_osa_dt = dt;
	}

	sprintf(time_str, "<%4d%02d%02d%02d%02d%02d.%03d>:", dt.m_year, dt.m_month, dt.m_day,
		dt.m_hours + 8, dt.m_minutes, dt.m_seconds, g_osa_time_millis_code);
}

#else
void osaGetTimeStr(char *time_str)
{
	struct timespec ts_now;
	struct tm       s_tm;
#ifdef WIN32
	osaClockGettime(CLOCK_REALTIME, &ts_now);
#else
	clock_gettime(CLOCK_REALTIME, &ts_now);
#endif
	/*ptm = localtime(&ts_now.tv_sec);*/
	nolocksLocaltime(&s_tm, ts_now.tv_sec, 0, 8);
	sprintf(time_str, "<%4d%02d%02d%02d%02d%02d.%03ld>[t %lu][p %d]:", s_tm.tm_year + 1900, s_tm.tm_mon + 1, s_tm.tm_mday,
		s_tm.tm_hour, s_tm.tm_min, s_tm.tm_sec, ts_now.tv_nsec / 1000000,
		GET_PTHREAD, GET_PID);
}
#endif

/*only internal use*/
int osaLogOutput(int log_level, const char *expanded_name, int expanded_num, char *log_content)
{
	static char data[1024]   = { 0 };
	char        time_str[128] = { 0 };
	char        data_name[MAX_DATA_NAME_LEN];

	if (log_level > OSA_LOG_DEBUG) {
		log_level = OSA_LOG_DEBUG;
	}

	sprintf(data_name, "%s", g_osa_log_prefix[log_level]);
	if (g_osa_log_module_name[0])
		strcat(data_name, g_osa_log_module_name);

	osaGetTimeStr(time_str);

	strncpy(data, time_str, sizeof(time_str));
	strcat(data, log_content);

	printf(data);

	osaWriteErrorFile(data_name, data, expanded_name, expanded_num);
	/*osaWriteErrorFileEx(data_name, data);*/

	return ERRNO_ESUCCESS;
}

int osaLogOutputEx(int log_level, const char *expanded_name, int expanded_num, char *log_content, int log_size)
{
	static char data[1024];
	char        time_str[32];
	char        data_name[MAX_DATA_NAME_LEN];
	int         data_len;

	if (log_level > OSA_LOG_DEBUG) {
		log_level = OSA_LOG_DEBUG;
	}

	sprintf(data_name, "%s", g_osa_log_prefix[log_level]);
	if (g_osa_log_module_name[0]) {
		strcat(data_name, g_osa_log_module_name);
	}

	osaGetTimeStr(time_str);
	memset(data, 0, sizeof(data));
	strncpy(data, time_str, sizeof(time_str));
	data_len = strlen(data);
	memcpy(data + data_len, log_content, log_size);
	data_len += log_size;

	osaWriteErrorFileEx(data_name, data, data_len, expanded_name, expanded_num);

	return ERRNO_ESUCCESS;
}

int osaLogPrintf(int log_level, const char *file, const char *function,
	long line, const char *expanded_name, int expanded_num, const char * fmt, ...)
{
	/*
	if (expanded_num < OSA_RECORD_INFO_COUNT && osa_record_info[expanded_num].used == FALSE) {
		return ERRNO_ESUCCESS;
	}
	*/
	static char sz_info[64 * 1024];
	char    file_path[256] = { 0 };
	long    str_len;
	va_list args;

	if (!g_osa_log_init_flag) {
		osaLogInit(OSA_LOG_DEBUG, (char *)OSA_LOG_DIR);
		osaLogInfo(LOG_EXPANDED_NAME, LOG_EXPANDED_NUM,
			"\nOSSP LOG VERSION [%s]\n", DEBUG_LOG_VERSIN);
	}

	if (!g_osa_log_init_flag)
		return ERRNO_EINIT;

	if (log_level > g_osa_log_level)
		return 0;

	/*printf("COMPILE_ROOT_DIR [%s].\n", COMPILE_ROOT_DIR);*/
	if (COMPILE_ROOT_DIR != NULL) {
		/*
		printf("COMPILE_ROOT_DIR != NULL\n");
		printf("strlen(file) [%d], strlen(COMPILE_ROOT_DIR) [%d]\n", strlen(file), strlen(COMPILE_ROOT_DIR));
		*/
		if (strlen(file) > strlen(COMPILE_ROOT_DIR)) {
			strncpy(file_path, file + strlen(COMPILE_ROOT_DIR), strlen(file) - strlen(COMPILE_ROOT_DIR));
			sprintf(sz_info, "%s:%ld %s ", file_path, line, function);
		}
		else {
			sprintf(sz_info, "%s:%ld %s ", file, line, function);
		}
	}
	else {
		sprintf(sz_info, "%s:%ld %s ", file, line, function);
	}
	va_start(args, fmt);
	vsprintf(sz_info + strlen(sz_info), fmt, args);
	va_end(args);

	str_len = strlen(sz_info);
	if (str_len >= MAX_DATA_LEN - 32)
	{
		osaWriteErrorFile((char *)"big_data_info", sz_info, expanded_name, expanded_num);
		return ERRNO_EINVALIDPARAM;
	}

	if (sz_info[str_len - 1] != '\n')
	{
		sz_info[str_len] = '\n';
		sz_info[str_len + 1] = 0;
	}

	return osaLogOutput(log_level, expanded_name, expanded_num, sz_info);
}

int osaLogPrintfEx(int log_level, const char *file, const char *function,
	long line, const char *expanded_name, int expanded_num, const char * log_text, int log_size)
{
	static char info[64 * 1024];
	long        info_len;

	if (!g_osa_log_init_flag)
		return ERRNO_EINIT;

	if (log_level > g_osa_log_level)
		return 0;

	memset(info, 0, sizeof(info));
	sprintf(info, "%s:%ld %s ", file, line, function);
	info_len = strlen(info);
	if ((int)(sizeof(info) - info_len - 1) < log_size) {
		osaWriteErrorFileEx("big_data_info", log_text, log_size, expanded_name, expanded_num);
		return ERRNO_EINVALIDPARAM;
	}

	memcpy(info + info_len, log_text, log_size);
	info_len += log_size;

	if (info_len >= MAX_DATA_LEN - 32)
	{
		osaWriteErrorFileEx("big_data_info", info, info_len, expanded_name, expanded_num);
		return ERRNO_EINVALIDPARAM;
	}

	if (info[info_len - 1] != '\n')
	{
		info[info_len] = '\n';
		info[info_len + 1] = 0;
	}

	return osaLogOutputEx(log_level, expanded_name, expanded_num, info, info_len);
}

int osaLogClose()
{
	int i;
	int count;

	if (!g_osa_log_init_flag) {
		return ERRNO_ESUCCESS;
	}

	osaSemWait(&g_osa_log_lock);
	count = sizeof(g_osa_log_open_flag) / sizeof(BOOL);
	for (i = 0; i < count; i++) {
		if (g_osa_log_open_flag[i]) {
			fclose(g_osa_log_fp[i]);
			g_osa_log_open_flag[i] = FALSE;
			g_osa_log_file_no[i] = 0;
		}
	}
	osaSemPost(&g_osa_log_lock);
	osaSemDestroy(&g_osa_log_lock);
	g_osa_log_init_flag = 0;
	return ERRNO_ESUCCESS;
}

int osaLogForceClose()
{
	int i;
	int count;

	if (!g_osa_log_init_flag) {
		return ERRNO_ESUCCESS;
	}

	/*osaSemWait(&g_osa_log_lock);*/
	count = sizeof(g_osa_log_open_flag) / sizeof(BOOL);
	for (i = 0; i < count; i++) {
		if (g_osa_log_open_flag[i]) {
			fclose(g_osa_log_fp[i]);
			g_osa_log_open_flag[i] = FALSE;
			g_osa_log_file_no[i] = 0;
		}
	}
	/*osaSemPost(&g_osa_log_lock);*/
	osaSemDestroy(&g_osa_log_lock);
	g_osa_log_init_flag = 0;
	return ERRNO_ESUCCESS;
}

int osaLogIsInit()
{
	return g_osa_log_init_flag;
}

int osaLogInit(int log_level, const char *error_file_prefix)
{
	char    *module_name;
	int      module_name_offset = 1;

	if (g_osa_log_init_flag)
		return ERRNO_ESUCCESS;

	if (error_file_prefix) {
		strncpy(g_osa_log_file_prefix, error_file_prefix, sizeof(g_osa_log_file_prefix) - 1);
	}
	else {
		strcpy(g_osa_log_file_prefix, "/tmp/osa");
		printf("error_file_prefix is NULL, module_name is set to (osa)\n");
	}

	module_name = strrchr(g_osa_log_file_prefix, '/');
	if (!module_name)
	{
		module_name = strrchr(g_osa_log_file_prefix, '\\');
		if (!module_name)
		{
			module_name = g_osa_log_file_prefix;
			module_name_offset = 0;
		}
	}
	g_osa_log_module_name[0] = '.';
#ifdef WIN32
	strncpy_s(g_osa_log_module_name + 1, sizeof(g_osa_log_module_name) - 1, (module_name + module_name_offset), sizeof(g_osa_log_module_name) - 2);
#else
	sprintf(g_osa_log_module_name + 1, "%s", (module_name + module_name_offset));
#endif
	g_osa_log_module_name[sizeof(g_osa_log_module_name) - 1] = 0;

	g_osa_log_level = log_level;

	osaSemInit(&g_osa_log_lock, 0, 1);

	g_osa_log_init_flag = 1;

	/*文件目录，模块名称*/
	strncpy(g_osa_log_module, module_name + module_name_offset, sizeof(g_osa_log_module) - 1);
	if (module_name_offset == 1)
	{
		memcpy(g_osa_log_path, g_osa_log_file_prefix, strlen(g_osa_log_file_prefix) - strlen(g_osa_log_module));
	}
	else
	{
		memcpy(g_osa_log_path, g_osa_log_file_prefix, strlen(g_osa_log_file_prefix));
	}

	osaLogCreateDir(g_osa_log_path);
#if 0
	cqWCHAR wFullPath[CQ_MAX_ABS_PATH] = { 0 };
	cq_decodeUtf8(g_sys_navigation_debug_log_path, strlen(g_sys_navigation_debug_log_path), wFullPath, sizeof(wFullPath) - sizeof(cqWCHAR));
	FileSys_createDeepDir(wFullPath);
#endif

	printf("LOG MODULE[%s] LOG PATH[%s]\n", g_osa_log_module, g_osa_log_path);

	return ERRNO_ESUCCESS;
}
