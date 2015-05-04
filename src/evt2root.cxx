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


int usage(const char *progName="") {
	fprintf(stderr,"Usage: %s [-b] -c configFile -o output.root input1.evt [input2.evt...]\n",progName);
	fprintf(stderr,"\t-c configFile\t Load the configuration file specified.\n");
	fprintf(stderr,"\t-f bufferFormat\t Indicate the format of the buffer to be read. Possible options include:\n");
	fprintf(stderr,"\t               \t  %s.\n",SUPPORTED_BUFFER_FORMATS);
	fprintf(stderr,"\t-m moduleType\t Indicate the module to be unpacked. Possible options include:\n");
	fprintf(stderr,"\t               \t  %s.\n",SUPPORTED_MODULES);
	fprintf(stderr,"\n");
	fprintf(stderr,"\t-b\tSet batch mode. (Progress output is suppressed.)\n");
	return 1;
}

int main (int argc, char *argv[])
{
	const char* outputFile = "";
	std::vector< baseModule* > modules;
	mainBuffer* buffer = nullptr;
	ConfigFile *conf = nullptr;
	std::vector< const char* > inputFiles;
	bool batchJob = false;

	int c;
	//Loop over options
	while ((c = getopt(argc,argv,":o:c:f:m:b")) != -1) {
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
			case 'o': outputFile = optarg; break;
			case 'b': batchJob = true; break;
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
	for (auto it = modules.begin(); it != modules.end(); ++it) {
		if (it != modules.begin()) printf(", ");
		printf("%s", (*it)->IsA()->GetName());
		buffer->AddModule(*it);
	}
	printf(".\n");

	TFile *file = new TFile(outputFile,"RECREATE");
	TTree *evtTree = new TTree("evtTree","Events");
	TTree *scalerTree = new TTree("scalerTree","Scalers");

#ifdef USE_EXPEVENT
	eventScaler *scaler = new eventScaler();
	eventData *data = new eventData();

	scalerTree->Branch("scaler","eventScaler",&scaler);
	evtTree->Branch("event","eventData",&data);
#endif

	//Add branch for each module
	std::map< std::string, unsigned int > moduleCount;
	for (auto it = modules.begin(); it != modules.end(); ++it) {
		std::string moduleName = (*it)->IsA()->GetName();
		std::string branchName = moduleName + std::to_string((long long unsigned int) moduleCount[moduleName]);
		evtTree->Branch(branchName.c_str(),moduleName.c_str(),*it);
		moduleCount[moduleName]++;
	}

	bool runEnded = false;
	bool runStarted = false;
	//Loop over files until we used all files or the run ended.
	for (unsigned int fileNum=0;fileNum<inputFiles.size() && !runEnded;fileNum++) {
		if (fileNum > 0) printf("\n");
		
		//Open the file for reading.
		if (!buffer->OpenFile(inputFiles[fileNum])) return 1;
		printf("Reading file: %s\n",buffer->GetFilename());

		while (buffer->ReadNextBuffer() > 0)
		{
			//If in batch mode the user will not see this.
			if (!batchJob) {
				//Print a buffer counter so the user sees that it is working.
				printf("Buffer: %d File: %5.2f%%\r",buffer->GetBufferNumber(),buffer->GetFilePositionPercentage());
				if (buffer->GetBufferNumber() % 100 == 0) {
					fflush(stdout);
				}
			}
			if (buffer->IsDataType()) {
				while (buffer->GetEventsRemaining()) {
					if (!buffer->ReadEvent()) break;

#ifdef USE_EXPEVENT
					data->Reset();
					data->SetValues(buffer->GetModules());
#endif

					evtTree->Fill();
				}
			}
			else if (buffer->IsScalerType()) {
				buffer->ReadScalers();
				scalerTree->Fill();
			}
			else if (buffer->IsRunBegin()) {
				//Some files have the start buffer written multiple times
				//We keep the last one
				if (runStarted) {
					fprintf(stderr,"WARNING: Multiple run begin buffers, keeping the last one.\n");
				}
				if (!runStarted && buffer->GetBufferNumber()>0) {
					fprintf(stderr,"WARNING: Buffer read before run started! Check input file order.\n");
				}
				buffer->ReadRunBegin();
				printf("%*c\r",100,' ');
				printf("Run %llu - %s\n",buffer->GetRunNumber(),buffer->GetRunTitle().c_str());
				//TObjString *runTitle = new TObjString (buffer->GetRunTitle().c_str());
				//evtTree->GetUserInfo()->Add(runTitle);
				evtTree->SetTitle(buffer->GetRunTitle().c_str());
				TParameter<int>("run",buffer->GetRunNumber()).Write();
				TParameter<time_t>("runStartTime",buffer->GetRunStartTime()).Write();
				//delete runTitle;

				runStarted = true;
			}
			else if (buffer->IsRunEnd())  {
				buffer->ReadRunEnd();
				runEnded = true;
			}
			/*else if (buffer->GetBufferType() == buffer->BUFFER_TYPE_RUNEND) {
				buffer->ReadRunEnd();
				TParameter<time_t>("runEndTime",buffer->GetRunEndTime()).Write();
				TParameter<int>("runTimeElapsed",buffer->GetElapsedRunTime()).Write();
				printf("Run Ended. Elapsed run time: %u s\n",buffer->GetElapsedRunTime());
				if (fileNum+1 < inputFiles.size()) {
					fprintf(stderr,"WARNING: Run ended before last file was scanned! Check input file order.\n");
				}
				runEnded = true;
			}*/
			else {
				fflush(stdout);
				fprintf(stderr,"WARNING: Buffer %d has unknown buffer type!",buffer->GetBufferNumber());
				buffer->PrintBufferHeader();
			}	
		}
		printf("Read %d buffers. %5.2f%% of file read.\n",buffer->GetBufferNumber(),buffer->GetFilePositionPercentage());
		buffer->CloseFile();
	}
	delete buffer;

	//Provide some error messages if the run start or end are not found.
	if (!runStarted) fprintf(stderr,"ERROR: Run start never found!\n");
	if (!runEnded) fprintf(stderr,"ERROR: Run end never found!\n");

	//Inform the user that we are writing the file.
	printf("Finishing up...");fflush(stdout);
	//Write the file
	file->Write(0,TObject::kWriteDelete);
	file->Close();
	printf("Done\n");

	return 0;
}
