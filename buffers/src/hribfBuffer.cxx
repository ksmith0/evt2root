#include "hribfBuffer.h"
#include "baseModule.h"

#include <sstream> 

hribfBuffer::hribfBuffer(int bufferSize,int headerSize, int wordSize) :
		moduleBuffer(bufferSize,headerSize,wordSize)
{
}
hribfBuffer::hribfBuffer(const char *filename, int bufferSize, 
	int headerSize, int wordSize) :
		moduleBuffer(headerSize,bufferSize,wordSize)
{
	OpenFile(filename);
}
/**
 * \param[in] verbose Verbosity flag.
 * \return The number of words left in the buffer.
 */
int hribfBuffer::ReadEvent(bool verbose) {
	ClearModules();

	unsigned int eventStartPos = GetBufferPosition();
	if (eventStartPos >= GetNumOfWords()) {
		fflush(stdout);
		fprintf(stderr,"WARNING: Attempted to read event after reaching end of buffer!\n");
		return -1;
	}

	if (verbose) {
		printf ("\nData Event %llu:\n",fEventNumber);
	}

	//Loop over each module
	for(unsigned int module=0;module<fModules.size();module++) {

		//Read out the current module
		fModules[module]->ReadEvent(this,verbose);

	}

	FillStorage();

	//Increment the number of Events read.
	fEventNumber++;

	//There is no indication of the number of events so we peak at the next
	// word to see if it is a trailer.
	if (GetBufferPositionBytes() < GetNumOfBytes()) {
		if (GetCurrentWord() != 0xFFFFFFFF) fNumOfEvents++;
	}

	return GetBufferPosition() - eventStartPos;

}

int hribfBuffer::ReadNextBuffer() 
{
	this->Clear();

	if (!mainBuffer::ReadNextBuffer()) return 0;

	fBufferType = GetWord();
	if (fBufferType == BUFFER_TYPE_EOF) SetNumOfWords(GetHeaderSize());
	else SetNumOfWords(GetWord() + GetHeaderSize());
	fBufferNumber++;
	//No information about number of events in header we assume there is
	// at least one.
	fNumOfEvents = 1;
	fEventNumber = 0;

	return GetNumOfWords();
}
/**Unpacks the current buffer based on the type. This needs to be called multiple 
 * time to completely unpack a physics event buffer.
 *
 * \param[in] verbose Verbosity flag.
 * \return True if the buffer has more content to unpack.
 */
void hribfBuffer::UnpackBuffer(bool verbose) {
	switch(fBufferType) {
		case BUFFER_TYPE_DATA:
			while (GetEventsRemaining())
				//We read an event and there are no more words left.
				if (!ReadEvent(verbose)) break;
			break;
		case BUFFER_TYPE_SCALERS: 
			ReadScalers(verbose);
			break;
		case BUFFER_TYPE_RUNBEGIN: 
			ReadRunBegin(verbose);
			break;
		case BUFFER_TYPE_DIR:
			ReadDir(verbose);
			break;
		case BUFFER_TYPE_DEAD:
			ReadDead(verbose);
			break;
		case BUFFER_TYPE_EOF: 
			break;
		case BUFFER_TYPE_PAC:
			ReadPAC(verbose);
			break;
		default: 
			fflush(stdout);
			fprintf(stderr,"WARNING: Unknown buffer type: %#010X '%s'.\n",(UInt_t)fBufferType,ConvertToString(fBufferType).c_str());
	}
}
void hribfBuffer::ReadPAC(bool verbose) {
	std::string PACStr = ReadString(GetNumOfWords() - GetHeaderSize(),verbose);

	if (verbose) printf("\t%s\n",PACStr.c_str());
}


/**DEAD buffer contains ASCII string of dead time information.
 *
 * \param[in] verbose Verbosity flag.
 */
void hribfBuffer::ReadDead(bool verbose) {
	std::string deadTimeStr = ReadString(GetNumOfWords() - GetHeaderSize(),verbose);

	if (verbose) printf("\t%s\n",deadTimeStr.c_str());
}

/**DIR buffer is usually found at the beginning of a file (run) and contains the following:
 * 	1. The length of a buffer (typically 8194).
 * 	2. The number of buffers in the file (run).
 * 	3. An unknown value
 * 	4. The run number.
 * 	5. An unknown value
 * 	6. An unknown value
 *
 * 	\bug The buffer length is hard coded to 8194 and the value from this buffer is ignored. 
 *
 * \param[in] verbose Verbosity flag.
 */
void hribfBuffer::ReadDir(bool verbose) {
	unsigned int bufferLength = GetWord();
	unsigned int numberOfBuffers = GetWord();
	unsigned int word1 = GetWord();
	unsigned int word2 = GetWord();
	fRunNum = GetWord();
	unsigned int word3 = GetWord();

	if (verbose) {
		printf("\t%#010X Buffer Length: %u\n",bufferLength,bufferLength);
		printf("\t%#010X Number of Buffers: %u\n",numberOfBuffers,numberOfBuffers);
		printf("\t%#010X Unknown Word: %u\n",word1,word1);
		printf("\t%#010llX Run Number: %llu\n",fRunNum,fRunNum);
		printf("\t%#010X Unknown Word: %u\n",word2,word2);
		printf("\t%#010X Unknown Word: %u\n",word3,word3);
	}
}

void hribfBuffer::PrintBufferHeader() 
{
	printf("\nBuffer Header Summary:\n");
	std::string type = ConvertToString(fBufferType);

	printf("\t%#010llX Buffer type: %s\n",fBufferType,type.c_str());
	printf("\t%#010llX Number of Words: %llu\n",GetNumOfWords(),GetNumOfWords());
	printf("\t%10c Buffer number: %llu\n",' ',fBufferNumber);
}
void hribfBuffer::ReadRunEnd(bool verbose) 
{
}

void hribfBuffer::ReadRunBegin(bool verbose)
{
	if (GetBufferType() != BUFFER_TYPE_RUNBEGIN) {
		fflush(stdout);
		fprintf(stderr,"ERROR: Not a run begin buffer!\n");
		return;
	}

	std::string facility = ReadString(2,verbose);
	if (verbose) printf("\t Facility: %s",facility.data());
	std::string fFormat = ReadString(2,verbose);
	if (verbose) printf("\t Format: %s",fFormat.data());
	std::string type = ReadString(4,verbose);
	if (verbose) printf("\t Type: %s",type.data());
	std::string date = ReadString(4,verbose);
	if (verbose) printf("\t Date: %s",date.data());

	fRunTitle = ReadString(20,verbose);
	if (verbose) printf("\t Title: %s\n",fRunTitle.c_str());

	fRunNum = GetWord();	
	if (verbose) printf("\t%#010llX Run Number: %llu\n",fRunNum,fRunNum);

	if (type.find("LIST DATA") == std::string::npos) {
		fflush(stdout);
		fprintf(stderr,"ERROR: Unknown type: %s\n",type.c_str());
	}
}

/**Scalers are saved as an ASCII string with a double space "  " separating
 * each entry. The entries are ordered in the following way:
 * 1. Leader "SCALER DUMP"
 * 2. Date (day-month-year)
 * 3. Time (hour:min:sec)
 * 4. Time elapsed since last scaler read.
 * 5. Boolean specifying whether the values were cleared since last read.
 * 	This specifies whether the scaler are incremental.
 * 6. Scalers as three entries.
 * 	a. Scaler name.
 * 	b. Scaler value.
 * 	c. Scaler rate?
 *
 * \param[in] verbose Verbosity flag.
 */
void hribfBuffer::ReadScalers(bool verbose) {
	int maxLength = 24;
	if (verbose) printf("\n");

	std::string leader = ReadStringBytes(13,verbose);
	if (verbose) printf("\t%-*s| Leader\n\n",maxLength,leader.c_str());
	
	//Build a timestamp from the date and time entries.
	std::string date = ReadStringBytes(22,verbose);
	struct tm tm;
	strptime(date.c_str(), "%d-%b-%y  %H:%M:%S", &tm);
	time_t t = mktime(&tm);
	if (verbose) printf("\t%-*s| Date: %s\n",maxLength,date.c_str(),ctime(&t));

	//Get time elapsed since last scaler dump.
	std::string duration = ReadStringBytes(11,verbose);
	UInt_t timeElapsed = std::stoi(duration.substr(duration.find("=")+1));
	if (verbose) printf("\t%-*s| Time Elapsed: %u s\n\n",maxLength,duration.c_str(),timeElapsed);

	//Get the incremental flag
	std::string clearCode = ReadStringBytes(11,verbose);
	bool incremental = false;
	if (clearCode.find_first_of("yY") != std::string::npos) 
		incremental = true;
	if (verbose) printf("\t%-*s| Incremental: %u\n\n",maxLength,clearCode.c_str(),incremental);

	ReadStringBytes(23,verbose);

	//Loop over the scalers and get the name and values.
	std::vector< std::pair< UInt_t, Float_t > > scalerValues;
	std::vector< std::string > scalerNames;
	std::string scalerName;
	while((scalerName = ReadStringBytes(15,verbose)).find_first_not_of(" ") != std::string::npos) {
		scalerNames.push_back(scalerName.substr(0,scalerName.find_first_of(" ")));

		std::string scalerVal1 = ReadStringBytes(9,verbose);
		std::string scalerVal2 = ReadStringBytes(12,verbose);
		scalerValues.push_back(std::make_pair(std::stoi(scalerVal1),std::stof(scalerVal2)));

		ReadStringBytes(4,verbose);

		if (verbose) {
			printf("\tScaler '%s' %d %.3e\n\n",scalerNames.back().c_str(),scalerValues.back().first,scalerValues.back().second);
		}
	}

}


/**L003 format has events that do not specify the length. If the length is
 * required it is determined by looking for a boundary word, 0xFFFFFFFF.
 * The buffer is the rewound for further reading.
 *
 * \return The length of the event in words.
 */
UInt_t hribfBuffer::GetEventLength() {
	UInt_t eventLength = 1;
	while (GetWord() != 0xFFFFFFFF) {
		eventLength++;
	}
	Seek(-eventLength);
	return ValidatedEventLength(eventLength);
}

bool hribfBuffer::IsDataType() {
	if (fBufferType == BUFFER_TYPE_DATA) return true;
	return false;
}
bool hribfBuffer::IsScalerType() {
	if (fBufferType == BUFFER_TYPE_SCALERS) return true;
	return false;
}
bool hribfBuffer::IsRunBegin() {
	if (fBufferType == BUFFER_TYPE_RUNBEGIN) return true;
	return false;
}
bool hribfBuffer::IsRunEnd() {
	if (fBufferType == BUFFER_TYPE_EOF) return true;
	return false;
}
