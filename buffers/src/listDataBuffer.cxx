#include "listDataBuffer.h"
#include "baseModule.h"

listDataBuffer::listDataBuffer(int bufferSize, int bufferHeaderSize,
	int wordSize) :
	mainBuffer(bufferHeaderSize,bufferSize,wordSize)
{

}

listDataBuffer::~listDataBuffer() {

}

void listDataBuffer::InitializeStorageManager() {
	RootStorageManager *manager = GetStorageManager();
	if (!manager) return;
	manager->CreateTree("data");


	std::vector<baseModule*> modules = GetModules();

	//Add branch for each module
	std::map< std::string, unsigned int > moduleCount;
	for (auto it = modules.begin(); it != modules.end(); ++it) {
		std::string moduleName = (*it)->IsA()->GetName();
		std::string branchName = moduleName + std::to_string((long long unsigned int) moduleCount[moduleName]);
		manager->CreateBranch("data",branchName.c_str(),moduleName.c_str(),*it);
		moduleCount[moduleName]++;
	}
}

void listDataBuffer::FillStorage() {
	RootStorageManager *manager = GetStorageManager();
	if (!manager) return;
	
	manager->Fill("data");
}

