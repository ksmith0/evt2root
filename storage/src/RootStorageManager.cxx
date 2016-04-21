#include "RootStorageManager.h"

RootStorageManager::RootStorageManager(const char* filename) :
	fFile(nullptr) 
{
	Open(filename);
}

/**We open a TFile with the option "CREATE"
 *
 * \param[in] fileName Filename to open.
 * \return Flag indicating the file is open.
 */
bool RootStorageManager::Open(const char* fileName) {
	if (fFile) Close();
	fFile = new TFile(fileName,"RECREATE");	
	return fFile->IsOpen();
}

/**We first write the file before closing it.
 */
bool RootStorageManager::Close() {
	if (fFile) {
		fFile->Write(0,TObject::kWriteDelete);
		fFile->Close();
		
		delete fFile;
		fFile = 0;
	}
	return true;
}

/**Create a new TTree with the name specified.
 * 
 * \param[in] treeName Name of the TTree.
 * \return True.
 */
bool RootStorageManager::CreateTree(const char* treeName) {
	if (!fFile) {
		fflush(stdout);
		fprintf(stderr,"WARNING: No ROOT output file is open!\n");
	}

	TTree *tree = new TTree(treeName,treeName);
	fTrees[treeName] = tree;
	return true;
}

TH1D* RootStorageManager::CreateHistogram(const char* histName, const char* histTitle, int numBins, int xMin, int xMax) {
	if (!fFile) {
		fflush(stdout);
		fprintf(stderr,"WARNING: No ROOT output file is open!\n");
	}
	
	TH1D *hist = new TH1D(histName, histTitle, numBins, xMin, xMax);
	fHists[histName] = hist;
	return hist;
}

void RootStorageManager::FillHistogram(const char* histName, double value) {
	TH1D *hist = GetHist(histName);
	if (!hist) {
		fflush(stdout);
		fprintf(stderr,"ERROR: Histogram '%s' not defined!\n",histName);
		return;
	}

	hist->Fill(value);
}
void RootStorageManager::SetBinContent(const char* histName, int bin, double value) {
	TH1D *hist = GetHist(histName);
	if (!hist) {
		fflush(stdout);
		fprintf(stderr,"ERROR: Histogram '%s' not defined!\n",histName);
		return;
	}

	hist->SetBinContent(bin,value);
}

/**Returns a nullptr if the corresponding tree is not found.
 *
 * \param[in] treeName Name of tree.
 * \return Pointer to corresponding tree.
 */
TTree* RootStorageManager::GetTree(const char *treeName) {
	std::map<std::string, TTree* >::iterator loc = fTrees.find(treeName);
	if (loc == fTrees.end()) return nullptr;
	return loc->second;
}
/**Returns a nullptr if the corresponding tree is not found.
 *
 * \param[in] histName Name of tree.
 * \return Pointer to corresponding tree.
 */
TH1D* RootStorageManager::GetHist(const char *histName) {
	std::map<std::string, TH1D* >::iterator loc = fHists.find(histName);
	if (loc == fHists.end()) return nullptr;
	return loc->second;
}






/**Create a branch from known ROOT types, i.e. float, int, etc.
 *
 * \param[in] treeName Name of the tree in which to add the branch.
 * \param[in] branchName Name given to the new branch.
 * \param[in] address Address to the branch data.
 * \param[in] leafList Description of the leaves in the branch.
 */
bool RootStorageManager::CreateBranch(const char* treeName, 
	const char* branchName, void *address, const char* leafList) {
	TTree *tree = GetTree(treeName);
	if (!tree) {
		fflush(stdout);
		fprintf(stderr,"ERROR: Cannot create column! Missing n-tuple '%s'!\n",treeName);
		return false;
	}
		
	tree->Branch(branchName,address,leafList);

	return true;
}


void RootStorageManager::Fill(const char* treeName) {
	TTree *tree = GetTree(treeName);
	if (!tree) {
		fflush(stdout);
		fprintf(stderr,"ERROR: Cannot fill row! Missing n-tuple '%s'!\n",treeName);
		return;
	}
	tree->Fill();
}
