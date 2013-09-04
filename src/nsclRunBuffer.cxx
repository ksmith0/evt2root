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
 *
 */
void nsclRunBuffer::ReadRunBegin(nsclBuffer *buffer,bool verbose) 
{
	if (buffer->GetBufferType() != BUFFER_TYPE_RUNBEGIN) {
		fprintf(stderr,"ERROR: Not a run begin buffer!\n");
		return;
	}

	fRunTitle = GetTitle(buffer, verbose);
	
	if (verbose) printf("\t0x%08X Empty Longword\n",buffer->GetLongWord());
	else buffer->Forward(2);

	fRunStartTime = GetTime(buffer,verbose);
}
void nsclRunBuffer::ReadRunEnd(nsclBuffer *buffer,bool verbose)
{
	if (buffer->GetBufferType() != BUFFER_TYPE_RUNEND) {
		fprintf(stderr,"ERROR: Not a run end buffer!\n");
		return;
	}
	GetTitle(buffer,verbose);

	fElapsedRunTime = buffer->GetWord() | (unsigned int) (buffer->GetWord() << 16);
	if (verbose) printf("\t0x%08X Elapsed Run Time: %u s\n",fElapsedRunTime,fElapsedRunTime);

	fRunEndTime = GetTime(buffer,verbose);
	

}
std::string nsclRunBuffer::GetTitle(nsclBuffer *buffer,bool verbose) 
{
	std::string title;
	int readWords = 0;
	for (int i=0;i<40;i++) {
		int word = buffer->GetWord();
		readWords++;
		if (word == 0) break;
		title.push_back(word & 0xFF);
		int letter = (word & 0xFF00) >> 8;
		if (letter)
			title.push_back(letter);
		if (verbose) {
			if ((readWords-1) % 5 == 0) printf("\n\t");
			if (letter)
				printf("0x%04X %c%c ",word,(word & 0xFF),letter);
			else
				printf("0x%04X %c  ",word,(word & 0xFF));
		}

	}
	if (verbose) {
		for (int i=readWords;i<40;i++) {
			if ((readWords-1) % 5 == 0) printf("\n\t");
			printf("0x%04X    ",buffer->GetWord());
			readWords++;
		}
		printf("\n\tTitle: %s\n",title.c_str());
	}
	buffer->Forward(40-readWords);

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
	int eventLength = buffer->GetNumOfWords()-16;
	printf("\nRun Buffer Dump Length: %d",eventLength);
	for (int i=0;i<eventLength;i++) { 
		if (i % 10 == 0) printf("\n\t");
		printf("0x%04X ",buffer->GetWord());
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
