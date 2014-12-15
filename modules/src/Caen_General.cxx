#include "Caen_General.h"

void Caen_General::ReadEvent(mainBuffer *buffer, bool verbose)
{
	//Get HEADER
	UInt_t datum = buffer->GetWord(4);
	int type = (datum & ALLH_TYPEMASK) >> ALLH_TYPESHIFT;
	int geo = (datum & ALLH_GEOMASK) >> ALLH_GEOSHIFT;
	if (verbose) printf ("\t%#010X type: %d geo: %2d ",datum,type,geo);
	if (type == HEADER) {
		int crate = (datum & HDRH_CRATEMASK) >> HDRH_CRATESHIFT;
		int count = (datum & HDRL_COUNTMASK) >> HDRL_COUNTSHIFT;
		if (verbose) printf("crate: %d count: %d\n",crate,count);
		//Loop over recorded channels
		for (int i=0;i<count;i++) {
			datum = buffer->GetWord(4);
			type = (datum & ALLH_TYPEMASK) >> ALLH_TYPESHIFT;
			geo = (datum & ALLH_GEOMASK) >> ALLH_GEOSHIFT;

			if (verbose) printf("\t%#010X type: %d geo: %2d ",datum,type,geo);
			if (type == DATA) {

				int channel = (datum & DATAH_CHANMASK) >> DATAH_CHANSHIFT;
				int value   = (datum & DATAL_DATAMASK);
				bool overflow = (datum & DATAL_OVBIT) != 0;
				bool underflow= (datum & DATAL_UNBIT) != 0;
				bool valid    = (datum& DATAL_VBIT)  != 0; //Only defined for V775
				if (verbose) {
					printf("ch: %2d value: %4d overflow:%d underflow:%d valid:%d\n",channel,value,overflow,underflow,valid);
				}
				//Write Data
			}
			else if(verbose) printf("\n");
		}
		//Get TRAILER
		datum = buffer->GetWord(4);
		type = (datum & ALLH_TYPEMASK) >> ALLH_TYPESHIFT;
		geo = (datum & ALLH_GEOMASK) >> ALLH_GEOSHIFT;
		if (verbose) printf ("\t%#08X type: %d geo: %2d\n",datum,type,geo);
	}
	else if (verbose) printf("\n");
}	
