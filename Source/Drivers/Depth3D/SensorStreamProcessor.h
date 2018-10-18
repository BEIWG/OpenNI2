#ifndef __SENSOR_STREAM_PROCESS_
#define __SENSOR_STREAM_PROCESS_
#include "DefaultParameters.h"

int GetDepthFromStream(char* stream, char* depth);
int GetIRFromStream(char* stream, char* IR);
int GetColorFromStream(char* stream, char* color);

void  DepthfilterSpeckles(unsigned char* img, int width, int height, int newVal, int maxSpeckleSize, int maxDiff);
#endif