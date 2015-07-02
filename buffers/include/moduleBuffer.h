#ifndef LISTDATABUFFER
#define LISTDATABUFFER

#include "mainBuffer.h"

class moduleBuffer : public mainBuffer {
	private:
		void InitializeStorageManager();
	protected:
		void FillStorage();
		void ClearModules();

	public:
		moduleBuffer(int bufferSize, int headerSize, int wordSize);
		virtual ~moduleBuffer();
		
};

#endif
