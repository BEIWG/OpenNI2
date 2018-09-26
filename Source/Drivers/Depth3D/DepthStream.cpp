#include <XnLog.h>
#include "DepthStream.h"
#include "DefaultParameters.h"
#include "SensorStreamProcessor.h"

using namespace depth3d;

bool gSoftFilterEnable = true;
int dMin = 0, dMax = 128, diff = 1;

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

int gDropFrame = 10;
char *DataCacheBuffer = NULL;
bool gDataUpdate = false;

xnl::CriticalSection m_frameSyncCs;

void uvc_cb(uvc_frame_t *uvc_frame, void *ptr) 
{
	uvc_frame_t *bgr;
	uvc_error_t ret;

	if (DataCacheBuffer == NULL) 
	{
		if (gDropFrame--)
			return;
			
		DataCacheBuffer = (char*)malloc(uvc_frame->data_bytes);
		if (!DataCacheBuffer) 
		{
			printf("unable to allocate  frame!");
			return;
		}
	} else {
		m_frameSyncCs.Lock();
		memcpy((unsigned char*)DataCacheBuffer, (unsigned char*)uvc_frame->data, uvc_frame->data_bytes);
		gDataUpdate = true;
		m_frameSyncCs.Unlock();
	}
}
#endif

OzDepthStream::OzDepthStream()
{
	m_videoMode.pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
	m_videoMode.fps = DEFAULT_FPS;
	m_videoMode.resolutionX = DEPTH_RESOLUTION_X;
	m_videoMode.resolutionY = DEPTH_RESOLUTION_Y;	
}

void OzDepthStream::stop()
{
	printf("OzDepthStream::stop()....\n");
	
	m_running = false;
	xnOSSleep(100);
	
	#ifndef _MSC_VER	 
	uvc_stop_streaming(devh);
	uvc_close(devh);
	uvc_unref_device(dev);
	uvc_exit(ctx);	

	if (DataCacheBuffer != NULL)
		free(DataCacheBuffer);	
	#endif		
}

OniStatus OzDepthStream::SetVideoMode(OniVideoMode* videoMode) 
{
	m_videoMode = *videoMode;
	return ONI_STATUS_OK;
}

OniStatus OzDepthStream::GetVideoMode(OniVideoMode* pVideoMode)
{
	*pVideoMode = m_videoMode;
	return ONI_STATUS_OK;
}

OniStatus OzDepthStream::convertDepthToColorCoordinates(StreamBase* colorStream, int depthX, int depthY, OniDepthPixel depthZ, int* pColorX, int* pColorY)
{ 
	return ONI_STATUS_NOT_SUPPORTED;
}

void OzDepthStream::Mainloop()
{
	int frameId = 1;
	XnStatus nRetVal = XN_STATUS_OK;
	    
	#ifdef _MSC_VER
	DWORD  duration, delay;
	DWORD start, finish; 
	char szCamName[20] = { 0 };
	long recv = 0;
	int openflag = 0;
             
	CCameraDS m_CamDS;
	int m_iCamCount = CCameraDS::CameraCount();
	if (m_iCamCount == 0) return;

	for (auto n = 0; n < m_iCamCount; n++)
	{
		int retval = m_CamDS.CameraName(n, szCamName, sizeof(szCamName));
		if (retval > 0)
		{
			if (!strcmp(szCamName, "FX3 UVC"))
				if ((m_CamDS.OpenCamera(n, false, DEPTH_RESOLUTION_X, DEPTH_RESOLUTION_Y)))
					openflag = 1;				
		}
	}
                
	if (openflag == 0)
	{
		printf("DepthStream: open camera failed....\n");
		return;
	}
	#endif
	while (m_running)
	{			
		#ifdef _MSC_VER			
		start = GetTickCount();
		#else
		while (!gDataUpdate) 
		{
			xnOSSleep(3);
		}
		#endif			
				
		OniFrame* pFrame = getServices().acquireFrame();
		if (pFrame == NULL) 
		{
			printf("Didn't get frame...\n"); 
			continue;
		}
	
		// Fill metadata
		XnUInt64 nTimestamp = 0;
		nRetVal = xnOSGetHighResTimeStamp(&nTimestamp);
		if (nRetVal != XN_STATUS_OK)
		{
			xnLogWarning("Depth3D Driver", "Failed to get timestamp from os: %s", xnGetStatusString(nRetVal));
		}
	
		pFrame->timestamp = nTimestamp;		
		pFrame->frameIndex = frameId++;
		pFrame->videoMode.pixelFormat = m_videoMode.pixelFormat;
		pFrame->videoMode.resolutionX = m_videoMode.resolutionX;
		pFrame->videoMode.resolutionY = m_videoMode.resolutionY;
		pFrame->videoMode.fps = m_videoMode.fps;
	
		pFrame->width = m_videoMode.resolutionX;
		pFrame->height = m_videoMode.resolutionY;
	
		pFrame->cropOriginX = pFrame->cropOriginY = 0;
		pFrame->croppingEnabled = FALSE;
		
		pFrame->sensorType = ONI_SENSOR_DEPTH;
		pFrame->stride = m_videoMode.resolutionX*sizeof(OniDepthPixel);
						
	      // Fill frame
		#ifdef _MSC_VER
		recv = m_CamDS.QueryFrame((char*)pFrame->data);
		if (recv == pFrame->dataSize)
		{		

			if (gSoftFilterEnable)
				DepthfilterSpeckles((unsigned char*)pFrame->data, pFrame->width, pFrame->height, (dMin - 1) * 16.0, 400, diff * 16.0);

			finish = GetTickCount();
			duration =1000/m_videoMode.fps;
			if (duration > (finish - start))
			{
				delay = duration - (finish - start);
				xnOSSleep(delay);
			}

			raiseNewFrame(pFrame);
		}

		#else      
		m_frameSyncCs.Lock();
		gDataUpdate = false;
		xnOSMemCopy((pFrame->data), DataCacheBuffer, pFrame->dataSize); 
		if (gSoftFilterEnable)
			DepthfilterSpeckles((unsigned char*)pFrame->data, pFrame->width, pFrame->height, (dMin - 1) * 16.0, 400, diff * 16.0);
		
		raiseNewFrame(pFrame);
		m_frameSyncCs.Unlock();
		#endif
		getServices().releaseFrame(pFrame);		
	}
}

