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

		int slot12[NUM_OF_DATA_CH];
		int slot14[NUM_OF_DATA_CH];


	ClassDef(msuEventData,1);
};

#endif
