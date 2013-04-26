#ifndef MSUSCALERDATA_H
#define MSUSCALERDATA_H

#define NUM_OF_SCALER_CH 32

#include "TObject.h"

class msuScalerData : public TObject
{
	public:
		unsigned int scalerTime;
		int scalerCount; 
		unsigned int scalers[NUM_OF_SCALER_CH];
		msuScalerData();
		void Clear();

	ClassDef(msuScalerData,1)
};

#endif
