// UVCExtensionApp.cpp : Defines the entry point for the console application.
//

#include <mfapi.h>
#include <mfplay.h>
#include <mfreadwrite.h>
#include <vector>
#include <string>
#include <ks.h>
#include <ksproxy.h>
#include <vidcap.h>
#include "UVCExtensionUnit.h"

// GUID of the extension unit, {ACB6890C-A3B3-4060-8B9A-DF34EEF39A2E}
//{ 0xacb6890c, 0xa3b3, 0x4060,{ 0x8b, 0x9a, 0xdf, 0x34, 0xee, 0xf3, 0x9a, 0x2e } };
// {B6EF686E-FF60-4853-9253-70F81F467731}
static const GUID xuGuidAN75779 =
{ 0xb6ef686e, 0xff60, 0x4853,{ 0x92, 0x53, 0x70, 0xf8, 0x1f, 0x46, 0x77, 0x31 } };
//{ 0x6e68efb6, 0x60ff, 0x5348, { 0x92, 0x53, 0x70, 0xf8, 0x1f, 0x46, 0x77, 0x31 } };


//Media foundation and DSHOW specific structures, class and variables
IMFMediaSource *pVideoSource = NULL;
IMFAttributes *pVideoConfig = NULL;
IMFActivate **ppVideoDevices = NULL;
IMFSourceReader *pVideoReader = NULL;

//Other variables
UINT32 noOfVideoDevices = 0;
WCHAR *szFriendlyName = NULL;


HRESULT SetGetSLXvalue(BYTE rd, UINT32 addr, UINT32 *pdata, BYTE len)
{
	ULONG  flags, readCount;
	BYTE   addSensor[16];

	addSensor[0] = (addr >>  0) & 0xff;
	addSensor[1] = (addr >>  8) & 0xff;
	addSensor[2] = (addr >> 16) & 0xff;
	addSensor[3] = (addr >> 24) & 0xff;
	addSensor[4] = 0x00; 

	flags = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;

#if 0
	for (int i = 0; i < 5;i++)
	printf("%02x ",addSensor[i]);
	printf("\n");
#endif

	if (SetGetExtensionUnit(xuGuidAN75779, 2, UVC_XU_I2C_ADDR_CONTROL, flags, (void*)addSensor, 5, &readCount))
	{
		return -1;
	}

	//Sleep(1);
	
	if (rd)
	{
		flags = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;
		Sleep(2000);
	}
	if (SetGetExtensionUnit(xuGuidAN75779, 2, UVC_XU_I2C_DATA_CONTROL, flags, (void*)pdata, len, &readCount))
	{
		return -2;
	}

	//Sleep(1);

	return 0;
}

int setBasicValue(UCHAR cmd, CHAR cs, UINT32 addr, UINT32 *pd, CHAR len)
{
	UINT32 addrNew = 0, flag = 0;
	flag = (cs & 3) << 16;

	cmd &= 0xff;
	switch (cmd)
	{
		case GC_WRITE	: 
		case SC_WRITE	:
		case I2C_WRITE	: addrNew = (addr & 0xffff) | flag; break;
		case NVM_WRITE	: addrNew = addr & 0x3FFFFFFF; break;
		default			: break;
	}

	addrNew |= (cmd << 24);
	return SetGetSLXvalue(0,addrNew, pd, len);
}

int getBasicValue(UCHAR cmd, CHAR cs, UINT32 addr, UINT32 *pd, CHAR len)
{
	UINT32 addrNew=0, flag=0;
	flag = (cs & 3) << 16;

	cmd &= 0xff;
	switch (cmd)
	{
		case GC_READ	: 
		case SC_READ	: 
		case I2C_READ	: addrNew = (addr & 0xffff) | flag; break;
		case NVM_READ	: addrNew = addr & 0x3FFFFFFF; break;
		default			: break;
	}
	addrNew |= (cmd & 0xff) << 24;
	return SetGetSLXvalue(1,addrNew, pd, len);
}

int UVCExtSet(int cs)
{
	HRESULT hr = S_OK;
	CHAR enteredStr[MAX_PATH], videoDevName[20][MAX_PATH];
	UINT32 selectedVal = 0xFFFFFFFF;
	ULONG flags, readCount;
	size_t returnValue;
	BYTE addSensor[5] = { 1,2,3,4,0};
	BYTE an75779FwVer[128] = { 2, 2, 12, 20, 64 }; //Write some random value
	BYTE writeCount;

	//Get all video devices
	GetVideoDevices();

	printf("Video Devices connected:\n");
	for (UINT32 i = 0; i < noOfVideoDevices; i++)
	{
		//Get the device names
		GetVideoDeviceFriendlyNames(i);
		wcstombs_s(&returnValue, videoDevName[i], MAX_PATH, szFriendlyName, MAX_PATH);
		printf("%d: %s\n", i, videoDevName[i]);

		//Store the App note firmware (Whose name is *FX3*) device index  
		if (!(strcmp(videoDevName[i], "FX3 UVC")))
			selectedVal = i;
	}

	//Specific to UVC extension unit in AN75779 appnote firmware
	if (selectedVal != 0xFFFFFFFF)
	{
		printf("\nFound \"FX3 UVC\" device\n");

		//Initialize the selected device
		InitVideoDevice(selectedVal);

#if 1
		UINT32 dstWord[16];
		UINT32 rcvWord[16];
		UINT32 rcvLen = 64;
		int mode;
		for (int j = 0; j < 1; j++)
		{
			for (int k = 0; k < 16; k++)
			{
				dstWord[k] = 0x0;
//				printf("WR:: dstWord[%2d] = %08x\n", k, dstWord[k]);
			}

			// please refer to ISPCR0, ISPCR0[20:13] and mode[7:0] are matched bit-by-bit
			// test pattern     : mode = 1 << 20;
			// bayer pattern    : mode = 0;
			// pre-process(L/R) : mode = 7 << 16; 5 << 16;
			// disparity-raw    : mode = (1 << 13) | (1 << 18) | (lr << 17) | (uf << 14) | (tf << 19)
			//                           lr: LR choice; uf: uniqueness filter enable; tf: texture filter enable


//			cs = 4;
//			cs = 11; // 4,8,11
//			cs = 1;
			switch (cs & 0xf)
			{
				case  0	:	mode = (1 << 7); break; // test pattern
				case  1	:	mode = 0; break; // bayer pattern
				case  2	:	mode = (3 << 3); break; // pre-filter reference
				case  3	:	mode = (5 << 3); break; // pre-filter target
				case  4	:	mode = (1 << 0) | (1 << 5) | (0 << 4); break;
				case  5	:	mode = (1 << 0) | (1 << 5) | (0 << 4) | (1 << 1) | (0 << 6); break;
				case  6	:	mode = (1 << 0) | (1 << 5) | (0 << 4) | (0 << 1) | (1 << 6); break;
				case  7	:	mode = (1 << 0) | (1 << 5) | (0 << 4) | (1 << 1) | (1 << 6); break;
				case  8	:	mode = (1 << 0) | (1 << 5) | (1 << 4); break;
				case  9	:	mode = (1 << 0) | (1 << 5) | (1 << 4) | (1 << 1) | (0 << 6); break;
				case 10	:	mode = (1 << 0) | (1 << 5) | (1 << 4) | (0 << 1) | (1 << 6); break;
				case 11	:	mode = (1 << 0) | (1 << 5) | (1 << 4) | (1 << 1) | (1 << 6); break;
				default	:	mode = 0; break;
			}

			if(cs >> 8) mode |= (1 << 2);
			printf("mode = %02x\n",mode);
			//mode = (1 << 7); // test pattern
			//mode
#if 0
			dstWord[0] = (2 << 24);
			dstWord[2] = (7 << 24);
			dstWord[7] = (9 << 24) | mode; // 
#else
			dstWord[0] = (10 << 24) | mode;
#endif
#if 1
//			if (!setGCValue(0, 0x0, dstWord))
			if (!setSCValue(0x0, dstWord))
//			if (!setI2CData(0, 0x0, dstWord))
//			if (!setFlashData(0xF00000, dstWord))
			{
				printf("WR:: OK\n");
			}

			Sleep(200);
#endif
			for (int k = 0; k < 16; k++)
			{
				rcvWord[k] = 0; 
			}

#if 1
			if (!getGCValue(0, 0x0, rcvWord))
			//if(!getSCValue(0x0,rcvWord))
			//if(!getI2CData(0, 0x3000, rcvWord))
			//if (!getFlashData(0xF00000, rcvWord))
			{
				printf("RD:: OK\n");
			}

			for (UINT32 i = 0; i < (rcvLen / 4); i++)
			{
				//if(rcvWord[i] != dstWord[i])
				  printf("RD:: dstWord[%2d] = %08x,rcvWord[%2d] = %08x\n", i, dstWord[i], i, rcvWord[i]);
			}
			
			Sleep(200);
#endif
		}
#else
		printf("\nSelect\n1. To Set Firmware Version \n2. To Get Firmware version\nAnd press Enter\n");
		fgets(enteredStr, MAX_PATH, stdin);
		fflush(stdin);
		selectedVal = atoi(enteredStr);

		if (selectedVal == 1)
			flags = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;
		else
			flags = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;

		printf("\nTrying to invoke UVC extension unit...\n");

//		if (!SetGetExtensionUnit(xuGuidAN75779, 2, 1, flags, (void*)addSensor, 5, &readCount))
		if (!SetGetExtensionUnit(xuGuidAN75779, 2, 1, flags, (void*)an75779FwVer, 5, &readCount))
		{
			printf("Found UVC extension unit\n");

			if (flags == (KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY))
			{
				printf("\nAN75779 Firmware Version Set to %d.%d (Major.Minor), Build Date: %d/%d/%d (MM/DD/YY)\n\n", an75779FwVer[0], an75779FwVer[1],
					an75779FwVer[2], an75779FwVer[3], an75779FwVer[4]);
			}
			else
			{
				printf("\nCurrent AN75779 Firmware Version is %d.%d (Major.Minor), Build Date: %d/%d/%d (MM/DD/YY)\n\n", an75779FwVer[0], an75779FwVer[1],
					an75779FwVer[2], an75779FwVer[3], an75779FwVer[4]);
			}
		}
#endif
	}
	else
	{
		printf("\nDid not find \"FX3\" device (AN75779 firmware)\n\n");
	}

	//Release all the video device resources
	for (UINT32 i = 0; i < noOfVideoDevices; i++)
	{
		SafeRelease(&ppVideoDevices[i]);
	}
	CoTaskMemFree(ppVideoDevices);
	SafeRelease(&pVideoConfig);
	SafeRelease(&pVideoSource);
	CoTaskMemFree(szFriendlyName);

	printf("\nExiting UVC Ext in 1 sec...\n");
	Sleep(100);

    return 0;
}

//Function to get UVC video devices
HRESULT GetVideoDevices()
{
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	MFStartup(MF_VERSION);

	// Create an attribute store to specify the enumeration parameters.
	HRESULT hr = MFCreateAttributes(&pVideoConfig, 1);
	CHECK_HR_RESULT(hr, "Create attribute store");

	// Source type: video capture devices
	hr = pVideoConfig->SetGUID(
		MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
		MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
	);
	CHECK_HR_RESULT(hr, "Video capture device SetGUID");

	// Enumerate devices.
	hr = MFEnumDeviceSources(pVideoConfig, &ppVideoDevices, &noOfVideoDevices);
	CHECK_HR_RESULT(hr, "Device enumeration");

done:
	return hr;
}

//Function to get device friendly name
HRESULT GetVideoDeviceFriendlyNames(int deviceIndex)
{
	// Get the the device friendly name.
	UINT32 cchName;

	HRESULT hr = ppVideoDevices[deviceIndex]->GetAllocatedString(
		MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
		&szFriendlyName, &cchName);
	CHECK_HR_RESULT(hr, "Get video device friendly name");

done:
	return hr;
}

//Function to initialize video device
HRESULT InitVideoDevice(int deviceIndex)
{
	HRESULT hr = ppVideoDevices[deviceIndex]->ActivateObject(IID_PPV_ARGS(&pVideoSource));
	CHECK_HR_RESULT(hr, "Activating video device");

	// Create a source reader.
	hr = MFCreateSourceReaderFromMediaSource(pVideoSource, pVideoConfig, &pVideoReader);
	CHECK_HR_RESULT(hr, "Creating video source reader");

done:
	return hr;
}

//Function to set/get parameters of UVC extension unit
HRESULT SetGetExtensionUnit(GUID xuGuid, DWORD dwExtensionNode, ULONG xuPropertyId, ULONG flags, void *data, int len, ULONG *readCount)
{
	GUID pNodeType;
	IUnknown *unKnown;
	IKsControl * ks_control = NULL;
	IKsTopologyInfo * pKsTopologyInfo = NULL;
	KSP_NODE kspNode;

	HRESULT hr = pVideoSource->QueryInterface(__uuidof(IKsTopologyInfo), (void **)&pKsTopologyInfo);
	CHECK_HR_RESULT(hr, "IMFMediaSource::QueryInterface(IKsTopologyInfo)");

	hr = pKsTopologyInfo->get_NodeType(dwExtensionNode, &pNodeType);
	CHECK_HR_RESULT(hr, "IKsTopologyInfo->get_NodeType(...)");

	hr = pKsTopologyInfo->CreateNodeInstance(dwExtensionNode, IID_IUnknown, (LPVOID *)&unKnown);
	CHECK_HR_RESULT(hr, "ks_topology_info->CreateNodeInstance(...)");

	hr = unKnown->QueryInterface(__uuidof(IKsControl), (void **)&ks_control);
	CHECK_HR_RESULT(hr, "ks_topology_info->QueryInterface(...)");

	kspNode.Property.Set = xuGuid;              // XU GUID
	kspNode.NodeId = (ULONG)dwExtensionNode;   // XU Node ID
	kspNode.Property.Id = xuPropertyId;         // XU control ID
	kspNode.Property.Flags = flags;             // Set/Get request

	hr = ks_control->KsProperty((PKSPROPERTY)&kspNode, sizeof(kspNode), (PVOID)data, len, readCount);
	CHECK_HR_RESULT(hr, "ks_control->KsProperty(...)");

done:
	SafeRelease(&ks_control);
	return hr;
}
