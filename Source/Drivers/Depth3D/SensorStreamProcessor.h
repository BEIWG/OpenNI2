#include <algorithm> 
#include "DefaultParameters.h"
typedef unsigned char uchar;

uint64_t GetTimestamp();

struct Point2s{
        short x;
        short y;

        Point2s(short a, short b)
        {
                x = a;
                y = b;
        }
};

int GetDepthDataFromStream(char* stream, char* depth);

int GetColorDataFromStream(char* stream, char* color);

int GetIRDataFromStream(char* stream, char* ir);


template <typename T>
void  DepthfilterSpeckles(uchar* img, int width, int height, int newVal, int maxSpeckleSize, int maxDiff)
{
        int npixels = width*height;
        size_t bufSize = npixels*(int)(sizeof(Point2s) + sizeof(int) + sizeof(uchar));
        uchar *_buf = (uchar*)malloc((int)bufSize);
        if (_buf == NULL)
        {
                printf("_buf malloc failed\n");
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
                T* ds = ((T*)img) + i;
                int* ls = labels + width*i;

                for (j = 0; j < width; j++)
                {
                        if (ds[j] != newVal)   // not a bad disparity
                        {
                                if (ls[j])     // has a label, check for bad label
                                {
                                        if (rtype[ls[j]]) // small region, zero out disparity
                                                ds[j] = (T)newVal;
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
                                                T* dpp = &(((T*)img)[width*p.y + p.x]);//&img.at<T>(p.y, p.x);
                                                T dp = *dpp;
                                                int* lpp = labels + width*p.y + p.x;

                                                if (p.y < height - 1 && !lpp[+width] && dpp[+dstep] != newVal && std::abs(dp - dpp[+dstep]) <= maxDiff)
                                                {
                                                        lpp[+width] = curlabel;
                                                        *ws++ = Point2s(p.x, p.y + 1);
                                                }

                                                if (p.y > 0 && !lpp[-width] && dpp[-dstep] != newVal && std::abs(dp - dpp[-dstep]) <= maxDiff)
                                                {
                                                        lpp[-width] = curlabel;
                                                        *ws++ = Point2s(p.x, p.y - 1);
                                                }

                                                if (p.x < width - 1 && !lpp[+1] && dpp[+1] != newVal && std::abs(dp - dpp[+1]) <= maxDiff)
                                                {
                                                        lpp[+1] = curlabel;
                                                        *ws++ = Point2s(p.x + 1, p.y);
                                                }

                                                if (p.x > 0 && !lpp[-1] && dpp[-1] != newVal && std::abs(dp - dpp[-1]) <= maxDiff)
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
                                                ds[j] = (T)newVal;
                                        }
                                        else
                                                rtype[ls[j]] = 0;   // large region label
                                }
                        }
                }
        }

        free(_buf);
}