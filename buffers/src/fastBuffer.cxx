#include "fastBuffer.h"
#include <TH1D.h>

fastBuffer::fastBuffer(int bufferSize, int bufferHeaderSize, int wordSize) :
	mainBuffer(bufferHeaderSize, bufferSize, wordSize),
	headerRead(false),
	listdata_(false),
	numActiveADCs(0),
	timestamp(0)
{

}

fastBuffer::~fastBuffer() 
{

}

void fastBuffer::InitializeStorageManager() {
	if (GetStorageManager()) {
		GetStorageManager()->CreateTree("data");
		GetStorageManager()->CreateBranch("data","adc",adcValues.data(),"adc[16]/s");
		GetStorageManager()->CreateBranch("data","mult",&multiplicity,"mult/s");
		GetStorageManager()->CreateBranch("data","timestamp",&timestamp,"timestamp/l");
		GetStorageManager()->CreateBranch("data","time",&time,"time/l");
	}
}

int fastBuffer::ReadNextBuffer() 
{
	Clear();
	if (!fFile.good()) {
		fflush(stdout);
		printf("ERROR: File not good.\n");
		return -1;
	}

	if (!headerRead) {
		fBufferType = RUN_BEGIN;
		return 1;
	}

	if (!listdata_) {
		std::string line = PeekLine();
		if (line.find("[LISTDATA]") != std::string::npos) {
			//Pull the List data header off the data.
			line = GetLine();
			listdata_ = true;
		}
		else if (line.find("[DATA") != std::string::npos) {
			fBufferType = HIST;
		
			return 1;
		}
		else {
			if (line.length() > 0) {
				line.erase(line.find_last_not_of(" \r\n")+1);
				fprintf(stderr,"ERROR: Unknown header tag: '%s'!\n",line.c_str());
			}
			return 0;
		}
	}

	//FAST list Buffer does not track buffer number, we iterate it manually.
	fBufferNumber++;

	fBufferBeginPos = GetFilePosition();

	//Specify that the buffer is big enough for a word.
	SetBufferSize(GetWordSize());
	//The fast list buffer has buffers exactly the size of the event.
	//Read the first word which should help indicate the size.
	fFile.read(&fBuffer[0], GetWordSize());
	//Check that we got the correct number of bytes.
	if (fFile.gcount() != GetWordSize()) {
		if (fFile.gcount() != 0)fprintf(stderr,"ERROR: Read %ld bytes expected %u!\n",fFile.gcount(),GetWordSize());
		return 0;
	}
	//Specify the size of the readable buffer.
	SetNumOfBytes(GetWordSize());

	UInt_t datum = GetWord();
	//Syncronization mark
	if (datum == 0xFFFFFFFF) {
		fBufferType = SYNC_MARK;
		return 1; 
	}
	else if ((datum >> 16 & 0x4000) == 0) {
		fBufferType = DATA;	

		//Now we determine how many ADCs were triggered
		ReadTriggeredADCs(datum);
	
		//Abort if the number of triggered ADCs is less than 1
		if (triggeredADCs.size() < 1) {
			fprintf(stderr,"ERROR: ADC Data event with no triggered ADCs! %#10X\n",datum);
			return 0;
		}
		//The 31st bit indicates a dummy word was inserted for an odd number of triggered ADCs. 
		if (((datum >> 31) && 0xF) == 0 && triggeredADCs.size() % 2 == 1) {
			fprintf(stderr,"ERROR: Odd number of ADCs triggered and no dummy word indicated.");
			return 0;
		}	

		//Prepare the buffer for the current event.
		//Each triggered ADC writes two bytes.
		UInt_t numBytes = 2 * triggeredADCs.size() + 4;
		//Events are packaged to fit into 32 bit words so we have to add space for padding
		if (triggeredADCs.size() % 2 == 1) numBytes += 2;
		SetBufferSize(numBytes);

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

		//No information about number of events in header we assume there is
		// at least one.
		fNumOfEvents = 1;
		fEventNumber = 0;

		return GetNumOfWords();
	}
	else if ((datum >> 16 & 0x4000) != 0) {
		fBufferType = TIME;	
		ReadTriggeredADCs(datum);

		return 1;
	}

	return 1;

}


/**Each ADC sets the bit corresponding the ADC number if it was triggered during a data event or alive
 * during a time stamp event. A vector of triggered ADCs is stored as fastBuffer::triggeredADCs.
 *
 * \param[in] datum The data word containg the ADC flags.
 */
void fastBuffer::ReadTriggeredADCs(UInt_t datum) {
	for (int adc = 0; adc < numActiveADCs; adc++) {
		char adcTriggered = (datum >> adc) & 0x1;
		if (adcTriggered == 1) 
			triggeredADCs.push_back(adc);
	}
}

/**
 *	\param[in] verbose Verbosity flag.
 */
void fastBuffer::UnpackBuffer(bool verbose) {
	switch (fBufferType) {
		case DATA:
			ReadEvent(verbose);
			break;
		case TIME:
			ReadTimeStamp(verbose);
			break;
		case RUN_BEGIN:
			ReadRunBegin(verbose);
			break;
		case SYNC_MARK:
			if (verbose) printf("\tSYNC MARK\n");
			break;
		case HIST:
			ReadHistogram(verbose);
			break;
		default:
			printf("ERROR: Unknown buffer type!\n");
	}
}


/**The time stamp indicates another tick in the 1 ms clock. These ticks may be downscaled based on the 
 * value in timerreduce.
 *
 * \param[in] verbose Verbosity flag.
 */
void fastBuffer::ReadTimeStamp(bool verbose) {
	//Iterate timestamp by one.
	timestamp++;	
	time += timeDownScale;

	if (verbose) {
		printf ("\nTime Stamp %llu:\n", timestamp);
		printf("\tTime: %llu ms\n",time);

		Seek(-1);
		//Print the first word with the triggered ADC bits.
		printf ("\t%#010llX Alive ADCs: ",GetWord());
		for (auto it = triggeredADCs.begin(); it != triggeredADCs.end(); ++it) {
			if (it != triggeredADCs.begin()) printf(", ");
			printf("%d",*it);
		}
		printf("\n");
	}
}


/**Read out the data for the stored histogram. It is assumed that the histogram has 
 * previously been defined.
 *
 * \param[in] verbose Verbosity flag.
 */
int fastBuffer::ReadHistogram(bool verbose) {
	//We expect the line to be in this format "[DATA0,8192 ]"
	//Where 0 is the ADC number and 8192 is the number of channels.
	std::string line = GetLine();
	int commaPos = line.find(",");
	int adcNum = std::stoi(line.substr(5, commaPos - 5).c_str());
	int numHistChannels = std::stoi(line.substr(commaPos+1,line.find_first_not_of("1234567890", commaPos+1) - commaPos - 1).c_str());

	if (verbose) printf("\nHISTDATA found for ADC %d, %d channels\n",adcNum,numHistChannels);
	RootStorageManager *manager = GetStorageManager();

	TH1D* hist;
	if (manager) {
		hist = manager->CreateHistogram("hMCARaw",";Channel;Counts / Channel", numHistChannels,0,numHistChannels);
	}

	for (int ch = 1; ch <= numHistChannels; ch++) {
		int value = atoi(GetLine().c_str());
		if (verbose) {
			printf("\tCh: %4d, Val: %d\n",ch, value);
		}
		if (hist) {
			hist->SetBinContent(ch,value);
		}
	}
	return 1;

}

int fastBuffer::ReadEvent(bool verbose) {
	unsigned int eventStartPos = GetBufferPosition();
	if (eventStartPos >= GetNumOfWords()) {
		fflush(stdout);
		fprintf(stderr,"WARNING: Attempted to read event after reaching end of buffer!\n");
		return -1;
	}

	multiplicity = triggeredADCs.size();

	if (verbose) {
		printf ("\nData Event %llu:\n",fEventNumber);
		Seek(-1);

		//Print the first word with the triggered ADC bits.
		printf ("\t%#010llX Triggered ADCs: ",GetWord());
		for (auto it = triggeredADCs.begin(); it != triggeredADCs.end(); ++it) {
			if (it != triggeredADCs.begin()) printf(", ");
			printf("%d",*it);
		}
		printf("\n");
	}

	//If there is an odd number of ADCs we seek over the dummy 16 bits.
	if (triggeredADCs.size() % 2 == 1) {
		if (verbose) printf("\t%#06X Dummy Word\n",(UShort_t)GetWord(2));
		else SeekBytes(2);
	}

	for (auto itr = triggeredADCs.begin(); itr != triggeredADCs.end(); ++itr) {
		UShort_t adc = *itr;
		adcValues[adc] = GetWord(2);
		if (verbose) {
			printf("\t%#06X",adcValues[adc]);
			printf(" ADC %d value: %u\n",adc,adcValues[adc]);
		}
	}

	if (GetStorageManager()) GetStorageManager()->Fill("data");

	fEventNumber++;

	return GetBufferPosition() - eventStartPos;

}

bool fastBuffer::IsDataType() {
	return (fBufferType == DATA);
}
bool fastBuffer::IsRunBegin() {
	return (fBufferType == RUN_BEGIN);
}

void fastBuffer::ReadRunBegin(bool verbose) {
	std::string line;

	if (verbose) printf("\n");
	int adcNum = -1;
	unsigned int linePosition;
	do {
 		linePosition = GetFilePosition();	
		line = GetLine();
		line.erase(line.find_last_not_of(" \r\n")+1);

		size_t pos;

		//Check if we have an equals sign
		// This indicates that we have a key value line.
		if ((pos = line.find("=")) != std::string::npos) {
			//Extract key and value
			std::string key = line.substr(0,pos);
			std::string value = line.substr(pos+1);

			if (verbose) {
				printf("%4u\t",linePosition);
				printf("\"%s\"",line.c_str());
				
				//Print a nice number of spaces
				int numSpaces = 27 - key.length() - value.length(); 
				if (numSpaces < 0 ) numSpaces = 1;
				printf("%*c",numSpaces,' ');

				printf("key[%s]='%s'\n",key.c_str(),value.c_str());
			}
	
			//We found an active ADC
			if (key == "active" && std::stoi(value) > 0) numActiveADCs++;
			//We found the downscaling for the timestamps
			else if (key == "timerreduce") timeDownScale = std::stoi(value);
		}
		//We found the header for an ADC block
		else if ((pos = line.find("[ADC")) != std::string::npos) {
			adcNum = std::stoi(line.substr(pos+4,line.find("]")-pos-4).c_str());
			if (verbose) {
				printf("\n");
				printf("%4u\t\"%s\"",linePosition,line.c_str());
				//Print a nice number of spaces
				printf("%*c",(adcNum < 10 ? 22 : 21),' ');
				printf("Found ADC %2d%*c\n",adcNum,16,' ');
			}
		}
		else if (verbose) {
			printf("%4u\t\"%s\"\n",linePosition,line.c_str());
		}


	} while(line.find("DATA") == std::string::npos);

	//Rewind to the beginning of the last line read.
	SeekFile(-(long int)(GetFilePosition() - linePosition));

	headerRead = true;
}

void fastBuffer::Clear() {
	mainBuffer::Clear();
	triggeredADCs.clear();
	adcValues.fill(0);
	multiplicity = 0;
}
