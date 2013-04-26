#include "msuScalerData.h"
	
ClassImp(msuScalerData);

msuScalerData::msuScalerData()
{
	Clear();
}

void msuScalerData::Clear()
{
	scalerCount = 0;
	scalerTime = 0;
	for (int ch=0;ch<NUM_OF_SCALER_CH;ch++) scalers[ch] = 0;
}
