#ifndef MSURINGSCALERDATA_H
#define MSURINGSCALERDATA_H

#define NUM_OF_SCALER_CH 330

#include "TObject.h"

class msuRingScalerData : public TObject
{
	public:
		int intervalStartOffset;
		int intervalEndOffset;
		time_t timeStamp;
		int scalerCount; 
		int scalers[NUM_OF_SCALER_CH];
		void Clear();

	ClassDef(msuRingScalerData,1)
};

#endif
