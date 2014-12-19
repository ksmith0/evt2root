#include "mainBuffer.h"
#include "modules.h"

mainBuffer::mainBuffer(unsigned int headerSize, unsigned int bufferSize, unsigned int wordSize) :
	fFileSize(0),
	fHeaderSize(headerSize),
	fWordSize(wordSize),
	fBufferBeginPos(0),
	fBufferNumber(-1)
{
	this->Clear();
	SetBufferSize(bufferSize * wordSize);

	fMiddleEndian.assign(8,false);
	MODULE_LIST(MODULE_FUNCTION)

}
mainBuffer::~mainBuffer() {
	CloseFile();
}

void mainBuffer::SetBufferSize(ULong64_t bufferSizeBytes) {
	fBufferSizeBytes = bufferSizeBytes;
	fBufferSize = bufferSizeBytes / GetWordSize();
	if (fBufferSizeBytes % GetWordSize() > 0) fBufferSize++;
	fBuffer.resize(fBufferSizeBytes);
}
/**Copies data from an provided address with a specified length into the 
 * buffer.
 */
/*
void mainBuffer::Copy(UInt_t *source, unsigned int length)
{
	if (fBufferSize < fWritePosition + length) {
		fprintf(stderr,"ERROR: Tried to write more words (%d) into buffer than the buffer size (%d)!\n",fWritePosition + length,fBufferSize);
		return;
	}
	memcpy(&fBuffer[fWritePosition],source,length * sizeof(fBuffer[0]));
	fWritePosition += length;

}
*/

void mainBuffer::OpenFile(const char *filename)
{
	try {
		fFile.open(filename, std::ios::in | std::ios::binary | std::ios::ate);
		fFileSize = fFile.tellg();
		fFile.clear();
		fFile.seekg(0);
		if (!fFile.good()) throw filename;
		fFileName = filename;
	}
	catch (char *e)
	{
		fprintf(stderr,"ERROR: Can't open evtfile %s\n",filename);
	}
	return;
}
void mainBuffer::CloseFile()
{
	fFile.close();
}
void mainBuffer::Clear()
{
	fCurrentByte = 0;	
	fWritePosition = 0;
	fFractionalWordPos = 0;
	fBufferType = 0;
	fNumWords = 0;
	fChecksum = 0;
	fRunNum = 0;
	fNumOfEvents = 0;
	fEventNumber = 0;
}
unsigned int mainBuffer::GetBufferNumber()
{
	return fBufferNumber;
}

void mainBuffer::DumpEvent() {
	if (GetBufferPosition() >= GetNumOfWords()) {
		fflush(stdout);
		fprintf(stderr,"WARNING: Attempted to dump event after reaching end of buffer!\n");
		return;
	}

	ULong64_t eventLength = GetEventLength();
	printf("\nEvent %llu, %llu words",fEventNumber,eventLength);
	if (eventLength == 0) return;
	
	unsigned short wordSize = GetWordSize();
	for (int i=0;i<eventLength;i++) { 
		if (i % (20/wordSize) == 0) printf("\n %4d",i);
		printf(" %#0*llX",2*wordSize+2,GetWord());
	}
	printf("\n");
	Seek(-eventLength);
}

void mainBuffer::SetNumOfBytes(ULong64_t numOfBytes) {
	if (numOfBytes > fBufferSizeBytes) {
		fflush(stdout);
		fprintf(stderr,"ERROR: Number of words specified greater than buffer size!. Setting number of words to buffer size.\n");
		numOfBytes = fBufferSizeBytes;
	}
	fNumBytes = numOfBytes;
	fNumWords = numOfBytes / GetWordSize();
	if (fNumBytes % GetWordSize() > 0) fNumWords++;
}
void mainBuffer::DumpScalers() {
	unsigned int pos = GetBufferPosition();
	//Rewind to beginning of buffer then Fwd over the header.
	Seek(-pos + fHeaderSize);

	unsigned int eventLength = GetNumOfWords() - GetHeaderSize();
	printf("\nScaler Dump Length: %d",eventLength);
	for (int i=0;i<eventLength;i++) { 
		if (i % (20/GetWordSize()) == 0) printf("\n %4d",i);
		printf(" %#0*llX",2*GetWordSize()+2,GetWord());
	}
	printf("\n");

	//Rewind to previous position
	Seek(-eventLength);
	Seek(-GetBufferPosition() + pos);
}
/**Get event length from the current word. 
 *
 * \return The length of the event in words inclusive of the event length word.
 */
UInt_t mainBuffer::GetEventLength() 
{
	return ValidatedEventLength(GetCurrentWord());
}

/**A bad buffer may have a large event length that exceeds the number of
 * words in a buffer. In these cases the buffer is fastforwarded and the
 * returned event length is zero.
 *
 * \param[in] eventLength Event length to be validated.
 * \return The validadated event length.
 */
UInt_t mainBuffer::ValidatedEventLength(UInt_t eventLength) {
	if (eventLength > GetNumOfWords()) {
		fflush(stdout);
		fprintf(stderr,"ERROR: Event length (%u) larger than number of words in buffer (%llu)! Skipping Buffer %d!\n",eventLength,GetNumOfWords(),GetBufferNumber());
		Seek(GetNumOfWords() - GetBufferPosition());
		return 0;
	}
	return eventLength;
}

ULong64_t mainBuffer::GetWord()
{
	return GetWord(fWordSize);
}
ULong64_t mainBuffer::GetWord(unsigned int numOfBytes) {
	return GetWord(numOfBytes,IsMiddleEndian(numOfBytes));
}
/**Get the specified number of bytes from the buffer.
 * If the requested number of bytes is a long word 
 * (two times the word size) and the buffer is big endian
 * then the returned long word has the two small word order
 * swapped.
 *
 * The returned word has all bytes higher than requested zero padded.
 *
 * \param[in] numOfBytes The length of the word in bytes.
 * \param[in] middleEndian Specify wheter the word order should be swapped.
 * \return A word of the requested size.
 */
ULong64_t mainBuffer::GetWord(unsigned int numOfBytes, bool middleEndian) {
	//Create a mask so we return values only as large as requested
	ULong64_t mask = 0xFFFFFFFFFFFFFFFF >> 8*(8-numOfBytes);

	if (numOfBytes > 8) {
		fflush(stdout);
		fprintf(stderr, "ERROR: Cannot retireve words larger than 8 bytes!\n");
		return mask;
	}
	if (GetBufferPositionBytes() >= fBufferSizeBytes) {
		fflush(stdout);
		fprintf(stderr,"ERROR: No bytes left in buffer %llu (%u/%u)!\n",fBufferNumber,GetBufferPositionBytes(),fBufferSizeBytes);
		return mask;
	}

	unsigned int bytesRemaining = GetNumOfBytes() - GetBufferPositionBytes();
	if (numOfBytes > bytesRemaining) {
		numOfBytes = bytesRemaining;
		fflush(stdout);
	}

	if (middleEndian && numOfBytes % 2 == 0) {
		UInt_t wordSize = numOfBytes / 2;
		ULong64_t word1 = GetWord(wordSize);
		ULong64_t word2 = GetWord(wordSize);
		return word2 | (word1 << (8 * wordSize));
	}

	ULong64_t retVal = 0;

	for (int i=0;i<numOfBytes;i++)
		retVal += (fBuffer.at(fCurrentByte++) & 0xFF) << 8*i;

	return retVal & mask;	
}
ULong64_t mainBuffer::GetCurrentWord()
{
	UInt_t retVal = GetWord();
	Seek(-1);
	return retVal;
}
ULong64_t mainBuffer::GetCurrentLongWord()
{
	ULong64_t retVal = GetLongWord();
	Seek(-2);

	return retVal;
}
ULong64_t mainBuffer::GetLongWord()
{
	return GetWord(2 * fWordSize);
}
/**Get a long word, two words together. The high bits may be in the first
 * or second word. The defualt is to reutrn the high bits from the second
 * word and low bits from the first word. 
 *
 *	\return A long word composed of two words.
 */
ULong64_t mainBuffer::GetLongWord(bool middleEndian)
{
	return GetWord(2 * fWordSize, middleEndian);
}
void mainBuffer::Seek(int numOfWords)
{
	SeekBytes(numOfWords * GetWordSize());
}
void mainBuffer::SeekBytes(int numOfBytes)
{
	if (fCurrentByte + numOfBytes < 0) 
		fCurrentByte = 0;
	else
		fCurrentByte += numOfBytes;
}


unsigned long int mainBuffer::GetFilePosition()
{
	if (fFile.good()) return fFile.tellg();
	else if (fFile.eof()) return fFileSize;
	return 0;
}
float mainBuffer::GetFilePositionPercentage() {
	float percent = ((long double)GetFilePosition()) / ((long double)GetFileSize()) * 100;
	return percent;
}


void mainBuffer::DumpHeader()
{
	unsigned int pos = GetBufferPositionBytes();
	SeekBytes(-pos);
	printf("\nBuffer Header:");
	for (int i=0;i<GetHeaderSize();i++) {
		if (i % (20/GetWordSize()) == 0) printf("\n %4d",i);
		UInt_t datum = GetWord();
		printf(" %#0*X",2*fWordSize+2,datum);
	}
	SeekBytes(-GetBufferPositionBytes() + pos);
	printf("\n");
}
void mainBuffer::DumpBuffer()
{
	unsigned int pos = GetBufferPositionBytes();
	SeekBytes(-pos);
	printf("\nBuffer Length %llu:", GetNumOfWords());
	for (int i=0;i<fBufferSize;i++) {
		if (i % (20/GetWordSize()) == 0) printf("\n %4d",i);
		UInt_t datum = GetWord();
		printf(" %#0*X",2*fWordSize+2,datum);
	}
	SeekBytes(-GetBufferPositionBytes() + pos);
	printf("\n");
}

void mainBuffer::DumpRunBuffer()
{	
	unsigned int pos = GetBufferPositionBytes();
	//Rewind to beginning of buffer then Fwd over the header.
	SeekBytes(-pos + GetHeaderSize()*GetWordSize());

	unsigned int eventLength = GetNumOfWords() - GetHeaderSize();
	printf("\nRun Buffer Dump Length: %u",eventLength);
	for (int i=0;i<eventLength;i++) { 
		if (i % (20/GetWordSize()) == 0) printf("\n %4d",i);
		printf(" %#0*llX",2*GetWordSize()+2,GetWord());
	}
	printf("\n");

	//Rewind to previous position
	SeekBytes(-GetBufferPositionBytes() + pos);
}

std::string mainBuffer::ConvertToString(ULong64_t datum) {
	std::string title;

	//Loop over the number of characters stored in a word
	for (int charCount=0;charCount<sizeof(datum);charCount++) {
		//Keep storing characters until string ends
		ULong64_t bitShift = 8 * charCount;
		char letter = (datum >> bitShift) & 0xFF;
		//Break if end string character found.
		if (letter == 0) break;
		title.push_back(letter);
	}

	return title;
}


std::string mainBuffer::ReadString(unsigned int maxWords, bool verbose) 
{
	std::string title;
	int readWords = 0;
	int printedWords = 0;
	//Number of spaces since last good word.
	int spaceCount = 0;

	bool stringComplete = false; //Check if string is done.
	//Number of words to print per line.
	int wordsPerLine;

	//Some buffers contain long list of spaces after string
	//if a verbose output we do  not want to print these.
	ULong64_t wordOfSpacesHex = 0;
	std::string wordOfSpaces;
	for (int i=0;i<GetWordSize();i++) wordOfSpaces.push_back(' ');

	if (verbose) {
		//Try to print a nice number of entries per line
		int verboseWordLength = 3 * GetWordSize() + 4;
		wordsPerLine = 75 / verboseWordLength;
	
		//Get the hex for a word of spaces for verbose output.
		for (int i=0;i<GetWordSize();i++) wordOfSpacesHex += 0X20 << 8*i;
	}

	//Loop over words until string is completed.
	for (int i=0;i<maxWords && !stringComplete;i++) {
		//Get the next word for conversion.
		UInt_t datum = GetWord();
		readWords++;

		//If the word is null then the string must be complete.
		if (datum == 0) {
			stringComplete = true;
			break;
		}
		
		//Convert the word to a string.
		std::string part = ConvertToString(datum);

		//If this word is just spaces we count it and continue.
		// We will append the spaces later if we find any useful text.
		if (part == wordOfSpaces) {
			spaceCount++;
			continue;
		}

		if (verbose) {
			//Print any spaces and corresponding hex words that may have
			// been skipped.
			for (int numSpaces=0; numSpaces<spaceCount;numSpaces++) {
				printedWords++;
				if ((printedWords-1) % wordsPerLine == 0) printf("\n\t");
				printf("%#0*llX ",2*GetWordSize()+2,wordOfSpacesHex);
				printf("%s ",wordOfSpaces.data());
			}
			printedWords++;
			if ((printedWords-1) % wordsPerLine == 0) printf("\n\t");
			//Print the current word and its converted string.
			printf("%#0*X ",2*GetWordSize()+2,datum);
			printf("%s ",part.data());
		}

		//If the length of the converted string is smaller than the word
		// then we must have read a string termination character. We now
		// pad the verbose output and marked the string as ccomplete.
		if (part.length() < GetWordSize()) {
			if (verbose) printf("%*c",GetWordSize() - (int)part.length(), ' ');
			stringComplete = true;
		}

		//We append all the spaces we previously skipped.
		for (int numSpaces=0; numSpaces<spaceCount;numSpaces++) 
			title.append(wordOfSpaces);
		spaceCount = 0;

		//Finally we append the part of the string we found.
		title.append(part);
	}

	//Seek over the remaining words.
	Seek(maxWords-readWords);
	if (verbose) printf("\n");

	return title;

}


void mainBuffer::SetMiddleEndian(unsigned int wordSize, bool middleEndian) {
	fMiddleEndian[wordSize] = middleEndian;
}

int mainBuffer::ReadNextBuffer() {
	Clear();
	if (!fFile.good()) {
		fflush(stdout);
		printf("ERROR: File not good.\n");
		return -1;
	}

	fBufferBeginPos = GetFilePosition();

	fFile.read(&fBuffer[0], GetBufferSizeBytes());
	if (fFile.gcount() != GetBufferSizeBytes()) {
		if (fFile.gcount() !=0) {
			fflush(stdout);
			fprintf(stderr,"ERROR: Read %ld bytes expected %u!\n",fFile.gcount(),GetBufferSize());
		}
		return 0;
	}

	SetNumOfWords(GetBufferSize());

	return fFile.gcount();
}

