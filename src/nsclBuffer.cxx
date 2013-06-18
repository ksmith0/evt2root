#include "nsclBuffer.h"

nsclBuffer::nsclBuffer(char *filename, unsigned int bufferSize)
	: fFileName(filename),
	fBufferSize(bufferSize)
{
	fBuffer = new unsigned short int[fBufferSize];
	this->fOpenFile(fFileName);
	this->Clear();
}
nsclBuffer::~nsclBuffer() {
	delete[] fBuffer;
}
int nsclBuffer::GetBufferType()
{
	return fBufferType;
}
int nsclBuffer::GetBufferSize()
{
	return fBufferSize;
}

unsigned int nsclBuffer::GetNumOfWords()
{
	return fNumWords;
}
void nsclBuffer::fOpenFile(char *filename)
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
void nsclBuffer::Clear()
{
	fReadWords = 0;
	fBufferType = 0;
	fNumWords = 0;
	fBufferType = 0;
	fChecksum = 0;
	fRunNum = 0;
	fBufferNumber = 0;
	fNumOfEvents = 0;
	fNumOfLAMRegisters = 0;
	fNumOfCPU = 0;
	fNumOfBitRegisters = 0;

	for (int i=0;i<fBufferSize;i++) fBuffer[i] = 0;
}
unsigned int nsclBuffer::GetBufferNumber()
{
	return fBufferNumber;
}
int nsclBuffer::GetNextBuffer() 
{
	this->Clear();

	if (feof(fFP)) return 1;
	else {
		int nRead = fread(fBuffer, 2, fBufferSize, fFP);
		if (nRead != fBufferSize) {
			fprintf(stderr,"ERROR: Incorrect buffer size!\n");
			return -1;
		}

		fNumWords = fBuffer[fReadWords++];
		fBufferType = fBuffer[fReadWords++];
		fChecksum = fBuffer[fReadWords++];
		fRunNum = fBuffer[fReadWords++];
		fBufferNumber = (fBuffer[fReadWords++]);
		fBufferNumber = fBufferNumber | (fBuffer[fReadWords++] << 16);
		fNumOfEvents = fBuffer[fReadWords++];
		fNumOfLAMRegisters = fBuffer[fReadWords++];
		fNumOfCPU = fBuffer[fReadWords++];
		fNumOfBitRegisters = fBuffer[fReadWords++];
		Forward(6);
	}

	return 0;
}

void nsclBuffer::PrintBufferHeader() 
{
	printf("\nBuffer Header Summary:\n");
	printf("\tNum of words: %d\n",fNumWords);
	printf("\tBuffer type: %d\n",fBufferType);
	printf("\tChecksum: %d\n",fChecksum);
	printf("\tRun number: %d\n",fRunNum);
	printf("\tBuffer number: %d\n",fBufferNumber);
	printf("\tNumber of events: %d\n",fNumOfEvents);
	printf("\tNumber of LAM registers: %d\n",fNumOfLAMRegisters);
	printf("\tNumber of CPU: %d\n",fNumOfCPU);
	printf("\tNumber of bit registers: %d\n",fNumOfBitRegisters);
}
unsigned int nsclBuffer::GetRunNumber() 
{
	return fRunNum;
}

unsigned int nsclBuffer::GetWord()
{
	if (fReadWords < fNumWords) {
		return fBuffer[fReadWords++];
	}
	return 0;
}
unsigned int nsclBuffer::GetLongWord()
{
	unsigned short int word[2];
	if (fReadWords+1 < fNumWords) {
		for (int i=0;i<2;i++) 
			word[i] = GetWord();
#ifdef VM_USB
		return (word[1]<<16) | (word[0]);
#else
		return (word[0]<<16) | (word[1]);
#endif
	}

	return 0xFFFFFFFF;

		
}
void nsclBuffer::Forward(int numOfWords)
{
	fReadWords += numOfWords;
}
void nsclBuffer::Rewind(int numOfWords)
{
	fReadWords -= numOfWords;
}

unsigned int nsclBuffer::GetPosition()
{
	return fReadWords;
}

void nsclBuffer::DumpHeader()
{
	printf("\nBuffer Header:");
	for (int i=0;i<16;i++) {
		if (i % 10 == 0) 
			printf("\n\t");
		printf("0x%04X ",fBuffer[i]);
	}
	printf("\n");
}

void nsclBuffer::DumpBuffer()
{
	printf("\nBuffer Length %d:", fNumWords);
	for (int i=0;i<fNumWords && i<fBufferSize;i++) {
		if (i % 10 == 0) printf("\n\t");
		printf("0x%04X ",fBuffer[i]);
	}
	printf("\n");
}


