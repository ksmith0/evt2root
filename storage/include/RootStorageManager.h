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
			void* address, const char* leafList);
		///Create a column in the n-tuple with the specified name.
		bool CreateBranch(const char* treeName, const char* branchName, 
			const char *className, void* address);
		///Set the address of the object in the specified column.
		void SetBranchAddress(const char* treeName, const char* branchName, 
			void* address) {};
		///Fill a row with values.
		void Fill(const char* treeName);

};



#endif
