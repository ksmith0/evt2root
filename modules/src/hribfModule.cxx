#include "hribfModule.h"
#include "mainBuffer.h"

ClassImp(hribfModule);

/**Each event consist of a series of 4 byte word containing the channel number of value.
 * The channel number is contained in the lowest order byte and the value is 
 * stored in the highest order two bytes.
 *
 * \param[in] buffer The buffer to extract the event from.
 * \param[in] Verbosity flag.
 */
void hribfModule::ReadEvent(mainBuffer *buffer, bool verbose) {
	Clear();

	while (buffer->GetBufferPositionBytes() < buffer->GetNumOfBytes()) {
		UInt_t datum = buffer->GetWord();

		//break if we find the trailer word.
		if (datum == -1) {
			if (verbose) printf("\t%#06X Trailer\n",datum);
			break;
		}
		UShort_t channel = datum & 0xFF;
		UShort_t value = datum >> 16;

		if (verbose) {
			printf("\t%#010X Channel: %d Value: %d\n",datum,channel,value);
		}

		if (fValues.size() <= channel) fValues.resize(channel+1);
		fValues[channel]=value;
	}
}

UShort_t hribfModule::GetValue(UShort_t ch) {
	if (fValues.size() > ch) return fValues.at(ch);
	return 0;
}
