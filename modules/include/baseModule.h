#ifndef BASEMODULE_H
#define BASEMODULE_H

//Declare the mainBuffer to be defined later.
class mainBuffer;

class baseModule 
{
	protected:
		///The crate that this module was in.
		int fCrate;
		///The slot the module was in.
		int fSlot;

	public:
		baseModule();
		virtual ~baseModule();
		///Return the crate number that this module was in.
		int GetCrate() {return fCrate;}
		///Return the slot number that this module was in.
		int GetSlot() {return fSlot;}
		virtual void ReadEvent(mainBuffer *buffer, bool verbose=false) {}

};

#endif
