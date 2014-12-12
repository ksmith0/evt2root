#include "nsclRunBuffer.h"

nsclRunBuffer::nsclRunBuffer()
{

}
/**Begin run buffer format:
 * 1. Run title (79 bytes)
 * 2. 0 (longword)
 * 3. Month
 * 4. Day
 * 5. Year
 * 6. Hour
 * 7. Minute
 * 8. Second 
 * 9. Ticks (64's of a second)
 *
 * \param[in] buffer Pointer to the buffer being read.
 * \param[in] verbose Verbosity flag.  
 *
 */
void nsclRunBuffer::ReadRunBegin(nsclBuffer *buffer,bool verbose) 
{
	if (buffer->GetBufferType() != BUFFER_TYPE_RUNBEGIN) {
		fprintf(stderr,"ERROR: Not a run begin buffer!\n");
		return;
	}
	
	if (buffer->IsRingBuffer()) {
		fRunNumber = buffer->GetWord();
		nsclBuffer::word timeOffset = buffer->GetWord();
		nsclBuffer::word timeStamp = buffer->GetWord();
		fRunStartTime = timeStamp;
		if (verbose) {
			printf("\n\tRun Number: %d\n",fRunNumber);
			printf("\tRun Start Time: %s",ctime(&fRunStartTime));
		}
	}
	else if(buffer->IsLDF()) {
		buffer->Forward(6);
		UInt_t date[6];
		for (int i=0;i<6;i++) date[i] = buffer->GetWord();
	}

	fRunTitle = GetTitle(buffer, verbose);
	
	//If traditional buffer get run time
	if (!buffer->IsRingBuffer()) {
		if (verbose) printf("\t0x%08X Empty Four Byte Word\n",buffer->GetFourByteWord());
		else buffer->Forward(2);

		fRunStartTime = GetTime(buffer,verbose);
		fRunNumber = buffer->GetRunNumber();
	}
}
void nsclRunBuffer::ReadRunEnd(nsclBuffer *buffer,bool verbose)
{
	if (buffer->GetBufferType() != BUFFER_TYPE_RUNEND) {
		fprintf(stderr,"ERROR: Not a run end buffer!\n");
		return;
	}
	
	if (buffer->IsRingBuffer()) {
		fRunNumber = buffer->GetWord();
		nsclBuffer::word timeOffset = buffer->GetWord();
		nsclBuffer::word timeStamp = buffer->GetWord();
		fElapsedRunTime = timeOffset;
		fRunEndTime = timeStamp;
		if (verbose) {
			printf("\n\tRun Number: %d\n",fRunNumber);
			printf("\tRun End Time: %s",ctime(&fRunEndTime));
		}
	}

	GetTitle(buffer,verbose);

	//If traditional buffer get run time
	if (!buffer->IsRingBuffer() && !buffer->IsLDF()) {
		fElapsedRunTime = buffer->GetWord() | (unsigned int) (buffer->GetWord() << 16);
		if (verbose) printf("\t0x%08X Elapsed Run Time: %u s\n",fElapsedRunTime,fElapsedRunTime);

		fRunEndTime = GetTime(buffer,verbose);
	}
	

}
std::string nsclRunBuffer::GetTitle(nsclBuffer *buffer,bool verbose) 
{
	std::string title;
	int readWords = 0;
	//Maximum number of words to look at for title.
	int maxWords = 0;
	if (buffer->IsRingBuffer()) maxWords = buffer->GetNumOfWords() - 3;
	else if (buffer->IsLDF()) maxWords = 10;
	else maxWords = 40;

	bool stringComplete = false; //Check if string is done.
	//Loop over words until string is completed.
	for (int i=0;i<maxWords && !stringComplete;i++) {
		nsclBuffer::word datum = buffer->GetWord();
		readWords++;
		if (verbose) {
			//Try to print a nice number of entries per line
			if ((readWords-1) % (12/WORD_SIZE) == 0) printf("\n\t");
			printf("%#0*X ",2*WORD_SIZE+2,datum);
		}
		if (datum == 0) {
			if (verbose) printf("%*c",buffer->GetWordSize()+1,' ');
			stringComplete = true;
			break;
		}
		//Loop over the number of characters stored in aword
		for (int charCount=0;charCount<buffer->GetWordSize();charCount++) {
			//Keep storing characters until string ends
			UShort_t bitShift = 8 * charCount;
			char letter = (datum >> bitShift) & 0xFF;
			if (letter != 0) {
				title.push_back(letter);
				if (verbose) printf("%c",letter);
			}
			else {
				stringComplete = true;
				//Pad the rest of the output
				if (verbose) printf("%*c",buffer->GetWordSize()-charCount,' ');
				break;
			}
		}
		if (verbose) printf(" ");
	}


	if (verbose) {
		//Print extraneous words
		for (int i=readWords;i<maxWords;i++) {
			nsclBuffer::word datum = buffer->GetWord();
			readWords++;
			if ((readWords-1) % (12/WORD_SIZE) == 0) printf("\n\t");
			printf("%#0*X %*c ",2*buffer->GetWordSize()+2,datum,buffer->GetWordSize(),' ');
		}
		//Print the finished title
		printf("\n\tTitle: %s\n",title.c_str());
	}
	//If we didn't print the extra word fast forward over them.
	else buffer->Forward(maxWords-readWords);

	return title;

}
time_t nsclRunBuffer::GetTime(nsclBuffer *buffer,bool verbose)
{
	struct tm tempTime;
	tempTime.tm_isdst = -1;
	tempTime.tm_mon = buffer->GetWord() - 1;
	tempTime.tm_mday = buffer->GetWord();
	tempTime.tm_year = buffer->GetWord() - 1900;
	tempTime.tm_hour = buffer->GetWord();
	tempTime.tm_min = buffer->GetWord();
	tempTime.tm_sec = buffer->GetWord();
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
void nsclRunBuffer::DumpRunBuffer(nsclBuffer *buffer)
{
	int eventLength = buffer->GetNumOfWords();
	printf("\nRun Buffer Dump Length: %d",eventLength);
	for (int i=0;i<eventLength;i++) { 
		if (i % (20/WORD_SIZE) == 0) printf("\n\t");
		printf("%#0*X ",2*WORD_SIZE+2,buffer->GetWord());
	}
	printf("\n");
	buffer->Rewind(eventLength);
}
std::string nsclRunBuffer::GetRunTitle()
{
	return fRunTitle;
}
time_t nsclRunBuffer::GetRunStartTime()
{
	return fRunStartTime;
}

time_t nsclRunBuffer::GetRunEndTime()
{
	return fRunEndTime;
}
unsigned int nsclRunBuffer::GetElapsedRunTime()
{
	return fElapsedRunTime;
}
