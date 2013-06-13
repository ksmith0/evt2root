#include "msuClassicBuffer.h"

msuClassicBuffer::msuClassicBuffer(char *filename)
{
	fReadWords = 0;
	fSubevtType = 0;
	this->fOpenFile(filename);
	this->Clear();
}
int msuClassicBuffer::GetSubEvtType()
{
	return fSubevtType;
}

int msuClassicBuffer::GetNumOfWords()
{
	return fNumWords;
}
void msuClassicBuffer::fOpenFile(char *filename)
{
	try {
		if ((fFP=fopen(filename,"r")) == NULL) throw filename;
	}
	catch (char *e)
	{
		fprintf(stderr,"ERROR[ClassicBuffer.cxx]: Can't open evtfile %s\n",filename);
	}
	return;
}
void msuClassicBuffer::Clear()
{
	fNumWords = 0;
	fSubevtType = 0;
	fChecksum = 0;
	fRunNum = 0;
	for (int i=0;i<BUFFER_SIZE;i++) fBuffer[i] = 0;
	fReadWords = 0;
}
int msuClassicBuffer::GetNextBuffer() 
{
	this->Clear();

	if (feof(fFP)) return 1;
	else {
		int nRead = fread(fBuffer, 2, BUFFER_SIZE, fFP);
		if (nRead != BUFFER_SIZE) {
			fprintf(stderr,"ERROR[ClassicBuffer.cxx]: Incorrect buffer size!\n");
			return -1;
		}

		fNumWords = fBuffer[fReadWords++];
		fSubevtType = fBuffer[fReadWords++];
		fChecksum = fBuffer[fReadWords++];
		fRunNum = fBuffer[fReadWords++];
		fBufferNumber = (fBuffer[fReadWords++] << 16) | (fBuffer[fReadWords++]);
		fNumOfEvents = fBuffer[fReadWords++];
		fNumOfLAMRegisters = fBuffer[fReadWords++];
		fNumOfCPU = fBuffer[fReadWords++];
		fNumOfBitRegisters = fBuffer[fReadWords++];
		Forward(6);
	}

	return 0;
}

void msuClassicBuffer::PrintBufferHeader() 
{
	printf("\nBuffer Header Summary:\n");
	printf("\tNum of words: %d\n",fNumWords);
	printf("\tChecksum: %d\n",fChecksum);
	printf("\tRun number: %d\n",fRunNum);
	printf("\tBuffer number: %d\n",fBufferNumber);
	printf("\tNumber of events: %d\n",fNumOfEvents);
	printf("\tNumber of LAM registers: %d\n",fNumOfLAMRegisters);
	printf("\tNumber of CPU: %d\n",fNumOfCPU);
	printf("\tNumber of bit registers: %d\n",fNumOfBitRegisters);
	printf("\n");
}
int msuClassicBuffer::GetRunNumber() 
{
	return fRunNum;
}
std::string msuClassicBuffer::GetRunTitle()
{
	if(!fRunTitle.length()) ReadRunTitle();
	return fRunTitle;
}

void msuClassicBuffer::ReadRunTitle() {
	if (GetSubEvtType() != SUBEVT_TYPE_RUNBEGIN) {
		fprintf(stderr,"ERROR[ClassicBuffer.cxx]: Not a run begin subevt!\n");
		return;
	}

	for (int i=0;i<32;i++) {
		int word = GetWord();
		if (word == 0) break;
		fRunTitle.push_back(word & 0xFF);
		int letter = (word & 0xFF00) >> 8;
		if (letter)
			fRunTitle.push_back(letter);
	}
}

unsigned int msuClassicBuffer::GetWord()
{
	if (fReadWords < fNumWords) {
		return fBuffer[fReadWords++];
	}
	return 0;
}
unsigned int msuClassicBuffer::GetLongWord()
{
	unsigned short int word[2];
	unsigned short int item;
	short goodWords=0;
	if (fReadWords+1 < fNumWords) {
		for (int i=0;i<2;i++) {
			item = GetWord();
				if (item != 0xffff) { 
					word[i] = item;
					goodWords++;
				}
		}
		if (goodWords <2) 
			return 0xFFFFFFFF;
	#ifdef VMUSB
		return (word[1]<<16) | (word[0]);
	#else
		return (word[0]<<16) | (word[1]);
	#endif
	}

	return 0xFFFFFFFF;

		
}
void msuClassicBuffer::Forward(int numOfWords)
{
	fReadWords += numOfWords;
}

int msuClassicBuffer::GetPosition()
{
	return fReadWords;
}

void msuClassicBuffer::DumpBuffer()
{
	printf("\n\n");
	unsigned int nRead = 0;
	while (nRead < fNumWords) {
		printf("0x%04X ",fBuffer[nRead++]);
		if ((nRead) % 10 == 0) {
			printf("\n");
		}
	}
	printf("\n");
}


