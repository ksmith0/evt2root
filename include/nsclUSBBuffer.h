#ifndef NSCLUSBBUFFER_H 
#define NSCLUSBBUFFER_H

#include "nsclClassicBuffer.h"

///Size of smallest word in bytes
#define WORD_SIZE 2 
///The size of the buffer header in words.
#define BUFFER_HEADER_SIZE 16
///Size of the buffer in words.
#define BUFFER_SIZE 13328

class nsclUSBBuffer : public nsclClassicBuffer {
	protected:
		///Return the length of the current event in words.
		virtual UInt_t GetEventLength();

		//Get the run time from a run begin or end buffer.
		virtual time_t GetRunTime(bool verbose=false);

	public:
		///Default constructor
		nsclUSBBuffer(const char* filename,bool moduleBoundaryWord=true, int bufferSize=BUFFER_SIZE, int headerSize=BUFFER_HEADER_SIZE, int wordSize=WORD_SIZE);
		virtual ~nsclUSBBuffer();
		///Read the next buffer.
		virtual int ReadNextBuffer();
		///Read a data event.
		virtual int ReadEvent(bool verbose=false);
};

#undef WORD_SIZE
#undef BUFFER_HEADER_SIZE
#undef BUFFER_SIZE

#endif
