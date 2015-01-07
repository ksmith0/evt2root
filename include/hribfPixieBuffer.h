#ifndef HRIBFPIXIEBUFFER_H
#define HRIBFPIXIEBUFFER_H

#include "mainBuffer.h"
#include "modules.h"

#include <vector>
#include <deque>

///Size of smallest word in bytes.
#define WORD_SIZE 4 
///Type of used to contain a word.
#define WORD_TYPE UInt_t 
///Type of used to contain a long word
#define LONG_WORD_TYPE ULong64_t 
///The size of the buffer header in words.
#define BUFFER_HEADER_SIZE 2
///LDF Buffer Size in words.
#define BUFFER_SIZE 8194

/*
enum BufferType
{
	///Physics data type.
	BUFFER_TYPE_DATA = 0x41544144, //"DATA"
	///Scaler type.
	BUFFER_TYPE_SCALERS = 20, 
	///EPICS
	BUFFER_TYPE_EPICS = 11, 
	///Run Begin type.
	BUFFER_TYPE_RUNBEGIN = 0x44414548, //"HEAD"
	///Run End type.
	BUFFER_TYPE_RUNEND = 2 
};
*/

///Unpacks LDF files from HRIBF
/**This implementation is geared towards unpacking of Pixie16 cards.
 */
class hribfPixieBuffer : public mainBuffer
{
	public:
		typedef WORD_TYPE word;
		typedef LONG_WORD_TYPE longword;

	private:
		UInt_t GetEventLength();
		///Grab the next spill and load into modules deques.
		int GetSpill(bool verbose = false);
		std::vector< std::deque< baseModule* > > fSpillModules;

	public:
		///Default Constructor
		hribfPixieBuffer(const char* filename);
		///Default Destructor
		virtual ~hribfPixieBuffer() {};
		///Read a physics event buffer
		int ReadEvent(eventData *data, bool verbose = false);
		///Read a physics event buffer
		void ReadScalers(eventScaler *data, bool verbose = false);
		///Retrieve the next buffer in the file
		int GetNextBuffer();
		
		///Print a summary of the buffer header.
		void PrintBufferHeader();
		///Output the run buffer
		void DumpRunBuffer();
		///Output the scalers.
		void DumpScalers();

};


#endif
