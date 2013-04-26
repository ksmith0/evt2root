#include "msuClassicScaler.h"


msuClassicScaler::msuClassicScaler()
{
	fScalerData = new msuScalerData();
	Clear();
}

void  msuClassicScaler::Clear() {

}
msuScalerData *msuClassicScaler::GetScalerData() 
{
	return fScalerData;
}

void msuClassicScaler::ReadScalers(msuClassicBuffer *buffer)
{
	Clear();
	if (buffer->GetSubEvtType() != SUBEVT_TYPE_SCALERS) {
		fprintf(stderr,"ERROR: Not a scaler subevt!\n");
		return;
	}

	fScalerData->scalerTime = buffer->GetWord();
	buffer->Forward(9);

	for (int ch = 0; ch < NUM_OF_SCALER_CH;ch++) 
		fScalerData->scalers[ch] = (buffer->GetWord()) | ((unsigned int) buffer->GetWord() << 16);
	
}
