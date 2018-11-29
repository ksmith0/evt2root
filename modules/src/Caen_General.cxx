#include "Caen_General.h"
#include "mainBuffer.h"

ClassImp(Caen_General)

void Caen_General::Clear() {
	values.clear();
	overflowBits.clear();
	underflowBits.clear();
	validBits.clear();
	geoAddress = -1;
	crate = -1;
	moduleMultiplicity = 0;
}
void Caen_General::ReadEvent(mainBuffer *buffer, bool verbose)
{
	Clear();

	//Get HEADER
	UInt_t datum_ = buffer->GetWord(4);
	int type_ = (datum_ & ALLH_TYPEMASK) >> ALLH_TYPESHIFT;
	int geo_ = (datum_ & ALLH_GEOMASK) >> ALLH_GEOSHIFT;
	geoAddress = geo_;
	if (verbose) printf ("\t%#010X type: %d geo: %2d ",datum_,type_,geoAddress);

	if (type_ == HEADER) {
		crate = (datum_ & HDRH_CRATEMASK) >> HDRH_CRATESHIFT;
		moduleMultiplicity = (datum_ & HDRL_COUNTMASK) >> HDRL_COUNTSHIFT;
		if (verbose) printf("crate: %d count: %d\n",crate,moduleMultiplicity);
		//Loop over recorded channels
		for (int i=0;i<moduleMultiplicity;i++) {
			datum_ = buffer->GetWord(4);
			type_ = (datum_ & ALLH_TYPEMASK) >> ALLH_TYPESHIFT;
			geo_ = (datum_ & ALLH_GEOMASK) >> ALLH_GEOSHIFT;

			if (verbose) printf("\t%#010X type: %d geo: %2d ",datum_,type_,geo_);
			if (type_ == DATA) {

				int channel_ = (datum_ & DATAH_CHANMASK) >> DATAH_CHANSHIFT;
				int value_   = (datum_ & DATAL_DATAMASK);
				bool overflow_  = (datum_ & DATAL_OVBIT) != 0;
				bool underflow_ = (datum_ & DATAL_UNBIT) != 0;
				bool valid_     = (datum_ & DATAL_VBIT)  != 0; //Only defined for V775

				if ((int) values.size() <= channel_) values.resize(channel_+1);
				if ((int) overflowBits.size() <= channel_) overflowBits.resize(channel_+1);
				if ((int) underflowBits.size() <= channel_) underflowBits.resize(channel_+1);
				if ((int) validBits.size() <= channel_) validBits.resize(channel_+1);
				values[channel_] = value_;
				overflowBits[channel_] = overflow_;
				underflowBits[channel_] = underflow_;
				validBits[channel_] = valid_;
								

				if (verbose) {
					printf("ch: %2d value: %4d overflow:%d underflow:%d valid:%d\n",channel_,value_,overflow_,underflow_,valid_);
				}
				//Write Data
			}
			else if(verbose) printf("\n");
		}
		//Get TRAILER
		datum_ = buffer->GetWord(4);
		type_ = (datum_ & ALLH_TYPEMASK) >> ALLH_TYPESHIFT;
		geo_ = (datum_ & ALLH_GEOMASK) >> ALLH_GEOSHIFT;
		if (verbose) printf ("\t%#08X type: %d geo: %2d\n",datum_,type_,geo_);
	}
	else if (verbose) printf("\n");
}	

UInt_t Caen_General::GetValue(UShort_t channel) {
	if (values.size() > channel) return values.at(channel);
	return 0;
}
