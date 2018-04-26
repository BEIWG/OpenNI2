#ifndef __IRSTREAM_H_
#define __IRSTREAM_H_

#include "StreamBase.h"

namespace depth3d {
class OzIRStream : public OzStream
{
public:
	OzIRStream();
	
	void stop();
	
	OniStatus SetVideoMode(OniVideoMode*);
	
	OniStatus GetVideoMode(OniVideoMode* pVideoMode);
private:
	void Mainloop();	
};
}
#endif