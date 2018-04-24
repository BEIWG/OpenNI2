#include "IRStream.h"
#include "DefaultParameters.h"
#include "SensorStreamProcessor.h"

using namespace depth3d;

void OzIRStream::stop()
{
	printf("OzIRStream::stop()....\n");
	m_running = false;
}

OniStatus OzIRStream::SetVideoMode(OniVideoMode*) 
{
	return ONI_STATUS_OK;
}

OniStatus OzIRStream::GetVideoMode(OniVideoMode* pVideoMode)
{
	pVideoMode->pixelFormat = ONI_PIXEL_FORMAT_GRAY8;
	pVideoMode->fps = DEFAULT_FPS;
	pVideoMode->resolutionX = IR_RESOLUTION_X;
	pVideoMode->resolutionY = IR_RESOLUTION_Y;
	return ONI_STATUS_OK;
}

void OzIRStream::Mainloop()
{

	int frameId = 1;             

	while (m_running)
	{
		OniFrame* pFrame = getServices().acquireFrame();

		if (pFrame == NULL) {printf("Didn't get frame...\n"); continue;}

		// Fill metadata
		pFrame->frameIndex = frameId++;

		pFrame->videoMode.pixelFormat = ONI_PIXEL_FORMAT_GRAY8;
		pFrame->videoMode.resolutionX = IR_RESOLUTION_X;
		pFrame->videoMode.resolutionY = IR_RESOLUTION_Y;
		pFrame->videoMode.fps = DEFAULT_FPS;

		pFrame->width = IR_RESOLUTION_X;
		pFrame->height = IR_RESOLUTION_Y;

		pFrame->cropOriginX = pFrame->cropOriginY = 0;
		pFrame->croppingEnabled = FALSE;

		pFrame->sensorType = ONI_SENSOR_IR;
		pFrame->stride = IR_RESOLUTION_X*sizeof(OniGrayscale8Pixel);

		// Fill fake data frame      
		xnOSMemSet(pFrame->data, 0xc0, pFrame->dataSize);
		pFrame->timestamp = GetTimestamp();
		raiseNewFrame(pFrame);
      
		getServices().releaseFrame(pFrame);
		xnOSSleep(33);
	}
}