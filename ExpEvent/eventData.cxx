#include "eventData.h"

ClassImp(eventData);

eventData::eventData()
{
	Clear();
}
void eventData::Clear()
{
	for (int i=0;i<32;i++) {
		slot12[i] = 0;	
		slot14[i] = 0;	
	}

}
