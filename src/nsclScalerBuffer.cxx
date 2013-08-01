#include "nsclScalerBuffer.h"


nsclScalerBuffer::nsclScalerBuffer()
{

}


void nsclScalerBuffer::ReadScalers(nsclBuffer *buffer, eventScaler *scaler)
{
	scaler->Reset();
	if (buffer->GetBufferType() != BUFFER_TYPE_SCALERS) {
		fprintf(stderr,"ERROR: Not a scaler buffer!\n");
		return;
	}

	scaler->scalerTime = buffer->GetWord();
	buffer->Forward(9);

	for (int ch = 0; ch < NUM_OF_SCALER_CH;ch++) 
		scaler->scalers[ch] = (buffer->GetWord()) | ((unsigned int) buffer->GetWord() << 16);
	
}
