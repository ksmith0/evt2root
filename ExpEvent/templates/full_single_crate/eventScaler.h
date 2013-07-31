#ifndef EVTSCALER_H
#define EVTSCALER_H

#define NUM_OF_SCALER_CH 32

#include "TObject.h"

class eventScaler : public TObject
{
	public:
		unsigned int scalerTime;
		int scalerCount; 
		unsigned int scalers[NUM_OF_SCALER_CH];

		eventScaler();
		void Reset();

	ClassDef(eventScaler,1)
};

#endif
