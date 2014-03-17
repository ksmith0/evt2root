#include "nsclBuffer.h"

nsclBuffer::nsclBuffer(const char *filename)
	: fFileName(filename),
	fIsRingBuffer(RING_BUFFER),
	fIsUSB(VM_USB),
	fWordSize(WORD_SIZE),
	fBuffer(0),
	fBufferNumber(0)
{
	if (BUFFER_SIZE > BUFFER_HEADER_SIZE) {
		fBufferSize = BUFFER_SIZE - BUFFER_HEADER_SIZE;
	}
	else fBufferSize = 0;
	fBufferHeader = new word[BUFFER_HEADER_SIZE];
	fBuffer = new word[fBufferSize];
	this->OpenFile(fFileName);
	this->Clear();
}
nsclBuffer::~nsclBuffer() {
	CloseFile();
	delete[] fBufferHeader;
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
unsigned int nsclBuffer::GetNumOfEvents()
{
	return fNumOfEvents;
}
void nsclBuffer::OpenFile(const char *filename)
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
void nsclBuffer::CloseFile()
{
	fclose(fFP);
}
void nsclBuffer::Clear()
{
	fReadWords = 0;
	fFractionalWordPos = 0;
	fBufferType = 0;
	fNumWords = 0;
	fBufferType = 0;
	fChecksum = 0;
	fRunNum = 0;
	fNumOfEvents = 0;
	fNumOfLAMRegisters = 0;
	fNumOfCPU = 0;
	fNumOfBitRegisters = 0;
}
unsigned int nsclBuffer::GetBufferNumber()
{
	return fBufferNumber;
}
/**Reads the next word from the file providing the size of the next buffer.
 * A large enough block of memory is reserved and the next buffer is read
 * into it. The buffer header is then disseminated.
 */
int nsclBuffer::GetNextBuffer() 
{
	UInt_t nRead = 0;
	this->Clear();

	if (feof(fFP)) {
		return 0;
	}
	else {
		//Get buffer header
		nRead = fread(fBufferHeader, fWordSize, BUFFER_HEADER_SIZE, fFP);

		if (nRead != BUFFER_HEADER_SIZE) {
			fprintf(stderr,"ERROR: Buffer header was incomplete! %d words read.\n",nRead);
			return 0;
		}

		if (IsRingBuffer()) {
			if (fNumWords % 4 != 0) {
				fprintf(stderr,"ERROR: Number of words (%d) is not a multiple of four!\n",fNumWords);
				return 0;
			}
			fNumWords = fBufferHeader[0] / fWordSize - BUFFER_HEADER_SIZE;
			fBufferType = fBufferHeader[1];
			if (fBufferType == BUFFER_TYPE_DATA) fNumOfEvents = 1;
			fBufferNumber++;

			//The "ring" buffer has buffers exactly the size of the event.
			fBufferSize = fNumWords;
			//If size of buffer in words (sizeof returns bits) is smaller
			// than the number of words then delete old buffer and reserve
			// a memory block large enough to read buffer.
			if (sizeof(fBuffer)/sizeof(fBuffer[0]) < fBufferSize) {
				delete fBuffer;
				fBuffer = new word[fBufferSize];
			}
		}	
		else {
			fNumWords = fBufferHeader[0] - BUFFER_HEADER_SIZE;
			fBufferType = fBufferHeader[1];
			fChecksum = fBufferHeader[2];
			fRunNum = fBufferHeader[3];
			fBufferNumber = (fBufferHeader[4]) | (fBufferHeader[5] << fWordSize*8);
			fNumOfEvents = fBufferHeader[6];
			fNumOfLAMRegisters = fBufferHeader[7];
			fNumOfCPU = fBufferHeader[8];
			fNumOfBitRegisters = fBufferHeader[9];
		}


		//Grab the entire buffer.
		nRead = fread(fBuffer, fWordSize, fBufferSize, fFP);
		if (nRead != fBufferSize) {
			fprintf(stderr,"ERROR: Incorrect read size! Read %d words, expected %d words.\n",nRead,fNumWords);
			return 0;
		}
		//RingBuffer and EPICS has extra 2 byte word?
		if (IsRingBuffer() && fBufferType == BUFFER_TYPE_EPICS) fseek(fFP,2,SEEK_CUR);
	}

	return nRead;
}

void nsclBuffer::PrintBufferHeader() 
{
	printf("\nBuffer Header Summary:\n");
	printf("\tNum of words: %d\n",fNumWords);
	printf("\tBuffer type: %d\n",fBufferType);
	if (!IsRingBuffer()) {
		printf("\tChecksum: %d\n",fChecksum);
		printf("\tRun number: %d\n",fRunNum);
		printf("\tBuffer number: %d\n",fBufferNumber);
		printf("\tNumber of events: %d\n",fNumOfEvents);
		printf("\tNumber of LAM registers: %d\n",fNumOfLAMRegisters);
		printf("\tNumber of CPU: %d\n",fNumOfCPU);
		printf("\tNumber of bit registers: %d\n",fNumOfBitRegisters);
	}
}
nsclBuffer::word nsclBuffer::GetRunNumber() 
{
	return fRunNum;
}

/**Reutrns a two byte word. If the fWordSize is 2 GetWord() is reutrned.
 * If the fWordSize is greater than 2 a fraction of the word is returned
 * starting at the low bits. If fWordSize is 1 a GetLongWord() is returned.
 * Does not support odd fWordSize.
 *
 * \return A two byte word.
 */
UShort_t nsclBuffer::GetTwoByteWord()
{
	if (fWordSize % 2 == 1) {
		fprintf(stderr,"ERROR: Odd size words are not supported\n");
		return 0;
	}
	if (fWordSize == 2) {
		return GetWord();
	}
	else if (fWordSize > 2 && fReadWords < fNumWords) {
		UShort_t retVal = (fBuffer[fReadWords] >> 16*fFractionalWordPos) & 0xFFFF;
		//Increase the fractional word count.
		fFractionalWordPos = (fFractionalWordPos+1) % (fWordSize/2);
		///Check if we need a new word.
		if (fFractionalWordPos == 0) fReadWords++;
		return retVal;
	}
	return 0;	
}
/**Returns a four byte word. If the fWordSize is 4 GetWord() is returned.
 * If fWordSize is greater than 4 a fraction of the word is returned
 * starting at the low bits. If fWordSize is 2 GetLongWord() is returned.
 * Does not support odd fWordSize. For buffers with word size less than
 * four the low and high bits may be reversed if specified.
 *
 * \param[in] reverse Specifiy if low and high bits should be reveresed.
 *
 * \return A four byte word.
 */
UInt_t nsclBuffer::GetFourByteWord(bool reverse)
{
	if (fWordSize % 2 == 1) {
		fprintf(stderr,"ERROR: Odd size words are not supported\n");
		return 0;
	}
	if (fWordSize == 4) {
		return GetWord();
	}
	else if (fWordSize > 4 && fReadWords < fNumWords) {
		UShort_t retVal = (fBuffer[fReadWords] >> 32*fFractionalWordPos) & 0xFFFFFFFF;
		//Increase the fractional word count.
		fFractionalWordPos = (fFractionalWordPos+1) % (fWordSize/4);
		///Check if we need a new word.
		if (fFractionalWordPos == 0) fReadWords++;
		return retVal;
	}
	else if (fWordSize == 2) {
		return GetLongWord(reverse);
	}
	return 0;
}
nsclBuffer::word nsclBuffer::GetWord()
{
	if (fReadWords < fNumWords) {
		return fBuffer[fReadWords++];
	}
	return 0;
}
/**Get a long word, two words together. The high bits may be in the first
 * or second word. The defualt is to reutrn the high bits from the second
 * word and low bits from the first word. This can be reversed by setting
 * \c reverse = true
 *	
 *	\param[in] reverse Reverse order of words.
 *
 *	\return A long word composed of two words.
 */
nsclBuffer::longword nsclBuffer::GetLongWord(bool reverse)
{
	word words[2] = {0};
	for (int i=0;i<2;i++) 
		words[i] = GetWord();
	if (reverse)
		return (words[0]<<16) | (words[1]);
	else
		return (words[1]<<16) | (words[0]);

	return 0;
}
void nsclBuffer::Forward(int numOfWords)
{
	fReadWords += numOfWords;
}
void nsclBuffer::Rewind(int numOfWords)
{
	fReadWords -= numOfWords;
}
/**Does not support rewinding by fractions of words.
 *	\param[in] numOfBytes Number of bytes to rewind buffer.
 */
void nsclBuffer::RewindBytes(int numOfBytes)
{
	if (numOfBytes < fWordSize || numOfBytes % fWordSize != 0) {
		fflush(stdout);
		fprintf(stderr,"ERROR: Rewinding by fractions of word size (%d) not supported!\n",fWordSize);
		return;
	}
	
	int numOfWords = numOfBytes / fWordSize;

	fReadWords -= numOfWords;
}

unsigned int nsclBuffer::GetPosition()
{
	return fReadWords;
}

void nsclBuffer::DumpHeader()
{
	printf("\nBuffer Header:");
	for (int i=0;i<BUFFER_HEADER_SIZE;i++) {
		if (i % (20/fWordSize) == 0) 
			printf("\n\t");
		printf("%#0*X ",2*fWordSize+2,fBufferHeader[i]);
	}
	printf("\n");
}

void nsclBuffer::DumpBuffer()
{
	printf("\nBuffer Length %d:", fNumWords);
	for (int i=0;i<GetNumOfWords();i++) {
		if (i % (20/fWordSize) == 0) printf("\n\t");
		printf("%#0*X ",2*fWordSize+2,fBuffer[i]);
	}
	printf("\n");
}


