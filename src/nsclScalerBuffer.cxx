#include "nsclScalerBuffer.h"


nsclScalerBuffer::nsclScalerBuffer() :
	fRunStartTime(0)
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

	//Number of scalers to be read
	UInt_t scalerCount = 0;

	if (buffer->IsRingBuffer()) {
		//Number of seconds elapsed when the scaler interval was started.
		UInt_t startTimeOffset = buffer->GetFourByteWord();	
		//Number of seconds elapsed when the scaler interval was ended.
		UInt_t endTimeOffset = buffer->GetFourByteWord();
		//Time stamp when scalers were read.
		UInt_t timeStamp = buffer->GetFourByteWord();
		scalerCount = buffer->GetFourByteWord();

		//Set scaler times.
		scaler->SetStartTime(fRunStartTime + startTimeOffset);
		scaler->SetEndTime(fRunStartTime + endTimeOffset);

		if (verbose) {
			printf ("\nScalers:\n");
			printf("\tStart Time Offset: %d\n",startTimeOffset);
			printf("\tEnd Time Offset: %d\n",endTimeOffset);
			printf("\tChannels: %d\n",scalerCount);
		}
	}
	else
	{
		scalerCount = buffer->GetNumOfEvents();
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
	}

		for (UInt_t ch = 0; ch < scalerCount;ch++) {
			//unsigned int value = (buffer->GetWord() | ((unsigned int) buffer->GetWord() << 16));
			UInt_t value = buffer->GetFourByteWord();
			scaler->SetValue(ch,value);
			if (verbose) printf("\t0x%08X ch: %d value: %u\n",value,ch,value);
		}
}

void nsclScalerBuffer::DumpScalers(nsclBuffer *buffer) {
	unsigned int eventLength = buffer->GetNumOfWords();
	printf("\nScaler Dump Length: %d\n",eventLength);
	for (int i=0;i<eventLength;i++) { 
		if (i % (20/buffer->GetWordSize()) == 0) printf("\n\t");
		printf("%#0*X ",2*buffer->GetWordSize()+2,buffer->GetWord());
	}
	printf("\n");
	buffer->Rewind(eventLength);
}
