#ifndef MSUEVENT_H
#define MSUEVENT_H

#include "msuClassicBuffer.h"
#include "msuEventData.h"

     // All data words have these bits:
    
static const unsigned int ALLH_TYPEMASK(0x7000000);
static const unsigned int ALLH_TYPESHIFT(24);
static const unsigned int ALLH_GEOMASK(0xf8000000);
static const unsigned int ALLH_GEOSHIFT(27);

    // High part of header.
    
static const unsigned int HDRH_CRATEMASK(0x00ff0000);
static const unsigned int HDRH_CRATESHIFT(16);

    // Low part of header.
    
static const unsigned int HDRL_COUNTMASK(0X3f00);
static const unsigned int HDRL_COUNTSHIFT(8);

    // High part of data:
    
static const unsigned int DATAH_CHANMASK(0x3f0000);
static const unsigned int DATAH_CHANSHIFT(16);

    // Low part of data
    
static const unsigned int DATAL_UNBIT(0x2000);
static const unsigned int DATAL_OVBIT(0x1000);
static const unsigned int DATAL_VBIT(0x40000);
static const unsigned int DATAL_DATAMASK(0x0fff);

	// Word types:
	
static const unsigned int HEADER(2);
static const unsigned int DATA(0);
static const unsigned int TRAILER(4);
static const unsigned int INVALID(6);

class msuEvent
{

	private:
		msuEventData *fData;
		
	public:
		msuEvent();
		void ReadEvent(msuClassicBuffer *buffer);
		msuEventData *GetEventData();
		void Clear();
		void DumpEvent(msuClassicBuffer *buffer); 



};

#endif
