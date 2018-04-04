/*********************************************************************
* Software License Agreement (BSD License)
*
*  Copyright (C) 2010-2012 Ken Tossell
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of the author nor other contributors may be
*     used to endorse or promote products derived from this software
*     without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*********************************************************************/
#include <stdio.h>
#include "libuvc/libuvc.h"
#include "uvc.h"

extern uvc_context_t *ctx;
extern uvc_error_t res;
extern uvc_device_t *dev;
extern uvc_device_handle_t *devh;
extern uvc_stream_ctrl_t ctrl;

int uvc_dirver_init(int width, int height, int fps, int vid, int pid, const char *sn, uvc_frame_callback_t *cb)
{
  res = uvc_init(&ctx, NULL);
  if (res < 0) {
    uvc_perror(res, "uvc_init");
    goto ERROR;
  }

  res = uvc_find_device(
      ctx, &dev,
      vid, pid, sn);
  if (res < 0) {
    uvc_perror(res, "uvc_find_device");
    goto ERROR;
  } else {
    res = uvc_open(dev, &devh);
    if (res < 0) {
      uvc_perror(res, "uvc_open");
      goto ERROR;
    } else {
      //uvc_print_diag(devh, stderr);
      res = uvc_get_stream_ctrl_format_size(
          devh, &ctrl, UVC_FRAME_FORMAT_YUYV, width, height, fps
      );

      //uvc_print_stream_ctrl(&ctrl, stderr);
      if (res < 0) {
        uvc_perror(res, "get_mode");
        goto ERROR;
      } else {
        res = uvc_start_streaming(devh, &ctrl, cb, NULL, 0);
        if (res < 0) {
          uvc_perror(res, "start_streaming");
          goto ERROR;
        }
      }
    }
  }

	return 1;
ERROR:
  return 0;
}

