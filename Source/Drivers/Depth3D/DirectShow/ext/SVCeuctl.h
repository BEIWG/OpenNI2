#pragma once
#include <mfapi.h>
#include <mfplay.h>
#include <mfreadwrite.h>
#include <vector>
#include <string>
#include <ks.h>
#include <ksproxy.h>
#include <vidcap.h>

//Macro to check the HR result
#define DEGPRINT(hr,msg, ...)  {printf("info: Function: %s %s failed, Error code: 0x%.2x \n", __FUNCTION__, msg,hr, __VA_ARGS__); }

class SVCeuctl 
{
public:
	SVCeuctl();
	~SVCeuctl();
	HRESULT Open(CHAR * VideoDevName,GUID xuGuid, DWORD dwExtensionNode);
	void SVCRelease();
	UINT8 XferSend(UINT8 cmd, UINT8 subaddrsize, UINT32 subaddr, UINT8 *data, UINT8 len);
	UINT8 XferRecv(UINT8 cmd, UINT8 subaddrsize, UINT32 subaddr, UINT8 *data, UINT8 len);
	UINT8 FPGASet(UINT8 addr, UINT8 data);
	UINT8 DPUSet(UINT16 addr, UINT32 data);
	UINT8 DPUGet(UINT16 addr, UINT32 *data);
	UINT8 FPGAGet(UINT8 addr, UINT8 *data);
	UINT8 SensorSet(UINT16 addr, UINT16 *data, UINT8 len);
	UINT8 SensorGet(UINT16 addr, UINT16 *data, UINT8 len);
private:

    //Function to get UVC video devices
    HRESULT GetVideoDevices(UINT32 *noOfVideoDevices);

    //Function to get device friendly name
    HRESULT SVCeuctl::GetVideoDeviceFriendlyNames(UINT32 index);

    //Function to initialize video device
    HRESULT SVCeuctl::InitVideoDevice(UINT32 index);

	HRESULT SVCeuctl::InitEUctl(DWORD dwExtensionNode);
    //Function to set/get parameters of UVC extension unit
	HRESULT SetGetExtensionUnit(GUID xuGuid, DWORD dwExtensionNode, IKsControl * ks_control, ULONG xuPropertyId, ULONG flags, void *data, int len, ULONG *readCount);
 
	HRESULT XferConfigSet(UINT8 cmd, UINT8 subaddrsize, UINT32 subaddr, UINT8 datacount);
	HRESULT XferConfigGet(UINT8 *cmd, UINT8 *subaddrsize, UINT32 *subaddr, UINT8 *datacount, UINT8 *status);
	HRESULT XferData(UINT8 flag, UINT8 *data, UINT8 datacount);

    CHAR            UVCDevName[MAX_PATH];
    GUID            mxuGuid;
    DWORD           mdwExtensionNode;
    UINT32          mselectedVal = 0xFFFFFFFF;
    
    IMFMediaSource  *mpVideoSource = NULL;
    IMFAttributes   *mpVideoConfig = NULL;
    IMFActivate     **mppVideoDevices = NULL;
    IMFSourceReader *mpVideoReader = NULL;
    IKsControl      *mks_control = NULL;
    //Other variables
    UINT32          mnoOfVideoDevices = 0;
    WCHAR           *mszFriendlyName = NULL;

};