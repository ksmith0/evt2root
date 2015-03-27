#ifndef MAESTROCHNBUFFER
#define MAESTROCHNBUFFER

#include "mainBuffer.h"

///Size of smallest word in bytes.
#define WORD_SIZE 2 
///The size of the buffer header in words.
#define BUFFER_HEADER_SIZE 0
///There is no fixed buffer size in the "ring" buffer.
#define BUFFER_SIZE 16


class TH1F;

/**Unpacks Maestro CHN Format files. More information available at
 * http://www.ortec-online.com/download/ORTEC-Software-File-Structure-Manual.pdf
 */
class maestroChnBuffer : public mainBuffer {
	private:
		bool fRunBeginRead;
		UShort_t fStartingChannel;
		Short_t fNumberOfChannels;
		///Pointer to output histogram.
		TH1F *fHist;
		///Counter of current histogram channel
		UInt_t fCurrentChannel;

	public:
		maestroChnBuffer(int headerSize = BUFFER_HEADER_SIZE, int bufferSize = BUFFER_SIZE, int wordSize = WORD_SIZE);
		virtual ~maestroChnBuffer();
		
		///Indicate if the current buffer contains physics data.
		virtual bool IsDataType();
		///Indicate if the current buffer contains scalers.
		virtual bool IsScalerType() {return false;};
		///Indicate if the current buffer is a run begin buffer.
		virtual bool IsRunBegin();
		///Indicate if the current buffer is a run end buffer.
		virtual bool IsRunEnd();

		///Read the run start buffer.
		virtual void ReadRunBegin(bool verbose=false);
		///Read the run end buffer.
		virtual void ReadRunEnd(bool verbose=false);
		///Read a data event.
		virtual int ReadEvent(bool verbose=false);
		///Reads current scaler event.
		virtual void ReadScalers(bool verbose = false) {};

		///Read the next buffer.
		virtual int ReadNextBuffer();
		///Unpack the current buffer.
		void UnpackBuffer(bool verbose = false);

		///Output the values stored in the buffer header.
		void PrintBufferHeader();
};

#undef WORD_SIZE
#undef BUFFER_HEADER_SIZE
#undef BUFFER_SIZE

#endif
