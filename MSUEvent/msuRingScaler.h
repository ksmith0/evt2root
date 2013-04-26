#ifndef MSURINGSCALER_H
#define MSURINGSCALER_H

#include "msuRingBuffer.h"
#include "msuRingScalerData.h"

class msuRingScaler 
{
	private:
		msuRingScalerData *fScalerData;
	public:
		msuRingScaler();
		void Clear();
		msuRingScalerData *GetScalerData();
		void ReadScalers(msuRingBuffer *buffer);

};

#endif
