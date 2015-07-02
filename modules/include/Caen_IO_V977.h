#ifndef CAEN_IO_V977
#define CAEN_IO_V977

#include "baseModule.h"

/**Readout class for Caen I/O V977:
 * http://www.caen.it/csite/CaenProd.jsp?idmod=295
 *
 * Reads a single word with each bit corresponding to a single channel.
 * The single word is passed to eventData with crate, slot and channel
 * number set to 0.
 */
class Caen_IO_V977 : public baseModule {
	private:
		UInt_t pattern;

	public:
		Caen_IO_V977() {Clear();}
		void ReadEvent(mainBuffer *buffer, bool verbose=false);
		void Clear() {pattern = 0;};

	ClassDef(Caen_IO_V977,1);
};

#endif
