#ifndef HRIBFBUFFER_H
#define HRIBFBUFFER_H

#include "mainBuffer.h"
#include <string>

///Size of smallest word in bytes.
#define WORD_SIZE 4 
///The size of the buffer header in words.
#define BUFFER_HEADER_SIZE 2
///There is no fixed buffer size in the "ring" buffer.
#define BUFFER_SIZE 8194

class hribfBuffer : public mainBuffer {
	private:	
		///Format of the buffer.
		std::string fFormat;

		///Return the length of the current event in words.
		virtual UInt_t GetEventLength();

	public:
		enum BufferType {
			///Physics data type.
			BUFFER_TYPE_DATA = 0x41544144, //"DATA"
			///Scaler type.
			BUFFER_TYPE_SCALERS = 0X4C414353, //"SCAL" 
			///Run Begin type.
			BUFFER_TYPE_RUNBEGIN = 0x44414548, //"HEAD"
			///End of File buffer
			BUFFER_TYPE_EOF = 0x20464F45, //"EOF " 
			///Deadtime buffer
			BUFFER_TYPE_DEAD = 0x44414544, //"DEAD" 
			BUFFER_TYPE_DIR = 0x20524944, //"DIR "
			BUFFER_TYPE_PAC = 0x20434150 //"PAC "
		};

		///Default constructor.
		hribfBuffer(int bufferSize=BUFFER_SIZE, 
			int headerSize=BUFFER_HEADER_SIZE, int wordSize=WORD_SIZE);
		///Construct and open the file.
		hribfBuffer(const char *filename, int bufferSize=BUFFER_SIZE, 
			int headerSize=BUFFER_HEADER_SIZE, int wordSize=WORD_SIZE);

		///Indicate if the current buffer contains physics data.
		virtual bool IsDataType();
		///Indicate if the current buffer contains scalers.
		virtual bool IsScalerType();
		///Indicate if the current buffer is a run begin buffer.
		virtual bool IsRunBegin();

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
		///Reads "DIR" Buffer
		void ReadDir(bool verbose = false);

		///Print a summary of the buffer header.
		void PrintBufferHeader();
};

#endif
