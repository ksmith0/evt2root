#ifndef MSUCLASSICBUFFER_H
#define MSUCLASSICBUFFER_H

#define BUFFER_SIZE 4096

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
		FILE *fFP;
		char *fFileName;
		///Number of words in buffer excluding the 16 word header.
		short int fNumWords;
		///Type of events in this buffer.
		short int fSubevtType;
		short int fChecksum;
		///Current run number.
		short int fRunNum;
		///Event buffer containing all events in this buffer.
		unsigned short int fBuffer[BUFFER_SIZE];
		int fReadWords;

		std::string fRunTitle;


		void fOpenFile(char *filename);
	public:
		void Clear();
		msuClassicBuffer(char *);
		///Return next event in current buffer 
		int GetNextBuffer();
		int GetNumOfWords();
		///Return current buffer type.
		int GetSubEvtType();
		///Reutrn current buffer position
		int GetPosition();
		///Read out next word in buffer.
		unsigned int GetWord();
		///Read out next long word in buffer.
		unsigned int GetLongWord();
		///Get run value for current buffer.
		int GetRun();
		///Get Run Title
		std::string GetRunTitle();
		void ReadRunTitle();
		///Skip forward in the buffer by n words.
		void Forward(int numOfWords);
		///Dump the current buffer in hex
		void DumpBuffer();

};

#endif
