
#include "es_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <iostream>
#include <stdarg.h> 
#ifdef WIN32
#include <sys\timeb.h>
#endif


int GetFileData(const char *pPath,  char *p_OutFile,  int *pFileSize)
{
	FILE *pFile = NULL;
	int pSize = 0;
	int rv = 0;
	//add secure marc
	//_CRT_SECURE_NO_WARNINGS
	pFile = fopen(pPath, "rb");
	if (pFile == NULL)
	{
		return -1;
	}

	fseek(pFile, 0, SEEK_END);
	pSize = ftell(pFile);
	if (pSize == NULL)
	{
		fclose(pFile);
		pFile = NULL;
		return -1;
	}
	if (p_OutFile == NULL)
	{
		*pFileSize = pSize;
		fclose(pFile);
		pFile = NULL;
		return 0;
	}
	rewind(pFile);
	char *pData = (char*)malloc(pSize);
	memset(pData, 0, pSize);

	int rev = fread(pData, 1, pSize, pFile);
	if (rev == 0 || rev != pSize)
	{
		free(pData);
		fclose(pFile);
		pFile = NULL;
		return -1;
	}
	*pFileSize = pSize;
	memcpy(p_OutFile, pData, pSize);
	free(pData);
	fclose(pFile);
	pFile = NULL;
	return 0;
}


int SetFileData(const char *pPath,  const char *pBuf, int dataLen)
{
	
	if (pPath == NULL  || pPath == '\0')
	{
		
		std::string filename(LOG_BASEPATH);
		filename += "tmp.dat";
		FILE *fp = fopen(filename.c_str(), "w+");
		if (fp != NULL)
		{
			fwrite(pBuf, 1, dataLen, fp);
			fclose(fp);
		}
		return  0;
	}
	
	FILE *fp = fopen(pPath, "w+");
	if (fp != NULL)
	{
		fwrite(pBuf, 1, dataLen, fp);
		fclose(fp);
	}
	else
	{
		return -1;
	}
	return 0;
}



int ES_WriteLog(char* path, char* fmt, ...)
{
    int logStrLen = 0;
    int rv = 0;
    char logData[1024];
    va_list vp;

#ifdef WIN32
    va_start(vp, fmt);
    logStrLen = vsnprintf_s(logData, sizeof(logData) - 2, fmt, vp);
    va_end(vp);
        strcat_s(logData, sizeof(logData), "\r\n");
#else
    va_start(vp, fmt);
    logStrLen = vsnprintf(logData, sizeof(logData) - 2, fmt, vp);
    va_end(vp);
        strncat(logData, "\r\n", sizeof(logData));
#endif


    rv = SetFileData(path, logData, logStrLen + 2);
    return rv;
}

char* ES_GetTimeNow(void)
{
    static char retbuf[50];
    char buf[40];
#ifdef WIN32
    struct tm ltm;
    struct _timeb tmb;

    _ftime(&tmb);
    localtime_s(&ltm, &tmb.time);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &ltm);
    sprintf_s(retbuf, "%s.%d", buf, tmb.millitm);
#else
    time_t tm;
    time(&tm);
    struct tm *ltm = localtime(&tm);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ltm);
        sprintf(retbuf, "%s.", buf);
#endif

    return retbuf;
}