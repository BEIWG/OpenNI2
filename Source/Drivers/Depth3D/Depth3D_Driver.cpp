#include "Depth3D_Driver.h"
#include "Depth3D_Device.h"
#include "DefaultParameters.h"

#ifdef _MSC_VER
#include "CameraDS.h"
#include "usb_camera.hpp"  
#else
#include "uvc.h"
extern void uvc_cb(uvc_frame_t *uvc_frame, void *ptr);
#endif

using namespace depth3d;

OzDriver::OzDriver(OniDriverServices* pDriverServices) : DriverBase(pDriverServices)
{
}

OniStatus OzDriver::initialize(oni::driver::DeviceConnectedCallback deviceConnectedCallback,	\
		oni::driver::DeviceDisconnectedCallback deviceDisconnectedCallback,	\
		oni::driver::DeviceStateChangedCallback deviceStateChangedCallback,	\
		void* pCookie)
{
	#ifdef _MSC_VER
	if (-1 == camera_init(DEPTH_RESOLUTION_X, DEPTH_RESOLUTION_Y))
		return ONI_STATUS_ERROR;			
	#else
	if(0 == uvc_dirver_init(DEPTH_RESOLUTION_X, DEPTH_RESOLUTION_Y, DEFAULT_FPS, USB_VID, USB_PID, USB_SN, uvc_cb))
	{
		printf("Init Sensor Driver Failed...\n");
		return ONI_STATUS_NO_DEVICE;	
	}
	#endif
	DriverBase::initialize(deviceConnectedCallback, deviceDisconnectedCallback, deviceStateChangedCallback, pCookie);
	OniDeviceInfo* pInfo = XN_NEW(OniDeviceInfo);
	xnOSStrCopy(pInfo->uri, DEVICE_URI, ONI_MAX_STR);
	xnOSStrCopy(pInfo->vendor, DEVICE_VENDOR, ONI_MAX_STR);
	m_devices[pInfo] = NULL;

	deviceConnected(pInfo);
	deviceStateChanged(pInfo, ONI_DEVICE_STATE_OK);
	return ONI_STATUS_OK;
}

void OzDriver::shutdown() {
}

ONI_EXPORT_DRIVER(OzDriver);