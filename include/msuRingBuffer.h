#ifndef MSURINGBUFFER_H
#define MSURINGBUFFER_H

#define BUFFER_SIZE 8192

#include <iostream>
#include <stdio.h>
#include <string.h>

enum SubEvtType
{
	SUBEVT_TYPE_BEGIN_RUN = 1, 
	SUBEVT_TYPE_SCALERS = 20,  
	SUBEVT_TYPE_DATA = 30 

};

class msuRingBuffer
{
	private:
		FILE *fFP;
		char *fFileName;
		///Number of words in buffer excluding the 16 word header.
		unsigned int fNumWords;
		///Type of events in this buffer.
		unsigned int fSubevtType;
		///Current run number.
		unsigned int fRunNum;
		unsigned int fTimeOffset;
		time_t fTimestamp;
		///Current run title
		char fRunTitle[673];
		///Event buffer containing all events in this buffer.
		unsigned int  fBuffer[BUFFER_SIZE];
		unsigned int fReadWords;
		///
		void fOpenFile(char *filename);
	public:
		///Reset current buffer values.
		void Clear();
		msuRingBuffer(char *);
		///Return next event in current buffer. 
		int GetNextBuffer();
		///Return the number of words in this buffer.
		int GetNumOfWords();
		///Return current buffer type.
		int GetSubEvtType();
		///Read out next word in buffer.
		unsigned int GetWord();
		///Skip forward in the buffer by n words.
		void Forward(int numOfWords);
		///Dump the current buffer in hex
		void DumpBuffer();
		void GetRunInfo();

};

#endif
