#ifndef CAEN_IO_V977
#define CAEN_IO_V977

#include "nsclBuffer.h"
#include "eventData.h"

/**Readout class for Caen I/O V977:
 * http://www.caen.it/csite/CaenProd.jsp?idmod=295
 *
 * Reads a single word with each bit corresponding to a single channel.
 * The single word is passed to eventData with crate, slot and channel
 * number set to 0.
 */
class Caen_IO_V977 {
	private:

	public:
		Caen_IO_V977() {}
		static void ReadEvent(nsclBuffer *buffer, eventData *data, bool verbose=false);

};

#endif
