#include "hribfModule.h"

ClassImp(hribfModule);

void hribfModule::ReadEvent(mainBuffer *buffer, bool verbose) {
	Clear();

	UShort_t datum;
	while ((datum=buffer->GetWord(2)) != 0xFFFF) {
		UShort_t channel = (datum & 0xFF);
		UShort_t value = buffer->GetWord(2);

		if (verbose) {
			printf("\t%#06X Channel: %d \t%#06X Value: %d\n",datum,channel,value,value);
		}

		if (fValues.size() <= channel) fValues.resize(channel+1);
		fValues[channel]=value;
	}

	//Rewind trailer word.
	buffer->SeekBytes(-2);
}

UShort_t hribfModule::GetValue(UShort_t ch) {
	if (fValues.size() > ch) return fValues.at(ch);
	else 0;
}
