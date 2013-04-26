#include "msuRingScaler.h"

msuRingScaler::msuRingScaler()
{
	fScalerData = new msuRingScalerData();
	Clear();
}

void  msuRingScaler::Clear() {

}
msuRingScalerData *msuRingScaler::GetScalerData() {
	return fScalerData; 
}

void msuRingScaler::ReadScalers(msuRingBuffer *buffer)
{
	this->Clear();
	if (buffer->GetSubEvtType() != SUBEVT_TYPE_SCALERS) {
		fprintf(stderr,"ERROR: Not a scaler subevt!\n");
		return;
	}
	
	fScalerData -> intervalStartOffset = buffer->GetWord();
	fScalerData -> intervalEndOffset = buffer->GetWord();
	fScalerData -> timeStamp = buffer->GetWord();
	fScalerData -> scalerCount = buffer->GetWord();

	for (int ch = 0; ch < fScalerData->scalerCount && ch < NUM_OF_SCALER_CH;ch++) 
		fScalerData->scalers[ch] = buffer->GetWord();
	
}
