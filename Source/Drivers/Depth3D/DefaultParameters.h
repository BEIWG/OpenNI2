#ifndef _DEFAULT_PARAMETERS_H
#define _DEFAULT_PARAMETERS_H

#include "Driver/OniDriverAPI.h"
#include "XnLib.h"
#include "XnHash.h"
#include "XnEvent.h"
#include "XnPlatform.h"


namespace depth3d {
	
#define USB_VID			0x00
#define USB_PID			0x00
#define USB_SN 			NULL

#define DEVICE_VENDOR		"beiwg"
#define DEVICE_URI		"Depth3D"

#define H_FOV			60.0
#define V_FOV			70.0

#define DEFAULT_FPS		0
#define IMAGE_RESOLUTION_X	640
#define IMAGE_RESOLUTION_Y	400

#define IR_RESOLUTION_X		640
#define IR_RESOLUTION_Y		400

#define DEPTH_RESOLUTION_X	640
#define DEPTH_RESOLUTION_Y	400
}
#endif