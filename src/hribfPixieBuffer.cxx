#include "hribfPixieBuffer.h"

hribfPixieBuffer::hribfPixieBuffer(const char* filename) : 
	mainBuffer(filename)
{
}
/**Read out Pixie buffers packed into the HRIBF (HHIRF) LDF files.
 *	The pixie modules dump large amounts of data that will not fit into a single buffer.
 * This data is sliced into spills and packed into separate buffers. These spills need to
 * be reconstructed as they are often chopped in the middle of a module dump. Also, we need
 * to check if the full spill was read out, often the first buffers start in the middle of a
 * spill.
 *
 * \param[in] data Pointer to data class.
 * \param[in] verbose Verbosity flag.
 * \return Returns a nonzero number if things worked as expected.
 *
 * \note If an event has zero length a new buffer is grabbed even if there may be a good event
 *  after the zero length event. This may be ok because an event can not be shroter then one
 *  word given that an event must contain an event length word.
 */
int hribfPixieBuffer::GetSpill(bool verbose /*=false*/)
{

	//Get event position and check if it is past buffer.
	unsigned int eventStartPos = GetPosition();
	if (eventStartPos >= GetNumOfWords()) {
		fflush(stdout);
		fprintf(stderr,"WARNING: Attempted to read event after reaching end of buffer!\n");
		return 0;
	}

	//Check that event has something in it
	UInt_t eventLength = GetEventLength();
	if (eventLength == 0) return 0;

	if (verbose) {
		Rewind(1);
		printf ("\nData Event:\n");
		printf("\t%#010x length: %d\n",GetWord(),eventLength);
	}

	//Get the spill information each dump of the pixie module is split into multiple spills across buffers.
	int numOfSpills = GetWord();
	int spillNumber = GetWord();

	if (verbose) {
		printf("\t%#010X Number of Spills: %d\n",numOfSpills,numOfSpills);
		printf("\t%#010X Spill Number: %d\n",spillNumber,spillNumber);
	}

	//We may start in the middle of a spill.
	if (spillNumber != 0) {
		if (verbose) printf("Unexpected spill number, %d!\n",spillNumber);
		//Fast-forward over the event length minus the header plus one for the boundary
		Forward(eventLength-2);

		//Iterate the event number
		fEventNumber++;
		return -1;
	}

	//Deal with the first spill (set up storage and deal with extra headers).
	//Construct a large array to store data.
	mainBuffer *spillBuffer = new mainBuffer(0,GetBufferSize() * numOfSpills,GetWordSize());

	int lastSpillNumber = 0;
	while (spillNumber < numOfSpills) {
		if (spillNumber != 0) {
			//Check that event has something in it
			UInt_t eventLength = GetEventLength();
			if (eventLength == 0) {
				GetNextBuffer();
				continue;
			}

			if (verbose) {
				Rewind(1);
				printf ("\nData Event:\n");
				printf("\t%#010x length: %d\n",GetWord(),eventLength);
			}

			//Get the spill information each dump of the pixie module is split into multiple spills across buffers.
			numOfSpills = GetWord();
			spillNumber = GetWord();

			if (verbose) {
				printf("\t%#010X Number of Spills: %d\n",numOfSpills,numOfSpills);
				printf("\t%#010X Spill Number: %d\n",spillNumber,spillNumber);
			}

			if (spillNumber != lastSpillNumber + 1) {
				fprintf(stderr,"ERROR: Unexpected spill number %d! Skipping buffer!\n",spillNumber);
				return -1;
			}
		}

		if (spillNumber < numOfSpills) {
			if (verbose) printf("\tPushing spill on data buffer\n");

			//Copy event into spill storage 
			spillBuffer->Copy(&fBuffer[GetPosition()],(eventLength-3));

			//Run the buffer forward.
			Forward(eventLength-3);
		}

		//Fast-forward over extra words
		int remainingWords = eventStartPos + eventLength - GetPosition();
		if (remainingWords > 0) {
			if (verbose) {
				for (int i=0;i<remainingWords;i++) 
					printf("\t%#0*X Extra Word?\n",2*GetWordSize()+2,GetWord());
			}
			else {
				Forward(remainingWords);
			}
		}


		//LDF has 0xFFFFFFFF at end of event
		if (verbose) 
			printf("\t%#0*X Boundary Word\n",2*GetWordSize()+2,GetWord());
		else Forward(1);
	}


	//We grabbed the entire buffer and can now unpack it.
	if (spillNumber+1 == numOfSpills) {
		if (verbose) printf("\n\tEntire spill grabbed, preparing to unpack.\n");

		//Loop over each module
		for(unsigned int moduleNum=0;moduleNum<fModules.size();moduleNum++) {


			while (spillBuffer->GetWritePosition() < spillBuffer->GetBufferSize()) {
				//Read out the current module
				UInt_t moduleStartPos = spillBuffer->GetPosition();
				//Get number of module words
				UInt_t moduleLength = spillBuffer->GetFourByteWord();
				//Get the current module number
				UInt_t moduleNum = spillBuffer->GetFourByteWord();

				if (verbose) {
					printf("\n\tNew Module\n");
					printf("\t%#010X Length of events: %d\n",moduleLength,moduleLength);
					printf("\t%#010X Module Number: %d\n",moduleNum,moduleNum);
				}

				//If the module number is large then this isn't pixie, skip it.
				if (moduleNum > 14) {
					spillBuffer->Forward(moduleLength - 2);
				}

				//Deque for each event from the module in this slot.
				std::deque< baseModule* > slotDeque;
				//Loop over all the words for this module
				while (spillBuffer->GetPosition() < moduleStartPos + moduleLength) {
					if (verbose) printf("\tModule Pos: %d / %d, Spill Pos: %d / %d\n",spillBuffer->GetPosition() - moduleStartPos,moduleLength,spillBuffer->GetPosition(),spillBuffer->GetWritePosition());

					MODULE_LIST *module = new MODULE_LIST();
					module->ReadEvent(spillBuffer,verbose);
					slotDeque->push_back(module);

					//Check how many words were read.
					if (spillBuffer->GetPosition() > spillBuffer->GetWritePosition()
						 || spillBuffer->GetPosition() > moduleStartPos + moduleLength) {
						fflush(stdout);
						fprintf(stderr,"ERROR: Module read too many words! (Module: %d)\n",moduleNum);
						break;
					}
				}
				//Push the module from this slot onto the storage vector.
				fSpillModules.push_back(slotDeque);
			}
		}
	}
}

int hribfPixieBuffer::ReadEvent(eventData *data, bool verbose /*=false*/) 
{
	GetSpill(false);

	data->Reset();

	for (unsigned int slot=0;slot<fSpillModules.size();slot++) {
		
	}

	//Perform calibration
	data->Calibrate();

	//Iterate the event number
	fEventNumber++;

	return 1;
}

void hribfPixieBuffer::ReadScalers(eventScaler *data, bool verbose /*=false*/) 
{
}

int hribfPixieBuffer::GetNextBuffer()
{
	fEventNumber = 0;
	UInt_t nRead = 0;
	this->Clear();

	if (feof(fFP)) {
		return 0;
	}
	else {
		//Get buffer header
		nRead = fread(fBufferHeader, fWordSize, BUFFER_HEADER_SIZE, fFP);
		//Check if we read the right number of words.
		if (nRead != BUFFER_HEADER_SIZE) {
			fprintf(stderr,"ERROR: Buffer header was incomplete! %d words read, expected %d words.\n",nRead,BUFFER_HEADER_SIZE);
			return 0;
		}

		fBufferType = fBufferHeader[0];
		fNumWords = fBufferHeader[1];
		fBufferNumber++;

		//Grab the entire buffer.
		nRead = fread(fBuffer, fWordSize, fBufferSize, fFP);
		if (nRead != fBufferSize) {
			fprintf(stderr,"ERROR: Incorrect read size! Read %d words, expected %d words.\n",nRead,fNumWords);
			return 0;
		}
		fWritePosition = fBufferSize;
		//Grab the number of spills as the number of events!
		if (fBufferType == BUFFER_TYPE_DATA) {
			fNumOfEvents = fBuffer[1];
		}
	}

	return nRead;



}
UInt_t hribfPixieBuffer::GetEventLength() 
{
	UInt_t eventLength = GetWord();

	//LDF event length divided by four
		if (eventLength == 0xFFFFFFFF) eventLength=0;
		else eventLength /= 4;


	//A bad buffer may have a large event length.
	if (eventLength > GetNumOfWords()) {
		fflush(stdout);
		fprintf(stderr,"ERROR: Event length (%u) larger than number of words in buffer (%u)! Skipping Buffer %d!\n",eventLength,GetNumOfWords(),GetBufferNumber());
		Forward(GetNumOfWords() - GetPosition());
		return 0;
	}
	return eventLength;


}

/**
 */
void hribfPixieBuffer::PrintBufferHeader() 
{
	printf("\nBuffer Header Summary:\n");
	std::string type;
	for (int i=0;i<GetWordSize();i++) {
		char letter = (fBufferType >> (8*i)) & 0xFF;
		type.push_back(letter);
	}

	printf("\tBuffer type: %s\n",type.c_str());
	printf("\tNumber of Words: %d\n", fNumWords);
	printf("\tBuffer number: %d\n",fBufferNumber);
	printf("\tNumber of events: %d\n",fNumOfEvents);
}

/**Dumps the hexadecimal representation of the run buffer.
 */
void hribfPixieBuffer::DumpRunBuffer() {

}
void hribfPixieBuffer::DumpScalers() {

}
