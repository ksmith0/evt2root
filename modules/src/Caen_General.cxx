#include "Caen_General.h"

void Caen_General::ReadEvent(nsclBuffer *buffer, eventData *data, bool verbose)
{

	//Indicate order of high and low bits. 
	// Traditional 4096 buffer has bits swapped.	
	bool reverseWords = true;
	if (buffer->IsUSB()) reverseWords = false;

	//Get HEADER
	UInt_t datum = buffer->GetFourByteWord(reverseWords);
	int type = (datum & ALLH_TYPEMASK) >> ALLH_TYPESHIFT;
	int geo = (datum & ALLH_GEOMASK) >> ALLH_GEOSHIFT;
	if (verbose) printf ("\t%#08x type: %d geo: %2d ",datum,type,geo);
	if (type == HEADER) {
		int crate = (datum & HDRH_CRATEMASK) >> HDRH_CRATESHIFT;
		int count = (datum & HDRL_COUNTMASK) >> HDRL_COUNTSHIFT;
		if (verbose) printf("crate: %d count: %d\n",crate,count);
		//Loop over recorded channels
		for (int i=0;i<count;i++) {
			datum = buffer->GetLongWord(reverseWords);
			type = (datum & ALLH_TYPEMASK) >> ALLH_TYPESHIFT;
			geo = (datum & ALLH_GEOMASK) >> ALLH_GEOSHIFT;
			if (type == DATA) {

				int channel = (datum & DATAH_CHANMASK) >> DATAH_CHANSHIFT;
				int value   = (datum & DATAL_DATAMASK);
				bool overflow = (datum & DATAL_OVBIT) != 0;
				bool underflow= (datum & DATAL_UNBIT) != 0;
				bool valid    = (datum& DATAL_VBIT)  != 0; //Only defined for V775
				if (verbose) {
					printf("\t%#08x type: %d geo: %2d ch: %2d value: %4d overflow:%d underflow:%d valid:%d\n",datum,type,geo,channel,value,overflow,underflow,valid);
				}

				if (!overflow && !underflow) {
					//Write DATA here
					data->SetValue(crate,geo,channel,value);
				}
			}
		}
		//Get TRAILER
		datum = buffer->GetFourByteWord(reverseWords);
		type = (datum & ALLH_TYPEMASK) >> ALLH_TYPESHIFT;
		geo = (datum & ALLH_GEOMASK) >> ALLH_GEOSHIFT;
		if (verbose) printf ("\t%#08x type: %d geo: %2d\n",datum,type,geo);
	}
	else if (verbose) printf("\n");
}	
