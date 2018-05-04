#include <XnLog.h>
#include "ColorStream.h"
#include "DefaultParameters.h"
#include "SensorStreamProcessor.h"

using namespace depth3d;
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
	m_running = false;
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
        
	while (m_running)
	{
		OniFrame* pFrame = getServices().acquireFrame();

		if (pFrame == NULL) {printf("Didn't get frame...\n"); continue;}

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
		xnOSMemSet(pFrame->data, 0xc0, pFrame->dataSize);     			
		raiseNewFrame(pFrame);
		getServices().releaseFrame(pFrame);
		xnOSSleep(33);
	}
}