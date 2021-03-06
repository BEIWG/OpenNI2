#include "Depth3D_Driver.h"
#include "Depth3D_Device.h"
#include "StreamBase.h"
#include "DefaultParameters.h"

#ifdef _MSC_VER
#include "CameraDS.h"
#include "usb_camera.hpp"   
#include "ext\DCamDrv.h"
#include "ext\SVCeuctl.h"
#else
#include "uvc.h"
#include "DCamDrv.h"

#ifdef ANDROID
#include <android/log.h>
#endif

#define DEBUG 1

#if DEBUG && defined(ANDROID)
#include <android/log.h>
#  define  LOGD(x...)  __android_log_print(ANDROID_LOG_INFO,"Depth3D",x)
#  define  LOGE(x...)  __android_log_print(ANDROID_LOG_ERROR,"Depth3D",x)
#else
#  define  LOGD(...)
#  define  LOGE(...)
#endif

#endif

using namespace depth3d;

#define STREAM_IR   1
#define STREAM_DEPTH    2
	
char *gStreamBuffer = NULL;
bool bStreamThreadRun  = false;
bool gColorDataUpdate = false;
bool gDepthDataUpdate = false;
bool gIRDataUpdate = false;
bool bStreamDepthRun = false;
bool bStreamColorRun = false;
bool bStreamIRRun = false;

XN_THREAD_HANDLE m_Handle;

#ifndef _MSC_VER
uvc_context_t *ctx;
uvc_error_t res;
uvc_device_t *dev;
uvc_device_handle_t *devh;
uvc_stream_ctrl_t ctrl;

int gDropFrame = 3;

void uvc_cb(uvc_frame_t *uvc_frame, void *ptr) 
{
	int stream_type = 0;
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
			if (stream_type != STREAM_DEPTH)
			{
				SME_Projection_Set(0);
				SME_DispDepth_En(1);
				SME_Depth_Sel(1);
				SME_InPhase_Set(2510*818);
				stream_type = STREAM_DEPTH;
			}		
				
			gDepthDataUpdate = true;
			gIRDataUpdate = false;
		} else if (bStreamIRRun) 
		{		
			if (stream_type != STREAM_IR)
			{
				SME_Projection_Set(0);
				SME_DispDepth_En(0);
				SME_Depth_Sel(0);
				SME_InPhase_Set(2510 * 15);
				stream_type = STREAM_IR;
			}	
					
			gIRDataUpdate = true;	
			gDepthDataUpdate = false;
		}	
	}
}
#endif

#ifdef _MSC_VER
static XN_THREAD_PROC threadStream(XN_THREAD_PARAM pThreadParam)
{
	static const GUID xuGuidAN75779 =
	{ 0x1B9EE0C6, 0xAFEF, 0x4531, { 0x89, 0x25, 0x88, 0xA0, 0xAC, 0xD6, 0xDE, 0x27 } };
	SVCeuctl m_EUctl;
	char szCamName[20] = { 0 };
	long recv = 0;
	int openflag = 0;
	int stream_type = 0;
    
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

	if (m_EUctl.Open("FX3 UVC", xuGuidAN75779, 2) != S_OK)
	{
		printf("(m_EUctl->Open FAIL \n");
		return FALSE;
	}

	DCAM_DRV_Init(&m_EUctl);

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

                if (bStreamDepthRun)
                {
                    if (stream_type != STREAM_DEPTH)
                    {
                        SME_Projection_Set(1);
                        SME_DispDepth_En(1);
                        SME_Depth_Sel(1);
                        SME_InPhase_Set(2510*818);
                        stream_type = STREAM_DEPTH;
                    }
                }
                else if (bStreamIRRun)
                {
                    if (stream_type != STREAM_IR)
                    {
                        SME_Projection_Set(1);
                        SME_DispDepth_En(0);
                        SME_Depth_Sel(0);
                        SME_InPhase_Set(2510 * 15);
                        stream_type = STREAM_IR;
                    }
                }

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
	
        //POWER DOWN LED
	SME_Projection_Set(0);
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
	
	DCAM_DRV_Init(devh);
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

void* OzDriver::enableFrameSync(oni::driver::StreamBase** pStreams, int streamCount)
{
    return NULL;
}

void OzDriver::disableFrameSync(void* frameSyncGroup)
{	
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
	
	//power down led
	SME_Projection_Set(0);
}

ONI_EXPORT_DRIVER(OzDriver);
