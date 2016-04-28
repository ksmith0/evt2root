/** \file
 * Converts evt binary files into ROOT files containing \c TTrees 
 * containing the event data and scaler data.
 */

#include <vector>
#include <unistd.h>

#include "configFile.h"
#include "supported.h"

#ifdef USE_EXPEVENT
#include "eventScaler.h"
#include "eventData.h"
#endif

#include "TClass.h"
#include "TFile.h"
#include "TTree.h"
#include "TParameter.h"
//#include "TObjString.h"

#include "RootStorageManager.h"


int usage(const char *progName="") {
	fprintf(stderr,"Usage: %s [-q] -c configFile -o output.root input1.evt [input2.evt...]\n",progName);
	fprintf(stderr,"\t-c configFile\t Load the configuration file specified.\n");
	fprintf(stderr,"\t-f bufferFormat\t Indicate the format of the buffer to be read. Possible options include:\n");
	fprintf(stderr,"\t               \t  %s.\n",SUPPORTED_BUFFER_FORMATS);
	fprintf(stderr,"\t-m moduleType\t Indicate the module to be unpacked. Possible options include:\n");
	fprintf(stderr,"\t               \t  %s.\n",SUPPORTED_MODULES);
	fprintf(stderr,"\t-b sourceID\t Indicate the builder source ID for the previously listed module.\n\t\t(Only used for the nscl ring buffer)\n");
	fprintf(stderr,"\n");
	fprintf(stderr,"\t-q\tSet quiet mode. (Progress output is suppressed.)\n");
	return 1;
}

int main (int argc, char *argv[])
{
	const char* outputFile = "";
	std::vector< baseModule* > modules;
	std::vector< int > moduleSourceIDs;
	mainBuffer* buffer = nullptr;
	ConfigFile *conf = nullptr;
	std::vector< const char* > inputFiles;
	bool quiet = false;

	int c;
	//Loop over options
	while ((c = getopt(argc,argv,":o:c:f:m:b:q")) != -1) {
		switch (c) {
			//config file
			case 'c': {
				//Load the configuration file.
				conf = new ConfigFile();
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
					fprintf(stderr,"WARNING: Overriding configuration file buffer format with command line option!\n");
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

					for (size_t i=0;i<conf->GetNumEntries("module");++i) {
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
					//If format already set we override it.
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
			//builder source ID
			case 'b':
				{
					if (modules.size() <= moduleSourceIDs.size()) {
						fprintf(stderr,"ERROR: Builder source ID specified prior to module declaration!\n");
						return 1;
					}
					//Fill missing source IDs with invalid ID.
					while (modules.size() - 1 > moduleSourceIDs.size()) moduleSourceIDs.push_back(-1);
					moduleSourceIDs.push_back(atoi(optarg));
					break;
				}

			case 'o': outputFile = optarg; break;
			case 'q': quiet = true; break;
			case '?':
			default:
				fprintf(stderr,"ERROR: Unknown option!");
				return usage(argv[0]);
		}
	}
	//Clean up config file.
	delete conf;
	
	//Check that an outputFile was given
	// and that there are more arguments to take as inputs.
	// The buffer has been defined.
	// And there are some modules to unpack.
	if (!strcmp(outputFile,"") || optind==argc || buffer==nullptr) {
		return usage(argv[0]);
	}
	//Get input file arguments.
	for (int i=optind;i<argc;i++) { 
		inputFiles.push_back(argv[i]);
	}

	//Add modules to buffer
	printf("Loaded modules: ");
	if (modules.empty()) printf("none");
	for (size_t i=0;i<modules.size();i++) {
		auto it = modules.begin() + i;

		//Print the name of the module being added.
		if (it != modules.begin()) printf(", ");
		printf("%s", (*it)->IsA()->GetName());

		int sourceID = 0;
		if (moduleSourceIDs.size() > i) {
			sourceID = moduleSourceIDs[i];
			printf(" (%d)",sourceID);
		}

		buffer->AddModule(*it, sourceID);
	}
	printf(".\n");

	RootStorageManager *storageManager = new RootStorageManager(outputFile);
	buffer->SetStorageManager(storageManager);

	bool runEnded = false;
	//Loop over files until we used all files or the run ended.
	for (unsigned int fileNum=0;fileNum<inputFiles.size() && !runEnded;fileNum++) {
		if (fileNum > 0) printf("\n");
		
		//Open the file for reading.
		if (!buffer->OpenFile(inputFiles[fileNum])) return 1;
		printf("Reading file: %s\n",buffer->GetFilename());

		while (buffer->ReadNextBuffer() > 0)
		{
			//If in quiet mode the user will not see this.
			if (!quiet) {
				if (buffer->GetBufferNumber() % 100000 == 0) {
					//Print a buffer counter so the user sees that it is working.
					printf("Buffer: %d File: %7.4f%%\r",buffer->GetBufferNumber(),buffer->GetFilePositionPercentage());
					fflush(stdout);
				}
			}
			//Do buffer specific tasks
			buffer->UnpackBuffer();
		}
		printf("Read %d buffers. %7.4f%% of file read.\n",buffer->GetBufferNumber(),buffer->GetFilePositionPercentage());
		buffer->CloseFile();
	}
	delete buffer;

	//Inform the user that we are writing the file.
	printf("Finishing up...");fflush(stdout);
	//Write the file
	storageManager->Close();
	printf("Done\n");
	
//	delete storageManager;

	return 0;
}
