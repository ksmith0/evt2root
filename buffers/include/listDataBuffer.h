#ifndef LISTDATABUFFER
#define LISTDATABUFFER

#include "mainBuffer.h"

class listDataBuffer : public mainBuffer {
	private:

	public:
		listDataBuffer(int bufferSize, int headerSize, int wordSize);
		virtual ~listDataBuffer();
		
};

#endif
