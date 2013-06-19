#ifndef NSCLBUFFER_H
#define NSCLBUFFER_H

#define BUFFER_SIZE 13328 //Typical value for VM USB 
//#define BUFFER_SIZE 4096 //Another typical value
#define VM_USB //Are we using the USB version?

#include <iostream>
#include <stdio.h>
#include <string.h>

enum BufferType
{
	BUFFER_TYPE_DATA = 1, 		//1
	BUFFER_TYPE_SCALERS,  		//2 
	BUFFER_TYPE_EPICS = 5,		//5
	BUFFER_TYPE_RUNBEGIN = 11, //11
	BUFFER_TYPE_RUNEND = 12 	//12

};

class nsclBuffer
{
	private:
		///File pointer to evt file.
		FILE *fFP;
		///File name of evt file.
		char *fFileName;
		///Buffer size.
		const unsigned int fBufferSize;
		///Number of words in buffer excluding the 16 word header.
		unsigned short int fNumWords;
		///Type of events in this buffer.
		unsigned short int fBufferType;
		///Checksum should evaluate to 0.
		short int fChecksum;
		///Current run number.
		unsigned short int fRunNum;
		///The current buffer number.
		unsigned int fBufferNumber;
		///The number of events in the current buffer.
		unsigned short int fNumOfEvents;
		///The number of LAM registers.
		unsigned short int fNumOfLAMRegisters;
		///The number of the CPU that generated the buffer.
		unsigned short int fNumOfCPU;
		///The number of bit registers.
		unsigned short int fNumOfBitRegisters;
		///Event buffer containing all events in this buffer.
		unsigned short int *fBuffer;
		///The number of words read in the current buffer.
		unsigned int fReadWords;

		///Open evt file.
		void fOpenFile(char *filename);
	public:
		///Default constructor
		nsclBuffer(char *filename, unsigned int bufferSize=BUFFER_SIZE);
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
		///Read out next word in buffer.
		unsigned int GetWord();
		///Read out next long word in buffer.
		unsigned int GetLongWord();
		///Get run value for current buffer.
		unsigned int GetRunNumber();
		///Skip forward in the buffer by n words.
		void Forward(int numOfWords);
		///Skip backward in the buffer by n words.
		void Rewind(int numOfWords);
		///Dump the current buffer header in hex.
		void DumpHeader();
		///Dump the current buffer in hex.
		void DumpBuffer();

};

#endif
