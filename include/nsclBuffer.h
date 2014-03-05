#ifndef NSCLBUFFER_H
#define NSCLBUFFER_H

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <Rtypes.h>

#include "evt_config.h"

#if RING_BUFFER && VM_USB
	#error "Cannot declare both USB and Ring Buffer types!"
#endif

#if RING_BUFFER
		///Size of smallest word in bytes.
		#define WORD_SIZE 4 
		///Type of used to contain a word.
		#define WORD_TYPE UInt_t 
		///Type of used to contain a long word
		#define LONG_WORD_TYPE ULong_t 
		///The size of the buffer header in words.
		#define BUFFER_HEADER_SIZE 2
		///There is no fixed buffer size in the "ring" buffer.
		#define BUFFER_SIZE 0

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
#else 
		///Size of smallest word in bytes
		#define WORD_SIZE 2 
		///Type of used to contain a word
		#define WORD_TYPE UShort_t 
		///Type of used to contain a long word
		#define LONG_WORD_TYPE UInt_t 
		///The size of the buffer header in words.
		#define BUFFER_HEADER_SIZE 16
		#ifndef BUFFER_SIZE
			#if VM_USB
				///Size of the buffer in words.
				#define BUFFER_SIZE 13328
			#else
				///Size of the buffer in words.
				#define BUFFER_SIZE 4096
			#endif
		#endif
		
	
		enum BufferType
		{
			BUFFER_TYPE_DATA = 1, 		
			BUFFER_TYPE_SCALERS = 2 , 
			BUFFER_TYPE_EPICS = 5,	
			BUFFER_TYPE_RUNBEGIN = 11,
			BUFFER_TYPE_RUNEND = 12 
		};
#endif

///Class to handle main buffer in NSCL evt files.
class nsclBuffer
{
	public:
		///The type of a standard word.
		typedef WORD_TYPE word;
		typedef LONG_WORD_TYPE longword;
	private:
		///File pointer to evt file.
		FILE *fFP;
		///File name of evt file.
		const char *fFileName;
		///Is this buffer from the NSCL "Ring" Buffer.
		const bool fIsRingBuffer;
		///Is this file from a VM USB setup.
		const bool fIsUSB;
		///Buffer size.
		unsigned int fBufferSize;
		///Size of a wor din the buffer.
		unsigned short int fWordSize;
		///Number of words in buffer excluding the 16 word header.
		word fNumWords;
		///Type of events in this buffer.
		word fBufferType;
		///Checksum should evaluate to 0.
		word fChecksum;
		///Current run number.
		word fRunNum;
		///The current buffer number.
		UInt_t fBufferNumber;
		///The number of events in the current buffer.
		word fNumOfEvents;
		///The number of LAM registers.
		word fNumOfLAMRegisters;
		///The number of the CPU that generated the buffer.
		word fNumOfCPU;
		///The number of bit registers.
		word fNumOfBitRegisters;
		///Event buffer header.
		word *fBufferHeader;
		///Event buffer containing all events in this buffer.
		word *fBuffer;
		///The number of words read in the current buffer.
		unsigned int fReadWords;
		///The portion of word read.
		unsigned short int fFractionalWordPos;

		///Open evt file.
		void OpenFile(const char *filename);
		///Close evt file.
		void CloseFile();
	public:
		///Default constructor
		nsclBuffer(const char *filename);
		virtual ~nsclBuffer();
		///Clear the current values for the buffer.
		void Clear();
		///Output the values stored in the buffer header.
		void PrintBufferHeader();
		///Return next buffer.
		int GetNextBuffer();
		///Return the number of words in the current buffer.
		unsigned int GetNumOfWords();
		///Return the number of events in the current buffer.
		unsigned int GetNumOfEvents();
		///Return the size for each buffer.
		int GetBufferSize();
		///Return current buffer type.
		int GetBufferType();
		///Returns the number of the current buffer.
		unsigned int GetBufferNumber();
		///Reutrn current buffer position.
		unsigned int GetPosition();
		///Read out next word in the buffer.
		word GetWord();
		///Get the size of a standard word.
		unsigned short int GetWordSize() {return fWordSize;}
		///Read out next long word in the buffer.
		longword GetLongWord(bool reverse = false);
		///Get the next two byte word in the buffer.
		UShort_t GetTwoByteWord();
		///Get the next four byte word in the buffer.
		UInt_t GetFourByteWord(bool reverse = false);
		///Get run value for current buffer.
		word GetRunNumber();
		///Check if this buffer is a ring buffer.
		bool IsRingBuffer() {return fIsRingBuffer;}
		///Check if this buffer is a USB type.
		bool IsUSB() {return fIsUSB;}
		///Skip forward in the buffer by n words.
		void Forward(int numOfWords);
		///Skip backward in the buffer by n words.
		void Rewind(int numOfWords);
		///Skip backward in the buffer by n bytes.
		void RewindBytes(int numOfBytes);
		///Dump the current buffer header in hex.
		void DumpHeader();
		///Dump the current buffer in hex.
		void DumpBuffer();

};

#endif
