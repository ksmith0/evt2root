#ifndef EVENTDATA_H
#define EVENTDATA_H

#include "TObject.h"

///Traditional data acquisiton setup at Univ. of Notre Dame 2013
/**This is a multipurpose setup used at ND for data acquistion. It consists
 * of a ADC in slot ## and a TDC in slot ##
 */ 
class eventData : public TObject
{
	private:
		int tdc[32];
		int adc[32];
	public:
		///Default Constructor.
		eventData();
		///Reset the current values.
		void Reset();
		///Record the value from the specified location.
		void SetValue(int crate, int slot, int ch, int value);



	ClassDef(eventData,1);
};

#endif


