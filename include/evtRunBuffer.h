#ifndef EVTRUNBUFFER_H
#define EVTRUNBUFFER_H

#include "msuClassicBuffer.h"

class evtRunBuffer
{
	private:
		std::string fRunTitle;
	public:
		///Default constructor.
		evtRunBuffer();
		///Get Run Title
		std::string GetRunTitle();
		///Read the run start buffer.
		void ReadRunBegin(msuClassicBuffer *buffer);


};
#endif
