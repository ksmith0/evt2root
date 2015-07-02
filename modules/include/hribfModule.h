#ifndef HRIBFMODULE_H
#define HRIBFMODULE_H

#include "baseModule.h"

class hribfModule : public baseModule {
	private:
		std::vector <UShort_t> values;
	public:
		hribfModule() {};
		
		void ReadEvent(mainBuffer *buffer, bool verbose=false);
		///Clear the stored values.
		void Clear() {values.clear();};
		///Get value
		UShort_t GetValue(UShort_t ch); 

	ClassDef(hribfModule,1);
};

#endif
