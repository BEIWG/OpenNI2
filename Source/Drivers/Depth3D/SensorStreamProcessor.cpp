#ifdef _MSC_VER
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include "SensorStreamProcessor.h"

int GetDepthDataFromStream(char* stream, char* depth)
{
	return 0;
}

int GetColorDataFromStream(char* stream, char* depth)
{
	return 0;
}

int GetIRDataFromStream(char* stream, char* depth)
{
	return 0;	
}

uint64_t GetTimestamp()
{
	uint64_t timestamp = 0;
#ifdef _MSC_VER
	SYSTEMTIME tv;
	GetLocalTime(&tv);
	timestamp = ((tv.wYear - 1970)*SECOND_PER_YEAR + (tv.wMonth - 1)*SECOND_PER_MONTH + (tv.wDay - 1)*SECOND_PER_DAY +
		tv.wHour*3600 + tv.wMinute*60 + tv.wSecond)*1000 + tv.wMilliseconds;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	timestamp = tv.tv_sec*1000 + tv.tv_usec/1000;
#endif	
	return timestamp;
}