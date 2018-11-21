#include "StreamBase.h"
#ifndef _MSC_VER
#include<sys/time.h>
#endif
#include <XnLog.h>
using namespace depth3d;

#define XN_MASK_DEVICE_SENSOR "Depth3D"
OzStream::~OzStream()
{
	stop();
}

OniStatus OzStream::start()
{
	xnOSCreateThread(threadFunc, this, &m_threadHandle);

	return ONI_STATUS_OK;
}

OniBool OzStream::isPropertySupported(int propertyId)
{
	OniBool status;

	xnLogVerbose(XN_MASK_DEVICE_SENSOR, "isPropertySupported %d", propertyId);
	switch(propertyId)
	{
		case ONI_STREAM_PROPERTY_VIDEO_MODE:
		case ONI_STREAM_PROPERTY_HORIZONTAL_FOV:
		case ONI_STREAM_PROPERTY_VERTICAL_FOV:
		case ONI_STREAM_PROPERTY_AUTO_WHITE_BALANCE:
		case ONI_STREAM_PROPERTY_AUTO_EXPOSURE:
		case ONI_STREAM_PROPERTY_EXPOSURE:
		case ONI_STREAM_PROPERTY_GAIN:
		case ONI_STREAM_PROPERTY_MAX_VALUE:
		case ONI_STREAM_PROPERTY_MIN_VALUE:
		case ONI_STREAM_PROPERTY_MIRRORING:
		{
				status = true;
				break;
		}
		
		default:
			status = false;
	}
	
	return status;
}

OniStatus OzStream::getProperty(int propertyId, void* data, int* pDataSize)
{
	OniStatus status = ONI_STATUS_NOT_SUPPORTED;

	xnLogVerbose(XN_MASK_DEVICE_SENSOR, "getProperty %d", propertyId);
	switch(propertyId)
	{
		case ONI_STREAM_PROPERTY_VIDEO_MODE:
			{
				if (*pDataSize != sizeof(OniVideoMode))
				{
					printf("Unexpected size: %d != %d\n", *pDataSize, (int)sizeof(OniVideoMode));
					status = ONI_STATUS_ERROR;
				}
				else
				{
					status = GetVideoMode((OniVideoMode*)data);
				}
				break;
			}
		case ONI_STREAM_PROPERTY_HORIZONTAL_FOV:
			{
				float* val = (float*)data;
				XnDouble tmp;
				tmp =  H_FOV;
				*val = (float)tmp;
				status = ONI_STATUS_OK;
				break;
			}		
		case ONI_STREAM_PROPERTY_VERTICAL_FOV:
			{
				float* val = (float*)data;
				XnDouble tmp;
				tmp =  V_FOV;
				*val = (float)tmp;
				status = ONI_STATUS_OK;
				break;
			}
		case ONI_STREAM_PROPERTY_MAX_VALUE:
			{
				int *val = (int*)data;
				*val = 0x0ffff;
				status = ONI_STATUS_OK;
				break;
			}

		case ONI_STREAM_PROPERTY_MIN_VALUE:
			{
				int *val = (int*)data;
				*val = 500;
				status = ONI_STATUS_OK;
				break;
			}
			
			
		default:
			status = ONI_STATUS_NOT_SUPPORTED;
	}


	return status;
}

OniStatus OzStream::setProperty(int propertyId, const void* data, int dataSize)
{
	xnLogVerbose(XN_MASK_DEVICE_SENSOR, "setProperty %d", propertyId);
	if (propertyId == ONI_STREAM_PROPERTY_VIDEO_MODE)
	{
		if (dataSize != sizeof(OniVideoMode))
		{
			printf("Unexpected size: %d != %d\n", dataSize, (int)sizeof(OniVideoMode));
			return ONI_STATUS_ERROR;
		}
		return SetVideoMode((OniVideoMode*)data);
	}

	return ONI_STATUS_NOT_IMPLEMENTED;
}

OniStatus OzStream::convertDepthToColorCoordinates(oni::driver::StreamBase* colorStream, 
								int depthX, int depthY, OniDepthPixel depthZ, int* pColorX, int* pColorY)
{
	*pColorX = depthX;
	*pColorY = depthY;
	
	return ONI_STATUS_OK;
}
