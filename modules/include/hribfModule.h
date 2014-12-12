#ifndef HRIBFMODULE_H
#define HRIBFMODULE_H

#include "mainBuffer.h"
#include "baseModule.h"

class hribfModule : public baseModule {
	private:
		std::vector <UShort_t> fValues;
	public:
		hribfModule() {};
		
		void ReadEvent(mainBuffer *buffer, bool verbose=false);
		///Clear the stored values.
		void Clear() {fValues.clear();};
		///Get value
		UShort_t GetValue(UShort_t ch); 

};

#endif
