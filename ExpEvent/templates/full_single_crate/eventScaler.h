#ifndef EVENTSCALER_H
#define EVENTSCALER_H

#define NUM_OF_SCALER_CH 32

#include "TObject.h"

class eventScaler : public TObject
{
	public:
		unsigned int scalerStartTime;
		unsigned int scalerEndTime;
		unsigned int scalers[NUM_OF_SCALER_CH];

		eventScaler();
		void Reset();
		void SetEndTime(unsigned int time);
		void SetStartTime(unsigned int time);
		void SetValue(int channel, unsigned int value);

	ClassDef(eventScaler,1)
};

#endif
