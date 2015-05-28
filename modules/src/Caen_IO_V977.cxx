#include "Caen_IO_V977.h"
#include "mainBuffer.h"

ClassImp(Caen_IO_V977)

void Caen_IO_V977::ReadEvent(mainBuffer *buffer, bool verbose)
{
	int datum = buffer->GetWord();
	//Set Value here
	if (verbose) printf("\t%#06x I/O Pattern\n",datum);
}
