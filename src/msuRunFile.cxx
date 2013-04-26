#include "msuRunFile.h"

msuRunFile::msuRunFile(char *filename) 
{
	this->fOpenFile(filename);
	fBuffer = new msuBuffer();
}
void msuRunFile::fOpenFile(char *filename)
{
	try {
		if ((fFP=fopen(filename,"r")) == NULL) throw filename;
	}
	catch (char *e)
	{
		fprintf(stderr,"ERROR: Can't open evtfile %s\n",filename);
	}
	return;
}
msuBuffer *msuRunFile::GetNextBuffer() 
{
	unsigned short int buffer[4096];
	int readWords = 0;
	int nRead = fread(buffer, 2, 4096, fFP);
	fBuffer->fReadWords = 0;
	fBuffer->fNumWords = buffer[readWords++];
	fBuffer->fSubevtType = buffer[readWords++];
	fBuffer->fChecksum = buffer[readWords++];
	fBuffer->fRunNum = buffer[readWords++];
	readWords += 12;
  	nRead += fread(fBuffer->fBuffer, 2, 4096 - nRead, fFP) ;
	if (nRead != 4096) {
		fprintf(stderr,"ERROR: Incorrect buffer size!\n");
		return NULL;
	}

	return fBuffer;
}

