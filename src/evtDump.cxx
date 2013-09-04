#include "nsclBuffer.h"
#include "nsclScalerBuffer.h"
#include "nsclRunBuffer.h"
#include "nsclEventBuffer.h"
#include <stdlib.h>

int main (int argc, char *argv[])
{
	if (argc < 2 || argc > 3) {
		fprintf(stderr,"Usage: %s [input.evt]\n",argv[0]);
		return 1;
	}

	nsclBuffer *buffer;
	if (argc == 3) 
		buffer = new nsclBuffer(argv[1],atoi(argv[2]));
	else
		buffer = new nsclBuffer(argv[1]);

	nsclScalerBuffer *scalerBuffer = new nsclScalerBuffer();
	nsclRunBuffer *runBuffer = new nsclRunBuffer();
	nsclEventBuffer *eventBuffer = new nsclEventBuffer();
	eventScaler *scaler = new eventScaler();
	eventData *data = new eventData();

	printf("Evt Dump: %s\n",argv[1]);
	printf("Buffer Size: %d\n",buffer->GetBufferSize());

	int cnt=0;
	while (buffer->GetNextBuffer() == 0)
	{
		buffer->DumpHeader();
		buffer->PrintBufferHeader();
		if (buffer->GetBufferType() == BUFFER_TYPE_DATA) {
			for (int i=0;i<buffer->GetNumOfEvents();i++) {
				printf("\nEvent: %d\n",i);
				eventBuffer->DumpEvent(buffer);
				eventBuffer->ReadEvent(buffer,data,true);
			}
		}
		else if (buffer->GetBufferType() == BUFFER_TYPE_SCALERS) {
			scalerBuffer->DumpScalers(buffer);
			scalerBuffer->ReadScalers(buffer,scaler,true);
		}
		else if (buffer->GetBufferType() == BUFFER_TYPE_RUNEND) {
			printf("Run Ended        \n");
			break;
		}
	}
}
