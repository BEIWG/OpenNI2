#ifndef _UVC_H_
#define _UVC_H_

#include "libuvc/libuvc.h"

int uvc_dirver_init(int width, int height, int fps, int vid, int pid, const char *sn, uvc_frame_callback_t *cb);

#endif