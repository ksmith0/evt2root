#ifndef NSCLSCALERBUFFER_H
#define NSCLSCALERBUFFER_H

#include "nsclBuffer.h"
#include "eventScaler.h"

class nsclScalerBuffer
{
	private:
	public:
		nsclScalerBuffer();
		void ReadScalers(nsclBuffer *buffer,eventScaler *scaler);

};

#endif
