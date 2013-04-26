#include "msuRingBuffer.h"

msuRingBuffer::msuRingBuffer(char *filename)
{
	fReadWords = 0;
	fSubevtType = 0;
	this->fOpenFile(filename);
	this->Clear();
}
int msuRingBuffer::GetSubEvtType()
{
	return fSubevtType;

}
void msuRingBuffer::DumpBuffer()
{
	printf("\n\n");
	unsigned int nRead = 0;
	while (nRead < fNumWords) {
		printf("0x%08X ",fBuffer[nRead++]);
		if ((nRead) % 10 == 0) {
			printf("\n");
		}
	}
	printf("\n");
}
int msuRingBuffer::GetNumOfWords()
{
	return fNumWords;
}
void msuRingBuffer::fOpenFile(char *filename)
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
void msuRingBuffer::Clear()
{
	fNumWords = 0;
	fSubevtType = 0;
	fRunNum = 0;
	for (int i=0;i<BUFFER_SIZE;i++) fBuffer[i] = 0;
	fReadWords = 0;
}
/**
 * \bug Does not correctly deal with event that have buffer lengths that are not multiples of 4.
 */
int msuRingBuffer::GetNextBuffer() 
{
	int nRead = 0;
	int eventLength = 0;
	this->Clear();

	if (feof(fFP)) {
		printf ("End of evt file.\n");
		return 1;
	}

	else {
		if (fread(fBuffer, 4, 2, fFP) != 2) {
			fprintf(stderr,"ERROR: Incorrect header size!\n");
			return -1;
		}
		//Get packet header including length of buffer and type
		//Get length of buffer (buffer length in bytes convert to words by divide by 4)
		eventLength = fBuffer[fReadWords];
		//Event length not a multiple of four, skip for now!
		if (eventLength % 4 != 0) {
			fseek(fFP,-8,SEEK_CUR);
			nRead = fread(fBuffer, 1, eventLength, fFP);
			fprintf(stderr,"\nWARNING: Wrong number of words!\n");
			this->Clear();
			return -1;
		}
		fNumWords = eventLength / 4;
		fReadWords++;
		fSubevtType = fBuffer[fReadWords++];

		//Rewind over header
		fseek(fFP,-8,SEEK_CUR);
		nRead = fread(fBuffer, 4, fNumWords, fFP);
		
		if ((unsigned int)nRead != fNumWords) {
			fprintf(stderr,"ERROR: Incorrect buffer size!\n");
			return -1;
		}

	}

	return 0;
}
/**Gets information about curent run: run number and title.
 */
void msuRingBuffer::GetRunInfo()
{
	fRunNum = fBuffer[fReadWords++];
	fTimeOffset = fBuffer[fReadWords++];
	fTimestamp = fBuffer[fReadWords++];
	for (int i=0;i<168;i++) {
		fRunTitle[4*i] = fBuffer[fReadWords+i];
		fRunTitle[4*i+1] = fBuffer[fReadWords + i] >> 8;
		fRunTitle[4*i+2] = fBuffer[fReadWords + i] >> 16;
		fRunTitle[4*i+3] = fBuffer[fReadWords + i] >> 24;
	}
		
}

unsigned int msuRingBuffer::GetWord()
{
	if (fReadWords < fNumWords) {
		return fBuffer[fReadWords++];
	}
	return 0;
}
void msuRingBuffer::Forward(int numOfWords)
{
	fReadWords += numOfWords;
	if (fReadWords > fNumWords) fReadWords = fNumWords;
}



