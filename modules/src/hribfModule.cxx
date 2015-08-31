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
	std::vector<UShort_t> multParameterChannels;

	while (buffer->GetBufferPositionBytes() < buffer->GetNumOfBytes()) {
		UInt_t datum = buffer->GetWord();

		//break if we find the trailer word.
		if (datum == (UInt_t) -1) {
			if (verbose) printf("\t%#06X Trailer\n",datum);
			if (!multParameterChannels.empty()) {
				fprintf(stderr,"ERROR: Multiple values set for buffer %d, event %d, parameter: (Only last values are stored!)\n",buffer->GetBufferNumber(),buffer->GetEventNumber());
				for (auto itr = multParameterChannels.begin(); itr != multParameterChannels.end(); ++itr) {
					if (itr != multParameterChannels.begin()) fprintf(stderr,", ");
					fprintf(stderr,"%3d",*itr);
				}
				fprintf(stderr,"!\n");
			}
			break;
		}
		mult++;
		UShort_t channel = datum & 0x7FFF;
		UShort_t value = datum >> 16;

		if (verbose) {
			printf("\t%#010X Channel: %d Value: %d\n",datum,channel,value);
		}

		if (values.size() <= channel) values.resize(channel+1);
		values[channel]=value;

		if (paramMults.size() <= channel) paramMults.resize(channel+1);
		paramMults[channel]++;
		
		if (paramMults[channel] == 2) multParameterChannels.push_back(channel);
		
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


void hribfModule::Clear() {
	values.clear();
	paramMults.clear();
	mult = 0;
}

ClassImp(hribfModule)

