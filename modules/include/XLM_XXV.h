#ifndef XLM_XXV_H
#define XLM_XXV_H

#include "baseModule.h"

class Xlm_Xxv : public baseModule 
{
	protected:

	public:
		Xlm_Xxv() {};
		virtual void ReadEvent(mainBuffer *buffer, bool verbose=false);
		virtual void Clear();

	ClassDef(Xlm_Xxv,1);
};

#endif
