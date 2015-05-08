#include "RootStorageManager.h"

RootStorageManager::RootStorageManager(const char* filename) :
	fFile(nullptr) 
{
	Open(filename);
}

/**We open a TFile with the option "CREATE"
 *
 * \param[in] name Filename to open.
 * \return Flag indicating the file is open.
 */
bool RootStorageManager::Open(const char* fileName) {
	if (fFile) Close();
	fFile = new TFile(fileName,"CREATE");	
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
 * \param[in] name Name of the TTree.
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

bool RootStorageManager::CreateBranch(const char* treeName, 
	const char* branchName, void *address, const char* className) {
	TTree *tree = GetTree(treeName);
	if (!tree) {
		fflush(stdout);
		fprintf(stderr,"ERROR: Cannot create column! Missing n-tuple '%s'!\n",treeName);
		return false;
	}
		
	//tree->Bronch(name,className,address);
	tree->Branch(branchName,className,address);

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
