#ifndef MSUEVENTDATA_H
#define MSUEVENTDATA_H

#define NUM_OF_DATA_SLOTS 17
#define NUM_OF_DATA_CH 32

#include "TObject.h"

///Traditional data acquisiton setup at Univ. of Notre Dame 2013
/**This is a multipurpose setup used at ND for data acquistion. It consists
 * of a ADC in slot ## and a TDC in slot ##
 */ 
class eventData : public TObject
{
	private:
		int tdc[NUM_OF_DATA_CH];
		int adc[NUM_OF_DATA_CH];
	public:
		///Default Constructor.
		eventData();
		///Clear the current values.
		void Clear();
		///Record the value from the specified location.
		void SetValue(int crate, int slot, int ch, int value);



	ClassDef(eventData,1);
};

#endif


