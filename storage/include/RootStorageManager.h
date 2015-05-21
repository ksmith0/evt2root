#ifndef ROOTSTORAGE_H
#define ROOTSTORAGE_H

#include <map>

#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TParameter.h"

class RootStorageManager {
	private:
		TFile *fFile;
		std::map< std::string, TTree* > fTrees;
		std::map< std::string, TH1D* > fHists;
		///Find the tree corresponding to the provided name.
		TTree* GetTree(const char* treeName);
		///Find the hist corresponding to the provided name.
		TH1D* GetHist(const char* histName);

	public:
		///Default constructor.
		RootStorageManager() {};
		///Construct the manager and open the specified output file.
		RootStorageManager(const char* filename);
		///Default destructor.
		~RootStorageManager() {Close();};
		
		///Open the specified file for writing.
		bool Open(const char* fileName);
		///Close the currently open file.
		bool Close();

		///Create an n-tuple with the specified name.
		bool CreateTree(const char* treeName);
		///Create a column in the n-tuple with the specified name.
		bool CreateBranch(const char* treeName, const char* branchName, 
			void* address, const char* leafList);
		///Create a column in the n-tuple with the specified name.
		template <typename T> bool CreateBranch(const char* treeName, const char* branchName, const char *className, T* address);
		///Set the address of the object in the specified column.
		void SetBranchAddress(const char* treeName, const char* branchName, 
			void* address) {};
		///Fill a row with values.
		void Fill(const char* treeName);
		///Add a parameter to the file.
		template <typename T> void AddParameter(const char* parName, T value);
		TH1D* CreateHistogram(const char* histName, const char* histTitle, int numBins, int xMin, int xMax);
		void FillHistogram(const char *histName, double value);
		void SetBinContent(const char *histName, int bin, double value);

};

template<typename T> void RootStorageManager::AddParameter(const char* parName, T value) {
	if (fFile) 
		TParameter<T>(parName,value).Write();
	else 
		fprintf(stderr,"ERROR: No file open for writing parameter %s!\n",parName);
}

/**Creates a branch from a class built with a ROOT dictionary.
 * 
 * \param[in] treeName Name of the tree in which to add the branch.
 * \param[in] branchName Name given to the new branch.
 * \param[in] className Name of the class type.
 * \param[in] address Address to the branch data.
 */
template<typename T> bool RootStorageManager::CreateBranch(const char* treeName, const char* branchName, const char* className, T *address) {
	TTree *tree = GetTree(treeName);
	if (!tree) {
		fflush(stdout);
		fprintf(stderr,"ERROR: Cannot create column! Missing n-tuple '%s'!\n",treeName);
		return false;
	}
		
	tree->Branch(branchName,className,address);

	return true;
}



#endif
