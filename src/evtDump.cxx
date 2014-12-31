#include <vector>
#include <unistd.h>

#include "nsclClassicBuffer.h"
#include "nsclUSBBuffer.h"
#include "nsclRingBuffer.h"
#include "hribfBuffer.h"

int usage(const char *progName="") {
	fprintf(stderr,"Usage: %s [-r] [-u] [-i bufferType] [-b bufferFormat] <-t bufferType> input.evt\n",progName);
	fprintf(stderr,"\t-b bufferFormat\t Indicate the format of the buffer to be read. Possible options include:\n");
	fprintf(stderr,"\t               \t  nsclClassic, nsclUSB, nsclRing, hribf.\n");
	fprintf(stderr,"\t-r\t Indicates raw buffer should be dumped.\n");
	fprintf(stderr,"\t-u\t Indicates physics data unpacking is ignored.\n");
	fprintf(stderr,"\t-t\t Only output buffers corresponding to the provided bufferType.\n");
	fprintf(stderr,"\t-i\t Ignore buffers corresponding to the provided bufferType.\n");
	return 1;
}

int main (int argc, char *argv[])
{
	enum class bufferFormat {
		NSCL_CLASSIC, NSCL_USB, NSCL_RING, HRIBF
	};
	bufferFormat format;
	bool useHribfBuffer = false;
	bool useNsclClassicBuffer = false;
	bool useNsclUSBBuffer = false;
	bool useNsclRingBuffer = true;

	std::vector< const char* > inputFiles;
	bool dumpRawBuffer = false;
	bool unpackPhysicsData = true;
	std::vector<int> bufferType;
	std::vector<int> ignoreBufferType;

	//Return usage if no arguments provided
	if (argc == 1) {return usage(argv[0]);}
	//Loop over options
	int c;
	while ((c = getopt(argc,argv,":rub:t:i:")) != -1) {
		switch (c) {
			case 'b':
				{
					std::string formatString = optarg;
					if (formatString == "nsclClassic") format = bufferFormat::NSCL_CLASSIC;
					else if (formatString == "nsclUSB") format = bufferFormat::NSCL_USB;
					else if (formatString == "nsclRing") format = bufferFormat::NSCL_RING;
					else if (formatString == "hribf") format = bufferFormat::HRIBF;
					else {
						fprintf(stderr,"ERROR: Unknown buffer format: %s.\n",optarg);
						return usage(argv[0]);
					}
					break;
				}
			case 'r':
				dumpRawBuffer = true;
				break;
			case 'u':
				unpackPhysicsData = false;
				break;
			case 't':
				{
					int type = atoi(optarg);
					bufferType.push_back(type);
					printf("Displaying only buffer type: %d\n",type);
					break;
				}
			case 'i':
				{
					int type = atoi(optarg);
					ignoreBufferType.push_back(type);
					printf("Ignoring buffer type: %d\n",type);
					break;
				}
			case '?': 
			default:
				return usage(argv[0]);
		}
	}

	//Get input file arguments. Ignores everything except the first
	for (int i=optind;i<argc;i++) { 
		inputFiles.push_back(argv[i]);
	}
	if (inputFiles.size() == 0) {
		fprintf(stderr,"ERROR: No input files specified!\n");
		return usage(argv[0]);
	}

	mainBuffer *buffer;
	switch(format) {
		case bufferFormat::HRIBF: buffer = new hribfBuffer(inputFiles[0]); break;
		case bufferFormat::NSCL_CLASSIC: buffer = new nsclClassicBuffer(inputFiles[0]); break;
		case bufferFormat::NSCL_USB: buffer = new nsclUSBBuffer(inputFiles[0]); break;
		case bufferFormat::NSCL_RING: buffer = new nsclRingBuffer(inputFiles[0]); break;
		default:
			fprintf(stderr,"ERROR: Unknown buffer format!\n");
			return 1;
	}

	printf("\n");

	printf("Evt Dump: %s\n",inputFiles[0]);
	printf("Word Size: %d Bytes\n",buffer->GetWordSize());
	printf("Header Size: %d words\n",buffer->GetHeaderSize());
	printf("Buffer Size: %d words\n",buffer->GetBufferSize());

	nextBuffer: while (buffer->ReadNextBuffer() > 0)
	{
		//Skip any specified ignore buffers.
		for (unsigned int i=0;i<ignoreBufferType.size();i++) 
			if (buffer->GetBufferType() == ignoreBufferType[i]) goto nextBuffer;

		//If not user specified buffer then we continue
		if (bufferType.size() > 0) {
			bool goodBuffer = false;
			for (unsigned int i=0;i<bufferType.size() && !goodBuffer;i++) { 
				if (buffer->GetBufferType() == bufferType[i]) goodBuffer=true;
			}
			if (!goodBuffer) goto nextBuffer;
		}

		printf("\nBuffer Position: %d Bytes (%.2f%%)",buffer->GetBufferBeginPosition(),buffer->GetFilePositionPercentage());

		//Print out information about buffer header.
		buffer->DumpHeader();
		buffer->PrintBufferHeader();

		//Dump the entire buffer if user specified.
		if (dumpRawBuffer) buffer->DumpBuffer();

		//Do buffer specific tasks
		buffer->UnpackBuffer(true);
		//Handle data events.
		if (unpackPhysicsData && buffer->IsDataType()) {
			while (buffer->GetEventsRemaining()) {
				if (!buffer->ReadEvent()) break;
			}
		}
	}
}
