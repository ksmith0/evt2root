#ifndef MSUCLASSICSCALER_H
#define MSUCLASSICSCALER_H

#include "msuClassicBuffer.h"
#include "eventScaler.h"

class msuClassicScaler
{
	private:
		eventScaler *fScalerData;
	public:
		msuClassicScaler();
		void Clear();
		eventScaler *GetScalerData();
		void ReadScalers(msuClassicBuffer *buffer);

};

#endif
