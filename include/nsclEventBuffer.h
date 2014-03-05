#ifndef NSCLEVENTBUFFER_H
#define NSCLEVENTBUFFER_H

#define MODULE_FUNCTION(NAME) modules.push_back(&NAME::ReadEvent);

#include <deque>

#include "nsclBuffer.h"
#include "eventData.h"
#include "modules.h"

class nsclEventBuffer
{

	private:
		std::deque< ModuleReadOut > modules;
	public:
		///Default constructor.
		nsclEventBuffer();
		///Reads current event and stores data.
		void ReadEvent(nsclBuffer *buffer, eventData *data, bool verbose = false);
		
		///Dumps the words in the current event.
		void DumpEvent(nsclBuffer *buffer); 



};

#endif
