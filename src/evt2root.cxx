/** \file
 * Converts evt binary files into ROOT files containing \c TTrees 
 * containing the event data and scaler data.
 */

#include <vector>

#include "nsclBuffer.h"
#include "nsclScalerBuffer.h"
#include "nsclRunBuffer.h"
#include "nsclEventBuffer.h"
#include "TTree.h"
#include "TFile.h"
#include "TParameter.h"
//#include "TObjString.h"


int usage(const char *progName="") {
	fprintf(stderr,"Usage: %s -o output.root input1.evt [input2.evt...]\n",progName);
	return 1;
}

int main (int argc, char *argv[])
{
	const char* outputFile = "";
	std::vector< const char* > inputFiles;
	bool batchJob = false;
	int c;
	//Loop over options
	while ((c = getopt(argc,argv,":o:b")) != -1) {
		if (c=='o') outputFile = optarg;
		if (c=='b') batchJob = true;
		else if (c=='?') return usage(argv[0]);
	}
	//Check that an outputFile was given
	// and that there are more arguments to take as inputs.
	if (!strcmp(outputFile,"") || optind==argc) {
		return usage(argv[0]);
	}
	//Get input file arguments.
	for (int i=optind;i<argc;i++) { 
		inputFiles.push_back(argv[i]);
	}

	nsclScalerBuffer *scalerBuffer = new nsclScalerBuffer();
	nsclRunBuffer *runBuffer = new nsclRunBuffer();
	nsclEventBuffer *eventBuffer = new nsclEventBuffer();
	eventScaler *scaler = new eventScaler();
	eventData *data = new eventData();

	
	TFile *file = new TFile(outputFile,"RECREATE");
	TTree *evtTree = new TTree("evtTree","Events");
	TTree *scalerTree = new TTree("scalerTree","Scalers");
	scalerTree->Branch("scaler","eventScaler",&scaler);
	evtTree->Branch("event","eventData",&data);

	bool runEnded = false;
	bool runStarted = false;
	//Loop over files until we used all files or the run ended.
	for (unsigned int fileNum=0;fileNum<inputFiles.size() && !runEnded;fileNum++) {
		if (fileNum > 0) printf("\n");
		printf("Reading file: %s\n",inputFiles[fileNum]);
		nsclBuffer *buffer = new nsclBuffer(inputFiles[fileNum]);
		while (buffer->GetNextBuffer() > 1)
		{
			if (!batchJob && buffer->GetBufferNumber() % 1000 == 0) printf("Buffer: %d\r",buffer->GetBufferNumber());
			if (buffer->GetBufferType() == BUFFER_TYPE_DATA) {
				for (int i=0;i<buffer->GetNumOfEvents();i++) {
					eventBuffer->ReadEvent(buffer,data);
					evtTree->Fill();
				}
			}
			else if (buffer->GetBufferType() == BUFFER_TYPE_SCALERS) {
				scalerBuffer->ReadScalers(buffer,scaler);
				scalerTree->Fill();
			}
			else if (buffer->GetBufferType() == BUFFER_TYPE_RUNBEGIN) {
				//Some files have the start buffer written multiple times
				//We only read the run start once
				if (!runStarted) {
					if (buffer->GetBufferNumber()>0) {
						fprintf(stderr,"WARNING: Buffer read before run started! Check input file order.\n");
					}
					runBuffer->ReadRunBegin(buffer);
					printf("Run %d - %s\n",runBuffer->GetRunNumber(),runBuffer->GetRunTitle().c_str());
					//TObjString *runTitle = new TObjString (runBuffer->GetRunTitle().c_str());
					//evtTree->GetUserInfo()->Add(runTitle);
					evtTree->SetTitle(runBuffer->GetRunTitle().c_str());
					TParameter<int>("run",runBuffer->GetRunNumber()).Write();
					TParameter<time_t>("runStartTime",runBuffer->GetRunStartTime()).Write();
					//delete runTitle;

					scalerBuffer->SetRunStartTime(runBuffer->GetRunStartTime());
					runStarted = true;
				}
			}
			else if (buffer->GetBufferType() == BUFFER_TYPE_RUNEND) {
				runBuffer->ReadRunEnd(buffer);
				TParameter<time_t>("runEndTime",runBuffer->GetRunEndTime()).Write();
				TParameter<int>("runTimeElapsed",runBuffer->GetElapsedRunTime()).Write();
				printf("Run Ended. Elapsed run time: %u s\n",runBuffer->GetElapsedRunTime());
				if (fileNum+1 < inputFiles.size()) {
					fprintf(stderr,"WARNING: Run ended before last file was scanned! Check input file order.\n");
				}
				runEnded = true;
				break;
			}
			//else 
			//	printf("Event Type: %d\n",buffer->GetBufferType());
		}
		delete buffer;
	}

	if (!runStarted) fprintf(stderr,"ERROR: Run start never found!\n");
	if (!runEnded) fprintf(stderr,"ERROR: Run end never found!\n");

	//scalerTree->Print();
	//evtTree->Print();
	printf("Finishing up...");fflush(stdout);
	file->Write();
	file->Close();
	printf("Done\n");

	return 0;
}
