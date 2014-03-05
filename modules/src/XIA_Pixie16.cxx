#include "XIA_Pixie16.h"

/**Does not support readout of raw energy sums and baseline, readout of
 * QDC, or readout of a combination of both. Currently stores only the 
 * associated energy. 
 */
void XIA_Pixie16::ReadEvent(nsclBuffer *buffer, eventData *data, bool verbose)
{
	UInt_t datum = buffer->GetFourByteWord();
	UShort_t chanID = (datum & CHANNELID_MASK) >> CHANNELID_SHIFT;
	UShort_t slotID = (datum & SLOTID_MASK) >> SLOTID_SHIFT;
	UShort_t crateID = (datum & CRATEID_MASK) >> CRATEID_SHIFT;
	UShort_t headerLength = (datum & HEADERLENGTH_MASK) >> HEADERLENGTH_SHIFT;
	UShort_t channelLength = (datum & CHANNELLENGTH_MASK) >> CHANNELLENGTH_SHIFT;
	UShort_t overflowCode = (datum & OVERFLOW_MASK) >> OVERFLOW_SHIFT;
	UShort_t finishCode = (datum & FINISHCODE_MASK) >> FINISHCODE_SHIFT;

	if (verbose) {
		printf("\t%#010x ",datum);
		printf("crate: %02d slot: %02d ch: %02d\n",crateID,slotID,chanID);
		printf("\t%*c header length: %2d channel length: %2d\n",10,' ',headerLength,channelLength);
		printf("\t%*c overflow: %d finish code: %d\n",10,' ',overflowCode,finishCode);
	}

	UInt_t timeLow = buffer->GetFourByteWord();
	if (verbose) printf("\t%#010x time stamp low bits\n",timeLow);

	datum = buffer->GetFourByteWord();
	UShort_t timeHigh = datum & LOWER16BIT_MASK;
	UShort_t timeCFD = (datum & UPPER16BIT_MASK)>>16;
	if (verbose) printf("\t%#010x CFD frac. time: %u, time stamp high bits\n",datum,timeCFD);

	ULong_t timestamp = timeLow + (timeHigh << 31);
	if (verbose) printf("\t%*c time stamp: %lu\n",10,' ',timestamp);

	datum = buffer->GetFourByteWord();
	UShort_t energy = datum & LOWER16BIT_MASK; 
	UShort_t traceLength = (datum & UPPER16BIT_MASK) >> 16;
	if (verbose) printf("\t%#010x trace length: %d energy %d\n",datum,traceLength,energy);

	data->SetValue(crateID,slotID,chanID,energy);

	//Determine remaining words in header
	bool readEnergySumsBaseLine = false;
	bool readQDCSums = false;
	if (headerLength == 8)  readEnergySumsBaseLine = true;
	else if (headerLength == 12) readQDCSums = true;
	else if (headerLength == 16) {
		readEnergySumsBaseLine = true;
		readQDCSums = true;
	}

	//Raw energy sums and baseline should be here.
	if (readEnergySumsBaseLine) {
	}
	//Raw QDC sum should be here.
	if (readQDCSums) {
	}

	//Get trace 
	for (int i=0;i<traceLength;i++) {
		buffer->GetFourByteWord();
	}

}
