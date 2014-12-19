#include "nsclClassicBuffer.h"

nsclClassicBuffer::nsclClassicBuffer(const char *filename,int bufferSize, int bufferHeaderSize, int wordSize) :
	mainBuffer(bufferHeaderSize,bufferSize,wordSize)
{
	OpenFile(filename);
	//Four byte words have the low and high bits swapped.
	SetMiddleEndian(4);
}
nsclClassicBuffer::~nsclClassicBuffer() {
}

/**Reads the next word from the file providing the size of the next buffer.
 * A large enough block of memory is reserved and the next buffer is read
 * into it. The buffer header is then disseminated.
 */
int nsclClassicBuffer::ReadNextBuffer() 
{
	if (mainBuffer::ReadNextBuffer() == 0) return 0;
	
	SetNumOfWords(GetWord());
	fBufferType = GetWord();
	fChecksum = GetWord();
	fRunNum = GetWord();
	fBufferNumber = (GetWord()) | (GetWord() << fWordSize*8);
	fNumOfEvents = GetWord();
	fNumOfLAMRegisters = GetWord();
	fNumOfCPU = GetWord();
	fNumOfBitRegisters = GetWord();

	Seek(6);

	fEventNumber = 0;

	return GetNumOfWords();
}
void nsclClassicBuffer::Clear()
{
	mainBuffer::Clear();
	fNumOfLAMRegisters = 0;
	fNumOfCPU = 0;
	fNumOfBitRegisters = 0;
}
void nsclClassicBuffer::PrintBufferHeader() 
{
	printf("\nBuffer Header Summary:\n");
	printf("\t%#06llX Num of words: %llu\n",GetNumOfWords(),GetNumOfWords());
	printf("\t%#06llX Buffer type: %llu\n",fBufferType,fBufferType);
	printf("\t%#06llX Checksum: %llu\n",fChecksum,fChecksum);
	printf("\t%#06llX Run number: %llu\n",fRunNum,fRunNum);
	printf("\t%#010llX Buffer number: %llu\n",fBufferNumber,fBufferNumber);
	printf("\t%#06llX Number of events: %llu\n",fNumOfEvents,fNumOfEvents);
	printf("\t%#06X Number of LAM registers: %u\n",fNumOfLAMRegisters,fNumOfLAMRegisters);
	printf("\t%#06X Number of CPU: %u\n",fNumOfCPU,fNumOfCPU);
	printf("\t%#06X Number of bit registers: %u\n",fNumOfBitRegisters,fNumOfBitRegisters);
}

void nsclClassicBuffer::ReadRunBegin(bool verbose) 
{
	ReadRunBeginEnd(fElapsedRunTime,fRunStartTime,fRunTitle,verbose);
}
void nsclClassicBuffer::ReadRunEnd(bool verbose) 
{
	ReadRunBeginEnd(fElapsedRunTime,fRunEndTime,fRunTitle,verbose);
}

/**Begin run buffer format:
 * 1. Run title (79 bytes)
 * 2. Elapsed time (longword, 4 B)
 * 3. Month
 * 4. Day
 * 5. Year
 * 6. Hour
 * 7. Minute
 * 8. Second 
 * 9. Ticks (64's of a second)
 * Each of these items is represented by a single word except for the run 
 * title which contatins one character per byte. 
 *
 * \param[out] elapsedTime Number of seconds elapsed during the run.
 * \param[out] timeStamp Timestamp of the buffer.
 * \param[out] runTitle Run Title
 * \param[in] verbose Verbosity flag.  
 *
 */
void nsclClassicBuffer::ReadRunBeginEnd(UInt_t elapsedTime, time_t &timeStamp, std::string &runTitle,bool verbose)
{
	//Make sure we are reading the buffer we expect
	if (GetBufferType() != BUFFER_TYPE_RUNEND && 
			GetBufferType() != BUFFER_TYPE_RUNBEGIN) 
	{
		fflush(stdout);
		fprintf(stderr,"ERROR: Not a run begin/end buffer!\n");
		return;
	}

	//Maximum number of words in title is flexible.
	// We pass the number of words in the event minus the three word preceeding it.
	runTitle = ReadString(40,verbose);
		
	//Print the finished title
	if (verbose) printf("\tTitle: %s\n",runTitle.c_str());
	
	fElapsedRunTime = GetLongWord(false);
	if (verbose) printf("\t%#010X Run Time Elapsed: %d s\n",fElapsedRunTime, fElapsedRunTime);

	timeStamp = GetRunTime(verbose);

}

time_t nsclClassicBuffer::GetRunTime(bool verbose)
{
	struct tm tempTime;
	tempTime.tm_isdst = -1;
	tempTime.tm_mon = GetWord() - 1;
	tempTime.tm_mday = GetWord();
	tempTime.tm_year = GetWord() - 1900;
	tempTime.tm_hour = GetWord();
	tempTime.tm_min = GetWord();
	tempTime.tm_sec = GetWord();
	if (verbose) {
		printf("\t0x%04X Month:\t%d\n",tempTime.tm_mon+1,tempTime.tm_mon+1);
		printf("\t0x%04X Day:\t%d\n",tempTime.tm_mday,tempTime.tm_mday);
		printf("\t0x%04X Year:\t%d\n",tempTime.tm_year+1900,tempTime.tm_year+1900);
		printf("\t0x%04X Hour:\t%d\n",tempTime.tm_hour,tempTime.tm_hour);
		printf("\t0x%04X Min:\t%d\n",tempTime.tm_min,tempTime.tm_min);
		printf("\t0x%04X Sec:\t%d\n",tempTime.tm_sec,tempTime.tm_sec);
		printf("\tRun Time: %s\n",asctime(&tempTime));
	}

	return mktime(&tempTime);
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
int nsclClassicBuffer::ReadEvent(bool verbose) {

	unsigned int eventStartPos = GetBufferPosition();
	if (eventStartPos >= GetNumOfWords()) {
		fflush(stdout);
		fprintf(stderr,"WARNING: Attempted to read event after reaching end of buffer!\n");
		return 0;
	}

	UInt_t eventLength = GetEventLength();
	if (eventLength == 0) return 1;

	if (verbose) {
		printf ("\nData Event:\n");
		printf("\t%#06X Length: %d\n",(UInt_t)GetWord(),eventLength);
	}
	else Seek(1);

	//Loop over each module
	int headerSize = 2;
	for(unsigned int module=0;module<fModules.size();module++) {
		int packetLength = GetWord();
		int packetTag = GetWord();
		if (verbose) {
			printf("\t%#06X Packet length: %d\n",packetLength,packetLength);
			printf("\t%#06X Packet tag: %d\n",packetTag,packetTag);
		}
		if (packetLength <= 2) continue;

		//Read out the current module
		fModules[module]->ReadEvent(this,verbose);

		//Check how many words were read.
		if (GetBufferPosition() - eventStartPos > eventLength) {
			fflush(stdout);
			fprintf(stderr,"ERROR: Module read too many words! (Buffer: %d)\n",GetBufferNumber());
			exit(8);
		}

	}

	//Fastforward over extra words
	int remainingWords = eventStartPos + eventLength - GetBufferPosition();
	if (remainingWords > 0) {
		if (verbose) {
			for (int i=0;i<remainingWords;i++) 
				printf("\t%#0*llX Extra Word?\n",2*GetWordSize()+2,GetWord());
		}
		else {
			Seek(remainingWords);
		}
	}

	fEventNumber++;

	return 1;
}

/**Typical scaler buffer:
 * 1. End time low order.
 * 2. End time high order.
 * 3. Three unused words.
 * 4. Start time low order.
 * 5. Start time high order.
 * 6. Three unsued words.
 * 7. Incremental scalers.
 * The number of incremental scalers is set by the number of events in the
 * buffer.
 *
 * \param[in] verbose Verbosity flag.
 */
void nsclClassicBuffer::ReadScalers(bool verbose)
{
	if (GetBufferType() != BUFFER_TYPE_SCALERS) {
		fflush(stdout);
		fprintf(stderr,"ERROR: Not a scaler buffer!\n");
		return;
	}

	//Number of scalers to be read
	UInt_t scalerCount = GetNumOfEvents();
	if (verbose) {
		printf ("\nScalers: Channels: %d\n",GetNumOfEvents());
	}

	unsigned int time;
	time = GetLongWord(/*middleEndian=*/false);
	//Store End Time
	if (verbose) {
		printf("\t%#010X End time: %u\n",time,time);
		for (int i=0;i<3;i++) printf("\t%#06llX Unused word\n",GetWord());
	}
	else Seek(3);

	time = GetLongWord(/*middleEndian=*/false);
	//Store Start Time
	if (verbose) {
		printf("\t%#010X Start time: %u\n",time,time);
		for (int i=0;i<3;i++) printf("\t%#06llX Unused word\n",GetWord());
	}
	else Seek(3);

	for (UInt_t ch = 0; ch < scalerCount;ch++) {
		//unsigned int value = (GetWord() | ((unsigned int) GetWord() << 16));
		UInt_t value = GetLongWord(/*middleEndian=*/false);
		//Store Data
		if (verbose) printf("\t%#010X ch: %d value: %u\n",value,ch,value);
	}
}
