/**\file Mesytec_ADC_MADC32.h
 *	Readout for Mesytec MADC-32:
 * http://www.mesytec.com/datasheets/MADC-32.pdf
 *
 * Authors: Tony Battaglia <abattagl@nd.edu>
 * 			Karl Smith <ksmit218@utk.edu>
 */

#ifndef MESYTEC_ADC_MADC32_H
#define MESYTEC_ADC_MADC32_H

#include "baseModule.h"

/**Class capable of reading Mesytec ADC MADC-32:
 * http://www.mesytec.com/datasheets/MADC-32.pdf
 */
class Mesytec_ADC_MADC32 : public baseModule {
	private:
		///Mask and shifts
		enum bitMasks {
			/// Word Signiture Mask
			SIG_MASK = 0xC0000000,
			/// Word Signiture Shift
			SIG_SHIFT = 30,

			/// Module ID Mask
			MDL_ID_MASK = 0xFF0000,
			/// Module ID Shift
			MDL_ID_SHIFT = 16,

			/// Channel Count Mask
			CH_CNT_MASK = 0xFFF,
			/// Channel Count Shift
			CH_CNT_SHIFT = 0,

			/// Channel Number Mask
			CH_NUM_MASK = 0x1F0000,
			/// Channel Number Shift
			CH_NUM_SHIFT = 16,

			/// Overflow Mask
			OVERFLOW_MASK = 0x4000,
			/// Overflow Shift
			OVERFLOW_SHIFT = 14,

			/// ADC DATUM Mask
			DATUM_MASK_8K = 0x1FFF,
			DATUM_MASK_4K = 0xFFF,
			DATUM_MASK_2K = 0x7FF,
			/// ADC DATUM Shift
			DATUM_SHIFT = 0,

			//ADC Resolution Mask
			RESOLUTION_MASK = 0x7000,
			//ADC Resolution Shift
			RESOLUTION_SHIFT = 12,

			/// Trigger/Time Stamp Mask
			TRIG_CNT_MASK = 0x3FFFFFFF,
			/// Trigger/Time Stamp Shift
			TRIG_CNT_SHIFT = 0
		};
		///Signiture Types
		enum class BufferType {
			///Header Signiture
			MESY_HEADER = 1,
			///Data Signiture
			MESY_DATA = 0,
			///Trailer Signiture
			MESY_TRAILER = 3
		};
		UShort_t adcValue[32];
		Bool_t overflow[32];
		UInt_t triggerCount;
		UShort_t resolution;

	public:
		Mesytec_ADC_MADC32();
		///Readout the module.
		void ReadEvent(mainBuffer *buffer, bool verbose=false); 
		///Clear the module values.
		void Clear();

	ClassDef(Mesytec_ADC_MADC32,1);
};


#endif
