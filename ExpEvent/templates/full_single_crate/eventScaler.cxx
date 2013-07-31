#include "eventScaler.h"
	
ClassImp(eventScaler);

eventScaler::eventScaler()
{
	Reset();
}

void eventScaler::Reset()
{
	scalerCount = 0;
	scalerTime = 0;
	for (int ch=0;ch<NUM_OF_SCALER_CH;ch++) scalers[ch] = 0;
}
