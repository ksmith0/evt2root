#include <vector>

#include "nsclBuffer.h"
#include "nsclScalerBuffer.h"
#include "nsclRunBuffer.h"
#include "nsclEventBuffer.h"

int usage(const char *progName="") {
	fprintf(stderr,"Usage: %s [-b] input.evt\n",progName);
	fprintf(stderr,"\t-b\t Indicates raw buffer should be dumped.\n");
	return 1;
}

int main (int argc, char *argv[])
{
	std::vector< const char* > inputFiles;
	bool dumpRawBuffer = false;

	//Loop over options
	int c;
	while ((c = getopt(argc,argv,"b")) != -1) {
		if (c=='b') dumpRawBuffer = true;
		else if (c=='?') return usage(argv[0]);
	}
	//Get input file arguments. Ignores everything except the first
	for (int i=optind;i<argc;i++) { 
		inputFiles.push_back(argv[i]);
	}

	nsclBuffer *buffer = new nsclBuffer(inputFiles[0]);

	nsclScalerBuffer *scalerBuffer = new nsclScalerBuffer();
	nsclRunBuffer *runBuffer = new nsclRunBuffer();
	nsclEventBuffer *eventBuffer = new nsclEventBuffer();
	eventScaler *scaler = new eventScaler();
	eventData *data = new eventData();

	printf("Evt Dump: %s\n",inputFiles[0]);
	printf("Buffer Size: %d\n",buffer->GetBufferSize());

	int cnt=0;
	while (buffer->GetNextBuffer() > 0)
	{
		buffer->DumpHeader();
		buffer->PrintBufferHeader();
		if (dumpRawBuffer) buffer->DumpBuffer();
		if (buffer->GetBufferType() == BUFFER_TYPE_DATA) {
			for (int i=0;i<buffer->GetNumOfEvents() && buffer->GetPosition() < buffer->GetNumOfWords();i++) {
				printf("\nEvent: %d\n",i);
				eventBuffer->DumpEvent(buffer);
				eventBuffer->ReadEvent(buffer,data,true);
			}
		}
		else if (buffer->GetBufferType() == BUFFER_TYPE_SCALERS) {
			scalerBuffer->DumpScalers(buffer);
			scalerBuffer->ReadScalers(buffer,scaler,true);
		}
		else if (buffer->GetBufferType() == BUFFER_TYPE_RUNBEGIN) {
			runBuffer->DumpRunBuffer(buffer);
			runBuffer->ReadRunBegin(buffer,true);
		}
		else if (buffer->GetBufferType() == BUFFER_TYPE_RUNEND) {
			runBuffer->DumpRunBuffer(buffer);
			runBuffer->ReadRunEnd(buffer,true);
			printf("Run Ended        \n");
			break;
		}
	}
}
