#include "eventData.h"

ClassImp(eventData);

eventData::eventData()
{
	Clear();
}
void eventData::Clear()
{
	for (int i=0;i<NUM_OF_DATA_SLOTS;i++) {
		for (int j=0;j<NUM_OF_DATA_CH;j++) {
			rawData[i][j] = 0;	
		}
	}
}
void eventData::SetValue(int crate, int slot, int ch, int value)
{
	if (crate==0) {
		if (slot>=0 && slot<NUM_OF_DATA_SLOTS) {
			if (ch>=0 && ch<NUM_OF_DATA_CH) {
				rawData[slot][ch] = value;
			}
		}
	}
}
