#ifndef MSUEVENTDATA_H
#define MSUEVENTDATA_H

#define NUM_OF_DATA_SLOTS 17
#define NUM_OF_DATA_CH 32

#include "TObject.h"

class msuEventData : public TObject
{
	private:
	public:
		msuEventData();
		void Clear();

		int slot14[32];

		int siDet[29];

		ClassDef(msuEventData,1);
};

#endif
