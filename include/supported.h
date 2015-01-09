#ifndef SUPPORTED_H
#define SUPPORTED_H

#define SUPPORTED_BUFFER_FORMATS "nsclClassic, nsclUSB, nsclRing, and hribf"
#define SUPPORTED_MODULES "Caen_General, Caen_IO_V977, and hribfModule"

#include <string>

#include "nsclClassicBuffer.h"
#include "nsclUSBBuffer.h"
#include "nsclRingBuffer.h"
#include "hribfBuffer.h"

#include "Caen_IO_V977.h"
#include "Caen_General.h"
#include "hribfModule.h"

enum class bufferFormat {
	UNKNOWN, NSCL_CLASSIC, NSCL_USB, NSCL_RING, HRIBF
};

mainBuffer *GetBufferPointer(std::string formatString) {
	if (formatString == "nsclClassic") return new nsclClassicBuffer();
	else if (formatString == "nsclUSB") return new nsclUSBBuffer();
	else if (formatString == "nsclRing") return new nsclRingBuffer();
	else if (formatString == "hribf") return new hribfBuffer();
	return nullptr;
}


baseModule *GetModulePointer(std::string moduleName) {
	if (moduleName == "hribfModule") return new hribfModule();
	else if (moduleName == "Caen_General") return new Caen_General();
	else if (moduleName == "Caen_IO_V977") return new Caen_IO_V977();
	return nullptr;
}
#endif
