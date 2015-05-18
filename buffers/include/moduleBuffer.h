#ifndef LISTDATABUFFER
#define LISTDATABUFFER

#include "mainBuffer.h"

class moduleBuffer : public mainBuffer {
	private:
		void InitializeStorageManager();
	protected:
		void FillStorage();

	public:
		moduleBuffer(int bufferSize, int headerSize, int wordSize);
		virtual ~moduleBuffer();
		
};

#endif
