#include "nsclRingBuffer.h"
#include "baseModule.h"

nsclRingBuffer::nsclRingBuffer(int bufferSize, int headerSize, 
	int wordSize) :
	moduleBuffer(bufferSize,headerSize,wordSize),
	fVersion(0)
{
	//Ring buffer event buffers only contain 1 event.
	fNumOfEvents = 1;
}

nsclRingBuffer::nsclRingBuffer(const char *filename,int bufferSize, 
	int headerSize, int wordSize) :
	moduleBuffer(bufferSize,headerSize,wordSize),
	fVersion(0)
{
	OpenFile(filename);

	//Ring buffer event buffers only contain 1 event.
	fNumOfEvents = 1;
}
nsclRingBuffer::~nsclRingBuffer() {
}
/**Reads the next word from the file providing the size of the next buffer.
 * A large enough block of memory is reserved and the next buffer is read
 * into it. The buffer header is then disseminated.
 * Ring buffer headers contain the following:
 * 1. Number of bytes in buffer.
 * 2. Buffer type.
 *
 * In case of the specific BUFFER_TYPE_FORMAT we read the format of the
 * buffer then rewind over the words read such that the user may do
 * something with the buffer. 
 *
 * \note The version buffer here may indicate
 *  that a paradigm shift is needed in the code organization.
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

	//Specify that the buffer is big enough for a word.
	SetBufferSize(GetWordSize());
	//The "ring" buffer has buffers exactly the size of the event.
	//Read the first word which should be the size of the buffer
	fFile.read(&fBuffer[0], GetWordSize());
	//Check that we got the correct number of bytes.
	if (fFile.gcount() != GetWordSize()) {
		if (fFile.gcount() !=0 )fprintf(stderr,"ERROR: Read %ld bytes expected %u!\n",fFile.gcount(),GetWordSize());
		return 0;
	}
	//Specify the size of the readable buffer.
	SetNumOfBytes(GetWordSize());

	//Prepare the buffer for the current event.
	SetBufferSize(GetWord());

	//Read the remaining buffer, besides the one word we have read placing
	// them into the buffer after the first word.
	fFile.read(&fBuffer[GetWordSize()], GetBufferSizeBytes() - GetWordSize());
	//Check that we read the correct numebr of bytes.
	if (fFile.gcount() != GetBufferSizeBytes() - GetWordSize()) {
		//We only complain if we read an unexpected number of bytes.
		if (fFile.gcount() !=0 ) {
			fflush(stdout);
			fprintf(stderr,"ERROR: Read %ld bytes expected %u!\n",fFile.gcount(),GetBufferSize());
		}
		return 0;
	}

	//Number of bytes is equal to the size of the buffer.
	SetNumOfBytes(GetBufferSizeBytes());
	//Get the buffer type.
	fBufferType = GetWord();

	//Ring Buffer does not track buffer number, we iterate it manually.
	fBufferNumber++;

	//Seek over the extra header words.
	//Assume that versions prior to 11 do not use the BUFFER_TYPE_FORMAT
	if (fVersion >= 11 || fBufferType == BUFFER_TYPE_FORMAT) 
		ReadBodyHeader();

	return GetNumOfWords();
}
/**\bug Header size is hardcoded here.
 */
void nsclRingBuffer::Clear() {
	mainBuffer::Clear();
	SetHeaderSize(2 * GetWordSize());
	fBodyHeader.fLength = 0;
	fBodyHeader.fTimeStamp = 0;
	fBodyHeader.fSourceID = 0;
	fBodyHeader.fBarrier = 0;	
}
/**The body header was first specified in ring buffer version 11.
 * The body header contains:
 * 1. Body header length (inclusive).
 * 2. Timestamp (8 Bytes).
 * 3. Source ID
 * 4. Barrier
 *
 * If there is no body header a null word is provided. Any specified extra 
 * bytes are seeked over.
 */
void nsclRingBuffer::ReadBodyHeader() {
	fBodyHeader.fLength = GetWord();
	if (fBodyHeader.fLength == 0) 
		//Increase the header size to include the null word.
		SetHeaderSize(GetHeaderSizeBytes() + GetWordSize());
	else {
		//Increase the header size to include the body header.
		SetHeaderSize(GetHeaderSizeBytes() + fBodyHeader.fLength);
		fBodyHeader.fTimeStamp = GetLongWord();
		fBodyHeader.fSourceID = GetWord();
		fBodyHeader.fBarrier = GetWord();

		//If the body header is larger then we skip over it.
		if (fBodyHeader.fLength > 5*GetWordSize()) 
			SeekBytes(fBodyHeader.fLength - 5 * GetWordSize());
	}
}
/**Unpacks the current buffer based on the type. Data buffers are ignored 
 * and left to the user to unpack for now.
 *
 * \bug EPICS buffers are ignored.
 *
 * \param[in] verbose Verbosity flag.
 */
void nsclRingBuffer::UnpackBuffer(bool verbose) {
	switch(fBufferType) {
		case BUFFER_TYPE_DATA:
		case BUFFER_TYPE_DATA_COUNT: 
			return;
		case BUFFER_TYPE_SCALERS: 
			ReadScalers(verbose);
			break;
		case BUFFER_TYPE_EPICS:
			return;
		case BUFFER_TYPE_RUNBEGIN: 
			ReadRunBegin(verbose);
			break;
		case BUFFER_TYPE_RUNEND: 
			ReadRunEnd(verbose);
			break;
		case BUFFER_TYPE_FORMAT: 
			ReadVersion(verbose);
			break;
		default: 
			fflush(stdout);
			fprintf(stderr,"WARNING: Unknown buffer type: %llu.\n",fBufferType);
			return;

	}
}

UInt_t nsclRingBuffer::ReadVersion(bool verbose) {
	if (fBufferType != BUFFER_TYPE_FORMAT) {
		fprintf(stderr,"ERROR: Not a format (version) buffer!\n");
		return 0;
	}	

	fVersion = GetWord();

	if(verbose) {
		printf("\t%#010X Version %d\n", fVersion, fVersion);
	}

	if (fVersion < 10 || fVersion > 11) {
		fprintf(stderr,"WARNING: Unknown version %d.\n",fVersion);
	}

	return 1;
}

/**The event length is returned without changing the position in the
 * buffer.
 *
 * \return Event length in words.
 */
UInt_t nsclRingBuffer::GetEventLength() {
	UInt_t eventLength = GetCurrentWord() / (GetWordSize()/2);

	return ValidatedEventLength(eventLength);
}

void nsclRingBuffer::PrintBufferHeader() 
{
	printf("\nBuffer Header Summary:\n");
	printf("\t%#010X Num of bytes: %u\n",(UInt_t)GetBufferSizeBytes(),(UInt_t)GetBufferSizeBytes());
	printf("\t%10c Num of words: %llu\n",' ',GetNumOfWords());
	printf("\t%10c Header size: %u words (%u Bytes)\n",' ',GetHeaderSize(),GetHeaderSizeBytes());

	printf("\t%#010X Buffer type: %u ",(UInt_t)fBufferType,(UInt_t)fBufferType);
	switch (fBufferType) {
		case BUFFER_TYPE_RUNBEGIN: printf("(Run Begin)");break;
		case BUFFER_TYPE_RUNEND: printf("(Run End)");break;
		case BUFFER_TYPE_RUNPAUSE: printf("(Run Pause)");break;
		case BUFFER_TYPE_RUNRESUME: printf("(Run Resume)");break;
		case BUFFER_PACKET_TYPES: printf("(Packet Types");break;
		case BUFFER_TYPE_EPICS: printf("(EPICS)");break;
		case BUFFER_TYPE_FORMAT: printf("(Format)");break;
		case BUFFER_TYPE_SCALERS: printf("(Incremental Scalers)");break;
		case BUFFER_TYPE_NONINCR_SCALERS: printf("(Nonincremental Scalers)");break;
		case BUFFER_TYPE_DATA: printf("(Physics Data)");break;
		case BUFFER_TYPE_DATA_COUNT: printf("(Data Count)");break;
		case BUFFER_TYPE_EVB_FRAGMENT: printf("(EVB Fragment)");break;
		case BUFFER_TYPE_EVB_UNKNOWN_PAYLOAD: printf("(EVB Unknown Payload)");break;
		case BUFFER_TYPE_EVB_GLOM_INFO: printf("(EVB GLOM Info)");break;
		default: printf("(Unknown)");break;
	}
	printf("\n");

	printf("\t%10c Buffer number: %u\n",' ',(UInt_t)fBufferNumber);
	printf("\t%#010X Body Header Length: %u\n",fBodyHeader.fLength,fBodyHeader.fLength);
	if (fBodyHeader.fLength > 0) {
		printf("\t%#018llX Timestamp: %llu\n",fBodyHeader.fTimeStamp,fBodyHeader.fTimeStamp);
		printf("\t%#010X Source ID: %u\n",fBodyHeader.fSourceID,fBodyHeader.fSourceID);
		printf("\t%#010X Barrier: %u\n",fBodyHeader.fBarrier,fBodyHeader.fBarrier);
	}

}

void nsclRingBuffer::ReadRunBegin(bool verbose) 
{
	UInt_t timeOffset, runNum = -1;
	ReadRunBeginEnd(runNum,timeOffset,fRunStartTime,fRunTitle,verbose);
	fRunNum = runNum;
}
void nsclRingBuffer::ReadRunEnd(bool verbose) 
{
	UInt_t runNum = -1;
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

	//The number of words read in the run buffer.
	UInt_t wordsRead = 3;

	//Version 11 has an extra word in the run buffer.
	if (fVersion >= 11) {
		Seek(1);
		wordsRead++;
	}

	//Maximum number of words in title is flexible.
	// We pass the number of words in the event minus the three word preceeding it.
	runTitle = ReadString(GetNumOfWords()-GetHeaderSize()-wordsRead,verbose);

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
	for(unsigned int module=0;module<fModules.size();module++) {

		//Read out the current module
		fModules[module]->ReadEvent(this,verbose);

		//Check how many words were read.
		if (GetBufferPositionBytes() - eventStartPos > eventLength) {
			fflush(stdout);
			fprintf(stderr,"ERROR: Module read too many words! (Buffer: %d)\n",GetBufferNumber());
		}

	}

	FillStorage();

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

	if (verbose) DumpScalers();

	//Number of scalers to be read
	UInt_t scalerCount = 0;

	//Number of seconds elapsed when the scaler interval was started.
	UInt_t startTimeOffset = GetWord();	
	//Number of seconds elapsed when the scaler interval was ended.
	UInt_t endTimeOffset = GetWord();
	//Time stamp when scalers were read.
	time_t timeStamp = GetWord();
	
	UInt_t intervalDivisor, isIncremental;
	if (fVersion >= 11) intervalDivisor = GetWord();
	scalerCount = GetWord();
	if (fVersion >= 11) isIncremental = GetWord();

	//Set scaler times.
	//scaler->SetStartTime(fRunStartTime + startTimeOffset);
	//scaler->SetEndTime(fRunStartTime + endTimeOffset);

	if (verbose) {
		printf ("\nScalers:\n");
		printf("\t%#010X Start Time Offset: %d\n",startTimeOffset,startTimeOffset);
		printf("\t%#010X End Time Offset: %d\n",endTimeOffset,endTimeOffset);
		printf("\t%#010X Time Stamp: %s\n",UInt_t(timeStamp),ctime(&timeStamp));
		if (fVersion >= 11) 
			printf("\t%#010X Interval Divisor: %u\n",intervalDivisor,intervalDivisor);
		printf("\t%#010X Channels: %d\n",scalerCount,scalerCount);
		if (fVersion >= 11) 
			printf("\t%#010X Is incremental: %u\n",isIncremental,isIncremental);
	}

	for (UInt_t ch = 0; ch < scalerCount;ch++) {
		//unsigned int value = (GetWord() | ((unsigned int) GetWord() << 16));
		UInt_t value = GetWord(4);
		//scaler->SetValue(ch,value);
		if (verbose) printf("\t0x%08X ch: %d value: %u\n",value,ch,value);
	}
}

bool nsclRingBuffer::IsDataType() {
	if (fBufferType == BUFFER_TYPE_DATA) return true;
	return false;
}
bool nsclRingBuffer::IsScalerType() {
	if (fBufferType == BUFFER_TYPE_SCALERS) return true;
	return false;
}
bool nsclRingBuffer::IsRunBegin() {
	if (fBufferType == BUFFER_TYPE_RUNBEGIN) return true;
	return false;
}
bool nsclRingBuffer::IsRunEnd() {
	if (fBufferType == BUFFER_TYPE_RUNEND) return true;
	return false;
}
