#ifndef NSCLRUNBUFFER_H
#define NSCLRUNBUFFER_H

#include "nsclBuffer.h"

class nsclRunBuffer
{
	private:
		std::string fRunTitle;
	public:
		///Default constructor.
		nsclRunBuffer();
		///Get Run Title
		std::string GetRunTitle();
		///Read the run start buffer.
		void ReadRunBegin(nsclBuffer *buffer);


};
#endif
