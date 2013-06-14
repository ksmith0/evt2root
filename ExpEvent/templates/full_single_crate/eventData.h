/*	Author: Karl Smith
 *	Date: June 14, 2013
 *
 */

#ifndef EVENTDATA_H
#define EVENTDATA_H

#define NUM_OF_DATA_SLOTS 17
#define NUM_OF_DATA_CH 32

#include "TObject.h"

///Event structure for a VME crate with 32 channel cards in it.
/**This is a multipurpose setup that can be used to unpack data 
 * from any system with one crate. It will pack the data into a
 * two-dimensional array with indices: slot and ch.
 */ 
class eventData : public TObject
{
	private:
		int rawData[NUM_OF_DATA_SLOTS][NUM_OF_DATA_CH];
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


