#include "imelog.h"
#include "osastroper.h"
#include <stdio.h>
#include <string.h>

#define EC_SUCCESS    0
#define OSA_MAX_BUF_SIZE    1024

/*
不是返回-1，是返回开始存在的那个位
start是包括0的，这是数组的下标
*/
int osaIsSub(int start, const char *str, const char *sub)
{
	int i;
	int k;
	int j;
	int len;
	if (str == NULL){
		return -1;
	}
	len = strlen(str);

	i = start;
	while (i < len) {
		k = i;

		for (j = 0; sub[j] && k < len; j++, k++){
			if (sub[j] != str[k]){
				break;
			}
		}
		if (!sub[j]) {/* 说明找到了 */
			return i;
		}
		
		/* 说明未找到 */
		i++;
	}

	return -1;
}

char * osaStrToToken(char *str, const char *delim, char **saveptr)
{
	int pos;

	if (str == NULL){
		str = *saveptr;
	}
	pos = osaIsSub(0, str, delim);
	if (pos != -1) {/* 说明找到分隔符位置了 */
		str[pos] = '\0';
		*saveptr = &str[pos + strlen(delim)];
		return str;
	} else {/* 说明无法找到分隔符位置了 */
		*saveptr = NULL;
		return str;
	}
}

/*
过程：以delim为分隔符，切分src_str字符串到pp_token中，切分token_count-1次
返回值：返回切分的元素个数
*/
int osaSplitStrToToken(char *src_str, const char *delim, char **pp_token, int token_count)
{
	char *saveptr = NULL;
	int   i;

	i = 0;
	pp_token[i] = osaStrToToken(src_str, delim, &saveptr);
	while (pp_token[i] != NULL)
	{
		if (++i >= token_count)
			break;
		pp_token[i] = osaStrToToken(NULL, delim, &saveptr);
	}

	return i;
}

/*
返回子串个数，不存在子串返回0
start是包括0的，这是数组的下标
*/
int osaGetSubCount(int start, const char *str, const char *sub)
{
	int i;
	int k;
	int j;
	int len;
	int count = 0;
	if (str == NULL){
		return count;
	}
	len = strlen(str);

	i = start;
	while (i < len) {
		k = i;

		for (j = 0; sub[j] && k < len; j++, k++){
			if (sub[j] != str[k]){
				break;
			}
		}
		if (!sub[j]) {/* 说明找到了 */
			count++;
			i += strlen(sub);
		} else {/* 说明未找到 */
			i++;
		}
	}

	return count;
}

int osaCalculateReplaceLength(const char *src_str, const char *old_str, const char *new_str, int max)
	/* max为最大替换次数（max为-1，计算全部可替换字符串） */
{
	int count = 0;
	int time = 0;/* 次数 */

	count = osaGetSubCount(0, src_str, old_str);
	if (max != -1 && count >= max){
		time = max;
	} else {/* count < max */
		time = count;
	}

	return strlen(src_str) + (strlen(new_str) - strlen(old_str)) * time;
}

/*
 替换字符串
 max为最大替换次数（max为-1，替换全部可替换字符串）
 dst_str字符串尾端不会增加'\0'
*/
int osaReplaceStr(const char *src_str, char *dst_str, const char *old_str, const char *new_str, int max)
{
	int src_i = 0;
	int src_j = 0;
	int dst_i = 0;
	int pos = 0;
	int j = 0;
	int count = 0;
	int src_str_len = strlen(src_str);

	while (src_i < src_str_len) {
		pos = osaIsSub(src_i, src_str, old_str);
		if (pos == -1) {/* 如果没有子字符串了，拷贝剩余数据 */
			for (src_j = src_i; src_j < src_str_len; src_j++) {/*strcpy(dst_str + dst_i, src_str + src_j);*/
				dst_str[dst_i++] = src_str[src_j];
			}
			break;
		}

		if (max != -1 && count >= max) {/* 说明已经替换满了，不需要替换了 */
			for (src_j = src_i; src_j < src_str_len; src_j++) {/*strcpy(dst_str + dst_i, src_str + src_j);*/
				dst_str[dst_i++] = src_str[src_j];
			}
			break;
		}
		count++;

		for (src_j = src_i; src_j < pos; src_j++) {/* 拷贝之前的数据 */
			dst_str[dst_i++] = src_str[src_j];
		}
		for (j = 0; new_str[j]; j++) {/* 拷贝替换字符串的数据 */
			dst_str[dst_i++] = new_str[j];
		}
		src_i = pos + strlen(old_str);
	}

	return count;
}

int osaTrimStr(char* str)
{
	long i, j, str_len;
	char temp[128];

	if (str == NULL) {
		return EC_SUCCESS;
	}

	str_len = strlen(str);

	if (str_len > 127) {
		str_len = 127;
	}

	for (i = 0; i < str_len; i++) {
		if (str[i] != ' ' && str[i] != '\t') {
			break;
		}
	}
	strcpy(temp, str + i);

	str_len = strlen(temp);
	for (j = str_len - 1; j > 0; j--) {
		if (temp[j] == ' ' || temp[j] == '\t' || temp[j] == '\r' || temp[j] == '\n') {
			continue;
		}
		break;
	}

	temp[j + 1] = 0;

	strcpy(str, temp);
	return EC_SUCCESS;
}

/*
int maxNum = -1
maxNum表示max num，指定要取得的内容数量的上限
此函数返回获得的数据个数
*/
int osaTakeTagTextFromFile(const char *file_path, const char *before_tag, const char *after_tag, char *text_list, int text_size, int max_num)
{
	int balance = 0;/*表示标签的数目*/

	char context[OSA_MAX_BUF_SIZE * 10];
	int ci = 0;
	int btagLen = strlen(before_tag);/*before tag length*/
	int bi = 0;
	int atagLen = strlen(after_tag);/*after tag length*/
	int ai = 0;
	/*int noteflag=0;*/

	/*TCHAR DealStr[10];*/
	char element[2]; element[1] = '\0';

	FILE *fp = NULL;
	fp = fopen(file_path, "rb");
	if (!fp) {
		osaLogError(LOG_EXPANDED_NAME, LOG_EXPANDED_NUM, "ERROR: the file [%s] was not opened.\n", file_path);
		return balance;
	}

	int RSIZE = 1 * sizeof(char);/*读取大小*/
	int j = 0;
	int k = -1;
	while (1) {
		if (j == k) {
			break;
		} else {
			k = j;
		}

		while (fread(element, RSIZE, 1, fp)) {
			/*cout<<DealStr[0];*/
			j++;/*用于最大的跳出*/
			if (before_tag[bi] == element[0]) {
				/*cout<<DealStr[0]<<endl;*/
				/*TEMP();*/
				bi++;
				if (bi == btagLen) {
					bi = 0;
					break;
				}
			} else {
				bi = 0;
			}
		}

		while (fread(element, RSIZE, 1, fp)) {
			/*cout<<DealStr[0];*/
			j++;/*用于最大的跳出*/
			if (after_tag[ai] == element[0]) {
				if (atagLen == 1) {/*当AfterTag长度只有1的时候会出现错误，加长这条就不会了2008年10月12日 09:27:26*/
					/*
					cout<<"进入1"<<endl;
					cout<<DealStr[0]<<endl;
					TEMP();
					*/
					ai = -1;
					break;
				}
				ai++;

				while (fread(element, RSIZE, 1, fp)) {
					/*
					cout<<"进入"<<endl;
					cout<<DealStr[0]<<endl;
					TEMP();
					*/
					j++;/*用于最大的跳出*/
					if (after_tag[ai] == element[0]) {
						ai++;
						if (ai == atagLen) { /*Ai暂作标志（表示已经放入了一个完整的元素）*/
							ai = -1;
							break;
						}
					} else {
						j -= ai; /* j--;*/

						/*
						int i = -ai;
						File_seek(&fs, FileSeek_Current, -ai * RSIZE);
						*/
						fseek(fp, -ai * RSIZE, SEEK_CUR);
						context[ci] = after_tag[0];
						ci++;
						context[ci] = '\0';/*用于安全检测*/
						/*if (Echeck.error_strlength(context, conLen - 10, _T("S_Sentence::TakeTagInToLinkFromFile()"), "0") == 1) { ai = 0; break; }*/

						ai = 0;
						break;
					}
				}
				if (ai == -1) {
					break;
				}
			} else {
				/*cout<<"OK"<<endl;*/
				/*cout<<DealStr[0];*/
				context[ci] = element[0];
				ci++;
				context[ci] = '\0';/*用于安全检测*/
				/*if (Echeck.error_strlength(Context, ConL - 10, _T("S_Sentence::TakeTagInToLinkFromFile()"), "0") == 1) { Ci = 0; break; }*/
			}
		}

		if (ai == -1) {/*满足条件的情况*/
			context[ci] = '\0';

			/*
			cout<<Context<<endl;
			MFC_ShowIO ppi;
			ppi.MessageShow(Context);
			cout<<"长度="<<strlen(Context)<<endl;
			cout<<"OK"<<endl;

			int linkL = link.ABS_Length();
			link.ST_Insert(linkL + 1, Context);
			link.insertText(-1, context);
			*/
			strncpy(text_list, context, text_size - 1);
			text_list += text_size;

			balance++;
			if (balance == max_num) break;
			ci = 0;
			ai = 0;
		}
	}
	fclose(fp);

	return balance;
}

/*
int maxNum = -1
maxNum表示max num，指定要取得的内容数量的上限
此函数返回获得的数据个数
*/
int osaTakeTagTextFromStr(const char *text, const char *before_tag, const char *after_tag, char *text_list, int text_size, int max_num)

{
	int balance = 0;/*表示标签的数目*/
	int textLen = (int)strlen(text);/*read length*/

	char context[OSA_MAX_BUF_SIZE * 10];/*【内部自我变数】原来这个数据项是1000*/
	int ci = 0;
	int btagLen = (int)strlen(before_tag);/*before tag length*/
	int bi = 0;
	int atagLen = (int)strlen(after_tag);/*after tag length*/
	int ai = 0;
	/*
	int noteflag=0;
	TCHAR DealStr[10];
	*/
	char element[2]; element[1] = '\0';

	int j = 0;
	int k = -1;
	while (1) {
		if (j == k) {
			break;
		} else {
			k = j;
		}
		/*
		cout<<j<<endl;
		cout<<"获得的文本"<<endl;
		cout<<Context<<endl;
		cout<<"OVER"<<endl;
		TEMP();
		int BreakFlag=0;
		*/
		while (j < textLen) {
			/*cout<<DealStr[0];*/
			element[0] = text[j];
			j++;/*用于最大的跳出*/
			if (before_tag[bi] == element[0]) {
				/*cout<<DealStr[0]<<endl;*/
				/*TEMP();*/
				bi++;
				if (bi == btagLen) {
					bi = 0;
					break;
				}
			} else {
				bi = 0;
			}
		}

		while (j < textLen) {
			/*cout<<DealStr[0];*/
			element[0] = text[j];
			j++;/*用于最大的跳出*/
			if (after_tag[ai] == element[0]) {
				if (atagLen == 1) {/*当AfterTag长度只有1的时候会出现错误，加上这条就不会了2008年10月12日 09:27:26*/
					ai = -1;
					break;
				}

				ai++;
				while (j < textLen) {
					element[0] = text[j];
					j++;/*用于最大的跳出*/
					if (after_tag[ai] == element[0]) {
						ai++;
						if (ai == atagLen) { /*Ai暂作标志（表示已经放入了一个完整的元素）*/
							ai = -1;
							break;
						}
					} else {
						j -= ai; /* j--;*/
						/*
						int i=-Ai;
						file.seekg(i,ios::cur);
						*/
						if (ci < sizeof(context)) {
							context[ci] = after_tag[0];
							ci++;
						}

						ai = 0;
						break;
					}
				}

				if (ai == -1) {
					break;
				}
			} else {
				/*cout<<"OK"<<endl;*/
				/*cout<<DealStr[0];*/
				if (ci < sizeof(context)) {
					context[ci] = element[0];
					ci++;
				}
			}
		}

		if (ai == -1)
		{
			context[ci] = '\0';
			/*
			cout<<Context<<endl;
			cout<<"长度="<<strlen(Context)<<endl;
			cout<<"OK"<<endl;
			int linkL = link.ABS_Length();
			link.ST_Insert(linkL + 1, Context);
			link.insertText(-1, context);
			*/
			strncpy(text_list, context, text_size - 1);
			text_list += text_size;

			balance++;
			if (balance == max_num) {
				break;
			}

			ci = 0;
			ai = 0;
		}
	}

	return balance;
}
