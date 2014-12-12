#ifndef NSCLCLASSICBUFFER_H 
#define NSCLCLASSICBUFFER_H

#include "mainBuffer.h"

///Size of smallest word in bytes
#define WORD_SIZE 2 
///The size of the buffer header in words.
#define BUFFER_HEADER_SIZE 16
///Size of the buffer in words.
#define BUFFER_SIZE 4096

class nsclClassicBuffer : public mainBuffer {
	protected:
		///The number of LAM registers.
		UShort_t fNumOfLAMRegisters;
		///The number of the CPU that generated the buffer.
		UShort_t fNumOfCPU;
		///The number of bit registers.
		UShort_t fNumOfBitRegisters;

		///Read the run begin or end buffer.
		virtual void ReadRunBeginEnd(UInt_t elapsedTime, time_t &timeStamp, std::string &runTitle, bool verbose=false);

		//Get the run time from a run begin or end buffer.
		virtual time_t GetRunTime(bool verbose=false);

	public:
		enum BufferType
		{
			///Physics data type.
			BUFFER_TYPE_DATA = 1, 		
			///Scaler type.
			BUFFER_TYPE_SCALERS = 2 , 
			///EPICS
			BUFFER_TYPE_EPICS = 5,	
			///Run Begin type.
			BUFFER_TYPE_RUNBEGIN = 11,
			///Run End type.
			BUFFER_TYPE_RUNEND = 12 
		};

		///Default constructor
		nsclClassicBuffer(const char* filename,int bufferSize=BUFFER_SIZE, int headerSize=BUFFER_HEADER_SIZE, int wordSize=WORD_SIZE);
		virtual ~nsclClassicBuffer();

		///Clear the current values for the buffer.
		virtual void Clear();

		///Read the next buffer.
		virtual int ReadNextBuffer();
		///Read the run start buffer.
		virtual void ReadRunBegin(bool verbose=false);
		///Read the run end buffer.
		virtual void ReadRunEnd(bool verbose=false);
		///Read a data event.
		virtual int ReadEvent(bool verbose=false);
		///Reads current scaler event.
		virtual void ReadScalers(bool verbose = false);

		///Output the values stored in the buffer header.
		void PrintBufferHeader();
};

#undef WORD_SIZE
#undef BUFFER_HEADER_SIZE
#undef BUFFER_SIZE

#endif
