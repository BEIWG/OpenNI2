#include "Depth3D_Driver.h"
#include "Depth3D_Device.h"
#include "DefaultParameters.h"

#ifdef _MSC_VER
#include "CameraDS.h"
#include "usb_camera.hpp"  
#else
#include "uvc.h"

#endif

using namespace depth3d;
char *gStreamBuffer = NULL;
bool bStreamThreadRun  = false;
bool gColorDataUpdate = false;
bool gDepthDataUpdate = false;
bool gIRDataUpdate = false;
bool bStreamDepthRun = false;
bool bStreamColorRun = false;
bool bStreamIRRun = false;

XN_THREAD_HANDLE m_Handle;

#ifdef _MSC_VER
#include "CameraDS.h"
#include "usb_camera.hpp"  
#else
#include "uvc.h"

uvc_context_t *ctx;
uvc_error_t res;
uvc_device_t *dev;
uvc_device_handle_t *devh;
uvc_stream_ctrl_t ctrl;

int gDropFrame = 3;

void uvc_cb(uvc_frame_t *uvc_frame, void *ptr) 
{
	uvc_frame_t *bgr;
	uvc_error_t ret;

	if (gStreamBuffer == NULL) 
	{
		if (gDropFrame--)
			return;
			
		gStreamBuffer = (char*)malloc(uvc_frame->data_bytes);
		if (!gStreamBuffer) 
		{
			printf("unable to allocate  frame!");
			return;
		}
	} else {
		memcpy((unsigned char*)gStreamBuffer, (unsigned char*)uvc_frame->data, uvc_frame->data_bytes);
		if (bStreamColorRun)
			gColorDataUpdate = true;
				
		if (bStreamDepthRun)
		{
			gDepthDataUpdate = true;
			gIRDataUpdate = false;
		} else if (bStreamIRRun) 
		{
			gIRDataUpdate = true;	
			gDepthDataUpdate = false;
		}	
	}
}
#endif

#ifdef _MSC_VER
static XN_THREAD_PROC threadStream(XN_THREAD_PARAM pThreadParam)
{
	char szCamName[20] = { 0 };
	long recv = 0;
	int openflag = 0;
    
	CCameraDS m_CamDS;
	int m_iCamCount = CCameraDS::CameraCount();
	if (m_iCamCount == 0) return 0;

	for (auto n = 0; n < m_iCamCount; n++)
	{
		int retval = m_CamDS.CameraName(n, szCamName, sizeof(szCamName));
		if (retval > 0)
		{
			if (!strcmp(szCamName, "FX3 UVC"))
				if ((m_CamDS.OpenCamera(n, false, DEPTH_RESOLUTION_X*2, DEPTH_RESOLUTION_Y)))
					openflag = 1;				
		}
	}
                
	if (openflag == 0)
	{
		printf("ColorStream: open camera failed....\n");
		return 0;
	}
	
	gStreamBuffer = (char*)malloc(IMAGE_RESOLUTION_X*IMAGE_RESOLUTION_Y*4);
	if (!gStreamBuffer) 
	{
		printf("ColorStream malloc buffer failed...\n");
		return 0;
	}

	recv = m_CamDS.QueryFrame(gStreamBuffer);				
	
	bStreamThreadRun = true;
	while(bStreamThreadRun)
	{
		if (!bStreamDepthRun && !bStreamColorRun && !bStreamIRRun) break;
		
		recv = m_CamDS.QueryFrame(gStreamBuffer);
		if (recv == IMAGE_RESOLUTION_X*IMAGE_RESOLUTION_Y*4)
		{	
			if (bStreamColorRun)
				gColorDataUpdate = true;
				
			if (bStreamDepthRun)
			{
				gDepthDataUpdate = true;
				gIRDataUpdate = false;
			} else if (bStreamIRRun) 
			{
				gIRDataUpdate = true;	
				gDepthDataUpdate = false;
			}			
		}	
	}
	
	free(gStreamBuffer);
	gStreamBuffer = NULL;
	XN_THREAD_PROC_RETURN(XN_STATUS_OK);
}
#endif

OzDriver::OzDriver(OniDriverServices* pDriverServices) : DriverBase(pDriverServices)
{
}

OniStatus OzDriver::initialize(oni::driver::DeviceConnectedCallback deviceConnectedCallback,	\
		oni::driver::DeviceDisconnectedCallback deviceDisconnectedCallback,	\
		oni::driver::DeviceStateChangedCallback deviceStateChangedCallback,	\
		void* pCookie)
{
	#ifdef _MSC_VER
	if (-1 == camera_init())
		return ONI_STATUS_ERROR;			
	#else
	if(0 == uvc_dirver_init(DEPTH_RESOLUTION_X*2, DEPTH_RESOLUTION_Y, DEFAULT_FPS, USB_VID, USB_PID, USB_SN, uvc_cb))
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
	
	#ifdef _MSC_VER
	xnOSCreateThread(threadStream, this, &m_Handle);
	#endif
	
	return ONI_STATUS_OK;
}

void OzDriver::shutdown() {
	printf("OzDriver::shutdown....\n");
	bStreamThreadRun = false;
	
	#ifndef _MSC_VER	 
	uvc_stop_streaming(devh);
	uvc_close(devh);
	uvc_unref_device(dev);
	uvc_exit(ctx);	

	if (gStreamBuffer != NULL)
		free(gStreamBuffer);	
	#endif	
		
	if (gStreamBuffer)
	{
		free(gStreamBuffer);
		gStreamBuffer = NULL;
	}
}

ONI_EXPORT_DRIVER(OzDriver);