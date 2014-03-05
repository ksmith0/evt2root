#include "nsclEventBuffer.h"


nsclEventBuffer::nsclEventBuffer() {
	MODULE_LIST(MODULE_FUNCTION)
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
 * \param buffer Pointer to the buffer being read.
 * \param data Pointer to eventData class to be filled.
 * \param verbose Verbosity flag. 
 *
 */
void nsclEventBuffer::ReadEvent(nsclBuffer *buffer, eventData *data, bool verbose) {
	data->Reset();

	unsigned int eventStartPos = buffer->GetPosition();
	unsigned int eventLength = buffer->GetWord();
	//Ring buffer event length is in two byte words
	if (buffer->IsRingBuffer()) eventLength /= buffer->GetWordSize()/2;
	if (verbose) {
		printf ("\nData Event:\n");
		printf("\t%#06x length: %d",eventLength,eventLength);
	}

	if (buffer->IsUSB()) {
		eventLength++;
		if (verbose) printf("+1");
	}	
	if (verbose) printf("\n");

	//Loop over remaining words. 
	//	Subtract one to catch single small word at end of buffer.
	for(unsigned int module=0;module<modules.size();module++) {
		if (!buffer->IsUSB() && !buffer->IsRingBuffer()) {
			int packetLength = buffer->GetWord();
			int packetTag = buffer->GetWord();
			if (verbose) {
				printf("\t%#06x Packet length: %d\n",packetLength,packetLength);
				printf("\t%#06x Packet tag: %d\n",packetTag,packetTag);
			}
			if (packetLength <= 2) continue;
/*
		//Special code for ND IO Register.
		if (packetTag == 10) {
			for (int i=2;i<packetLength;i++) {
				int value = buffer->GetWord();
				readWords++;
				if (verbose) {
					printf("\t0x%04X value: %4d\n",value,value);
				}
				data->SetValue(0,13,-1,value);
			}
		}
*/

		}
	
		int datum;
		bool boundaryWord = false;

		//If we are using the USB version we need to check if we
		// are at boundary word.
		//	(This occurs when the module is not readout and the
		//	 header and trailer are not written into the buffer)
		if (buffer->IsUSB()) {
			int datum = buffer->GetFourByteWord();
			if (datum == 0xFFFFFFFF) boundaryWord = true;
			buffer->RewindBytes(4);
		}

		//Read out the current module
		if (!boundaryWord) modules[module](buffer,data,verbose);

		//Check how many words were read.
		if (buffer->GetPosition() - eventStartPos > eventLength) {
			fprintf(stderr,"ERROR: Module read too many words!\n");
		}

		//USB version has module boundary word of 0xffffffff
		if (buffer->IsUSB()) {
			int datum = buffer->GetFourByteWord();
			if (verbose) printf("\t%#04x Boundary Word\n",datum);
		}
	}

	//Fastforward over extra words
	int remainingWords = eventStartPos + eventLength - buffer->GetPosition();
	if (remainingWords > 0) {
		if (verbose) {
			for (int i=0;i<remainingWords;i++) 
				printf("\t%#0*x Extra Word?\n",2*buffer->GetWordSize()+2,buffer->GetWord());
		}
		else {
			buffer->Forward(remainingWords);
		}
	}
	

	//Perform calibration
	data->Calibrate();

}
void Prep(nsclBuffer *buffer) {


}
void nsclEventBuffer::DumpEvent(nsclBuffer *buffer) {
	nsclBuffer::word datum = buffer->GetWord();
	nsclBuffer::word eventLength = datum;
	//Ring buffer gives length in 2 byte words
	if (buffer->IsRingBuffer()) eventLength /= buffer->GetWordSize()/2; 
	printf("\nEvent Dump Length: %d",eventLength);

	if (buffer->IsUSB()) {
		eventLength++;
		printf("+1");
	}
	unsigned short wordSize = buffer->GetWordSize();
	printf("\n\t%#0*x ",2*wordSize+2,datum);
	for (int i=1;i<eventLength;i++) { 
		if (i % (20/wordSize) == 0) printf("\n\t");
		printf("%#0*x ",2*wordSize+2,buffer->GetWord());
	}
	printf("\n");
	buffer->Rewind(eventLength);
}
