#include "nsclScalerBuffer.h"


nsclScalerBuffer::nsclScalerBuffer()
{

}

/**Typical scaler buffer:
 * 1. End time low order.
 * 2. End time high order.
 * 3. Three unused words.
 * 4. Start time low order.
 * 5. Start time high order.
 * 6. Three unsued words.
 * 7. Incremental scalers.
 */
void nsclScalerBuffer::ReadScalers(nsclBuffer *buffer, eventScaler *scaler, bool verbose)
{
	scaler->Reset();
	if (buffer->GetBufferType() != BUFFER_TYPE_SCALERS) {
		fprintf(stderr,"ERROR: Not a scaler buffer!\n");
		return;
	}

	if (verbose) {
		printf ("\nScalers: Channels: %d\n",buffer->GetNumOfEvents());
	}

	unsigned int time;
	time = (buffer->GetWord() | ((unsigned int) buffer->GetWord() << 16));
	scaler->SetEndTime(time);
	if (verbose) {
		printf("\t0x%08X End time: %u\n",time,time);
		for (int i=0;i<3;i++) printf("\t0x%04X Unused word\n",buffer->GetWord());
	}
	else buffer->Forward(3);
	time = (buffer->GetWord() | ((unsigned int) buffer->GetWord() << 16));
	scaler->SetStartTime(time);
	if (verbose) {
		printf("\t0x%08X Start time: %u\n",time,time);
		for (int i=0;i<3;i++) printf("\t0x%04X Unused word\n",buffer->GetWord());
	}
	else buffer->Forward(3);

	for (int ch = 0; ch < buffer->GetNumOfEvents();ch++) {
		unsigned int value = (buffer->GetWord() | ((unsigned int) buffer->GetWord() << 16));
		scaler->SetValue(ch,value);
		if (verbose) printf("\t0x%08X ch: %d value: %u\n",value,ch,value);
	}
	
}

void nsclScalerBuffer::DumpScalers(nsclBuffer *buffer) {
	int eventLength = 2*buffer->GetNumOfEvents()+10;
	printf("\nScaler Dump Length: %d\n",eventLength);
	for (int i=0;i<eventLength;i++) { 
		if (i % 10 == 0) printf("\n\t");
		printf("0x%04X ",buffer->GetWord());
	}
	printf("\n");
	buffer->Rewind(eventLength);
}
