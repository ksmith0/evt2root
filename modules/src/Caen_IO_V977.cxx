#include "Caen_IO_V977.h"

void Caen_IO_V977::ReadEvent(nsclBuffer *buffer, eventData *data, bool verbose) 
{
	int datum = buffer->GetWord();
	data->SetValue(0,0,0,datum);	
	if (verbose) printf("\t%#06x I/O Pattern\n",datum);
}
