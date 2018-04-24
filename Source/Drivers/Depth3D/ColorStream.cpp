#include "ColorStream.h"
#include "DefaultParameters.h"
#include "SensorStreamProcessor.h"

using namespace depth3d;

void OzImageStream::stop()
{
	printf("OzImageStream::stop()....\n");
	m_running = false;
}

OniStatus OzImageStream::SetVideoMode(OniVideoMode*) 
{
	return ONI_STATUS_OK;
}

OniStatus OzImageStream::GetVideoMode(OniVideoMode* pVideoMode)
{
	pVideoMode->pixelFormat = ONI_PIXEL_FORMAT_RGB888;
	pVideoMode->fps = DEFAULT_FPS;
	pVideoMode->resolutionX = IMAGE_RESOLUTION_X;
	pVideoMode->resolutionY = IMAGE_RESOLUTION_Y;
	return ONI_STATUS_OK;
}

void OzImageStream::Mainloop()
{
	int frameId = 1;
        
	while (m_running)
	{
		OniFrame* pFrame = getServices().acquireFrame();

		if (pFrame == NULL) {printf("Didn't get frame...\n"); continue;}

		// Fill metadata
		pFrame->frameIndex = frameId++;

		pFrame->videoMode.pixelFormat = ONI_PIXEL_FORMAT_RGB888;
		pFrame->videoMode.resolutionX = IMAGE_RESOLUTION_X;
		pFrame->videoMode.resolutionY = IMAGE_RESOLUTION_Y;
		pFrame->videoMode.fps = DEFAULT_FPS;

		pFrame->width = IMAGE_RESOLUTION_X;
		pFrame->height = IMAGE_RESOLUTION_Y;

		pFrame->cropOriginX = pFrame->cropOriginY = 0;
		pFrame->croppingEnabled = FALSE;

		pFrame->sensorType = ONI_SENSOR_COLOR;
		pFrame->stride = IMAGE_RESOLUTION_X*3;
		pFrame->timestamp = GetTimestamp();
		// Fill fake data frame
		xnOSMemSet(pFrame->data, 0xc0, pFrame->dataSize);     			
		raiseNewFrame(pFrame);
		getServices().releaseFrame(pFrame);
		xnOSSleep(33);
	}
}