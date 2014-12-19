#include "hribfBuffer.h"

#include <sstream> 

hribfBuffer::hribfBuffer(const char *filename, int bufferSize, 
	int headerSize, int wordSize) :
		mainBuffer(headerSize,bufferSize,wordSize)
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
		printf ("\nData Event:\n");
	}

	//Loop over each module
	int headerSize = 2;
	for(unsigned int module=0;module<fModules.size();module++) {

		//Read out the current module
		fModules[module]->ReadEvent(this,verbose);

	}

	//Fastforward over extra words
	while (GetBufferPosition() < GetNumOfWords()) {
		UInt_t trailer = GetWord();
			
		if (trailer == 0xFFFFFFFF) {
			if (verbose) printf("\t%#010X Trailer\n",trailer);
			break;
		}
		if (verbose) printf("\t%#010X Extra Word?\n",trailer);
	}

		
	//Increment the number of Events read.
	fEventNumber++;

	//There is no indication of the number of events so we peak at the next
	// word to see if it is a trailer.
	if (GetWord() < 0xFFFFFFFF) fNumOfEvents++;
	Seek(-1);

	return GetBufferPosition() - eventStartPos;

}

int hribfBuffer::ReadNextBuffer() 
{
	this->Clear();

	if (!mainBuffer::ReadNextBuffer()) return 0;

	fBufferType = GetWord();
	if (fBufferType == BUFFER_TYPE_RUNEND) SetNumOfWords(2);
	else SetNumOfWords(GetWord());
	fBufferNumber++;
	//No information about number of events in header we assume there is
	// at least one.
	fNumOfEvents = 1;
	fEventNumber = 0;

	return GetNumOfWords();
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
	if (GetBufferType() != BUFFER_TYPE_RUNEND) {
		fflush(stdout);
		fprintf(stderr,"ERROR: Not a run begin buffer!\n");
		return;
	}

/*
	std::string deadTime = ReadString(GetNumOfWords(),verbose);
	if (verbose) {
		printf("\t String: ");
		int length=63;
		for (int i=0;i<deadTime.length()/length;i++) {
			if (i>0) printf("\t%9c",' ');
			printf("%*s\n",length,deadTime.substr(i*length,length).c_str());
		}
		printf("\n");
	}
*/
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

void hribfBuffer::ReadScalers(bool verbose) {
	std::string scalerString = ReadString(GetNumOfWords(),verbose);
	//if (verbose) printf("\t Word: %s\n",scalerString.c_str());
	
	std::vector<std::string> wordVector;
	std::size_t prev = 0, pos;
    while ((pos = scalerString.find("  ", prev)) != std::string::npos)
    {
        if (pos > prev)
            wordVector.push_back(scalerString.substr(prev, pos-prev));
        prev = pos+1;
    }	

	struct tm tm;
	strptime((wordVector[1]+wordVector[2]).c_str(), "%d-%b-%y %H:%M:%S", &tm);
	time_t t = mktime(&tm);

	if (verbose) {
		printf("\tLeader: %s\n",wordVector[0].c_str());
		printf("\tDate: %s",ctime(&t));
		for (unsigned int i=3;i<wordVector.size();i++) {
			printf("\tExtra Word: %s\n",wordVector[i].c_str());
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


