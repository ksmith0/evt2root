/**\file XIA_Pixie16.h
 *	Readout for the XIA Pixie-16 Digitizer:
 * http://www.xia.com/DGF_Pixie-16.html 
 *
 * Authors: Karl Smith <ksmit218@utk.edu>
 */

#ifndef XIA_PIXIE16_H
#define XIA_PIXIE16_H

#include "baseModule.h"

///Class capable of reading out a XIA Pixie-16 Digitizer.
/**The class handles readout of the XIA Pixie-16 Digitizer:
 * http://www.xia.com/DGF_Pixie-16.html 
 */
class XIA_Pixie16 : public baseModule {
	private:
		enum masks {
			CHANNELID_MASK      =        0xF,  // Bits 0-3 inclusive
			CHANNELID_SHIFT     =          0,  // Bits 0-3 inclusive
			SLOTID_MASK         =       0xF0,  // Bits 4-7 inclusive
			SLOTID_SHIFT        =          4,  // Bits 4-7 inclusive
			CRATEID_MASK        =      0xF00,  // Bits 8-11 inclusive
			CRATEID_SHIFT       =          8,  // Bits 8-11 inclusive
			HEADERLENGTH_MASK   =    0x1F000,  // Bits 12-16 inclusive
			HEADERLENGTH_SHIFT  =         12,  // Bits 12-16 inclusive
			CHANNELLENGTH_MASK  = 0x3FFE0000,  // Bits 17-29 inclusive
			CHANNELLENGTH_SHIFT =         17,  // Bits 17-29 inclusive
			OVERFLOW_MASK       = 0x40000000,  // Bit 30 has overflow information (1 - overflow)
			OVERFLOW_SHIFT      =         30,  // Bit 30 has overflow information (1 - overflow)
			FINISHCODE_MASK     = 0x80000000,  // Bit 31 has pileup information (1 - pileup)
			FINISHCODE_SHIFT    =         31,  // Bit 31 has pileup information (1 - pileup)
			LOWER16BIT_MASK     =     0xFFFF,  // Lower 16 bits
			UPPER16BIT_MASK     = 0xFFFF0000,  // Upper 16 bits
			LOWER12BIT_MASK     =      0xFFF   // Lower 12 bits
		};

	public:
		XIA_Pixie16() {}
		///Readout the module.
		void ReadEvent(mainBuffer *buffer, bool verbose=false);
		///Clear or reset any event information.
		void Clear();

	ClassDef(XIA_Pixie16,1);
};

#endif
