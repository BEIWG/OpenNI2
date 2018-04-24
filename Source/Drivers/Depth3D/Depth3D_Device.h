#ifndef __DEPTH3D_DEVICE_H_
#define __DEPTH3D_DEVICE_H_
#include "Driver/OniDriverAPI.h"
#include "XnLib.h"
#include "XnHash.h"
#include "XnEvent.h"
#include "XnPlatform.h"

namespace depth3d {

class OzDevice : public oni::driver::DeviceBase 
{
public:
	OzDevice(OniDeviceInfo* pInfo, oni::driver::DriverServices& driverServices);
	OniDeviceInfo* GetInfo();

	OniStatus getSensorInfoList(OniSensorInfo** pSensors, int* numSensors);

	oni::driver::StreamBase* createStream(OniSensorType sensorType);

	void destroyStream(oni::driver::StreamBase* pStream);

	OniStatus  getProperty(int propertyId, void* data, int* pDataSize);

	OniBool isCommandSupported(int commandId);

	OniStatus tryManualTrigger();
	
	OniBool isImageRegistrationModeSupported(OniImageRegistrationMode mode);
	
private:
	OzDevice(const OzDevice&);
	void operator=(const OzDevice&);

	OniDeviceInfo* m_pInfo;
	int m_numSensors;
	OniSensorInfo m_sensors[10];
	oni::driver::DriverServices& m_driverServices;
};
} // namespace depth3d
#endif