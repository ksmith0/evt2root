#include "nsclEventBuffer.h"


nsclEventBuffer::nsclEventBuffer() {
	fData = new eventData();
}
void nsclEventBuffer::Clear()
{
	
}
/**Typical event buffer:
 * 1. Event word count. (See Below)
 * 2. Bit register.
 * 3. LAM mask longword.
 * 4. Event packets ...
 *
 * The VM-USB word count is non-inclusive, while other systems it is 
 * inclusive. VM-USB seems to be missing the bit register and the LAM 
 * mask longword. Non-USB version also has a preheader longword with the
 * number of words for that module included. This longword appears for
 * every event, even if the module is completely zero-suppressed. VM-USB 
 * systems have a longword with all bits set following each module. This
 * appears for every event, even if the module is completely 
 * zero-suppressed.
 *
 * \param buffer Pointer to the buffer being read.
 * \param verbose Verbosity flag. 
 */
void nsclEventBuffer::ReadEvent(nsclBuffer *buffer, bool verbose) {
	fData->Clear();
	int readWords=0;

	int eventLength = buffer->GetWord();
	readWords++;
	if (verbose) {
		printf ("\nData Event:\n");
		printf("\t0x%04X length: %d",eventLength,eventLength);
	}
#ifdef VM_USB
	eventLength++;
	if (verbose) printf("+1");
#endif
	if (verbose) printf("\n");

#ifndef VM_USB
	unsigned int bitRegister = buffer->GetWord();
	readWords++;
	if (verbose) printf("\t0x%04X Bit Register\n",bitRegister);
	for (int i=1;i<bitRegister;i++) {
		unsigned int lamMask = buffer->GetWord();
		readWords++; 
		if (verbose) printf("\t0x%04X LAM mask %d\n",lamMask,i);
	}
#endif

	int crate = -1;
	//Loop over remaining words. 
	//	Subtract one to catch single small word at end of buffer.
	while ((eventLength-1)>readWords) {
		int datum;
#ifndef VM_USB
		int packetLength = buffer->GetWord();
		readWords++; 
		int packetTag = buffer->GetWord();
		if (verbose) {
			printf("\t0x%04X Packet length: %d\n",packetLength);
			printf("\t0x%04X Packet tag: %d\n",packetTag);
		}
		if (packetLength <= 2) continue;
#endif
		//Get HEADER
		datum = buffer->GetLongWord();
		readWords++; readWords++;
		int type = (datum & ALLH_TYPEMASK) >> ALLH_TYPESHIFT;
		int slot = (datum & ALLH_GEOMASK) >> ALLH_GEOSHIFT;
		if (verbose) printf ("\t0x%08X type: %d slot: %2d ",datum,type,slot);
		if (type == HEADER) {
			crate = (datum & HDRH_CRATEMASK) >> HDRH_CRATESHIFT;
			int count = (datum & HDRL_COUNTMASK) >> HDRL_COUNTSHIFT;
			if (verbose) printf("crate: %d count: %d\n",crate,count);
			//Loop over recorded channels
			for (int i=0;i<count;i++) {
				datum = buffer->GetLongWord();
				readWords++; readWords++;
				type = (datum & ALLH_TYPEMASK) >> ALLH_TYPESHIFT;
				slot = (datum & ALLH_GEOMASK) >> ALLH_GEOSHIFT;
				if (type == DATA) {

					int channel = (datum & DATAH_CHANMASK) >> DATAH_CHANSHIFT;
					int value   = (datum & DATAL_DATAMASK);
					bool overflow = (datum & DATAL_OVBIT) != 0;
					bool underflow= (datum & DATAL_UNBIT) != 0;
					bool valid    = (datum& DATAL_VBIT)  != 0;
					if (verbose) {
						printf("\t0x%08X type: %d slot: %2d ch: %2d value: %4d overflow:%d underflow:%d valid:%d\n",datum,type,slot,channel,value,overflow,underflow,valid);
					}

					if (!overflow && !underflow) {
						//Write DATA here
						fData->SetValue(crate,slot,channel,value);
					}
				}
			}
			//Get TRAILER
			datum = buffer->GetLongWord();
			readWords++; readWords++;
			type = (datum & ALLH_TYPEMASK) >> ALLH_TYPESHIFT;
			slot = (datum & ALLH_GEOMASK) >> ALLH_GEOSHIFT;
			if (verbose) printf ("\t0x%08X type: %d slot: %2d\n",datum,type,slot);
		}
		else if (verbose) printf("\n");
	}
	while (eventLength > readWords) {
		int extraWord = buffer->GetWord();
		readWords++;
		if (verbose) printf("\t0x%04X Extra Word?\n",extraWord);
	}
	if (verbose) printf("\n");

}

void nsclEventBuffer::DumpEvent(nsclBuffer *buffer) {
	int eventLength = buffer->GetWord();
	printf("\nEvent Dump Length: %d",eventLength);
#ifdef VM_USB
	eventLength++;
	printf("+1");
#endif
	printf("\n");
	printf("\t0x%04X ",eventLength);
	for (int i=1;i<eventLength;i++) { 
		if (i % 10 == 0) printf("\n\t");
		printf("0x%04X ",buffer->GetWord());
	}
	printf("\n");
	buffer->Rewind(eventLength);
}
eventData *nsclEventBuffer::GetEventData() 
{
	return fData;
}
