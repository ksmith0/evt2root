#ifndef NSCLSCALERBUFFER_H
#define NSCLSCALERBUFFER_H

#include "nsclBuffer.h"
#include "eventScaler.h"

class nsclScalerBuffer
{
	private:
		time_t fRunStartTime;
	public:
		nsclScalerBuffer();
		void ReadScalers(nsclBuffer *buffer,eventScaler *scaler, bool verbose = false);
		void SetRunStartTime(time_t runStartTime) {fRunStartTime = runStartTime;}
		void DumpScalers(nsclBuffer *buffer);

};

#endif
