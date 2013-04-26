#ifndef MSURUNFILE_H 
#define MSURUNFILE_H

#include <iostream>
#include <stdio.h>
#include "msuBuffer.h"

class msuRunFile
{
	private:
		FILE *fFP;
		char *fFileName;
		void fOpenFile(char *filename);
		msuBuffer *fBuffer;
	public:
		msuRunFile(char *fileName);
		msuBuffer *GetNextBuffer();
		

};

#endif
