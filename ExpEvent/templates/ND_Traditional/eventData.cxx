#include "eventData.h"

ClassImp(eventData);

eventData::eventData()
{
	Reset();
}
void eventData::Reset()
{
	for (int i=0;i<32;i++) {
		tdc[i] = 0;	
		adc[i] = 0;	
	}

}
void eventData::SetValue(int crate, int slot, int ch, int value)
{
	if (crate==0) {
		if (ch>=0 && ch<32) {
			if (slot==12) tdc[ch] = value;
			else if (slot==14) adc[ch] = value;
		}
	}
}
