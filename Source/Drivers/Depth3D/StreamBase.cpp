#include "StreamBase.h"
#ifndef _MSC_VER
#include<sys/time.h>
#endif

using namespace depth3d;

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
	switch(propertyId)
	{
		case ONI_STREAM_PROPERTY_VIDEO_MODE:
		case ONI_STREAM_PROPERTY_HORIZONTAL_FOV:
		case ONI_STREAM_PROPERTY_VERTICAL_FOV:
		case ONI_STREAM_PROPERTY_AUTO_WHITE_BALANCE:
		case ONI_STREAM_PROPERTY_AUTO_EXPOSURE:
		case ONI_STREAM_PROPERTY_EXPOSURE:
		case ONI_STREAM_PROPERTY_GAIN:
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
			
		default:
			status = ONI_STATUS_NOT_SUPPORTED;
	}


	return status;
}

OniStatus OzStream::setProperty(int propertyId, const void* data, int dataSize)
{
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

