#ifndef EVENTSCALER_H
#define EVENTSCALER_H

#define NUM_OF_SCALER_CH 32

#include "TObject.h"

class eventScaler : public TObject
{
	private:
		unsigned int scalerTime;
		int scalerCount; 
		unsigned int scalers[NUM_OF_SCALER_CH];
	public:
		eventScaler();
		void Reset();

	ClassDef(eventScaler,1)
};

#endif
