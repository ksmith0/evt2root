#include "nsclUSBBuffer.h"
#include "baseModule.h"

nsclUSBBuffer::nsclUSBBuffer(int bufferSize, int bufferHeaderSize,
	 int wordSize) :
	nsclClassicBuffer(bufferSize,bufferHeaderSize,wordSize)
{ 

	//Four byte words have the low and high bits in the correct order.
	SetMiddleEndian(4,false);
}

nsclUSBBuffer::nsclUSBBuffer(const char *filename, int bufferSize,
	int bufferHeaderSize, int wordSize) :
	nsclClassicBuffer
		(filename,bufferSize,bufferHeaderSize,wordSize)
{ 

	//Four byte words have the low and high bits in the correct order.
	SetMiddleEndian(4,false);
}

nsclUSBBuffer::~nsclUSBBuffer() 
{ }

/**Reads the next word from the file providing the size of the next buffer.
 * A large enough block of memory is reserved and the next buffer is read
 * into it. The buffer header is then disseminated.
 */
int nsclUSBBuffer::ReadNextBuffer() 
{
	if (mainBuffer::ReadNextBuffer() == 0) return 0;

	SetNumOfWords(GetWord());
	fBufferType = GetWord();
	fChecksum = GetWord();
	fRunNum = GetWord();
	fBufferNumber = GetLongWord();
	fNumOfEvents = GetWord();
	fNumOfLAMRegisters = GetWord();
	fNumOfCPU = GetWord();
	fNumOfBitRegisters = GetWord();

	Seek(6);

	fEventNumber = 0;

	return GetNumOfWords();
}

/*The run time is stored as seven words:
 * 1. Month
 * 2. Day
 * 3. Year
 * 4. Hour
 * 5. Minute
 * 6. Second 
 * 7. Ticks (64's of a second)
 *
 * The ticks are ignored. Assuming the time is not daylight savings time
 *  (DST).
 *
 * \param[in] verbose Verbosity flag.
 * \return The decoded timestamp.
 */
time_t nsclUSBBuffer::GetRunTime(bool verbose)
{
	struct tm tempTime;
	//Assuming that the time stamp is not DST.
	tempTime.tm_isdst = -1;
	tempTime.tm_mon = GetWord();
	tempTime.tm_mday = GetWord();
	tempTime.tm_year = GetWord();
	tempTime.tm_hour = GetWord();
	tempTime.tm_min = GetWord();
	tempTime.tm_sec = GetWord();
	if (verbose) {
		printf("\t0x%04X Month:\t%d\n",tempTime.tm_mon+1,tempTime.tm_mon+1);
		printf("\t0x%04X Day:\t%d\n",tempTime.tm_mday,tempTime.tm_mday);
		printf("\t0x%04X Year:\t%d (%d)\n",tempTime.tm_year,tempTime.tm_year,tempTime.tm_year + 1900);
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
int nsclUSBBuffer::ReadEvent(bool verbose) {
	ClearModules();

	unsigned int eventStartPos = GetBufferPosition();
	if (eventStartPos >= GetNumOfWords()) {
		fflush(stdout);
		fprintf(stderr,"WARNING: Attempted to read event after reaching end of buffer!\n");
		return 0;
	}

	Int_t datum;
	UInt_t eventLength = GetEventLength();
	if (eventLength == 0) return 1;

	if (verbose) {
		datum = GetWord();
		printf ("\nData Event:\n");
		printf("\t%#06X Length: %d (%d+1)\n",datum,eventLength,datum);
	}
	else Seek(1);

	//Loop over each module
	for(unsigned int module=0;module<fModules.size();module++) {
		//Read out the current module
		// We need to check if we are at boundary word.
		//	(This occurs when the module is not readout and the
		//	 header and trailer are not written into the buffer)
		if (GetCurrentLongWord() != 0xFFFFFFFF) 
			fModules[module]->ReadEvent(this,verbose);

		//Check how many words were read.
		if (GetBufferPosition() - eventStartPos > eventLength) {
			fflush(stdout);
			fprintf(stderr,"ERROR: Module read too many words! (Buffer: %d)\n",GetBufferNumber());
			exit(8);
		}

		//Some version of USB buffer have boundary words, some do not.
		//Check if the current word in a module boundary word, if so we
		//skip it.
		if (GetCurrentLongWord() == 0xFFFFFFFF) {
			if (verbose) printf("\t%#04X Boundary Word\n",(UInt_t)GetLongWord());
			else Seek(2);
		}

	}

	FillStorage();

	//Fastforward over extra words
	int remainingWords = eventStartPos + eventLength - GetBufferPosition();
	if (remainingWords > 0) {
		if (verbose) {
			for (int i=0;i<remainingWords;i++) 
				printf("\t%#0*X Extra Word?\n",2*GetWordSize()+2,(UShort_t)GetWord());
		}
		else {
			Seek(remainingWords);
		}
	}

	fEventNumber++;

	return 1;
}

/**The VM USB version has a noninclusive word count.
 *
 * \return The length of the event in words.
 */
UInt_t nsclUSBBuffer::GetEventLength() {
	return ValidatedEventLength(GetCurrentWord()+1);
}

