#include "nsclEventBuffer.h"


nsclEventBuffer::nsclEventBuffer() {
	fData = new eventData();
}
void nsclEventBuffer::Clear()
{
	
}
/**Typical event buffer:
 * 1. Event word count (includes self).
 * 2. Bit register
 * 3. LAM mask longword
 * 4. Event packets ...
 *
 * VM-USB seems to be missing the bit register and the LAM mask longword. 
 * Occasionaly the VM-USB word count seems to ignore the event word count
 * word (non-inclusive). To ensure this word is treated correctly define
 * the preproccesor variable \c NONINCLUSIVE_EVT_WORD_COUNT.
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
#ifdef NONINCLUSIVE_EVT_WORD_CNT
	eventLength++;
	if (verbose) printf("+1");
#endif
	if (verbose) printf("\n");

#ifndef VM_USB
	unsigned int bitRegister = buffer->GetWord();
	readWords++;
	if (verbose) printf("\t0x%04X Bit Register?\n",bitRegister);
	for (int i=0;i<bitRegister;i++) {
		unsigned int lamMask = buffer->GetWord();
		readWords++;
		if (verbose) printf("\t0x%08X LAM %d mask?\n",lamMask,i);
	}
#endif

	int crate = -1;
	for (int i=(readWords+1)/2;i<eventLength/2;i++) {
		int warningSlot = -1;
		int datum = buffer->GetLongWord();
		readWords++; readWords++;
		int type = (datum & ALLH_TYPEMASK) >> ALLH_TYPESHIFT;
		int slot = (datum & ALLH_GEOMASK) >> ALLH_GEOSHIFT;
		if (verbose) {
			printf ("\t0x%08X type: %d slot: %2d ",datum,type,slot);
		}

		if (type == DATA) {
	
			int channel = (datum & DATAH_CHANMASK) >> DATAH_CHANSHIFT;
			int value   = (datum & DATAL_DATAMASK);
			bool overflow = (datum & DATAL_OVBIT) != 0;
			bool underflow= (datum & DATAL_UNBIT) != 0;
			bool valid    = (datum& DATAL_VBIT)  != 0;
			if (verbose) {
				printf("ch: %2d value: %4d overflow:%d underflow:%d valid:%d",channel,value,overflow,underflow,valid);
			}
			
			if (!overflow && !underflow) {
				//Write DATA here
				fData->SetValue(crate,slot,channel,value);
			}
		}
		else if (type == HEADER) {
			crate = (datum & HDRH_CRATEMASK) >> HDRH_CRATESHIFT;
			if (verbose) printf("crate: %d ",crate);
		}
		if (verbose) {
			printf("\n");
		}
	}
	while (eventLength > readWords) {
		int extraWord = buffer->GetWord();
		readWords++;
		if (verbose) printf("\t0x%04X Extra Word?\n",extraWord);
	}

}

void nsclEventBuffer::DumpEvent(nsclBuffer *buffer) {
	int eventLength = buffer->GetWord();
	printf("\nEvent Dump Length: %d",eventLength);
#ifdef NONINCLUSIVE_EVT_WORD_CNT
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
