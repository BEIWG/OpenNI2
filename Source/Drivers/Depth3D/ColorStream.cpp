#include <XnLog.h>
#include "ColorStream.h"
#include "DefaultParameters.h"
#include "SensorStreamProcessor.h"

using namespace depth3d;
extern char *gStreamBuffer;
extern bool gColorDataUpdate;
extern bool bStreamColorRun;

OzImageStream::OzImageStream()
{
	m_videoMode.pixelFormat = ONI_PIXEL_FORMAT_RGB888;
	m_videoMode.fps = DEFAULT_FPS;
	m_videoMode.resolutionX = IMAGE_RESOLUTION_X;
	m_videoMode.resolutionY = IMAGE_RESOLUTION_Y;		
}

void OzImageStream::stop()
{
	printf("OzImageStream::stop()....\n");
	bStreamColorRun = false;
	m_running = false;
	xnOSSleep(100);	
}

OniStatus OzImageStream::SetVideoMode(OniVideoMode* videoMode) 
{
	m_videoMode = *videoMode;
	return ONI_STATUS_OK;
}

OniStatus OzImageStream::GetVideoMode(OniVideoMode* pVideoMode)
{
	*pVideoMode = m_videoMode;
	return ONI_STATUS_OK;
}

void OzImageStream::Mainloop()
{
	int frameId = 1;
	XnStatus nRetVal = XN_STATUS_OK;
	#ifdef _MSC_VER
	DWORD  duration, delay;
	DWORD start, finish; 
	#endif
	
	bStreamColorRun = true;
	while (m_running)
	{		

		while (!gColorDataUpdate) 
		{
			xnOSSleep(1);
		}	

		#ifdef _MSC_VER			
		start = GetTickCount();
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

		pFrame->sensorType = ONI_SENSOR_COLOR;
		pFrame->stride = m_videoMode.resolutionX*3;

		// Fill fake data frame
		
		if (gColorDataUpdate)
		{	
			gColorDataUpdate = false;
			GetColorFromStream(gStreamBuffer, (char*)pFrame->data);	

		    #ifdef _MSC_VER	    
			finish = GetTickCount();
			duration =1000/m_videoMode.fps;
			if (duration > (finish - start))
			{
				delay = duration - (finish - start);
				xnOSSleep(delay);
			}
			#endif
			raiseNewFrame(pFrame);
		}

		getServices().releaseFrame(pFrame);		
	}
}

