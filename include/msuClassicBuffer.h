#ifndef MSUCLASSICBUFFER_H
#define MSUCLASSICBUFFER_H

#define BUFFER_SIZE 13328 
#define VMUSB

#include <iostream>
#include <stdio.h>
#include <string.h>

enum SubEvtType
{
	SUBEVT_TYPE_DATA = 1, 		//1
	SUBEVT_TYPE_SCALERS,  		//2 
	SUBEVT_TYPE_EPICS = 5,		//5
	SUBEVT_TYPE_RUNBEGIN = 11, //11
	SUBEVT_TYPE_RUNEND = 12 	//12

};

class msuClassicBuffer
{
	private:
		//File pointer to evt file.
		FILE *fFP;
		//File name of evt file.
		char *fFileName;
		///Number of words in buffer excluding the 16 word header.
		short int fNumWords;
		///Type of events in this buffer.
		short int fSubevtType;
		///Checksum should evaluate to 0.
		short int fChecksum;
		///Current run number.
		short int fRunNum;
		///The current buffer number.
		unsigned int fBufferNumber;
		///The number of events in the current buffer.
		short int fNumOfEvents;
		///The number of LAM registers.
		short int fNumOfLAMRegisters;
		///The number of the CPU that generated the buffer.
		short int fNumOfCPU;
		///The number of bit registers.
		short int fNumOfBitRegisters;
		///Event buffer containing all events in this buffer.
		unsigned short int fBuffer[BUFFER_SIZE];
		///The number of words read in the current buffer.
		int fReadWords;

		///Open evt file.
		void fOpenFile(char *filename);
	public:
		///Default constructor
		msuClassicBuffer(char *filename);
		///Clear the current values for the buffer.
		void Clear();
		///Output the values stored in the buffer header.
		void PrintBufferHeader();
		///Return next buffer.
		int GetNextBuffer();
		///Return the number of words in the current buffer.
		int GetNumOfWords();
		///Return current buffer type.
		int GetSubEvtType();
		///Reutrn current buffer position.
		int GetPosition();
		///Read out next word in buffer.
		unsigned int GetWord();
		///Read out next long word in buffer.
		unsigned int GetLongWord();
		///Get run value for current buffer.
		int GetRunNumber();
		///Skip forward in the buffer by n words.
		void Forward(int numOfWords);
		///Dump the current buffer in hex.
		void DumpBuffer();

};

#endif
