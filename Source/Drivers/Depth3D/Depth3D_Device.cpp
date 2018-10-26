#include "Depth3D_Device.h"
#include "DepthStream.h"
#include "ColorStream.h"
#include "IRStream.h"
#include "DefaultParameters.h"

using namespace depth3d;

OzDevice::OzDevice(OniDeviceInfo* pInfo, oni::driver::DriverServices& driverServices) : m_pInfo(pInfo), m_driverServices(driverServices)
{
	m_numSensors = 3;

	m_sensors[0].pSupportedVideoModes = XN_NEW_ARR(OniVideoMode, 1);
	m_sensors[0].sensorType = ONI_SENSOR_DEPTH;
	m_sensors[0].numSupportedVideoModes = 1;
	m_sensors[0].pSupportedVideoModes[0].pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
	m_sensors[0].pSupportedVideoModes[0].fps = DEFAULT_FPS;
	m_sensors[0].pSupportedVideoModes[0].resolutionX = DEPTH_RESOLUTION_X;
	m_sensors[0].pSupportedVideoModes[0].resolutionY = DEPTH_RESOLUTION_Y;

	m_sensors[1].pSupportedVideoModes = XN_NEW_ARR(OniVideoMode, 1);
	m_sensors[1].sensorType = ONI_SENSOR_COLOR;
	m_sensors[1].numSupportedVideoModes = 1;
	m_sensors[1].pSupportedVideoModes[0].pixelFormat = ONI_PIXEL_FORMAT_RGB888;
	m_sensors[1].pSupportedVideoModes[0].fps = DEFAULT_FPS;
	m_sensors[1].pSupportedVideoModes[0].resolutionX = IMAGE_RESOLUTION_X;
	m_sensors[1].pSupportedVideoModes[0].resolutionY = IMAGE_RESOLUTION_Y;

	m_sensors[2].pSupportedVideoModes = XN_NEW_ARR(OniVideoMode, 1);
	m_sensors[2].sensorType = ONI_SENSOR_IR;
	m_sensors[2].numSupportedVideoModes = 1;
	m_sensors[2].pSupportedVideoModes[0].pixelFormat = ONI_PIXEL_FORMAT_GRAY8;
	m_sensors[2].pSupportedVideoModes[0].fps = DEFAULT_FPS;
	m_sensors[2].pSupportedVideoModes[0].resolutionX = IR_RESOLUTION_X;
	m_sensors[2].pSupportedVideoModes[0].resolutionY = IR_RESOLUTION_Y;	
}
OniDeviceInfo* OzDevice::GetInfo()
{
	return m_pInfo;
}

OniStatus OzDevice::getSensorInfoList(OniSensorInfo** pSensors, int* numSensors)
{
	*numSensors = m_numSensors;
	*pSensors = m_sensors;

	return ONI_STATUS_OK;
}

oni::driver::StreamBase* OzDevice::createStream(OniSensorType sensorType)
{
	if (sensorType == ONI_SENSOR_DEPTH)
	{
		OzDepthStream* pDepth = XN_NEW(OzDepthStream);
		return pDepth;
	}
	
	if (sensorType == ONI_SENSOR_COLOR)
	{
		OzImageStream* pImage = XN_NEW(OzImageStream);
		return pImage;
	}

	if (sensorType == ONI_SENSOR_IR)
	{
		OzIRStream* pImage = XN_NEW(OzIRStream);
		return pImage;
	}
		
	m_driverServices.errorLoggerAppend("OzDevice: Can't create a stream of type %d", sensorType);
	return NULL;
}

void OzDevice::destroyStream(oni::driver::StreamBase* pStream)
{
	//XN_DELETE(pStream);
}

OniStatus  OzDevice::getProperty(int propertyId, void* data, int* pDataSize)
{
	OniStatus rc = ONI_STATUS_OK;

	switch (propertyId)
	{
	case ONI_DEVICE_PROPERTY_DRIVER_VERSION:
		{
			if (*pDataSize == sizeof(OniVersion))
			{
				OniVersion* version = (OniVersion*)data;
				version->major = version->minor = version->maintenance = version->build = 2;
			}
			else
			{
				m_driverServices.errorLoggerAppend("Unexpected size: %d != %d\n", *pDataSize, sizeof(OniVersion));
				rc = ONI_STATUS_ERROR;
			}
		}
		break;
	default:
		m_driverServices.errorLoggerAppend("Unknown property: %d\n", propertyId);
		rc = ONI_STATUS_ERROR;
	}
	return rc;
}

OniBool OzDevice::isCommandSupported(int commandId)
{
	return ONI_STATUS_NOT_IMPLEMENTED;
}

OniStatus OzDevice::tryManualTrigger()
{
	return ONI_STATUS_NOT_IMPLEMENTED;
}

OniBool OzDevice::isImageRegistrationModeSupported(OniImageRegistrationMode mode)
{
	return (mode == ONI_IMAGE_REGISTRATION_DEPTH_TO_COLOR || mode == ONI_IMAGE_REGISTRATION_OFF);
}