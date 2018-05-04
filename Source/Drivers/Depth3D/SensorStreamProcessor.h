#ifndef __SENSOR_STREAM_PROCESS_
#define __SENSOR_STREAM_PROCESS_
#include "DefaultParameters.h"

int GetDepthDataFromStream(char* stream, char* depth);

int GetColorDataFromStream(char* stream, char* color);

int GetIRDataFromStream(char* stream, char* ir);

void  DepthfilterSpeckles(unsigned char* img, int width, int height, int newVal, int maxSpeckleSize, int maxDiff);
#endif