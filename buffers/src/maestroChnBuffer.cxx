#include "maestroChnBuffer.h"
#include "TDirectory.h"
#include "TH1F.h"
#include "TParameter.h"

maestroChnBuffer::maestroChnBuffer(int headerSize, int bufferSize, int wordSize) : 
	mainBuffer(headerSize,bufferSize,wordSize),
	fHist(NULL),
	fCurrentChannel(0),
	fNumberOfChannels(0)
{
	fRunBeginRead = false;
}

maestroChnBuffer::~maestroChnBuffer() {
}

int maestroChnBuffer::ReadNextBuffer() {
	if (mainBuffer::ReadNextBuffer() == 0) return 0;
	fBufferNumber++;
	fEventNumber = 0;
	fNumOfEvents = 1;
	return GetNumOfWords();
}

void maestroChnBuffer::UnpackBuffer(bool verbose)
{
	if (IsDataType())
		ReadEvent(verbose);
	else if (IsRunBegin()) 
		ReadRunBegin(verbose);
	else if (IsRunEnd())
		ReadRunEnd(verbose);
	
}	
void maestroChnBuffer::PrintBufferHeader() {

	printf("\nBuffer Header Summary:\n");
	printf("\tBuffer Number: %d\n",GetBufferNumber());
}

/**Run begin is a 32 byte record containing the following:
 * 1. Type (2 Bytes)
 * 2. MCA Number  (2 Bytes)
 * 3. Segment Number (2 Bytes)
 * 4. Storage Seconds (2 Bytes)
 * 5. Real Time (4 Bytes)
 * 6. Live Time (4 Bytes)
 * 7. Storage Date (8 Bytes)
 * 8. Storage Time (4 Bytes)
 * 9. Starting Channel (2 Bytes)
 * 10. Number of Chhanels (2 Bytes)
 *
 * Type should be set to -1. The storage seconds, time and date are ASCII
 * representations of the time storage was started. The date has an extra
 * character on the end. Real and live time are counts of a 50 Hz clock.
 */
void maestroChnBuffer::ReadRunBegin(bool verbose) 
{
	Short_t type = GetWord(2);		

	if (type != -1) {
		fflush(stdout);
		fprintf(stderr,"ERROR: Input data has invalid CHN type!\n");
		return;
	}
	Short_t MCANum = GetWord(2);
	Short_t segmentNum = GetWord(2);

	if (verbose) {
		printf("\t%#06hX Type: %hd\n",type,type);
		printf("\t%#06hX MCA: %hd\n",MCANum,MCANum);
		printf("\t%#06hX Segment Number: %hd\n",segmentNum,segmentNum);
		printf("\tStorage Seconds String:");
	}

	std::string storageTime = ReadString(1,verbose); 

	UInt_t realTime = GetWord(4);
	UInt_t liveTime = GetWord(4);

	if (verbose) {
		printf("\t%#010X Real Time: %.2fs\n",realTime,(float)realTime/50);
		printf("\t%#010X Live Time: %.2fs\n",liveTime,(float)liveTime/50);
	}

	if (verbose) printf("\tStorage Date String:");
	storageTime.append(ReadString(4,verbose).substr(0,7));
	if (verbose) printf("\tStorage Time String:");
	storageTime.append(ReadString(2,verbose));

	//Build a timestamp from the date and time and seconds entries.
	struct tm tm;
	strptime(storageTime.c_str(), "%S%d%b%y%H%M", &tm);
	time_t t = mktime(&tm);

	if (verbose) printf("\t%s Time: %s",storageTime.c_str(),ctime(&t));

	fStartingChannel = GetWord(2);
	fNumberOfChannels = GetWord(2);

	if (verbose) {
		printf("\t%#06hX Starting Channel: %hd\n",fStartingChannel,fStartingChannel);
		printf("\t%#06hX Number of Channels: %hd\n",fNumberOfChannels,fNumberOfChannels);
	}

	if (gDirectory->IsWritable()) {
		TParameter<time_t>("runStartTime",t).Write();
		TParameter<Float_t>("liveTime",liveTime/50).Write();
		TParameter<Float_t>("realTime",realTime/50).Write();
	}
	fRunBeginRead = true;
}

int maestroChnBuffer::ReadEvent(bool verbose) {
	if (!fHist) {
		fHist = new TH1F("hMCA",";Channel;Counts / Channel",fNumberOfChannels,fStartingChannel,fStartingChannel+fNumberOfChannels);
		fCurrentChannel = 1;
	}

	for (int word=0;word<8;word++) {
		int value = GetWord(4);
		if (verbose) {
			printf("\tCh: %4d, Val: %d\n",fCurrentChannel, value);
		}
		fHist->SetBinContent(fCurrentChannel++,value);
	}
	fEventNumber++;
	return 0;
}

void maestroChnBuffer::ReadRunEnd(bool verbose) {
	//If the histogram was created and the current directory is writable
	//	then write out the histogram.
	if (fHist && gDirectory->IsWritable()) {
		fHist->Write();
		delete fHist;
		fHist = 0;
	}

	Short_t trailerType = GetWord(2);
	if (verbose) printf("\t%#06X Trailer Type: %d\n",trailerType,trailerType);
	if (trailerType == -102) {
		SeekBytes(2);
		Int_t energyCal[3], peakCal[3];
		for (int i=0;i<3;i++) energyCal[i] = GetWord(4);
		for (int i=0;i<3;i++) peakCal[i] = GetWord(4);
		if (verbose) {
			for (int i=0;i<3;i++) printf("\t%#010X Energy Calib     Par %d: %+d\n",energyCal[i],i,energyCal[i]);
			for (int i=0;i<3;i++) printf("\t%#010X Peak Shape Calib Par %d: %+d\n",peakCal[i],i,peakCal[i]);
		}		
	}
}

bool maestroChnBuffer::IsRunBegin() {
	if (GetBufferNumber() == 1) return true;
	return false;
}
bool maestroChnBuffer::IsDataType() {
	if (GetBufferNumber() > 1 && GetBufferNumber() <= fNumberOfChannels / 8 + 1) return true;
	return false;
}
bool maestroChnBuffer::IsRunEnd() {
	if (GetBufferNumber() > fNumberOfChannels / 8 + 1) return true;
	return false;
}
