#ifndef NSCLRINGBUFFER_H
#define NSCLRINGBUFFER_H

#include "mainBuffer.h"

///Size of smallest word in bytes.
#define WORD_SIZE 4 
///The size of the buffer header in words.
#define BUFFER_HEADER_SIZE 2
///There is no fixed buffer size in the "ring" buffer.
#define BUFFER_SIZE 0

///Class to handle main buffer in NSCL evt files.
class nsclRingBuffer : public mainBuffer
{
	private:
		///Read the run begin or end buffer.
		void ReadRunBeginEnd(UInt_t &runNum, UInt_t &elpasedTime, time_t &timeStamp, std::string &runTitle, bool verbose=false);

		///Return the length of the current event in words.
		virtual UInt_t GetEventLength();

	public:
		enum BufferType
		{
			///Physics data type.
			BUFFER_TYPE_DATA = 30,
			///Scaler type.
			BUFFER_TYPE_SCALERS = 20, 
			///EPICS
			BUFFER_TYPE_EPICS = 11, 
			///Run Begin type.
			BUFFER_TYPE_RUNBEGIN = 1,
			///Run End type.
			BUFFER_TYPE_RUNEND = 2 
		};
		///Default constructor
		nsclRingBuffer(const char *filename, int bufferSize=BUFFER_SIZE, int headerSize=BUFFER_HEADER_SIZE, int wordSize=WORD_SIZE);
		virtual ~nsclRingBuffer();

		///Read the next buffer.
		int ReadNextBuffer();
		///Read the run start buffer.
		void ReadRunBegin(bool verbose=false);
		///Read the run end buffer.
		void ReadRunEnd(bool verbose=false);
		///Read a data event.
		int ReadEvent(bool verbose=false);
		///Reads current scaler event.
		void ReadScalers(bool verbose = false);

		///Output the values stored in the buffer header.
		void PrintBufferHeader();
};

#undef WORD_SIZE
#undef BUFFER_HEADER_SIZE
#undef BUFFER_SIZE


#endif
