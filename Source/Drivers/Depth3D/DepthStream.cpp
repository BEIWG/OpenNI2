#include <XnLog.h>
#include "DepthStream.h"
#include "DefaultParameters.h"
#include "SensorStreamProcessor.h"

using namespace depth3d;

bool gSoftFilterEnable = true;
int dMin = 0, dMax = 128, diff = 1;
extern char *gStreamBuffer;
extern bool gDepthDataUpdate;
extern bool bStreamDepthRun;

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
	
	bStreamDepthRun = false;
	m_running = false;
	xnOSSleep(100);
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
	#endif
	
	bStreamDepthRun = true;
	while (m_running)
	{			
		while (!gDepthDataUpdate) 
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
		
		pFrame->sensorType = ONI_SENSOR_DEPTH;
		pFrame->stride = m_videoMode.resolutionX*sizeof(OniDepthPixel);
						
		// Fill frame
		if (gDepthDataUpdate)
		{
			gDepthDataUpdate = false;
			
			GetDepthFromStream(gStreamBuffer, (char*)pFrame->data);
			if (gSoftFilterEnable)
				DepthfilterSpeckles((unsigned char*)pFrame->data, pFrame->width, pFrame->height, (dMin - 1) * 16.0, 400, diff * 16.0);
			
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

