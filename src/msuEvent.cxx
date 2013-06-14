#include "msuEvent.h"


msuEvent::msuEvent() {
	fData = new eventData();
}
void msuEvent::Clear()
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
void msuEvent::ReadEvent(msuClassicBuffer *buffer, bool verbose) {
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

	int startData = 0;
#ifndef VM_USB
	unsigned int bitRegister = buffer->GetWord();
	readWords++;
	if (verbose) printf("\t0x%04X Bit Register?\n",bitRegister);
	unsigned int lamMask = buffer->GetLongWord();
	readWords++; readWords++;
	if (verbose) printf("\t0x%08X LAM mask?\n",lamMask);
	startData = 2;	
#endif

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
				//ADC slot == 14
				if (slot == 14) { 

					if (channel > 31) {
						fprintf(stderr,"\nERROR: Channel %d invalid!\n",channel);
						fData->Clear();
						return;
					}

					if (channel >= 0 && channel < 32)
						fData->slot14[channel] = value;
				}
				//TDC slot == 14
				/*
				else if (slot == 14) {
					if (channel > 31) {
						fprintf(stderr,"\nERROR: Channel %d invalid!\n",channel);
						fData->Clear();
						return;
					}

					fData->slot16[channel] = value;

				}
				*/
#ifdef UNKNOWN_SLOT_WARNING
				else { 
					if (slot != warningSlot) {
						fprintf(stderr,"WARNING: Buffer: %d Unknown slot %d\n",buffer->GetBufferNumber(),slot);
						warningSlot = slot;
					}
					fprintf (stderr,"0x%08X type: %d slot: %d ",datum,type,slot);
					fprintf(stderr,"ch: %d value: %d overflow:%d underflow:%d valid:%d\n",channel,value,overflow,underflow,valid);
				}
#endif
			}
		}
		else if (type == HEADER) {
			int crate = (datum & HDRH_CRATEMASK) >> HDRH_CRATESHIFT;
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

void msuEvent::DumpEvent(msuClassicBuffer *buffer) {
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
eventData *msuEvent::GetEventData() 
{
	return fData;
}
