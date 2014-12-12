#include <vector>
#include <unistd.h>

#include "nsclClassicBuffer.h"
#include "nsclUSBBuffer.h"
#include "nsclRingBuffer.h"
#include "hribfBuffer.h"

int usage(const char *progName="") {
	fprintf(stderr,"Usage: %s [-b] [-t bufferType] input.evt\n",progName);
	fprintf(stderr,"\t-b\t Indicates raw buffer should be dumped.\n");
	fprintf(stderr,"\t-t\t Only output buffers corresponding to the provided bufferType.\n");
	return 1;
}

int main (int argc, char *argv[])
{
	std::vector< const char* > inputFiles;
	bool dumpRawBuffer = false;
	int bufferType = -1;

	//Loop over options
	int c;
	while ((c = getopt(argc,argv,"bt:")) != -1) {
		if (c=='b') dumpRawBuffer = true;
		if (c=='t') bufferType = atoi(optarg);
		else if (c=='?') return usage(argv[0]);
	}
	//Get input file arguments. Ignores everything except the first
	for (int i=optind;i<argc;i++) { 
		inputFiles.push_back(argv[i]);
	}

	hribfBuffer *buffer = new hribfBuffer(inputFiles[0]);
//	nsclClassicBuffer *buffer = new nsclClassicBuffer(inputFiles[0]);
//	nsclUSBBuffer *buffer = new nsclUSBBuffer(inputFiles[0]);
//	nsclRingBuffer *buffer = new nsclRingBuffer(inputFiles[0]);

	printf("Evt Dump: %s\n",inputFiles[0]);
	printf("Word Size: %d Bytes\n",buffer->GetWordSize());
	printf("Header Size: %d words\n",buffer->GetHeaderSize());
	printf("Buffer Size: %d words\n",buffer->GetBufferSize());

	int cnt=0;
	while (buffer->ReadNextBuffer() > 0)
	{
		//If not user specified buffer then we continue
		if (bufferType != -1 && buffer->GetBufferType() != bufferType) continue;

		printf("\nBuffer Position: %d Bytes",buffer->GetBufferBeginPosition());

		//Print out information about buffer header.
		buffer->DumpHeader();
		buffer->PrintBufferHeader();

		//Dump the entire buffer if user specified.
		if (dumpRawBuffer) buffer->DumpBuffer();

		//Do buffer specific tasks
		if (buffer->GetBufferType() == buffer->BUFFER_TYPE_DATA) {
			while (buffer->GetEventsRemaining()) {
				buffer->DumpEvent();
				if (!buffer->ReadEvent(true)) break;
			}
		}
		else if (buffer->GetBufferType() == buffer->BUFFER_TYPE_SCALERS) {
			buffer->DumpScalers();
			buffer->ReadScalers(true);
		}
		else if (buffer->GetBufferType() == buffer->BUFFER_TYPE_RUNBEGIN) {
			buffer->DumpRunBuffer();
			buffer->ReadRunBegin(true);
		}
		else if (buffer->GetBufferType() == buffer->BUFFER_TYPE_RUNEND) {
			buffer->DumpRunBuffer();
			buffer->ReadRunEnd(true);
			printf("Run Ended        \n");
			break;
		}
	}
}
