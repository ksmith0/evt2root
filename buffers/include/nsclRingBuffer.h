#ifndef NSCLRINGBUFFER_H
#define NSCLRINGBUFFER_H

#include "moduleBuffer.h"

///Size of smallest word in bytes.
#define WORD_SIZE 4 
///The size of the buffer header in words.
#define BUFFER_HEADER_SIZE 0
///There is no fixed buffer size in the "ring" buffer.
#define BUFFER_SIZE 0

///Class to handle main buffer in NSCL evt files.
class nsclRingBuffer : public moduleBuffer
{
	private:
		///The buffer's version.
		UInt_t fVersion;
		///Container for information stored in header of format buffer.
		struct bodyHeader {
			///The size of the body header in bytes. 
			UInt_t fLength;
			///The time stamp specified in the body header.
			ULong64_t fTimeStamp;
			///The source ID specified in the body header.
			UInt_t fSourceID;
			///The barrier specified in the body header.
			UInt_t fBarrier;
		};

		bodyHeader fBodyHeader;

		///Clear the current values for the buffer.
		void Clear();
		
		///Read the body of the header.
		void ReadBodyHeader();
		///Read the buffer version
		UInt_t ReadVersion(bool verbose = false);

		///Read the run begin or end buffer.
		void ReadRunBeginEnd(UInt_t &runNum, UInt_t &elpasedTime, time_t &timeStamp, std::string &runTitle, bool verbose=false);

		///Return the length of the current event in words.
		virtual UInt_t GetEventLength();

	public:
		enum BufferType
		{
			///Run Begin type.
			BUFFER_TYPE_RUNBEGIN = 1,
			///Run End type.
			BUFFER_TYPE_RUNEND = 2,
			///Run pause type.
			BUFFER_TYPE_RUNPAUSE = 3,
			///Run resume type.
			BUFFER_TYPE_RUNRESUME = 4,

			///Packet types
			BUFFER_PACKET_TYPES = 10,
			///EPICS
			BUFFER_TYPE_EPICS = 11, 
			///Ring format buffer.
			BUFFER_TYPE_FORMAT = 12,

			///Incremental Scaler type.
			BUFFER_TYPE_SCALERS = 20, 
			///Nonincremental scaler type.
			BUFFER_TYPE_NONINCR_SCALERS = 21,

			///Physics data type.
			BUFFER_TYPE_DATA = 30,
			///Physics data count.
			BUFFER_TYPE_DATA_COUNT = 31,

			///Event builder type.
			BUFFER_TYPE_EVB_FRAGMENT = 40,
			///Event builder fragment who payload isn't a ring item,
			BUFFER_TYPE_EVB_UNKNOWN_PAYLOAD = 41,
			///Event builder GLOM parameters.
			BUFFER_TYPE_EVB_GLOM_INFO =42
		};

		///Default constructor
		nsclRingBuffer(int bufferSize=BUFFER_SIZE, 
			int headerSize=BUFFER_HEADER_SIZE, int wordSize=WORD_SIZE);
		nsclRingBuffer(const char *filename, int bufferSize=BUFFER_SIZE, 
			int headerSize=BUFFER_HEADER_SIZE, int wordSize=WORD_SIZE);
		virtual ~nsclRingBuffer();

		///Indicate if the current buffer contains physics data.
		virtual bool IsDataType();
		///Indicate if the current buffer contains scalers.
		virtual bool IsScalerType();
		///Indicate if the current buffer is a run begin buffer.
		virtual bool IsRunBegin();
		///Indicate if the current buffer is a run end buffer.
		virtual bool IsRunEnd();

		///Read the next buffer.
		int ReadNextBuffer();
		///Unpack the buffer.
		void UnpackBuffer(bool verbose = false);
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
