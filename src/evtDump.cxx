#include <vector>
#include <unistd.h>

#include "TClass.h"

#include "configFile.h"
#include "supported.h"

int usage(const char *progName="") {
	fprintf(stderr,"Usage: %s [-r] [-u] [-t bufferType] [-i bufferType] [-s numBuffersSkipped] -f bufferFormat -m moduleType [-c configFile] input.evt\n",progName);
	fprintf(stderr,"\t-c configFile\t Load the configuration file specified.\n");
	fprintf(stderr,"\t-f bufferFormat\t Indicate the format of the buffer to be read. Possible options include:\n");
	fprintf(stderr,"\t               \t  %s.\n",SUPPORTED_BUFFER_FORMATS);
	fprintf(stderr,"\t-m moduleType\t Indicate the module to be unpacked. Possible options include:\n");
	fprintf(stderr,"\t               \t  %s.\n",SUPPORTED_MODULES);
	fprintf(stderr,"\n");
	fprintf(stderr,"\t-r\t Indicates raw buffer should be dumped.\n");
	fprintf(stderr,"\t-u\t Indicates physics data unpacking is ignored.\n");
	fprintf(stderr,"\t-t\t Only output buffers corresponding to the provided bufferType. May be called multiple times.\n");
	fprintf(stderr,"\t-i\t Ignore buffers corresponding to the provided bufferType. May be called multiple times.\n");
	fprintf(stderr,"\t-s\t Skip the number of buffers specified.\n");
	fprintf(stderr,"\n");
	fprintf(stderr,"\t-h\t This help menu.\n");
	return 1;
}

int main (int argc, char *argv[])
{
	bool useHribfBuffer = false;
	bool useNsclClassicBuffer = false;
	bool useNsclUSBBuffer = false;
	bool useNsclRingBuffer = true;
	const char* configFilename = "";

	std::vector< const char* > inputFiles;
	std::vector< baseModule* > modules;
	mainBuffer* buffer = nullptr;
	ConfigFile *conf = nullptr;

	int skipBuffers = 0; //Number of buffers to skip
	bool dumpRawBuffer = false;
	bool unpackPhysicsData = true;
	std::vector<int> bufferType;
	std::vector<int> ignoreBufferType;

	//Return usage if no arguments provided
	if (argc == 1) {return usage(argv[0]);}
	//Loop over options
	int c;
	while ((c = getopt(argc,argv,":c:f:m:rut:i:s:h")) != -1) {
		switch (c) {
			//config file
			case 'c': {
				//Load the configuration file.
				conf = new ConfigFile;
				if (!conf->ReadFile(optarg)) {
					fprintf(stderr,"ERROR: Unable to read configuration file!\n");
					return 1;
				}

				//If the format hasn't been specified we try to read it from config.
				if (buffer == nullptr) {
					//No format was provided
					if (conf->GetNumEntries("format") == 0) {
						fflush(stdout);
						fprintf(stderr,"ERROR: Buffer format not specified in configuration file!\n");
						fprintf(stderr,"       Specify the format with key 'format'.\n");
						fprintf(stderr,"       Supported buffer formats: %s\n",SUPPORTED_BUFFER_FORMATS); 
						return 1;
					}

					buffer = GetBufferPointer(conf->GetOption("format"));
					if (buffer == nullptr) {
						fprintf(stderr,"ERROR: Unknown buffer format: %s.\n",optarg);
						fprintf(stderr,"       Supported buffer formats: %s\n",SUPPORTED_BUFFER_FORMATS); 
						return 1;
					}
				}
				//If the format was already specified we warn the user.
				else {
					fflush(stdout);
					fprintf(stderr,"WARNING: Overiding configuration file buffer format with command line option!\n");
				}

				//Get the modules
				if (modules.empty()) {
					//No modules were provided
					if (conf->GetNumEntries("module") == 0) {
						fflush(stdout);
						fprintf(stderr,"ERROR: Buffer module list not specified in configuration file!\n");
						fprintf(stderr,"       Specify modules with key 'module'.\n");
						fprintf(stderr,"       Supported modules are: %s\n",SUPPORTED_BUFFER_FORMATS); 
						return 1;
					}

					for (int i=0;i<conf->GetNumEntries("module");++i) {
						baseModule *modulePtr = GetModulePointer(conf->GetOption("module",i));
						if (modulePtr == nullptr) {
							fflush(stdout);
							fprintf(stderr,"ERROR: Unknown module %s, supported modules are:\n",conf->GetOption("module",i).c_str());
							fprintf(stderr,"       %s\n",SUPPORTED_MODULES);
							return 1;
						}
						modules.push_back(modulePtr);
					}
				}
				else {
					fflush(stdout);
					fprintf(stderr,"WARNING: Overriding configuration file module list with command line options!\n");
				}

				break;
			}

			//buffer type
			case 'f':
				{
					//If format already set we overide it.
					if (buffer != nullptr) {
						fflush(stdout);
						fprintf(stderr,"WARNING: Overriding file buffer format with command line option: %s!\n",optarg);
					}

					//Get the format
					buffer = GetBufferPointer(optarg);
					//Handle unknown format
					if (buffer == nullptr) {
						fprintf(stderr,"ERROR: Unknown buffer format: %s.\n",optarg);
						fprintf(stderr,"       Supported buffer formats: %s\n",SUPPORTED_BUFFER_FORMATS); 
						return 1;
					}

				
					break;
				}

			//modules
			case 'm':
				{
					if (conf) {
						fflush(stdout);
						fprintf(stderr,"WARNING: Overriding module list with command line options!\n");
						modules.clear();
					}
						
					baseModule *modulePtr = GetModulePointer(optarg);
					if (modulePtr == nullptr) {
						fflush(stdout);
						fprintf(stderr,"ERROR: Unknown module %s, supported modules are:\n",optarg);
						fprintf(stderr,"       %s\n",SUPPORTED_MODULES);
						return 1;
					}
					modules.push_back(modulePtr);
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
			//Specify the number of buffers to skip.
			case 's': skipBuffers = atoi(optarg); break;
			//Help menu
			case 'h': return usage(argv[0]);
			//unknown option
			case '?': 
			default:
				return usage(argv[0]);
		}
	}
	//Clean up config file.
	delete conf;
	//Check that there are more arguments to take as inputs.
	if (optind==argc) {
		fflush(stdout);
		fprintf(stderr,"ERROR: No input files specified!\n");
		return 1;
	}
	
	//Get input file arguments.
	for (int i=optind;i<argc;i++) { 
		inputFiles.push_back(argv[i]);
	}

	//\bug Reads only the first input file.
	if (!buffer->OpenFile(inputFiles[0])) return 1;

	//Print some useful buffer information.
	printf("Evt Dump: %s\n",buffer->GetFilename());
	printf("Word Size: %d Bytes\n",buffer->GetWordSize());
	printf("Header Size: %d words\n",buffer->GetHeaderSize());
	printf("Buffer Size: %d words\n",buffer->GetBufferSize());

	//Add modules to buffer
	if (!modules.empty()) {
		printf("Loaded modules: ");
		for (auto it = modules.begin(); it != modules.end(); ++it) {
			if (it != modules.begin()) printf(", ");
			printf("%s", (*it)->IsA()->GetName());
			buffer->AddModule(*it);
		}
		printf(".\n");
	}
	else {
		fflush(stdout);
		fprintf(stderr,"WARNING: No modules specified data events will not be unpacked!\n");
		unpackPhysicsData = false;
	}
	
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
