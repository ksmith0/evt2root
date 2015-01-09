#include "XIA_Pixie16.h"
#include "mainBuffer.h"

ClassImp(XIA_Pixie16);

/**Unpack the Pizie16 modules according to the Rev D, Rev F format.
 * 
 * \bug Does not support readout of readout of QDC sums. 
 */
void XIA_Pixie16::ReadEvent(mainBuffer *buffer, bool verbose)
{
	UInt_t datum = buffer->GetWord(4);
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

	UInt_t timeLow = buffer->GetWord(4);
	if (verbose) printf("\t%#010x time stamp low bits\n",timeLow);

	datum = buffer->GetWord(4);
	UShort_t timeHigh = datum & LOWER16BIT_MASK;
	UShort_t timeCFD = (datum & UPPER16BIT_MASK)>>16;
	if (verbose) printf("\t%#010x CFD frac. time: %u, time stamp high bits\n",datum,timeCFD);

	ULong_t timestamp = timeLow + (timeHigh << 31);
	if (verbose) printf("\t%*c time stamp: %lu\n",10,' ',timestamp);

	datum = buffer->GetWord(4);
	UShort_t energy = datum & LOWER16BIT_MASK; 
	UShort_t traceLength = ((datum & UPPER16BIT_MASK) >> 16) / 2;
	if (verbose) printf("\t%#010x trace length: %d energy %d\n",datum,traceLength,energy);

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
		UInt_t trailingEnergySum = buffer->GetWord(4);
		UInt_t leadingEnergySum = buffer->GetWord(4);
		UInt_t gapEnergySum = buffer->GetWord(4);
		UInt_t baseline = buffer->GetWord(4);
		if (verbose) {
			printf("\t%#010x Trailing Energy Sum: %d\n",trailingEnergySum,trailingEnergySum);
			printf("\t%#010x Leading Energy Sum: %d\n",leadingEnergySum,leadingEnergySum);
			printf("\t%#010x Gap Energy Sum: %d\n",gapEnergySum,gapEnergySum);
			printf("\t%#010x Baseline Value: %d\n",baseline,baseline);

		}
	}
	//Raw QDC sum should be here.
	if (readQDCSums) {
		fflush(stdout);
		fprintf(stderr,"ERROR: QDC Sums not supported!\n");
	}

	//Get trace 
	int wordSize = buffer->GetWordSize();
	for (int i=0;i<traceLength;i++) {
		datum=buffer->GetWord(4);
		if (verbose) {
			if (i==0) printf("\t");
			else if (i % (20/wordSize) == 0) printf("\n\t");
			printf("%#0*x ",2*wordSize+2,datum);
		}
	}
	if (verbose && traceLength > 0) printf("\n");


}
