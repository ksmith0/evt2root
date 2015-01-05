#include <vector>
#include <unistd.h>

#include "nsclClassicBuffer.h"
#include "nsclUSBBuffer.h"
#include "nsclRingBuffer.h"
#include "hribfBuffer.h"

#include "Caen_IO_V977.h"
#include "Caen_General.h"
#include "hribfModule.h"

int usage(const char *progName="") {
	fprintf(stderr,"Usage: %s [-r] [-u] [-t bufferType] [-i bufferType] [-s numBuffersSkipped] <-f bufferFormat>  input.evt\n",progName);
	fprintf(stderr,"\t-f bufferFormat\t Indicate the format of the buffer to be read. Possible options include:\n");
	fprintf(stderr,"\t               \t  nsclClassic, nsclUSB, nsclRing, hribf.\n");
	fprintf(stderr,"\t-r\t Indicates raw buffer should be dumped.\n");
	fprintf(stderr,"\t-u\t Indicates physics data unpacking is ignored.\n");
	fprintf(stderr,"\t-t\t Only output buffers corresponding to the provided bufferType.\n");
	fprintf(stderr,"\t-i\t Ignore buffers corresponding to the provided bufferType.\n");
	fprintf(stderr,"\t-s\t Skip the number of buffers specifed.\n");
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
	int skipBuffers = 0; //Number of buffers to skip
	bool dumpRawBuffer = false;
	bool unpackPhysicsData = true;
	std::vector<int> bufferType;
	std::vector<int> ignoreBufferType;

	//Return usage if no arguments provided
	if (argc == 1) {return usage(argv[0]);}
	//Loop over options
	int c;
	while ((c = getopt(argc,argv,":ruf:t:i:s:")) != -1) {
		switch (c) {
			//buffer type
			case 'f':
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
			//raw buffers
			case 'r':
				dumpRawBuffer = true;
				break;
			//unpack physics data
			case 'u':
				unpackPhysicsData = false;
				break;
			//specify specific buffer types to dump
			case 't':
				{
					int type = atoi(optarg);
					bufferType.push_back(type);
					printf("Displaying only buffer type: %d\n",type);
					break;
				}
			//specify specific buffer types to ignore
			case 'i':
				{
					int type = atoi(optarg);
					ignoreBufferType.push_back(type);
					printf("Ignoring buffer type: %d\n",type);
					break;
				}
			//Specify the number of buffes to skip.
			case 's': skipBuffers = atoi(optarg); break;
			//unknown option
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

	//Build correct buffer object.
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

	//\bug Hardcoded until config file is built
	buffer->AddModule(new hribfModule());

	//Print some useful buffer information.
	printf("\n");
	printf("Evt Dump: %s\n",inputFiles[0]);
	printf("Word Size: %d Bytes\n",buffer->GetWordSize());
	printf("Header Size: %d words\n",buffer->GetHeaderSize());
	printf("Buffer Size: %d words\n",buffer->GetBufferSize());

	//Loop over each buffer. Number of words read is returned.
	nextBuffer: while (buffer->ReadNextBuffer() > 0)
	{
		//Skip the first n buffers specified.
		if (buffer->GetBufferNumber() < skipBuffers) goto nextBuffer;

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
			//Loop over all events in a buffer.
			while (buffer->GetEventsRemaining()) {
				if (!buffer->ReadEvent(true)) break;
			}
		}
	}
}
