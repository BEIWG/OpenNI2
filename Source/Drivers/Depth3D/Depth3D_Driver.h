#ifndef __DEPTH3D_DRIVER_H_
#define __DEPTH3D_DRIVER_H_

#include "Driver/OniDriverAPI.h"
#include "XnLib.h"
#include "XnHash.h"
#include "XnEvent.h"
#include "XnPlatform.h"
#include "Depth3D_Device.h"
#include "DefaultParameters.h"

namespace depth3d {

class OzDriver : public oni::driver::DriverBase 
{
public:
	OzDriver(OniDriverServices* pDriverServices);

	OniStatus initialize(oni::driver::DeviceConnectedCallback deviceConnectedCallback,	\
			oni::driver::DeviceDisconnectedCallback deviceDisconnectedCallback,	\
			oni::driver::DeviceStateChangedCallback deviceStateChangedCallback,	\
			void* pCookie);

	virtual oni::driver::DeviceBase* deviceOpen(const char* uri, const char* /*mode*/)
	{
		for (xnl::Hash<OniDeviceInfo*, oni::driver::DeviceBase*>::Iterator iter = m_devices.Begin(); iter != m_devices.End(); ++iter)
		{
			if (xnOSStrCmp(iter->Key()->uri, uri) == 0)
			{
				// Found
				if (iter->Value() != NULL)
				{
					// already using
					return iter->Value();
				}

				OzDevice* pDevice = XN_NEW(OzDevice, iter->Key(), getServices());
				iter->Value() = pDevice;
				return pDevice;
			}
		}

		getServices().errorLoggerAppend("Looking for '%s'", uri);
		return NULL;
	}

	virtual void deviceClose(oni::driver::DeviceBase* pDevice)
	{
		for (xnl::Hash<OniDeviceInfo*, oni::driver::DeviceBase*>::Iterator iter = m_devices.Begin(); iter != m_devices.End(); ++iter)
		{
			if (iter->Value() == pDevice)
			{
				iter->Value() = NULL;
				XN_DELETE(pDevice);
										
				return;
			}
		}

		// not our device?!
		XN_ASSERT(FALSE);
	}

	virtual OniStatus tryDevice(const char* uri)
	{
		if (xnOSStrCmp(uri, DEVICE_URI) != 0)
		{
			return ONI_STATUS_ERROR; 
		} 

		OniDeviceInfo* pInfo = XN_NEW(OniDeviceInfo);
		xnOSStrCopy(pInfo->uri, DEVICE_URI, ONI_MAX_STR);
		xnOSStrCopy(pInfo->vendor, "BWG", ONI_MAX_STR);
		m_devices[pInfo] = NULL;

		deviceConnected(pInfo);

		return ONI_STATUS_OK;
	}

	void shutdown();

	virtual void* enableFrameSync(oni::driver::StreamBase** pStreams, int streamCount);
	virtual void disableFrameSync(void* frameSyncGroup);
	
protected:
        typedef struct
        {
            OzDevice* pDevice;
        } FrameSyncGroup;

	XN_THREAD_HANDLE m_threadHandle;

	xnl::Hash<OniDeviceInfo*, oni::driver::DeviceBase*> m_devices;
};
} // namespace
#endif 