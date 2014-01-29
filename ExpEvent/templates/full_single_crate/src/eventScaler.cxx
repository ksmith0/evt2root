#include "eventScaler.h"
	
ClassImp(eventScaler);

eventScaler::eventScaler()
{
	Reset();
}

void eventScaler::Reset()
{
	scalerStartTime = 0;
	scalerEndTime = 0;
	for (int ch=0;ch<NUM_OF_SCALER_CH;ch++) scalers[ch] = 0;
}

void eventScaler::SetEndTime(unsigned int time)
{
	scalerEndTime = time;
}

void eventScaler::SetStartTime(unsigned int time)
{
	scalerStartTime = time;
}

void eventScaler::SetValue(int channel,unsigned int value)
{
	if (channel >= 0 && channel < NUM_OF_SCALER_CH) 
		scalers[channel] = value;
	else
		fprintf("stderr","WARNING: Unexpected scaler channel: %d Ignored!\n",channel);
}
