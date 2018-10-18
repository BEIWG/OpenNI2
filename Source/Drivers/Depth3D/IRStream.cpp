#include <XnLog.h>
#include "IRStream.h"
#include "DefaultParameters.h"
#include "SensorStreamProcessor.h"

using namespace depth3d;
extern char *gStreamBuffer;
extern bool gIRDataUpdate;
extern bool bStreamIRRun;

OzIRStream::OzIRStream()
{
	m_videoMode.pixelFormat = ONI_PIXEL_FORMAT_GRAY8;
	m_videoMode.fps = DEFAULT_FPS;
	m_videoMode.resolutionX = IR_RESOLUTION_X;
	m_videoMode.resolutionY = IR_RESOLUTION_Y;		
}

void OzIRStream::stop()
{
	printf("OzIRStream::stop()....\n");
	bStreamIRRun = false;
	m_running = false;
	xnOSSleep(100);
}

OniStatus OzIRStream::SetVideoMode(OniVideoMode* videoMode) 
{
	m_videoMode = *videoMode;
	return ONI_STATUS_OK;
}

OniStatus OzIRStream::GetVideoMode(OniVideoMode* pVideoMode)
{
	*pVideoMode = m_videoMode;
	return ONI_STATUS_OK;
}

void OzIRStream::Mainloop()
{

	int frameId = 1;
	XnStatus nRetVal = XN_STATUS_OK;
	#ifdef _MSC_VER
	DWORD  duration, delay;
	DWORD start, finish; 
	#endif
	
	bStreamIRRun = true;
	while (m_running)
	{	
		while (!gIRDataUpdate) 
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

		pFrame->sensorType = ONI_SENSOR_IR;
		pFrame->stride = m_videoMode.resolutionX*sizeof(OniGrayscale8Pixel);

		// Fill fake data frame      
		if (gIRDataUpdate)
		{	
			gIRDataUpdate = false;
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

