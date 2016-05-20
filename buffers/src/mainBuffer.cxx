#include "mainBuffer.h"

mainBuffer::mainBuffer(unsigned int headerSize, unsigned int bufferSize, unsigned int wordSize) :
	fFileSize(0),
	fStorageManager(nullptr),
	fWordSize(wordSize),
	fBufferBeginPos(0),
	fBufferNumber(0)
{
	this->Clear();
	SetBufferSize(bufferSize * wordSize);
	SetHeaderSize(headerSize * wordSize);

	fMiddleEndian.assign(8,false);

}
mainBuffer::~mainBuffer() {
	CloseFile();
}
void mainBuffer::SetHeaderSize(ULong64_t headerSizeBytes) {
	fHeaderSizeBytes = headerSizeBytes;
	fHeaderSize = headerSizeBytes / GetWordSize();
	if (fHeaderSizeBytes % GetWordSize() > 0) fHeaderSize++;
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

bool mainBuffer::OpenFile(const char *filename)
{
	fFile.open(filename, std::ios::in | std::ios::binary | std::ios::ate);
	fFileSize = fFile.tellg();
	fFile.clear();
	fFile.seekg(0);
	if (!fFile.good()) {
		fflush(stdout);
		fprintf(stderr,"ERROR: Unable to open: %s\n",filename);
		return false;
	}
	fFileName = filename;
	return true;
}
void mainBuffer::CloseFile()
{
	fFile.close();
	fFileSize = 0;
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
	for (ULong64_t i=0;i<eventLength;i++) { 
		if (i % (20/wordSize) == 0) printf("\n %4llu",i);
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
	unsigned int pos = GetBufferPositionBytes();
	//Rewind to beginning of buffer then Fwd over the header.
	SeekBytes(-pos + GetHeaderSizeBytes());

	unsigned int eventLength = GetNumOfWords() - GetHeaderSize();
	printf("\nScaler Dump Length: %d",eventLength);
	for (unsigned int i=0;i<eventLength;i++) { 
		if (i % (20/GetWordSize()) == 0) printf("\n %4u",i);
		printf(" %#0*llX",2*GetWordSize()+2,GetWord());
	}
	printf("\n");

	//Rewind to previous position
	Seek(-eventLength);
	SeekBytes(-GetBufferPositionBytes() + pos);
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
	unsigned int numBytes = fWordSize; //Number of bytes to request.
	unsigned int bytesRemaining = GetNumOfBytes() - GetBufferPositionBytes(); //Bytes left in buffer.
	//Only request the number of remaining bytes.
	if (fWordSize > bytesRemaining) {
		numBytes = bytesRemaining;
	}

	return GetWord(numBytes);
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

	//We do not have a container large enought to store it.
	if (numOfBytes > 8) {
		fflush(stdout);
		fprintf(stderr, "ERROR: Cannot retireve words larger than 8 bytes!\n");
		return 0xFFFFFFFFFFFFFFFF;
	}

	//The user tries to read more bytes than are available.
	if (GetBufferPositionBytes() >= fNumBytes) {
		fflush(stdout);
		fprintf(stderr,"\nERROR: No bytes left in buffer %llu (%u/%llu)!\n",fBufferNumber,GetBufferPositionBytes(),fNumBytes);
		return mask;
	}

	//Determine how many bytes are left in the buffer.
	unsigned int bytesRemaining = GetNumOfBytes() - GetBufferPositionBytes();
	//Requested more bytes than the buffer has, we set the request to what is left.
	if (numOfBytes > bytesRemaining) {
		fflush(stdout);
		fprintf(stderr,"WARNING: Requested more bytes (%d) than those remaining (%d)! Returning remaing bytes.\n",numOfBytes,bytesRemaining);
		numOfBytes = bytesRemaining;
	}

	//If the word is middle endian we need to reverse the order.
	if (middleEndian && numOfBytes % 2 == 0) {
		UInt_t wordSize = numOfBytes / 2;
		ULong64_t word1 = GetWord(wordSize);
		ULong64_t word2 = GetWord(wordSize);
		return word2 | (word1 << (8 * wordSize));
	}

	ULong64_t retVal = 0;

	for (unsigned int i=0;i<numOfBytes;i++) 
		retVal += (ULong64_t) (fBuffer.at(fCurrentByte++) & 0xFF) << 8*i;

	return retVal & mask;	
}
ULong64_t mainBuffer::GetCurrentWord()
{
	unsigned int byteOffset = GetBufferPositionBytes() % fWordSize; //The offset in bytes due to a fractional word read.
	//Seek backward over the offset to get the actual "word" as the user expects.
	SeekBytes(-byteOffset);
	//Get the word to return
	ULong64_t retVal = GetWord();
	//Seek back over the word read and the offset.
	SeekBytes(-fWordSize + byteOffset);

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
void mainBuffer::SeekFile(int numOfBytes) 
{
	fFile.seekg(numOfBytes, std::ios_base::cur);
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
	//If the header size is zero there is nothing to do.
	if (GetHeaderSize() == 0) return;

	unsigned int pos = GetBufferPositionBytes();
	SeekBytes(-pos);
	printf("\nBuffer Header:");
	for (unsigned int i=0;i<GetHeaderSize();i++) {
		if (i % (20/GetWordSize()) == 0) printf("\n %4u",i);
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
	for (unsigned int i=0;i<GetNumOfWords();i++) {
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
	for (unsigned int i=0;i<eventLength;i++) { 
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
	for (size_t charCount=0;charCount<sizeof(datum);charCount++) {
		//Keep storing characters until string ends
		ULong64_t bitShift = 8 * charCount;
		char letter = (datum >> bitShift) & 0xFF;
		//Break if end string character found.
		if (letter == 0) break;
		title.push_back(letter);
	}

	return title;
}


std::string mainBuffer::ReadString(unsigned int maxWords, bool verbose) {
	return ReadStringBytes(maxWords * GetWordSize(), verbose);
}

std::string mainBuffer::ReadStringBytes(unsigned int maxBytes, bool verbose) 
{
	std::string title = "";
	unsigned int readBytes = 0;

	//Loop over words until string is completed.
	for (readBytes=0;readBytes < maxBytes; readBytes++) {
		//Get the next word for conversion.
		char datum = GetWord(1);

		//If the word is null then the string must be complete.
		if (datum == 0) {
			//We increase readBytes as we read 0 then we break.
			readBytes++;
			break;
		}
/*
		//If this word is just spaces we count it and continue.
		// We will append the spaces later if we find any useful text.
		if (datum == 0x20) {
			spaceCount++;
			continue;
		}

		//Append spaces we previously had found
		for (int i=0;i<spaceCount;i++) title.append(" ");
		spaceCount = 0;
*/
		//Append the character.
		title += datum;

	}

	if (verbose) {
		//Try to print a nice number of entries per line
		int verboseWordLength = 3 * GetWordSize() + 4;
		//Number of words to print per line.
		int wordsPerLine = 75 / verboseWordLength;

		UShort_t byteOffset = ((fCurrentByte - readBytes) % fWordSize);
		//Loop over every byte in the output
		for (size_t i=0;i<title.length(); i+= fWordSize) {
			Short_t wordSize = fWordSize;
			if (i == 0) {
				printf("\t");
				//We try to align the hex output with the word borders
				wordSize = fWordSize - byteOffset;
			}
			else if (((i + byteOffset)/fWordSize) % wordsPerLine == 0) printf("\n\t");
			std::string part = title.substr(i,wordSize);
			UInt_t value = 0;
			for (size_t j=0;j<part.length();j++) 
				value += part.data()[j] << 8*j << 8*(fWordSize - wordSize);
			printf("%#0*X %*s ",2*fWordSize+2,value,fWordSize,part.c_str());

			if (wordSize!=fWordSize) i -= fWordSize - wordSize;
		}
	}

	//Seek over the remaining words.
	SeekBytes(maxBytes-readBytes);
	if (verbose && !title.empty()) printf("\n");

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

std::string mainBuffer::PeekLine() {
	unsigned long int filePos = GetFilePosition();	
	std::string line = GetLine();
	//We rewind if we are not at the end of the file.
	// Be sure to cast as these are unsigned.
	if (!fFile.eof())	SeekFile(-(long int)(GetFilePosition() - filePos));	
	return line;
}
std::string mainBuffer::GetLine() {
	std::string line;
	std::getline(fFile,line);
	return line;
}

/**
 * \param[in] manager Pointer to a RootStorageManager.
 */
void mainBuffer::SetStorageManager(RootStorageManager *manager) {
	fStorageManager = manager;
	InitializeStorageManager();
}

RootStorageManager* mainBuffer::GetStorageManager() {
	return fStorageManager;
}
