#pragma once

#include <mfapi.h>
#include <mfplay.h>
#include <mfreadwrite.h>
#include <vector>
#include <string>
#include <ks.h>
#include <ksproxy.h>
#include <vidcap.h>

//Macro to define propertyID
#define UVC_XU_I2C_ADDR_CONTROL                             (0x0001)
#define UVC_XU_I2C_DATA_CONTROL                             (0x0002)

//Macro to define cmdID, the same in FPGA logic
#define GC_WRITE                                            0x00
#define GC_READ                                             0x01
#define SC_WRITE                                            0x02
#define SC_READ                                             0x03
#define I2C_WRITE                                           0x04
#define I2C_READ                                            0x05
#define NVM_WRITE                                           0x80
#define NVM_READ                                            0xC0

//Macro to define function
#define setGCValue(cs, addr, pd)           setBasicValue(0x00, cs, addr, pd, 64)
#define getGCValue(cs, addr, pd)           getBasicValue(0x01, cs, addr, pd, 64)
#define setSCValue(addr, pd)               setBasicValue(0x02, 0, addr, pd, 64)
#define getSCValue(addr, pd)               getBasicValue(0x03, 0, addr, pd, 64)
#define setI2CData(cs,addr, pd)            setBasicValue(0x04, cs, addr, pd, 64)
#define getI2CData(cs,addr, pd)            getBasicValue(0x05, cs, addr, pd, 64)
#define setFlashData(addr,pd)              setBasicValue(0x80, 0, addr, pd, 64)
#define getFlashData(addr,pd)              getBasicValue(0xC0, 0, addr, pd, 64)

#define getNormalData(addr,pd)             getBasicValue(NVM_READ, 0, addr, pd, 64)

//Macro to check the HR result
#define CHECK_HR_RESULT(hr, msg, ...) if (hr != S_OK) {printf("info: Function: %s, %s failed, Error code: 0x%.2x \n", __FUNCTION__, msg, hr, __VA_ARGS__); goto done; }

//Templates for the App
template <class T> void SafeRelease(T **ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

//Function to get UVC video devices
extern HRESULT GetVideoDevices();

//Function to get device friendly name
extern HRESULT GetVideoDeviceFriendlyNames(int deviceIndex);

//Function to initialize video device
extern HRESULT InitVideoDevice(int deviceIndex);

//Function to set/get parameters of UVC extension unit
extern HRESULT SetGetExtensionUnit(GUID xuGuid, DWORD dwExtensionNode, ULONG xuPropertyId, ULONG flags, void *data, int len, ULONG *readCount);
