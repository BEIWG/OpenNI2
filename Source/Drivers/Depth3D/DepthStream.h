#ifndef __DEPTHSTREAM_H_
#define __DEPTHSTREAM_H_

#include "StreamBase.h"

namespace depth3d {
class OzDepthStream : public OzStream
{
public:	
	OzDepthStream();
	
	void stop();
	
	OniStatus SetVideoMode(OniVideoMode* videoMode);
	
	OniStatus GetVideoMode(OniVideoMode* pVideoMode);
	
	OniStatus convertDepthToColorCoordinates(StreamBase* colorStream, int depthX, int depthY, OniDepthPixel depthZ, int* pColorX, int* pColorY);
	
private:
	void Mainloop();	
};
}

#endif