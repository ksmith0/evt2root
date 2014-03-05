#ifndef CAEN_GENERAL_H
#define CAEN_GENERAL_H

#include "nsclBuffer.h"
#include "eventData.h"

/**Class capable of reading most Caen cards. Caen cards use a common
 * architecture for arranging output. This class is known to support 
 * the Caen V785 32-channel Peak Sensing ADC: 
 * http://www.caen.it/csite/CaenProd.jsp?idmod=37
 * and the Caen V775 32-channel Multievent TDC:
 * http://www.caen.it/csite/CaenProd.jsp?idmod=35
 */
class Caen_General {
	private:
		enum masks {
			// All data words have these bits:
			ALLH_TYPEMASK = 0x7000000,
			ALLH_TYPESHIFT = 24,
			ALLH_GEOMASK = 0xf8000000,
			ALLH_GEOSHIFT = 27,

			// High part of header.
			HDRH_CRATEMASK = 0x00ff0000,
			HDRH_CRATESHIFT = 16,

			// Low part of header.
			HDRL_COUNTMASK = 0X3f00,
			HDRL_COUNTSHIFT = 8,

			// High part of data:
			DATAH_CHANMASK = 0x3f0000,
			DATAH_CHANSHIFT = 16,

			// Low part of data
			DATAL_UNBIT = 0x2000,
			DATAL_OVBIT = 0x1000,
			DATAL_VBIT = 0x40000,
			DATAL_DATAMASK = 0x0fff
		};
		enum types {
			// Word types:
			HEADER = 2,
			DATA = 0,
			TRAILER = 4,
			INVALID = 6
		};

	public:
		Caen_General() {}
		static void ReadEvent(nsclBuffer *buffer, eventData *data, bool verbose=false);


};

#endif
