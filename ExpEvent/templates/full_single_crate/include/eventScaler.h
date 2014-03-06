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

		///Default Constructor
		eventScaler();
		virtual ~eventScaler() {};
		///Reset the scaler values.
		void Reset();
		///Specify the run end time.
		void SetEndTime(unsigned int time);
		///Specify the run start time.
		void SetStartTime(unsigned int time);
		///Set the scaler value.
		void SetValue(int channel, unsigned int value);

	ClassDef(eventScaler,1)
};

#endif
