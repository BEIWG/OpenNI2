/*****************************************************************************
*                                                                            *
*  OpenNI 2.x Alpha                                                          *
*  Copyright (C) 2012 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of OpenNI.                                              *
*                                                                            *
*  Licensed under the Apache License, Version 2.0 (the "License");           *
*  you may not use this file except in compliance with the License.          *
*  You may obtain a copy of the License at                                   *
*                                                                            *
*      http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                            *
*  Unless required by applicable law or agreed to in writing, software       *
*  distributed under the License is distributed on an "AS IS" BASIS,         *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and       *
*  limitations under the License.                                            *
*                                                                            *
*****************************************************************************/
#include "Driver/OniDriverAPI.h"
#include "XnLib.h"
#include "XnHash.h"
#include "XnEvent.h"
#include "XnPlatform.h"
#ifdef _MSC_VER
#include "camerads.h"
#include "usb_camera.hpp"  
#else
#include "uvc.h"
#include <mutex>

uvc_context_t *ctx;
uvc_error_t res;
uvc_device_t *dev;
uvc_device_handle_t *devh;
uvc_stream_ctrl_t ctrl;

static int frame_drop = 10;
static char *data_tmp = NULL;
static int data_flag = 0;
static std::mutex frame_mutex;
#endif

#define USB_VID                 0x00
#define USB_PID                 0x00
#define USB_SN                  NULL

#define DEVICE_VENDOR           "beiwg"
#define DEVICE_URI              "Depth3D"

#define H_FOV                   60.0
#define V_FOV                   70.0

#define DEFAULT_FPS             0

#define DEPTH_RESOLUTION_X      640
#define DEPTH_RESOLUTION_Y      400

#define IMAGE_RESOLUTION_X      640
#define IMAGE_RESOLUTION_Y      400

#define IR_RESOLUTION_X         640
#define IR_RESOLUTION_Y         400

#ifndef _MSC_VER
void uvc_cb(uvc_frame_t *uvc_frame, void *ptr) 
{
    uvc_frame_t *bgr;
    uvc_error_t ret;

	if (data_tmp == NULL) 
	{
		if (frame_drop--)
			return;
			
		data_tmp = (char*)malloc(uvc_frame->data_bytes);
        if (!data_tmp) 
        {
            printf("unable to allocate  frame!");
            return;
  	    }
	} else {
		frame_mutex.lock();
		memcpy((unsigned char*)data_tmp, (unsigned char*)uvc_frame->data, uvc_frame->data_bytes);
		data_flag = 1;
		frame_mutex.unlock();
	}
}
#endif
class OzStream : public oni::driver::StreamBase
{
public:
	~OzStream()
	{
		stop();
	}

	OniStatus start()
	{
		xnOSCreateThread(threadFunc, this, &m_threadHandle);

		return ONI_STATUS_OK;
	}

	void stop()
	{
		m_running = false;
	}

	virtual OniStatus SetVideoMode(OniVideoMode*) = 0;
	virtual OniStatus GetVideoMode(OniVideoMode* pVideoMode) = 0;

	OniBool isPropertySupported(int propertyId)
	{
			OniBool status;
			switch(propertyId)
			{
				case ONI_STREAM_PROPERTY_VIDEO_MODE:
				case ONI_STREAM_PROPERTY_HORIZONTAL_FOV:
				case ONI_STREAM_PROPERTY_VERTICAL_FOV:
//				case ONI_STREAM_PROPERTY_AUTO_WHITE_BALANCE:
//				case ONI_STREAM_PROPERTY_AUTO_EXPOSURE:
//				case ONI_STREAM_PROPERTY_EXPOSURE:
//				case ONI_STREAM_PROPERTY_GAIN:
				{
						status = true;
						break;
				}
				
				default:
					status = false;
			}
			
			return status;
	}

	OniStatus getProperty(int propertyId, void* data, int* pDataSize)
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

	OniStatus setProperty(int propertyId, const void* data, int dataSize)
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

	virtual void Mainloop() = 0;
	
protected:
	// Thread
	static XN_THREAD_PROC threadFunc(XN_THREAD_PARAM pThreadParam)
	{
		OzStream* pStream = (OzStream*)pThreadParam;
		pStream->m_running = true;
		pStream->Mainloop();

		XN_THREAD_PROC_RETURN(XN_STATUS_OK);
	}

	bool m_running;

	XN_THREAD_HANDLE m_threadHandle;
};

class OzDepthStream : public OzStream
{
public:

	OzDepthStream()
	{        

	}

	~OzDepthStream()
	{
#ifndef _MSC_VER
		if (data_tmp != NULL)
			free(data_tmp);
					 
		uvc_stop_streaming(devh);
		uvc_close(devh);
		uvc_unref_device(dev);
		uvc_exit(ctx);	
#endif		
	}

	OniStatus SetVideoMode(OniVideoMode*) 
	{
		return ONI_STATUS_OK;
	}
	
	OniStatus GetVideoMode(OniVideoMode* pVideoMode)
	{
		pVideoMode->pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
		pVideoMode->fps = DEFAULT_FPS;
		pVideoMode->resolutionX = DEPTH_RESOLUTION_X;
		pVideoMode->resolutionY = DEPTH_RESOLUTION_Y;
		return ONI_STATUS_OK;
	}
	
	OniStatus convertDepthToColorCoordinates(StreamBase* colorStream, int depthX, int depthY, OniDepthPixel depthZ, int* pColorX, int* pColorY)
	{ 
		return ONI_STATUS_NOT_SUPPORTED;
	}
	
private:

	void Mainloop()
	{
		int frameId = 1;             
#ifdef _MSC_VER
		char szCamName[20] = { 0 };
        long recv = 0;
        int openflag = 0;
             

		CCameraDS m_CamDS;
        int m_iCamCount = CCameraDS::CameraCount();
        if (m_iCamCount == 0)
        {
            return;
         }

		for (auto n = 0; n < m_iCamCount; n++)
        {
			int retval = m_CamDS.CameraName(n, szCamName, sizeof(szCamName));
            if (retval > 0)
            {
				if (!strcmp(szCamName, "FX3 UVC"))
					if ((m_CamDS.OpenCamera(n, false, DEPTH_RESOLUTION_X, DEPTH_RESOLUTION_Y)))
					{
                                openflag = 1;
                    }
             }
         }
                
        if (openflag == 0)
        {
			printf("DepthStream: open camera failed....\n");
            return;
		}
#endif
		while (m_running)
		{
			OniFrame* pFrame = getServices().acquireFrame();

			if (pFrame == NULL) {printf("Didn't get frame...\n"); continue;}

			// Fill metadata
			pFrame->frameIndex = frameId;

			pFrame->videoMode.pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
			pFrame->videoMode.resolutionX = DEPTH_RESOLUTION_X;
			pFrame->videoMode.resolutionY = DEPTH_RESOLUTION_Y;
			pFrame->videoMode.fps = DEFAULT_FPS;

			pFrame->width = DEPTH_RESOLUTION_X;
			pFrame->height = DEPTH_RESOLUTION_Y;

			pFrame->cropOriginX = pFrame->cropOriginY = 0;
			pFrame->croppingEnabled = FALSE;

			pFrame->sensorType = ONI_SENSOR_DEPTH;
			pFrame->stride = DEPTH_RESOLUTION_X*sizeof(OniDepthPixel);
			pFrame->timestamp = frameId*33000;
			
            // Fill frame
#ifdef _MSC_VER
             recv = m_CamDS.QueryFrame((char*)pFrame->data);
             if (recv == pFrame->dataSize)
             {
                 raiseNewFrame(pFrame);
                 frameId++;
             }			
#else      
            frame_mutex.lock();
            if (data_flag == 1) 
            {
                data_flag = 0;
                //cv::Mat input_mat(DEPTH_RESOLUTION_Y, DEPTH_RESOLUTION_X, CV_16SC1, data_tmp);
                //cv::filterSpeckles(input_mat, (dMin - 1)*0, 200, diff*16.0);
                //xnOSMemCopy((pFrame->data), data_tmp/*input_mat.data*/, pFrame->dataSize);
                xnOSMemCopy((pFrame->data), data_tmp, pFrame->dataSize); 
                raiseNewFrame(pFrame);
				frameId++;
            }
            frame_mutex.unlock();
#endif
			getServices().releaseFrame(pFrame);
			xnOSSleep(33);
		}
	}
};

class OzImageStream : public OzStream
{
public:
    
	OzImageStream()
	{

	}

	OniStatus SetVideoMode(OniVideoMode*) 
	{
		return ONI_STATUS_OK;
	}
	
	OniStatus GetVideoMode(OniVideoMode* pVideoMode)
	{
		pVideoMode->pixelFormat = ONI_PIXEL_FORMAT_RGB888;
		pVideoMode->fps = DEFAULT_FPS;
		pVideoMode->resolutionX = IMAGE_RESOLUTION_X;
		pVideoMode->resolutionY = IMAGE_RESOLUTION_Y;
		return ONI_STATUS_OK;
	}

private:

	void Mainloop()
	{
		int frameId = 1;
                
		while (m_running)
		{
                        
			xnOSSleep(33);
			//			printf("Tick");
			OniFrame* pFrame = getServices().acquireFrame();

			if (pFrame == NULL) {printf("Didn't get frame...\n"); continue;}
               
			// Fill metadata
			pFrame->frameIndex = frameId;

			pFrame->videoMode.pixelFormat = ONI_PIXEL_FORMAT_RGB888;
			pFrame->videoMode.resolutionX = IMAGE_RESOLUTION_X;
			pFrame->videoMode.resolutionY = IMAGE_RESOLUTION_Y;
			pFrame->videoMode.fps = DEFAULT_FPS;

			pFrame->width = IMAGE_RESOLUTION_X;
			pFrame->height = IMAGE_RESOLUTION_Y;

			pFrame->cropOriginX = pFrame->cropOriginY = 0;
			pFrame->croppingEnabled = FALSE;

			pFrame->sensorType = ONI_SENSOR_COLOR;
			pFrame->stride = IMAGE_RESOLUTION_X*3;
			pFrame->timestamp = frameId*33000;

			// Fill fake data frame
            xnOSMemSet(pFrame->data, 0xc0, pFrame->dataSize);
			raiseNewFrame(pFrame);
			frameId++;

			raiseNewFrame(pFrame);
			getServices().releaseFrame(pFrame);

			frameId++;
		}
	}
};

class OzIRStream : public OzStream
{
public:

	OzIRStream()
	{        

	}

	~OzIRStream()
	{
	}

	OniStatus SetVideoMode(OniVideoMode*) 
	{
		return ONI_STATUS_OK;
	}
	
	OniStatus GetVideoMode(OniVideoMode* pVideoMode)
	{
		pVideoMode->pixelFormat = ONI_PIXEL_FORMAT_GRAY8;
		pVideoMode->fps = DEFAULT_FPS;
		pVideoMode->resolutionX = IR_RESOLUTION_X;
		pVideoMode->resolutionY = IR_RESOLUTION_Y;
		return ONI_STATUS_OK;
	}
	
private:

	void Mainloop()
	{
    
		int frameId = 1;             
        
		while (m_running)
		{
			OniFrame* pFrame = getServices().acquireFrame();

			if (pFrame == NULL) {printf("Didn't get frame...\n"); continue;}

			// Fill metadata
			pFrame->frameIndex = frameId;

			pFrame->videoMode.pixelFormat = ONI_PIXEL_FORMAT_GRAY8;
			pFrame->videoMode.resolutionX = IR_RESOLUTION_X;
			pFrame->videoMode.resolutionY = IR_RESOLUTION_Y;
			pFrame->videoMode.fps = DEFAULT_FPS;

			pFrame->width = IR_RESOLUTION_X;
			pFrame->height = IR_RESOLUTION_Y;

			pFrame->cropOriginX = pFrame->cropOriginY = 0;
			pFrame->croppingEnabled = FALSE;

			pFrame->sensorType = ONI_SENSOR_IR;
			pFrame->stride = IR_RESOLUTION_X*sizeof(OniGrayscale8Pixel);
			pFrame->timestamp = frameId*33000;

            // Fill fake data frame      
            xnOSMemSet(pFrame->data, 0xc0, pFrame->dataSize);
			raiseNewFrame(pFrame);
			frameId++;
              
			getServices().releaseFrame(pFrame);
			xnOSSleep(33);
		}
	}
};

class OzDevice : public oni::driver::DeviceBase
{
public:
	OzDevice(OniDeviceInfo* pInfo, oni::driver::DriverServices& driverServices) : m_pInfo(pInfo), m_driverServices(driverServices)
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
		m_sensors[2].pSupportedVideoModes[0].resolutionY = IR_RESOLUTION_X;	
	}
	OniDeviceInfo* GetInfo()
	{
		return m_pInfo;
	}

	OniStatus getSensorInfoList(OniSensorInfo** pSensors, int* numSensors)
	{
		*numSensors = m_numSensors;
		*pSensors = m_sensors;

		return ONI_STATUS_OK;
	}

	oni::driver::StreamBase* createStream(OniSensorType sensorType)
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

	void destroyStream(oni::driver::StreamBase* pStream)
	{
		XN_DELETE(pStream);
	}

	OniStatus  getProperty(int propertyId, void* data, int* pDataSize)
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

	OniBool isCommandSupported(int commandId)
	{
		return ONI_STATUS_NOT_IMPLEMENTED;
	}

	OniStatus tryManualTrigger()
	{
		return ONI_STATUS_NOT_IMPLEMENTED;
	}
	
	OniBool isImageRegistrationModeSupported(OniImageRegistrationMode mode)
	{
		//return (mode == ONI_IMAGE_REGISTRATION_DEPTH_TO_COLOR || mode == ONI_IMAGE_REGISTRATION_OFF);
		return (mode == ONI_IMAGE_REGISTRATION_OFF);
	}
	
private:
	OzDevice(const OzDevice&);
	void operator=(const OzDevice&);

	OniDeviceInfo* m_pInfo;
	int m_numSensors;
	OniSensorInfo m_sensors[10];
	oni::driver::DriverServices& m_driverServices;
};

class OzDriver : public oni::driver::DriverBase
{
public:
	OzDriver(OniDriverServices* pDriverServices) : DriverBase(pDriverServices)
	{
	}

	OniStatus initialize(oni::driver::DeviceConnectedCallback deviceConnectedCallback,				\
										oni::driver::DeviceDisconnectedCallback deviceDisconnectedCallback,					\
										oni::driver::DeviceStateChangedCallback deviceStateChangedCallback,					\
										void* pCookie)
	{
#ifdef _MSC_VER
            if (-1 == camera_init(DEPTH_RESOLUTION_X, DEPTH_RESOLUTION_Y))
			return ONI_STATUS_ERROR;			
#else
			if(0 == uvc_dirver_init(DEPTH_RESOLUTION_X, DEPTH_RESOLUTION_Y, DEFAULT_FPS, USB_VID, USB_PID, USB_SN, uvc_cb))
			{
				printf("Init Sensor Driver Failed...\n");
				return ONI_STATUS_NO_DEVICE;	
			}
#endif
			DriverBase::initialize(deviceConnectedCallback, deviceDisconnectedCallback, deviceStateChangedCallback, pCookie);
			OniDeviceInfo* pInfo = XN_NEW(OniDeviceInfo);
			xnOSStrCopy(pInfo->uri, DEVICE_URI, ONI_MAX_STR);
			xnOSStrCopy(pInfo->vendor, DEVICE_VENDOR, ONI_MAX_STR);
			m_devices[pInfo] = NULL;

            deviceConnected(pInfo);
            deviceStateChanged(pInfo, ONI_DEVICE_STATE_OK);
            return ONI_STATUS_OK;
	}

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

	void shutdown() {
	}

protected:

	XN_THREAD_HANDLE m_threadHandle;

	xnl::Hash<OniDeviceInfo*, oni::driver::DeviceBase*> m_devices;
};

ONI_EXPORT_DRIVER(OzDriver);
