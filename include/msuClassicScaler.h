#ifndef MSUCLASSICSCALER_H
#define MSUCLASSICSCALER_H

#include "msuClassicBuffer.h"
#include "msuScalerData.h"

class msuClassicScaler
{
	private:
		msuScalerData *fScalerData;
	public:
		msuClassicScaler();
		void Clear();
		msuScalerData *GetScalerData();
		void ReadScalers(msuClassicBuffer *buffer);

};

#endif
