#ifndef HRIBFMODULE_H
#define HRIBFMODULE_H

#include "baseModule.h"

///Designed to unpack traditional HRIBF LDF data buffers. 
/**These buffers to have a fixed format that does not depend on the actual phsyical modules connected. 
 * This module is provided for flexibility such that the possibility of reading various modules is 
 * permitted. It may become obsolete in the future.
 */
class hribfModule : public baseModule {
	private:
		///Vector storing the values for each channel.
		std::vector <UShort_t> values;
		///The number of times a parameter was set in a given event.
		std::vector <UInt_t> paramMults;
		///The multiplicity of the number of channels fired in an event.
		unsigned int mult;
	public:
		///Default constructor.
		hribfModule() {};

		///Read an event from the buffer.		
		void ReadEvent(mainBuffer *buffer, bool verbose=false);
		///Clear the stored values.
		void Clear();
		///Get value for specified channel.
		UShort_t GetValue(const UShort_t ch); 
	
	/// \cond This is just for ROOT and doesn't need to be documented
	ClassDef(hribfModule,1);
	/// \endcond
};

#endif
