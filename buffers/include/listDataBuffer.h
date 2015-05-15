#ifndef LISTDATABUFFER
#define LISTDATABUFFER

#include "mainBuffer.h"

class listDataBuffer : public mainBuffer {
	private:
		void InitializeStorageManager();
	protected:
		void FillStorage();

	public:
		listDataBuffer(int bufferSize, int headerSize, int wordSize);
		virtual ~listDataBuffer();
		
};

#endif
