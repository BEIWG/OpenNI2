#ifndef __COLORSTREAM_H_
#define __COLORSTREAM_H_

#include "StreamBase.h"

namespace depth3d {
class OzImageStream : public OzStream
{
public: 
	OzImageStream();
	
	void stop();
	
	OniStatus SetVideoMode(OniVideoMode*);
	
	OniStatus GetVideoMode(OniVideoMode* pVideoMode);
private:
	void Mainloop();	
};
}
#endif