#include "Mesytec_ADC_MADC32.h"
#include "mainBuffer.h"

ClassImp(Mesytec_ADC_MADC32)

/**
 */
Mesytec_ADC_MADC32::Mesytec_ADC_MADC32() {
	Clear();
}
/**
 */
void Mesytec_ADC_MADC32::Clear() {
	for (int i=0;i<32;i++) {
		adcValue[i] = 0;
		overflow[i] = false;
		triggerCount = 0;
	}
}

/**The buffer is read out fromt he specified buffer.
 *
 * \param[in] buffer The buffer to be read.
 * \param[in] verbose Verbosity flag.
 *
 * \bug Assumes nsclBuffer::GetLongWord() returns a four byte word.
 */
void Mesytec_ADC_MADC32::ReadEvent(mainBuffer *buffer, bool verbose)
{
	Clear();
	
	//Get Header Long Word
	int datum = buffer->GetLongWord();
	//Get Header type
	int type = (datum & SIG_MASK) >> SIG_SHIFT;
	//Get Module ID (slot)
	int slot = (datum & MDL_ID_MASK) >> MDL_ID_SHIFT;	
	//Get ADC resolution
	resolution = (datum & RESOLUTION_MASK) >> RESOLUTION_SHIFT;
	//Get correct ADC mask
	int datum_mask = 0;
	if (resolution == 0) datum_mask = DATUM_MASK_2K; 
	else if (resolution == 1 || resolution == 2) datum_mask = DATUM_MASK_4K; 
	else if (resolution == 3 || resolution == 4) datum_mask = DATUM_MASK_8K; 
	if (verbose) printf ("\t0x%08X type: %d slot: %2d resolution: %d",datum,type,slot,resolution);		

	if ((BufferType) type == BufferType::MESY_HEADER) {				
		//Get Channel Count
		int count = (datum & CH_CNT_MASK) >> CH_CNT_SHIFT;
		if (verbose) printf(" count: %d\n",count);
		//Loop Over Recorded Channels
		for (int i=0; i<count-1;i++){
			//Get Data Long Word
			datum = buffer->GetLongWord();
			//Get Data Type
			type = (datum & SIG_MASK) >> SIG_SHIFT;					
			//If type data (b00 = 0)
			if ((BufferType) type == BufferType::MESY_DATA){
				//Get Channel Number
				int channel = (datum & CH_NUM_MASK) >> CH_NUM_SHIFT;						
				//Get Overflow
				bool overflowValue = (datum & OVERFLOW_MASK) >> OVERFLOW_SHIFT;
				//Get ADC value
				UShort_t value = (datum & datum_mask) >> DATUM_SHIFT;
				if (verbose) {
					printf("\t0x%08x type: %d ch: %2d value: %4d overflow:%d\n",datum,type,channel,value,overflowValue);
				}
				//Write DATA
				if (channel >= 0 && channel < 32) {
					adcValue[channel] = value;
					overflow[channel] = overflow;
				}
				else
					fprintf(stderr,"ERROR: Invalid Mesytec MADC32 channel: %d!",channel);
			}
		}
		//Get Trailer Long Word
		datum = buffer->GetLongWord();
		type = (datum & SIG_MASK) >> SIG_SHIFT;
		//Get Trigger/Time Stamp
		triggerCount = (datum & TRIG_CNT_MASK) >> TRIG_CNT_SHIFT;
		if (verbose) printf ("\t0x%08X type: %d trig count: %d\n",datum,type,triggerCount);
	}
	else {
		if (verbose) {
			printf("\n");
			fflush(stdout);
		}
		fprintf(stderr,"ERROR: Expected Mesytec MADC-32 header!\n");
	}
}
