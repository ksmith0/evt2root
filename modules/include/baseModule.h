#ifndef BASEMODULE_H
#define BASEMODULE_H

#include "TObject.h"

//Declare the mainBuffer to be defined later.
class mainBuffer;

class baseModule : public TObject 
{
	protected:

	public:
		baseModule();
		virtual ~baseModule();
		virtual void ReadEvent(mainBuffer *buffer, bool verbose=false) {}

	ClassDef(baseModule,1);
};

#endif
