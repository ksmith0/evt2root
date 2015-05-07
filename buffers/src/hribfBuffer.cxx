#include "hribfBuffer.h"
#include "baseModule.h"

#include <sstream> 

hribfBuffer::hribfBuffer(int bufferSize,int headerSize, int wordSize) :
		listDataBuffer(headerSize,bufferSize,wordSize)
{
}
hribfBuffer::hribfBuffer(const char *filename, int bufferSize, 
	int headerSize, int wordSize) :
		listDataBuffer(headerSize,bufferSize,wordSize)
{
	OpenFile(filename);
}
int hribfBuffer::ReadEvent(bool verbose) {

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
	int headerSize = 2;
	for(unsigned int module=0;module<fModules.size();module++) {

		//Read out the current module
		fModules[module]->ReadEvent(this,verbose);

	}

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
/**Unpacks the current buffer based on the type. Data buffers are ignored 
 * and left to the user to unpack for now.
 *
 * \param[in] verbose Verbosity flag.
 */
void hribfBuffer::UnpackBuffer(bool verbose) {
	switch(fBufferType) {
		case BUFFER_TYPE_DATA:
			return;
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
		case BUFFER_TYPE_PAC:
			break;
		default: 
			fflush(stdout);
			fprintf(stderr,"WARNING: Unknown buffer type: %#010X '%s'.\n",(UInt_t)fBufferType,ConvertToString(fBufferType).c_str());
			return;

	}
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
	//The number of words includes the header.
	std::string scalerString = ReadString(GetNumOfWords()-GetHeaderSize(),verbose);
	
	//Build a vector of each entry in the scaler string.
	//Entries are separated by a double space, "  ".
	std::vector<std::string> scalerEntries;
	size_t start = 0, stop = 0;
	while ((stop = scalerString.find("  ",start)) != std::string::npos) {
		stop = scalerString.find_first_not_of(" ",stop);
		if (stop == std::string::npos) break;
		scalerEntries.push_back(scalerString.substr(start, stop-start-2));
		start = stop;
	}
	scalerEntries.push_back(scalerString.substr(start));

	//Build a timestamp from the date and time entries.
	struct tm tm;
	strptime((scalerEntries[1]+scalerEntries[2]).c_str(), "%d-%b-%y %H:%M:%S", &tm);
	time_t t = mktime(&tm);

	//Get time elapsed since last scaler dump.
	UInt_t timeElapsed = std::stoi(scalerEntries[3].substr(scalerEntries[3].find("=")+1));

	//Get the incremental flag
	bool incremental = false;
	if (scalerEntries[4].find_first_of("yY") != std::string::npos) 
		incremental = true;

	//Loop over the scalers and get the name and values.
	std::vector< std::pair< UInt_t, Float_t > > scalerValues;
	std::vector< std::string > scalerNames;
	for (unsigned int i=5;i<scalerEntries.size();i+=3) {
		scalerNames.push_back(scalerEntries[i].substr(0,scalerEntries[i].find_last_not_of(" ")+1));

		std::pair< UInt_t, Float_t > scalerValue(0,0);
		if (i+2 < scalerEntries.size())
			scalerValue = std::make_pair(std::stoi(scalerEntries[i+1]),std::stof(scalerEntries[i+2]));
		else if (i+1 < scalerEntries.size()) 
			scalerValue = std::make_pair(std::stoi(scalerEntries[i+1]),0);
		scalerValues.push_back(scalerValue);

	}

	if (verbose) {
		//Determine the maximum length to make output look nice
		int maxLength = (scalerEntries[1]+scalerEntries[2]).length();
		int maxScalerNameLength = 0;
		for (size_t i=5;i<scalerEntries.size();i+=3) {
			std::string scalerEntry;
			for (unsigned int j=0;j<3 && i+j < scalerEntries.size();j++) 
				scalerEntry.append(scalerEntries.at(i+j));
			size_t stringLen = scalerEntry.length();
			if (stringLen > maxLength) maxLength = stringLen;

			//Get padding size for scaler names
			unsigned int scalerCount = (i-5)/3;
			if (scalerCount < scalerNames.size()) {
				size_t nameLength = scalerNames.at(scalerCount).length();
				if (nameLength > maxScalerNameLength) 
					maxScalerNameLength = nameLength;
			}
		}

		printf("\n");
		printf("\t%-*s| Leader\n",maxLength,scalerEntries[0].c_str());
		printf("\t%-*s| Date: %s",maxLength,(scalerEntries[1]+scalerEntries[2]).c_str(),ctime(&t));
		printf("\t%-*s| Time Elapsed: %u s\n",maxLength,scalerEntries[3].c_str(),timeElapsed);
		printf("\t%-*s| Incremental: %u\n",maxLength,scalerEntries[4].c_str(),incremental);
		
		//Print out the scalers
		for (unsigned int i=5;i<scalerEntries.size();i+=3) {
			//Printf the scaler entry string
			std::string scalerEntry;
			for (unsigned int j=0;j<3 && i+j < scalerEntries.size();j++) 
				scalerEntry.append(scalerEntries.at(i+j));
			printf("\t%-*s| Scaler ",maxLength,scalerEntry.c_str());
			
			//Print the converted values
			unsigned int scalerCount = (i-5)/3;
			int padding = maxScalerNameLength + 1; //Pad scaler names
			if (scalerCount < scalerNames.size()) {
				printf("'%s'",scalerNames.at(scalerCount).c_str());
				padding -= scalerNames.at(scalerCount).length();
			}
			printf("%*c",padding,' ');
			if (scalerCount < scalerValues.size()) 
				printf("%6d %-.3e",scalerValues.at(scalerCount).first,scalerValues.at(scalerCount).second);
			printf("\n");
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
