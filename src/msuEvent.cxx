#include "msuEvent.h"


msuEvent::msuEvent() {
	fData = new msuEventData();
}
void msuEvent::Clear()
{
	
}
void msuEvent::ReadEvent(msuClassicBuffer *buffer) {
	int datum=0;
	int type;
	int slot;
	bool verbose = false;

	int eventLength = buffer->GetWord();
	
	fData->Clear();

	if (verbose) {
		printf ("New Event length:%d\n",eventLength);
		fflush(stdout);
	}
	for (int i=0;i<eventLength / 2;i++) {
		datum = buffer->GetLongWord();
		type = (datum & ALLH_TYPEMASK) >> ALLH_TYPESHIFT;
		slot = (datum & ALLH_GEOMASK) >> ALLH_GEOSHIFT;
		if (verbose) {
			printf ("0x%08X type: %d slot: %2d ",datum,type,slot);
			fflush(stdout);
		}

		if (type == DATA) {
	
			int channel = (datum & DATAH_CHANMASK) >> DATAH_CHANSHIFT;
			int value   = (datum & DATAL_DATAMASK);
			bool overflow = (datum & DATAL_OVBIT) != 0;
			bool underflow= (datum & DATAL_UNBIT) != 0;
			bool valid    = (datum& DATAL_VBIT)  != 0;
			if (verbose) {
				printf("ch: %2d value: %4d overflow:%d underflow:%d valid:%d",channel,value,overflow,underflow,valid);
				fflush(stdout);
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
				else { 
					fprintf(stderr,"ERROR: Unknown slot %d\n",slot);
					fprintf (stderr,"0x%08X type: %d slot: %d ",datum,type,slot);
					fprintf(stderr,"ch: %d value: %d overflow:%d underflow:%d valid:%d\n",channel,value,overflow,underflow,valid);
				}
			}
		}
		else if (type == HEADER) {
			int crate = (datum & HDRH_CRATEMASK) >> HDRH_CRATESHIFT;
			printf("crate: %d ",crate);
		}
		if (verbose) {
			printf("\n");
			fflush(stdout);
		}
	}
}

void msuEvent::DumpEvent(msuClassicBuffer *buffer) {
	int eventLength = buffer->GetWord();
	printf("Event Length: %d\n",eventLength);
	printf("0x%04X ",eventLength);
	for (int i=1;i<=eventLength;i++) { 
		if ((i) % 10 == 0) {
			printf("\n");
		}
		printf("0x%04X ",buffer->GetWord());
	}
	printf("\n");
	fflush(stdout);

}
msuEventData *msuEvent::GetEventData() 
{
	return fData;
}
