#include "msuClassicBuffer.h"
#include "msuClassicScaler.h"
#include "evtRunBuffer.h"
#include "msuEvent.h"
#include "TTree.h"
#include "TFile.h"
#include "TParameter.h"

int main (int argc, char *argv[])
{
	if (argc != 3) {
		fprintf(stderr,"Usage: %s [input.evt] [output.root]\n",argv[0]);
		return 1;
	}

	msuClassicBuffer *buffer = new msuClassicBuffer(argv[1]);
	msuClassicScaler *scaler = new msuClassicScaler();
	evtRunBuffer *runBuffer = new evtRunBuffer();
	msuEvent *event = new msuEvent();

	
	TFile *file = new TFile(argv[2],"RECREATE");
	TTree *evtTree = new TTree("evtTree","Events");
	TTree *scalerTree = new TTree("scalerTree","Scalers");
	scalerTree->Branch("scaler","msuScalerData",scaler->GetScalerData());
	evtTree->Branch("event","msuEventData",event->GetEventData());

	int cnt=0;
	while (buffer->GetNextBuffer() == 0)
	{
		//printf("\nsubEvt Type:%02d ",buffer->GetSubEvtType());
		//printf("numWords:%02d\n",buffer->GetNumOfWords());
		if (buffer->GetSubEvtType() == SUBEVT_TYPE_DATA) {
			while (buffer->GetPosition() < buffer->GetNumOfWords()) { 
				
				event->ReadEvent(buffer);
				evtTree->Fill();
			}
		}
		else if (buffer->GetSubEvtType() == SUBEVT_TYPE_SCALERS) {
			scaler->ReadScalers(buffer);
			scalerTree->Fill();
		}
		else if (buffer->GetSubEvtType() == SUBEVT_TYPE_RUNBEGIN) {
			runBuffer->ReadRunBegin(buffer);
			printf("Run %d - %s\n",buffer->GetRunNumber(),runBuffer->GetRunTitle().c_str());
			evtTree->SetTitle(runBuffer->GetRunTitle().c_str());
			TParameter<int>("run",buffer->GetRunNumber()).Write();
		}
		else if (buffer->GetSubEvtType() == SUBEVT_TYPE_RUNEND) {
			printf("Run Ended        \n");
			break;
		}
		//else 
		//	printf("Event Type: %d\n",buffer->GetSubEvtType());
		printf("Buffer: %d\r",cnt);
		cnt++;
	}

	//scalerTree->Print();
	//evtTree->Print();
	file->Write();
	file->Close();

	return 0;
}
