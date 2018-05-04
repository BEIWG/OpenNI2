#ifdef _MSC_VER
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include "SensorStreamProcessor.h"

typedef unsigned char uchar;

struct Point2s{
        short x;
        short y;

        Point2s(short a, short b)
        {
                x = a;
                y = b;
        }
};

template <typename T>
T abs(T a, T b)
{
	if (a >= b)
		return (a - b);
	else
		return (b - a);
}

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


void  DepthfilterSpeckles(uchar* img, int width, int height, int newVal, int maxSpeckleSize, int maxDiff)
{
        int npixels = width*height;
        size_t bufSize = npixels*(int)(sizeof(Point2s) + sizeof(int) + sizeof(uchar));
        uchar *_buf = (uchar*)malloc((int)bufSize);
        if (_buf == NULL)
        {
                //printf("_buf malloc failed\n");
                return;
        }

        uchar* buf = _buf;
        int i, j, dstep = (int)(width/*img.step/sizeof(T)*/);
        int* labels = (int*)buf;
        buf += npixels*sizeof(labels[0]);
        Point2s* wbuf = (Point2s*)buf;
        buf += npixels*sizeof(wbuf[0]);
        uchar* rtype = (uchar*)buf;
        int curlabel = 0;

        // clear out label assignments
        memset(labels, 0, npixels*sizeof(labels[0]));

        for (i = 0; i < height; i++)
        {
                short* ds = ((short*)img) + i*width;
                int* ls = labels + width*i;

                for (j = 0; j < width; j++)
                {
                        if (ds[j] != newVal)   // not a bad disparity
                        {
                                if (ls[j])     // has a label, check for bad label
                                {
                                        if (rtype[ls[j]]) // small region, zero out disparity
                                                ds[j] = (short)newVal;
                                }
                                // no label, assign and propagate
                                else
                                {
                                        Point2s* ws = wbuf; // initialize wavefront
                                        Point2s p((short)j, (short)i);  // current pixel
                                        curlabel++; // next label
                                        int count = 0;  // current region size
                                        ls[j] = curlabel;

                                        // wavefront propagation
                                        while (ws >= wbuf) // wavefront not empty
                                        {
                                                count++;
                                                // put neighbors onto wavefront
                                                short* dpp = (short*)img+ width*p.y + p.x;//&img.at<T>(p.y, p.x);
                                                short dp = *dpp;
                                                int* lpp = labels + width*p.y + p.x;

                                                if (p.y < height - 1 && !lpp[+width] && dpp[+dstep] != newVal && abs<short>(dp, dpp[+dstep]) <= maxDiff)
                                                {
                                                        lpp[+width] = curlabel;
                                                        *ws++ = Point2s(p.x, p.y + 1);
                                                }

                                                if (p.y > 0 && !lpp[-width] && dpp[-dstep] != newVal && abs<short>(dp, dpp[-dstep]) <= maxDiff)
                                                {
                                                        lpp[-width] = curlabel;
                                                        *ws++ = Point2s(p.x, p.y - 1);
                                                }

                                                if (p.x < width - 1 && !lpp[+1] && dpp[+1] != newVal && abs<short>(dp, dpp[+1]) <= maxDiff)
                                                {
                                                        lpp[+1] = curlabel;
                                                        *ws++ = Point2s(p.x + 1, p.y);
                                                }

                                                if (p.x > 0 && !lpp[-1] && dpp[-1] != newVal && abs<short>(dp, dpp[-1]) <= maxDiff)
                                                {
                                                        lpp[-1] = curlabel;
                                                        *ws++ = Point2s(p.x - 1, p.y);
                                                }

                                                // pop most recent and propagate
                                                // NB: could try least recent, maybe better convergence
                                                p = *--ws;
                                        }

                                        // assign label type
                                        if (count <= maxSpeckleSize)   // speckle region
                                        {
                                                rtype[ls[j]] = 1;   // small region label
                                                ds[j] = (short)newVal;
                                        }
                                        else
                                                rtype[ls[j]] = 0;   // large region label
                                }
                        }
                }
        }

        free(_buf);
}