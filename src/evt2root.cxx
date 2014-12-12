/** \file
 * Converts evt binary files into ROOT files containing \c TTrees 
 * containing the event data and scaler data.
 */

#include <vector>
#include <unistd.h>

#include "nsclClassicBuffer.h"
#include "nsclUSBBuffer.h"
#include "nsclRingBuffer.h"
#include "hribfBuffer.h"

#include "eventScaler.h"
#include "eventData.h"

#include "TTree.h"
#include "TFile.h"
#include "TParameter.h"
//#include "TObjString.h"


int usage(const char *progName="") {
	fprintf(stderr,"Usage: %s [-b] -o output.root input1.evt [input2.evt...]\n",progName);
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
		hribfBuffer *buffer = new hribfBuffer(inputFiles[fileNum]);
		while (buffer->ReadNextBuffer() > 0)
		{
			//If in batch mode the user will not see this.
			if (!batchJob) {
				//Print a buffer counter so the user sees that it is working.
				if (buffer->GetBufferNumber() % 100 == 0) {printf("Buffer: %d\r",buffer->GetBufferNumber());fflush(stdout);}
			}
			if (buffer->GetBufferType() == buffer->BUFFER_TYPE_DATA) {
				while (buffer->GetEventsRemaining()) {
					data->Reset();
					buffer->ReadEvent();
					data->SetValues(buffer->GetModules());
					evtTree->Fill();
				}
			}
			else if (buffer->GetBufferType() == buffer->BUFFER_TYPE_SCALERS) {
				buffer->ReadScalers();
				scalerTree->Fill();
			}
			else if (buffer->GetBufferType() == buffer->BUFFER_TYPE_RUNBEGIN) {
				//Some files have the start buffer written multiple times
				//We keep the last one
				if (runStarted) {
					fprintf(stderr,"WARNING: Multiple run begin buffers, keeping the last one.\n");
				}
				if (!runStarted && buffer->GetBufferNumber()>0) {
					fprintf(stderr,"WARNING: Buffer read before run started! Check input file order.\n");
				}
				buffer->ReadRunBegin();
				printf("Run %d - %s\n",buffer->GetRunNumber(),buffer->GetRunTitle().c_str());
				//TObjString *runTitle = new TObjString (buffer->GetRunTitle().c_str());
				//evtTree->GetUserInfo()->Add(runTitle);
				evtTree->SetTitle(buffer->GetRunTitle().c_str());
				TParameter<int>("run",buffer->GetRunNumber()).Write();
				TParameter<time_t>("runStartTime",buffer->GetRunStartTime()).Write();
				//delete runTitle;

				runStarted = true;
			}
			else if (buffer->GetBufferType() == buffer->BUFFER_TYPE_RUNEND) {
				buffer->ReadRunEnd();
				TParameter<time_t>("runEndTime",buffer->GetRunEndTime()).Write();
				TParameter<int>("runTimeElapsed",buffer->GetElapsedRunTime()).Write();
				printf("Run Ended. Elapsed run time: %u s\n",buffer->GetElapsedRunTime());
				if (fileNum+1 < inputFiles.size()) {
					fprintf(stderr,"WARNING: Run ended before last file was scanned! Check input file order.\n");
				}
				runEnded = true;
				break;
			}
			//else 
			//	printf("Event Type: %d\n",buffer->GetBufferType());
		}
		printf("Read %d buffers.\n",buffer->GetBufferNumber());
		delete buffer;
	}

	//Provide some error messages if the reun start or end are not found.
	if (!runStarted) fprintf(stderr,"ERROR: Run start never found!\n");
	if (!runEnded) fprintf(stderr,"ERROR: Run end never found!\n");

	//Inform th user that we are writing the file.
	printf("Finishing up...");fflush(stdout);
	//Write the file
	file->Write();
	file->Close();
	printf("Done\n");

	return 0;
}
