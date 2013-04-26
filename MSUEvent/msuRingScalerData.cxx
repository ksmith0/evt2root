#include "msuRingScalerData.h"
	
ClassImp(msuRingScalerData);

void msuRingScalerData::Clear()
{
	intervalStartOffset = 0;
	intervalEndOffset = 0;
	timeStamp = 0;
	scalerCount = 0;
	for (int ch=0;ch<NUM_OF_SCALER_CH;ch++) scalers[ch] = 0;
}
