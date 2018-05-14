/************************************************************************/
/*   日志操作 ，分为操作日志（operation.log）和错误日志文件(error.log)                                                                  */
/************************************************************************/
#ifndef __ES_LOG__
#define __ES_LOG__

#if defined(_WIN32)
#define LOG_BASEPATH	"G:\\tmp\\"
#else
#define LOG_BASEPATH  "/tmp/"
#endif

#define LOG_TRACE 1
#define LOG_ERROR 1

/*定义业务日志文件路径*/
#define TRACE_LOG_FILE  LOG_BASEPATH"es_clttrace.log"

/*定义错误日志文件路径*/
#define ERROR_LOG_FILE  LOG_BASEPATH"es_clterror.log"

#if (LOG_TRACE == 1)	
#if defined(_WIN32)
#define ES_LOG_TRACE(fmt, ...) \
    ES_WriteLog(TRACE_LOG_FILE, "[Time:%s Func: %s  LineNum: %d] "fmt, \
                ES_GetTimeNow() , __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define ES_LOG_TRACE(fmt, ...) \
    ES_WriteLog(TRACE_LOG_FILE, "[Time:%s Func: %s  LineNum: %d] "fmt, \
                ES_GetTimeNow() , __FUNCTION__, __LINE__,##__VA_ARGS__)
#endif
#else
#define ES_LOG_TRACE(fmt, ...)
#endif

#if (LOG_ERROR == 1)
#if defined(_WIN32)
#define ES_LOG_ERROR(fmt, ...)   \
    ES_WriteLog(ERROR_LOG_FILE, "[Time:%s Func: %s  LineNum: %d] "fmt, \
                ES_GetTimeNow() , __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define ES_LOG_ERROR(fmt, ...)   \
    ES_WriteLog(ERROR_LOG_FILE, "[Time:%s Func: %s  LineNum: %d] "fmt, \
                ES_GetTimeNow() , __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#else
#define ES_LOG_ERROR(fmt, ...)
#endif



int ES_WriteLog(char* path, char* fmt, ...);
char* ES_GetTimeNow(void);
int SetFileData(const char *pPath, const char *pBuf, int dataLen);
int GetFileData(const char *pPath, char *p_OutFile, int *pFileSize);
#endif