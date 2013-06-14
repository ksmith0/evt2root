#include "msuClassicBuffer.h"
#include "msuClassicScaler.h"
#include "evtRunBuffer.h"
#include "msuEvent.h"
#include <stdlib.h>

int main (int argc, char *argv[])
{
	if (argc < 2 || argc > 3) {
		fprintf(stderr,"Usage: %s [input.evt]\n",argv[0]);
		return 1;
	}

	msuClassicBuffer *buffer;
	if (argc == 3) 
		buffer = new msuClassicBuffer(argv[1],atoi(argv[2]));
	else
		buffer = new msuClassicBuffer(argv[1]);

	msuClassicScaler *scaler = new msuClassicScaler();
	evtRunBuffer *runBuffer = new evtRunBuffer();
	msuEvent *event = new msuEvent();

	printf("Evt Dump: %s\n",argv[1]);
	printf("Buffer Size: %d\n",buffer->GetBufferSize());

	int cnt=0;
	while (buffer->GetNextBuffer() == 0)
	{
		buffer->DumpHeader();
		buffer->PrintBufferHeader();
		if (buffer->GetBufferType() == BUFFER_TYPE_DATA) {
			while (buffer->GetPosition() < buffer->GetNumOfWords()) { 
				event->DumpEvent(buffer);
				event->ReadEvent(buffer,true);
			}
		}
		else if (buffer->GetBufferType() == BUFFER_TYPE_RUNEND) {
			printf("Run Ended        \n");
			break;
		}
	}
}
