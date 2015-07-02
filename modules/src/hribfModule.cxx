#include "hribfModule.h"
#include "mainBuffer.h"

/**Each event consist of a series of 4 byte word containing the channel number of value.
 * The channel number is contained in the lowest order byte and the value is 
 * stored in the highest order two bytes.
 *
 * \param[in] buffer The buffer to extract the event from.
 * \param[in] verbose Verbosity flag.
 */
void hribfModule::ReadEvent(mainBuffer *buffer, bool verbose) {
	Clear();

	while (buffer->GetBufferPositionBytes() < buffer->GetNumOfBytes()) {
		UInt_t datum = buffer->GetWord();

		//break if we find the trailer word.
		if (datum == (UInt_t) -1) {
			if (verbose) printf("\t%#06X Trailer\n",datum);
			break;
		}
		UShort_t channel = datum & 0xFF;
		UShort_t value = datum >> 16;

		if (verbose) {
			printf("\t%#010X Channel: %d Value: %d\n",datum,channel,value);
		}

		if (values.size() <= channel) values.resize(channel+1);
		values[channel]=value;
	}
}

/**If the requested channel is outside the vector the default value of zero is returned.
 *
 * \param[in] ch The channel from which to get the value.
 * \return The value of the specified channel.
 */
UShort_t hribfModule::GetValue(const UShort_t ch) {
	if (values.size() > ch) return values.at(ch);
	else fprintf(stderr,"ERROR: No known channel!\n");
	return 0;
}

ClassImp(hribfModule)

