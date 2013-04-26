#include "msuEventData.h"

ClassImp(msuEventData);

msuEventData::msuEventData()
{
	Clear();
}
void msuEventData::Clear()
{
	for (int i=0;i<32;i++)
		slot14[i] = 0;	

	for (int i=0;i<29;i++)
		siDet[i] = 0;
}
