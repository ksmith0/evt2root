#ifndef ROOTSTORAGE_H
#define ROOTSTORAGE_H

#include <map>

#include "TFile.h"
#include "TTree.h"

class RootStorageManager {
	private:
		TFile *fFile;
		std::map< std::string, TTree* > fTrees;
		///Find the tree corresponding to the provided name.
		TTree* GetTree(const char* treeName);

	public:
		///Default constructor.
		RootStorageManager() {};
		///Construct the manager and open the specified output file.
		RootStorageManager(const char* filename);
		///Default destructor.
		~RootStorageManager() {};
		
		///Open the specified file for writing.
		bool Open(const char* fileName);
		///Close the currently open file.
		bool Close();

		///Create an n-tuple with the specified name.
		bool CreateTree(const char* treeName);
		///Create a column in the n-tuple with the specified name.
		bool CreateBranch(const char* treeName, const char* branchName, 
			void* address, const char* className);
		///Set the address of the object in the specified column.
		void SetBranchAddress(const char* treeName, const char* branchName, 
			void* address) {};
		///Fill a row with values.
		void Fill(const char* treeName);

	///Template to create a column in the n-tuple with the specified name.
	template <class type> bool CreateBranch(const char* treeName,
		const char* branchName, type address) {
		TTree *tree = GetTree(treeName);
		if (!tree) {
			fflush(stdout);
			fprintf(stderr,"ERROR: Cannot create column! Missing n-tuple '%s'!\n",treeName);
			return false;
		}
		
		tree->Branch(branchName,address);

		return true;
	}

};

#endif
