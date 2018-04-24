#include "DepthStream.h"
#include "DefaultParameters.h"
#include "SensorStreamProcessor.h"

using namespace depth3d;
#ifndef _MSC_VER
#include "uvc.h"

uvc_context_t *ctx;
uvc_error_t res;
uvc_device_t *dev;
uvc_device_handle_t *devh;
uvc_stream_ctrl_t ctrl;

static int frame_drop = 10;
static char *data_tmp = NULL;
static int data_flag = 0;
xnl::CriticalSection m_frameSyncCs;

void uvc_cb(uvc_frame_t *uvc_frame, void *ptr) 
{
	uvc_frame_t *bgr;
	uvc_error_t ret;

	if (data_tmp == NULL) 
	{
		if (frame_drop--)
			return;
			
		data_tmp = (char*)malloc(uvc_frame->data_bytes);
        	if (!data_tmp) 
        	{
			printf("unable to allocate  frame!");
			return;
  	    	}
	} else {
		m_frameSyncCs.Lock();
		memcpy((unsigned char*)data_tmp, (unsigned char*)uvc_frame->data, uvc_frame->data_bytes);
		data_flag = 1;
		m_frameSyncCs.Unlock();
	}
}
#endif

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

	if (data_tmp != NULL)
		free(data_tmp);	
	#endif		
}

OniStatus OzDepthStream::SetVideoMode(OniVideoMode*) 
{
	return ONI_STATUS_OK;
}

OniStatus OzDepthStream::GetVideoMode(OniVideoMode* pVideoMode)
{
	pVideoMode->pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
	pVideoMode->fps = DEFAULT_FPS;
	pVideoMode->resolutionX = DEPTH_RESOLUTION_X;
	pVideoMode->resolutionY = DEPTH_RESOLUTION_Y;
	return ONI_STATUS_OK;
}

OniStatus OzDepthStream::convertDepthToColorCoordinates(StreamBase* colorStream, int depthX, int depthY, OniDepthPixel depthZ, int* pColorX, int* pColorY)
{ 
	return ONI_STATUS_NOT_SUPPORTED;
}

void OzDepthStream::Mainloop()
{
	int frameId = 1;             
	#ifdef _MSC_VER
	char szCamName[20] = { 0 };
        long recv = 0;
        int openflag = 0;
             

	CCameraDS m_CamDS;
        int m_iCamCount = CCameraDS::CameraCount();
        if (m_iCamCount == 0)
        {
		return;
	}

	for (auto n = 0; n < m_iCamCount; n++)
        {
		int retval = m_CamDS.CameraName(n, szCamName, sizeof(szCamName));
		if (retval > 0)
		{
			if (!strcmp(szCamName, "FX3 UVC"))
				if ((m_CamDS.OpenCamera(n, false, DEPTH_RESOLUTION_X, DEPTH_RESOLUTION_Y)))
				{
					openflag = 1;
				}
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
		m_CamDS.WaitForCompletion();
		#else
		while (!data_flag) 
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
		pFrame->frameIndex = frameId++;
	
		pFrame->videoMode.pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
		pFrame->videoMode.resolutionX = DEPTH_RESOLUTION_X;
		pFrame->videoMode.resolutionY = DEPTH_RESOLUTION_Y;
		pFrame->videoMode.fps = DEFAULT_FPS;
	
		pFrame->width = DEPTH_RESOLUTION_X;
		pFrame->height = DEPTH_RESOLUTION_Y;
	
		pFrame->cropOriginX = pFrame->cropOriginY = 0;
		pFrame->croppingEnabled = FALSE;
		
		pFrame->sensorType = ONI_SENSOR_DEPTH;
		pFrame->stride = DEPTH_RESOLUTION_X*sizeof(OniDepthPixel);
		pFrame->timestamp = GetTimestamp();
				
	      // Fill frame
		#ifdef _MSC_VER
		recv = m_CamDS.ReadFrame((char*)pFrame->data);
		if (recv == pFrame->dataSize)
		{				
			raiseNewFrame(pFrame);
		}			
		#else      
	        m_frameSyncCs.Lock();
	        data_flag = 0;
	        //cv::Mat input_mat(DEPTH_RESOLUTION_Y, DEPTH_RESOLUTION_X, CV_16SC1, data_tmp);
	        //cv::filterSpeckles(input_mat, (dMin - 1)*0, 200, diff*16.0);
	        //xnOSMemSet(pFrame->data, 0x01, pFrame->dataSize);
	        xnOSMemCopy((pFrame->data), data_tmp, pFrame->dataSize); 
	        raiseNewFrame(pFrame);
	        m_frameSyncCs.Unlock();
		#endif
		getServices().releaseFrame(pFrame);		
	}
}

