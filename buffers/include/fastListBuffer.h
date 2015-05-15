#ifndef FASTLISTBUFFER_H
#define FASTLISTBUFFER_H

#include "mainBuffer.h"
#include <array>

///Size of smallest word in bytes
#define WORD_SIZE 4 
///The size of the buffer header in words.
#define BUFFER_HEADER_SIZE 0
///Size of the buffer in words.
#define BUFFER_SIZE 0

class fastListBuffer : public mainBuffer {
	private:
		///Flag indicating if the header was read.
		bool headerRead;
		///A count of the number of active ADCs during acquistion.
		unsigned short numActiveADCs;
		///A vector of which ADCs were triggered during an event.
		std::vector<unsigned short> triggeredADCs;
		///An array of ADC values from an event.
		std::array<UShort_t,16> adcValues;

		///Read out the ASCII header.
		unsigned int ReadHeader(bool verbose);

	public:
		///Constructor
		fastListBuffer(int bufferSize=BUFFER_SIZE, int bufferHeaderSize=BUFFER_HEADER_SIZE, int wordSize=WORD_SIZE);
		///Destructor
		virtual ~fastListBuffer();

		enum BufferType
		{
			///ADC_Data
			DATA = 1, 		
			///Timestamp
			TIME,
			///Syncronization mark.
			SYNC_MARK,
			///The begining of the data file.
			RUN_BEGIN
		};

		///Clear the current values for the buffer.
		virtual void Clear();

		///Reads the next buffer.
		virtual int ReadNextBuffer();

		///Output the values stored in the buffer header.
		virtual void PrintBufferHeader() {}

		///Indicate if the current buffer contains physics data.
		virtual bool IsDataType();
		///Indicate if the current buffer contains scalers.
		virtual bool IsScalerType() {return false;}
		///Indicate if the current buffer is a run begin buffer.
		virtual bool IsRunBegin();
		///Indicate if the current buffer is a run end buffer.
		virtual bool IsRunEnd() {return false;}

		///Unpack the current buffer
		virtual void UnpackBuffer(bool verbose = false);
		///Reads current event and stores data.
		virtual int ReadEvent(bool verbose = false);
		///Reads current scaler event.
		virtual void ReadScalers(bool verbose = false) {}
		///Read the run start buffer.
		virtual void ReadRunBegin(bool verbose = false);
		///Read the run end buffer.
		virtual void ReadRunEnd(bool verbose = false) {}

		///Storage Manager initialization.
		virtual void InitializeStorageManager();
};

#undef WORD_SIZE
#undef BUFFER_HEADER_SIZE
#undef BUFFER_SIZE


#endif
