#ifndef NSCLSCALERBUFFER_H
#define NSCLSCALERBUFFER_H

#include "nsclBuffer.h"
#include "eventScaler.h"

class nsclScalerBuffer
{
	private:
		eventScaler *fScalerData;
	public:
		nsclScalerBuffer();
		void Clear();
		eventScaler *GetScalerData();
		void ReadScalers(nsclBuffer *buffer);

};

#endif
