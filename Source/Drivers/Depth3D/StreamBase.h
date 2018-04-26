#ifndef __STREAMBASE_H_
#define __STREAMBASE_H_

#include "Driver/OniDriverAPI.h"
#include "XnLib.h"
#include "XnHash.h"
#include "XnEvent.h"
#include "XnPlatform.h"
#include "DefaultParameters.h"

namespace depth3d {
class OzStream : public oni::driver::StreamBase
{
public:
	~OzStream();

	OniStatus start();

	virtual void stop() { m_running = false; }

	virtual OniStatus SetVideoMode(OniVideoMode* videoMode) = 0;
	virtual OniStatus GetVideoMode(OniVideoMode* pVideoMode) = 0;

	OniBool isPropertySupported(int propertyId);
	OniStatus getProperty(int propertyId, void* data, int* pDataSize);
	OniStatus setProperty(int propertyId, const void* data, int dataSize);

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
	OniVideoMode m_videoMode;
	XN_THREAD_HANDLE m_threadHandle;
};
}
#endif