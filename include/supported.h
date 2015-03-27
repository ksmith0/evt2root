#ifndef SUPPORTED_H
#define SUPPORTED_H

#define SUPPORTED_BUFFER_FORMATS "nsclClassic, nsclUSB, nsclRing, hribf, and maestroChn"
#define SUPPORTED_MODULES "Caen_General, Caen_IO_V977, Mesytec_ADC_MADC32 and hribfModule"

#include <string>

#include "nsclClassicBuffer.h"
#include "nsclUSBBuffer.h"
#include "nsclRingBuffer.h"
#include "hribfBuffer.h"
#include "maestroChnBuffer.h"

#include "Caen_IO_V977.h"
#include "Caen_General.h"
#include "Mesytec_ADC_MADC32.h"
#include "hribfModule.h"

enum class bufferFormat {
	UNKNOWN, NSCL_CLASSIC, NSCL_USB, NSCL_RING, HRIBF
};

mainBuffer *GetBufferPointer(std::string formatString) {
	if (formatString == "nsclClassic") return new nsclClassicBuffer();
	else if (formatString == "nsclUSB") return new nsclUSBBuffer();
	else if (formatString == "nsclRing") return new nsclRingBuffer();
	else if (formatString == "hribf") return new hribfBuffer();
	else if (formatString == "maestroChn") return new maestroChnBuffer();
	return nullptr;
}


baseModule *GetModulePointer(std::string moduleName) {
	if (moduleName == "hribfModule") return new hribfModule();
	else if (moduleName == "Caen_General") return new Caen_General();
	else if (moduleName == "Caen_IO_V977") return new Caen_IO_V977();
	else if (moduleName == "Mesytec_ADC_MADC32") return new Mesytec_ADC_MADC32();
	return nullptr;
}
#endif
