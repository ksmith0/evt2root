#include "nsclRingBuffer.h"


nsclRingBuffer::nsclRingBuffer(const char *filename,int bufferSize, int headerSize, int wordSize) :
	mainBuffer(headerSize,bufferSize,wordSize)
{
	OpenFile(filename);
	SetBufferSize(wordSize);
}
nsclRingBuffer::~nsclRingBuffer() {
}
/**Reads the next word from the file providing the size of the next buffer.
 * A large enough block of memory is reserved and the next buffer is read
 * into it. The buffer header is then disseminated.
 * Ring buffer headers contain the following:
 * 1. Number of bytes in buffer.
 * 2. Buffer type.
 */
int nsclRingBuffer::ReadNextBuffer() 
{
	Clear();
	if (!fFile.good()) {
		fflush(stdout);
		printf("ERROR: File not good.\n");
		return -1;
	}

	fBufferBeginPos = GetFilePosition();

	SetBufferSize(GetWordSize());
	//The "ring" buffer has buffers exactly the size of the event.
	//Read the first word which should be the size of the buffer
	fFile.read(&fBuffer[0], GetWordSize());
	if (fFile.gcount() != GetWordSize()) {
		if (fFile.gcount() !=0 )fprintf(stderr,"ERROR: Read %ld bytes expected %u!\n",fFile.gcount(),GetWordSize());
		return 0;
	}
	SetNumOfBytes(GetWordSize());
	SetBufferSize(GetWord());

	//Number of words is equal to the size of the buffer.
	SetNumOfBytes(fBufferSizeBytes);

	//Read the remaining buffer, besides the one word we have read.
	fFile.read(&fBuffer[GetWordSize()], GetBufferSizeBytes() - GetWordSize());
	if (fFile.gcount() != GetBufferSizeBytes() - GetWordSize()) {
		if (fFile.gcount() !=0 ) {
			fflush(stdout);
			fprintf(stderr,"ERROR: Read %ld bytes expected %u!\n",fFile.gcount(),GetBufferSize());
		}
		return 0;
	}

	fBufferType = GetWord();
	//Ring buffer event buffers only contain 1 event.
	if (fBufferType == BUFFER_TYPE_DATA) fNumOfEvents = 1;

	//Ring Buffer does not track buffer number, we iterate it manually.
	fBufferNumber++;

	return GetNumOfWords();
}
/**The event length is returned without changing the position in the
 * buffer.
 *
 * \return Event length in words.
 */
UInt_t nsclRingBuffer::GetEventLength() {
	UInt_t datum = GetCurrentWord();
	UInt_t eventLength = GetCurrentWord() / (GetWordSize()/2);

	return ValidatedEventLength(eventLength);
}

void nsclRingBuffer::PrintBufferHeader() 
{
	printf("\nBuffer Header Summary:\n");
	printf("\t%#010X Num of bytes: %u\n",(UInt_t)fBufferSizeBytes,(UInt_t)fBufferSizeBytes);
	printf("\t%10c Num of words: %llu\n",' ',GetNumOfWords());
	printf("\t%#010X Buffer type: %u\n",(UInt_t)fBufferType,(UInt_t)fBufferType);
	printf("\t%10c Buffer number: %u\n",' ',(UInt_t)fBufferNumber);
}

void nsclRingBuffer::ReadRunBegin(bool verbose) 
{
	UInt_t timeOffset, runNum;
	ReadRunBeginEnd(runNum,timeOffset,fRunStartTime,fRunTitle,verbose);
	fRunNum = runNum;
}
void nsclRingBuffer::ReadRunEnd(bool verbose) 
{
	UInt_t runNum;
	ReadRunBeginEnd(runNum,fElapsedRunTime,fRunEndTime,fRunTitle,verbose);
	fRunNum = runNum;
}
/**Begin run buffer format contains the four following items:
 * 1. Run Number
 * 2. Time Offset
 * 3. Time Stamp 
 * 4. Run title 
 * Each of these items is represented by a single word except for the run 
 * title which contatins one character per byte. 
 *
 * \param[out] runNum Run Number.
 * \param[out] elapsedTime Number of seconds elapsed during the run.
 * \param[out] timeStamp Timestamp of the buffer.
 * \param[out] runTitle Run Title
 * \param[in] verbose Verbosity flag.  
 *
 */
void nsclRingBuffer::ReadRunBeginEnd(UInt_t &runNum, UInt_t &elapsedTime, time_t &timeStamp, std::string &runTitle,bool verbose)
{
	//Make sure we are reading the buffer we expect
	if (GetBufferType() != BUFFER_TYPE_RUNEND && 
			GetBufferType() != BUFFER_TYPE_RUNBEGIN) 
	{
		fprintf(stderr,"ERROR: Not a run begin/end buffer!\n");
		return;
	}

	runNum = GetWord();
	elapsedTime = GetWord();
	timeStamp = GetWord();
	if (verbose) {
		printf("\n\t%#010X Run Number: %d\n",runNum,runNum);
		printf("\t%#010X Run Time Elapsed: %d s\n",elapsedTime,elapsedTime);
		printf("\t%#010X Run Buffer Timestamp: %s",UInt_t(timeStamp),ctime(&timeStamp));
	}

	//Maximum number of words in title is flexible.
	// We pass the number of words in the event minus the three word preceeding it.
	runTitle = ReadString(GetNumOfWords()-3,verbose);

	//Print the finished title
	if (verbose) printf("\tTitle: %s\n",runTitle.c_str());
	
}


/**Typical event buffer:
 * 1. Event word count. (See Below)
 * 2. Event packets ...
 *
 * The VM-USB word count is non-inclusive, while other systems it is 
 * inclusive. The Non-USB version has an event packet header with two
 * words incidacting the number of words for that module included as well 
 * as a tag for that packet. These words appear for every event, even if
 * the module is completely zero-suppressed. VM-USB systems have a
 * longword with all bits set following each module. This appears for
 * every event, even if the module is completely zero-suppressed.
 *
 * The actual unpacking is handled by the module classes specified in
 * evt_config.h. The specific unpacking is implementation specific.
 *
 * \param verbose Verbosity flag. 
 *
 */
int nsclRingBuffer::ReadEvent(bool verbose) {
	unsigned int eventStartPos = GetBufferPositionBytes();
	if (eventStartPos >= GetNumOfBytes()) {
		fflush(stdout);
		fprintf(stderr,"WARNING: Attempted to read event after reaching end of buffer!\n");
		return 0;
	}

	UInt_t eventLength = GetEventLength();
	if (eventLength == 0) return 1;

	if (verbose) {
		printf ("\nData Event:\n");
		printf("\t%#010X Length: %d\n",(UInt_t)GetWord(),eventLength);
	}
	else Seek(1);

	eventLength *= GetWordSize();

	//Loop over each module
	int headerSize = 2;
	for(unsigned int module=0;module<fModules.size();module++) {

		//Read out the current module
		fModules[module]->ReadEvent(this,verbose);

		//Check how many words were read.
		if (GetBufferPositionBytes() - eventStartPos > eventLength) {
			fflush(stdout);
			fprintf(stderr,"ERROR: Module read too many words! (Buffer: %d)\n",GetBufferNumber());
		}

	}

	//Fastforward over extra words
	int remainingBytes = eventStartPos + eventLength - GetBufferPositionBytes();
	if (remainingBytes > 0) {
		if (verbose) {
			for (int i=0;i<remainingBytes;i+=GetWordSize()) 
				if (remainingBytes >= GetWordSize()) 
					printf("\t%#0*X Extra Word?\n",2*GetWordSize()+2,(UInt_t)GetWord());
				else {
					printf("\t%#0*X Extra Bytes?\n",2*remainingBytes+2,(UInt_t)GetWord(remainingBytes));
				}
		}
		else {
			SeekBytes(remainingBytes);
		}
	}

	fEventNumber++;

	return GetBufferPositionBytes() - eventStartPos;

}

/**Typical scaler buffer:
 * 1. Start time of scalers. (Number of seconds after run start when scaler
 * 	recording started.)
 * 2. End time of scalers. (Number of seconds after run start when scaler
 * 	recording endeded.)
 * 3. Timestamp when scalers were recorded.
 * 4. Number of scaler channels read.
 * 5. Incremental scalers.
 */
void nsclRingBuffer::ReadScalers(bool verbose)
{
	if (GetBufferType() != BUFFER_TYPE_SCALERS) {
		fprintf(stderr,"ERROR: Not a scaler buffer!\n");
		return;
	}

	//Number of scalers to be read
	UInt_t scalerCount = 0;

	//Number of seconds elapsed when the scaler interval was started.
	UInt_t startTimeOffset = GetWord();	
	//Number of seconds elapsed when the scaler interval was ended.
	UInt_t endTimeOffset = GetWord();
	//Time stamp when scalers were read.
	time_t timeStamp = GetWord();
	scalerCount = GetWord();

	//Set scaler times.
	//scaler->SetStartTime(fRunStartTime + startTimeOffset);
	//scaler->SetEndTime(fRunStartTime + endTimeOffset);

	if (verbose) {
		printf ("\nScalers:\n");
		printf("\t%#010X Start Time Offset: %d\n",startTimeOffset,startTimeOffset);
		printf("\t%#010X End Time Offset: %d\n",endTimeOffset,endTimeOffset);
		printf("\t%#010X Time Stamp: %s\n",UInt_t(timeStamp),ctime(&timeStamp));
		printf("\t%#010X Channels: %d\n",scalerCount,scalerCount);
	}

	for (UInt_t ch = 0; ch < scalerCount;ch++) {
		//unsigned int value = (GetWord() | ((unsigned int) GetWord() << 16));
		UInt_t value = GetWord(4);
		//scaler->SetValue(ch,value);
		if (verbose) printf("\t0x%08X ch: %d value: %u\n",value,ch,value);
	}
}
