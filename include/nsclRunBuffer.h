#ifndef NSCLRUNBUFFER_H
#define NSCLRUNBUFFER_H

#include <time.h>
#include "mainBuffer.h"

class nsclRunBuffer
{
	private:
		std::string fRunTitle;
		time_t fRunStartTime; 
		time_t fRunEndTime; 
		UInt_t fRunNumber;
		unsigned int fElapsedRunTime;

		time_t GetTime(mainBuffer *buffer, bool verbose = false);
		std::string GetTitle(mainBuffer *buffer, bool verbose = false);
	public:
		///Default constructor.
		nsclRunBuffer();
		///Get Run Title
		std::string GetRunTitle();
		///Read the run start buffer.
		void ReadRunBegin(mainBuffer *buffer,bool verbose=false);
		void ReadRunEnd(mainBuffer *buffer,bool verbose=false);
		///Get run start time.
		time_t GetRunStartTime();
		time_t GetRunEndTime();
		///Get the run number.
		Int_t GetRunNumber() {return fRunNumber;}
		unsigned int GetElapsedRunTime();
		void DumpRunBuffer(mainBuffer *buffer);


};
#endif
