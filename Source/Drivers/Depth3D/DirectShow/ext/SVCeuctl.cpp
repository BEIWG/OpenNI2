// UVCExtensionApp.cpp : Defines the entry point for the console application.
//
#include "SVCeuctl.h"

#define EU_CS_XFER_CONFIG       (0x01)
#define EU_CS_XFER_DATA         (0x02)

#define EU_CS_FLASH_ERASE		(0x03)
#define EU_CS_FLASH_RWCONFIG    (0x04)
#define EU_CS_FLASH_DATA        (0x05)

#define EU_SET_CUR              (KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY)
#define EU_GET_CUR              (KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY)
#define EU_XFER_DATA_GET		(0x01)
#define EU_XFER_DATA_SET		(0x02)

//Function to get UVC video devices
HRESULT SVCeuctl::GetVideoDevices(UINT32 *noOfVideoDevices)
{
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	MFStartup(MF_VERSION);
	HRESULT hr = S_OK;
	// Create an attribute store to specify the enumeration parameters.
	hr = MFCreateAttributes(&mpVideoConfig, 1);
	
	if (hr != S_OK)
	{
		DEGPRINT(hr,"MFCreateAttributes()");
        return hr;
    }

	// Source type: video capture devices
	hr = mpVideoConfig->SetGUID(
		MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
		MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
		);
	if (hr != S_OK)
    {
		DEGPRINT(hr,"pVideoConfig->SetGUID()");
        return hr;
    }

	// Enumerate devices.	
	hr = MFEnumDeviceSources(mpVideoConfig, &mppVideoDevices, noOfVideoDevices);

	//printf("ppVideoDevices[0] = 0x%x \n", mppVideoDevices[0]);

	if (hr != S_OK)
    {
		DEGPRINT(hr,"MFEnumDeviceSources()");
        return hr;
    }
	return S_OK;
}

//Function to get device friendly name
HRESULT SVCeuctl::GetVideoDeviceFriendlyNames(UINT32 index)
{
	// Get the the device friendly name.
	UINT32 cchName;
	HRESULT hr = S_OK;

	hr = mppVideoDevices[index]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,&mszFriendlyName, &cchName);
	

	if (hr != S_OK)
    {
		DEGPRINT(hr,"mppVideoDevices->GetAllocatedString()");
        return hr;
    }
    
	return S_OK;
    
}

//Function to initialize video device
HRESULT SVCeuctl::InitVideoDevice(UINT32 index)
{
	HRESULT hr = S_OK;
	
	hr = mppVideoDevices[index]->ActivateObject(IID_PPV_ARGS(&mpVideoSource));
	if (hr != S_OK)
    {
		DEGPRINT(hr,"mppVideoDevices->ActivateObject()");
        return hr;
    }
	// Create a source reader.
	
	hr = MFCreateSourceReaderFromMediaSource(mpVideoSource, mpVideoConfig, &mpVideoReader);
	if (hr != S_OK)
    {
		DEGPRINT(hr,"MFCreateSourceReaderFromMediaSource()");
        return hr;
    }
    
    return S_OK;
}

//Function to set/get parameters of UVC extension unit
HRESULT SVCeuctl::InitEUctl(DWORD dwExtensionNode)
{
	HRESULT hr = S_OK;
	GUID pNodeType;
	IUnknown *unKnown;
	IKsTopologyInfo * pKsTopologyInfo = NULL;
	
	hr = mpVideoSource->QueryInterface(__uuidof(IKsTopologyInfo), (void **)&pKsTopologyInfo);
	if (hr != S_OK)
    {
		DEGPRINT(hr,"mpVideoSource->QueryInterface() \n");
        return hr;
    }

	
	hr = pKsTopologyInfo->get_NodeType(dwExtensionNode, &pNodeType);
	if (hr != S_OK)
    {
		DEGPRINT(hr,"pKsTopologyInfo->get_NodeType()");
        return hr;
    }

	
	hr = pKsTopologyInfo->CreateNodeInstance(dwExtensionNode, IID_IUnknown, (LPVOID *)&unKnown);
	if (hr != S_OK)
    {
		DEGPRINT(hr,"pKsTopologyInfo->CreateNodeInstance()");
        return hr;
    }

	
	hr = unKnown->QueryInterface(__uuidof(IKsControl), (void **)&mks_control);
	if (hr != S_OK)
    {
		DEGPRINT(hr,"unKnown->QueryInterface()");
        return hr;
    }

	return S_OK;
}

//Function to set/get parameters of UVC extension unit
HRESULT SVCeuctl::SetGetExtensionUnit(GUID xuGuid, DWORD dwExtensionNode,IKsControl * ks_control, ULONG xuPropertyId, ULONG flags, void *data, int len, ULONG *readCount)
{
	HRESULT hr = S_OK;
	KSP_NODE kspNode;	
	kspNode.Property.Set = xuGuid;              // XU GUID
	kspNode.NodeId = (ULONG)dwExtensionNode;    // XU Node ID
	kspNode.Property.Id = xuPropertyId;         // XU control ID
	kspNode.Property.Flags = flags;             // Set/Get request
	
	hr = ks_control->KsProperty((PKSPROPERTY)&kspNode, sizeof(kspNode), (PVOID)data, len, readCount);
	if (hr != S_OK)
    {
		DEGPRINT(hr,"ks_control->KsProperty((PKSPROPERTY)");
        return hr;
    }
    
	return S_OK;
}

SVCeuctl::SVCeuctl()
{   
    
	memset(&mxuGuid, 0, sizeof(GUID));
    mdwExtensionNode    = 0;
    mselectedVal        = 0xFFFFFFFF;    
    mpVideoSource       = NULL;
    mpVideoConfig       = NULL;
    mppVideoDevices     = NULL;
    mpVideoReader       = NULL;
    mks_control         = NULL;
    mnoOfVideoDevices   = 0;
    mszFriendlyName     = NULL;   
}

HRESULT SVCeuctl::Open(CHAR * VideoDevName,GUID xuGuid, DWORD dwExtensionNode)
{
	HRESULT hr = S_OK;
	CHAR    GetDevName[20][MAX_PATH];	    
	size_t  returnValue;
	strcpy(UVCDevName, VideoDevName);

    mxuGuid = xuGuid;
    mdwExtensionNode = dwExtensionNode;
    
	//Get all video devices    
	
	hr = GetVideoDevices(&mnoOfVideoDevices);
	if (hr !=S_OK)
    {		
		DEGPRINT(hr,"GetVideoDevices()\n");
		return hr;
    }
	//printf("mnoOfVideoDevices:%x\n", mnoOfVideoDevices);
	//printf("Video Devices connected:\n");

	//printf("mppVideoDevices[0] = 0x%x \n", mppVideoDevices[0]);
	for (UINT32 i = 0; i < mnoOfVideoDevices; i++)
	{
		//Get the device names
		
		hr = GetVideoDeviceFriendlyNames(i);
		if (hr !=S_OK)
        {
			DEGPRINT(hr,"GetVideoDeviceFriendlyNames()\n");
			return hr;
        }
        
		wcstombs_s(&returnValue, GetDevName[i], MAX_PATH, mszFriendlyName, MAX_PATH);
		printf("%d: %s\n", i, GetDevName[i]);

		if (!(strcmp(GetDevName[i], UVCDevName)))
			mselectedVal = i;
	}

	if (mselectedVal == 0xFFFFFFFF)
	{
		printf("\canot Found \ %s \" device\n",UVCDevName);
        return S_FALSE;
    }
    
	
	hr = InitVideoDevice(mselectedVal);
	if (hr !=S_OK)
    {
		DEGPRINT(hr,"InitVideoDevice()");
		return hr;
    }
    
	
	hr = InitEUctl(mdwExtensionNode);
	if (hr !=S_OK)
    {
		DEGPRINT(hr,"InitEUctl()");
        return hr;
    }
    
    return S_OK;    
}

void SVCeuctl::SVCRelease()
{
        //Release all the video device resources
	for (UINT32 i = 0; i < mnoOfVideoDevices; i++)
	{
		mppVideoDevices[i]->Release();
        mppVideoDevices[i] = NULL;
	}
    
	CoTaskMemFree(mppVideoDevices);
    
    mks_control->Release();
    mks_control = NULL;
    
    mpVideoConfig->Release();
    mpVideoConfig = NULL;
    
    mpVideoSource->Release();
    mpVideoSource = NULL;

	CoTaskMemFree(mszFriendlyName);
    CoTaskMemFree(UVCDevName);
    
    mnoOfVideoDevices = 0;
	memset(&mxuGuid,0,sizeof(GUID));
    mdwExtensionNode    = 0;
    mselectedVal        = 0xFFFFFFFF;    
    mpVideoReader       = NULL;
}

SVCeuctl::~SVCeuctl()
{
    SVCRelease();
}

int SVCeuctl::eu_set_ctrl(UINT8 ctrl, void *data, int len)
{
	ULONG readCount;
	HRESULT hr = S_OK;
	hr = SetGetExtensionUnit(mxuGuid, mdwExtensionNode, mks_control, ctrl, EU_SET_CUR, (void *)data, len, &readCount);

	if (hr != S_OK)
	{
		DEGPRINT(hr, "SetGetExtensionUnit()");
	}
	else
	{
		readCount = len;
	}
	//printf("readCount:%d\r\n", readCount);
	return readCount;
}

int SVCeuctl::eu_get_ctrl(UINT8 ctrl, void *data, int len)
{
	ULONG readCount;
	HRESULT hr = S_OK;
	hr = SetGetExtensionUnit(mxuGuid, mdwExtensionNode, mks_control, ctrl, EU_GET_CUR, (void *)data, len, &readCount);
	//printf("readCount:%d\r\n", readCount);
	if (hr != S_OK)
	{
		DEGPRINT(hr, "SetGetExtensionUnit()");
	}

	return readCount;
}

HRESULT SVCeuctl::XferConfigSet(UINT8 cmd, UINT8 subaddrsize, UINT32 subaddr, UINT8 datacount)
{	
	UINT8 data[8];
	ULONG readCount;
	HRESULT hr = S_OK;
	data[0] = cmd;
	data[1] = subaddrsize;
	data[2] = (subaddr ) & 0x000000ff;
	data[3] = (subaddr >> 8 ) & 0x000000ff;
	data[4] = (subaddr >> 16) & 0x000000ff;
	data[5] = (subaddr >> 24) & 0x000000ff;
	data[6] = datacount;
	data[7] = 0x00;

	hr = SetGetExtensionUnit(mxuGuid, mdwExtensionNode, mks_control, EU_CS_XFER_CONFIG, EU_SET_CUR, (void *)data, 8, &readCount);

	if (hr != S_OK)
	{
		DEGPRINT(hr, "XferConfigSet()");
	}
	return hr;
}

HRESULT SVCeuctl::SME_FlashErase(UINT8 mode, UINT32 flashaddr)
{
	UINT8 data[8];
	ULONG readCount;
	HRESULT hr = S_OK;
	data[0] = mode;
	data[1] = (flashaddr >> 0 ) & 0x000000ff;
	data[2] = (flashaddr >> 8 ) & 0x000000ff;
	data[3] = (flashaddr >> 16) & 0x000000ff;
	data[4] = (flashaddr >> 24) & 0x000000ff;
	data[5] = 0x00;

	hr = SetGetExtensionUnit(mxuGuid, mdwExtensionNode, mks_control, EU_CS_FLASH_ERASE, EU_SET_CUR, (void *)data, 6, &readCount);

	if (hr != S_OK)
	{
		DEGPRINT(hr, "SME_FlashErase()");
	}
	return hr;
}

HRESULT SVCeuctl::SME_FlashWRConfig(UINT8 cmd,UINT32 flashaddr, UINT16 len)
{
	UINT8 data[8];
	ULONG readCount;
	HRESULT hr = S_OK;

	data[0] = (flashaddr >> 0) & 0x000000ff;
	data[1] = (flashaddr >> 8) & 0x000000ff;
	data[2] = (flashaddr >> 16) & 0x000000ff;
	data[3] = (flashaddr >> 24) & 0x000000ff;

	if (len > 256)
	{
		data[4] = 255;
		DEGPRINT(hr, "SME_FlashWRConfig() len must <= 256 ");

	}
	else
	{
		data[4] = len-1;
	}
	
	data[5] = cmd;

	hr = SetGetExtensionUnit(mxuGuid, mdwExtensionNode, mks_control, EU_CS_FLASH_RWCONFIG, EU_SET_CUR, (void *)data, 6, &readCount);

	if (hr != S_OK)
	{
		DEGPRINT(hr, "SME_FlashWRConfig()");
	}
	return hr;
}

HRESULT SVCeuctl::SME_FlashWriteData(UINT8 *buf, UINT16 len)
{

	UINT8 data[256];
	ULONG readCount;
	HRESULT hr = S_OK;
	int repeat_t = 0;

	if (len > 256)
	{
		len = 256;
		DEGPRINT(hr, "SME_FlashWriteData() len must be less than 256");
		//return S_FALSE;
	}

	memset(data,255,256);
	memcpy(data, buf, len);

	if ((len % 64) != 0)
	{
		repeat_t = (len / 64) + 1;
	}
	else
	{
		repeat_t = (len / 64);
	}

	for(int i = 0; i < repeat_t; i++)
	{
		hr = SetGetExtensionUnit(mxuGuid, mdwExtensionNode, mks_control, EU_CS_FLASH_DATA, EU_SET_CUR, (void *)(data+i*64), 64, &readCount);
	}


	if (hr != S_OK)
	{
		DEGPRINT(hr, "SME_FlashWRConfig()");
	}
	return hr;
}

HRESULT SVCeuctl::SME_FlashReadData(UINT8 *buf, UINT16 len)
{

	UINT8 data[256];
	ULONG readCount;
	HRESULT hr = S_OK;
	int repeat_t = 0;
	memset(data, 0, 256);

	if (len > 256)
	{
		len = 256;
		DEGPRINT(hr, "SME_FlashReadData() len must be less than 256");
		//return S_FALSE;
	}

	if ((len % 64) != 0)
	{
		repeat_t = (len / 64) + 1;
	}
	else
	{
		repeat_t = (len / 64);
	}

	for (int i = 0; i < repeat_t; i++)
	{
		hr = SetGetExtensionUnit(mxuGuid, mdwExtensionNode, mks_control, EU_CS_FLASH_DATA, EU_GET_CUR, (void *)(&data[i * 64]), 64, &readCount);
		//printf("read from %x \r\n", (void *)(data + i * 64));
	}

	if (hr != S_OK)
	{
		DEGPRINT(hr, "SME_FlashReadData()");
	}

	memcpy(buf, data, len);

	return hr;
}

HRESULT SVCeuctl::XferConfigGet(UINT8 *cmd, UINT8 *subaddrsize, UINT32 *subaddr, UINT8 *datacount, UINT8 *status)
{
	HRESULT hr = S_OK;
	ULONG readCount;
	UINT8 data[8];


	hr = SetGetExtensionUnit(mxuGuid, mdwExtensionNode, mks_control, EU_CS_XFER_CONFIG, EU_GET_CUR, (void *)data, 8, &readCount);

	if (hr != S_OK)
	{
		DEGPRINT(hr, "XferConfigGet()");
		return hr;
	}

	
	*cmd = data[0];
	*subaddrsize = data[1];
	*subaddr = (data[2]) | (data[3] << 8) | (data[4] << 16) | (data[5] << 24);
	*datacount = data[6];
	*status = data[7];


	return hr;
}


HRESULT SVCeuctl::XferData(UINT8 flag, UINT8 *data, UINT8 datacount)
{	
	ULONG readCount;
	HRESULT hr = S_OK;
	ULONG euflags = 0;
	UINT8 tmp_data[64];

	if (flag == EU_XFER_DATA_SET)
	{
		memcpy(tmp_data, data, datacount);
		hr = SetGetExtensionUnit(mxuGuid, mdwExtensionNode, mks_control, EU_CS_XFER_DATA, EU_SET_CUR, (void *)tmp_data, 64, &readCount);
	}
	else if (flag == EU_XFER_DATA_GET)
	{
		hr = SetGetExtensionUnit(mxuGuid, mdwExtensionNode, mks_control, EU_CS_XFER_DATA, EU_GET_CUR, (void *)tmp_data, 64, &readCount);
		memcpy(data,tmp_data,datacount);
	}
	else
	{
		return S_FALSE;
	}

	

	if (hr != S_OK)
	{
		DEGPRINT(hr, "XferData()");
	}

	return hr;
	//return S_OK;
}

UINT8 SVCeuctl::XferSend(UINT8 cmd, UINT8 subaddrsize, UINT32 subaddr, UINT8 *data, UINT8 len)
{
	UINT8 status = 0x00;

	if (XferConfigSet(cmd, subaddrsize, subaddr, len) != S_OK)
	{
		return 0xfd;
	}

	if (XferData(EU_XFER_DATA_SET, data, len) != S_OK)
	{
		return 0xfd;
	}
	if (XferConfigGet(&cmd, &subaddrsize, &subaddr, &len, &status) != S_OK)
	{
		return 0xfd;
	}

	return status;
}

UINT8 SVCeuctl::XferRecv(UINT8 cmd, UINT8 subaddrsize, UINT32 subaddr, UINT8 *data, UINT8 len)
{
	UINT8 status = 0x00;
	HRESULT hr = S_OK;
	if (XferConfigSet(cmd, subaddrsize, subaddr, len) != S_OK)
	{
		return 0xfd;
	}

	hr = XferData(EU_XFER_DATA_GET, data, len);

	if (hr != S_OK)
	{
		//hr = hr >> 8;
		//return hr;
		return 0xfe;
	}

	if (XferConfigGet(&cmd, &subaddrsize, &subaddr, &len, &status) != S_OK)
	{
		return 0xfd;
	}

	return status;
}
    
UINT8 SVCeuctl::DPUSet(UINT16 addr, UINT32 data)
{

	UINT8 devid = 0xac;
	UINT8 subaddrsize = 2;
	UINT32 subaddr = (UINT32)(addr & 0x0000ffff);
	UINT8 datacount = 4;
	UINT8 status = 0x00;

	status = XferSend(devid, subaddrsize, subaddr, (UINT8*)&data, datacount);

	if (status != 0x00)
		DEGPRINT(status, "DUPSet()");

	return status;

}

UINT8 SVCeuctl::DPUGet(UINT16 addr, UINT32 *data)
{
	UINT8 devid = 0xac;
	UINT8 subaddrsize = 2;
	UINT32 subaddr = (UINT32)(addr & 0x0000ffff);
	UINT8 datacount = 4;
	UINT8 status = 0x00;

	status = XferRecv(devid, subaddrsize, subaddr, (UINT8*)data, datacount);

	if (status != 0x00)
		DEGPRINT(status, "DPUGet()");

	return status;
}

UINT8 SVCeuctl::FPGASet(UINT8 addr, UINT8 data)
{

	UINT8 devid = 0x66;
	UINT8 subaddrsize = 1;
	UINT32 subaddr = (UINT32)(addr & 0x000000ff);
	UINT8 datacount = 1;
	UINT8 status = 0x00;

	status = XferSend(devid, subaddrsize, subaddr, &data, datacount);

	if (status != 0x00)
		DEGPRINT(status, "FPGASet()");

	return status;

}

UINT8 SVCeuctl::FPGAGet(UINT8 addr, UINT8 *data)
{
	UINT8 devid = 0x66;
	UINT8 subaddrsize = 1;
	UINT32 subaddr = (UINT32)(addr & 0x000000ff);
	UINT8 datacount = 1;
	UINT8 status = 0x00;

	status = XferRecv(devid, subaddrsize, subaddr, data, datacount);

	if (status != 0x00)
		DEGPRINT(status, "FPGAGet()");

	return status;
}


UINT8 SVCeuctl::SensorSet(UINT16 addr, UINT16 *data, UINT8 len)
{

	UINT8 devid = 0xC0;//20
	UINT8 subaddrsize = 2;
	UINT32 subaddr = (UINT32)(addr & 0x0000ffff);
	UINT8 datacount = len<<1;
	UINT8 status=0x00;
	UINT16 tmpdata[64];

	if (len > 32)
	{
		return 0xfe;
	}

	for (int i = 0; i < len; i++)
	{
		tmpdata[i] = ((data[i] >> 8) & 0x00ff) | ((data[i] << 8) & 0xff00);
	}

	status = XferSend(devid, subaddrsize, subaddr, (UINT8 *)tmpdata, datacount);

	if (status != 0x00)
		DEGPRINT(status, "SensorSet()");
	return status;

}

UINT8 SVCeuctl::SensorGet(UINT16 addr, UINT16 *data, UINT8 len)
{
	UINT8 devid = 0xC0;//20
	UINT8 subaddrsize = 2;
	UINT32 subaddr = (UINT32)(addr & 0x0000ffff);
	UINT8 datacount = len<<1;
	UINT8 status=0x00;
	UINT16 tmpdata[64];

	if (len > 32)
	{
		return 0xfe;
	}

	status = XferRecv(devid, subaddrsize, subaddr, (UINT8 *)tmpdata, datacount);

	for (int i = 0; i < len; i++)
	{
		data[i] = ((tmpdata[i] >> 8) & 0x00ff) | ((tmpdata[i] << 8) & 0xff00);
	}

	if (status != 0x00)
		DEGPRINT(status, "SensorGet()");
	return status;
}