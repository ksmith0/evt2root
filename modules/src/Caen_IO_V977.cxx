#include "Caen_IO_V977.h"

void Caen_IO_V977::ReadEvent(mainBuffer *buffer, bool verbose)
{
	int datum = buffer->GetWord();
	//Set Value here
	if (verbose) printf("\t%#06x I/O Pattern\n",datum);
}
