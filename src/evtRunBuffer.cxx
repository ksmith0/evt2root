#include "evtRunBuffer.h"

evtRunBuffer::evtRunBuffer()
{

}
/**Begin run buffer format:
 * 1. Run title (79 bytes)
 * 2. 0 (longword)
 * 3. Month
 * 4. Day
 * 5. Year
 * 6. Hour
 * 7. Minute
 * 8. Second 
 * 9. Ticks (64's of a second)
 *
 * \param[in] buffer Pointer to the buffer being read.
 *
 * \bug If there are a number of 0s the run title is terminated. Should 
 * 	properly read 79 bytes to get run title.
 */
void evtRunBuffer::ReadRunBegin(msuClassicBuffer *buffer) {
	if (buffer->GetSubEvtType() != SUBEVT_TYPE_RUNBEGIN) {
		fprintf(stderr,"ERROR: Not a run begin subevt!\n");
		return;
	}

	for (int i=0;i<32;i++) {
		int word = buffer->GetWord();
		if (word == 0) break;
		fRunTitle.push_back(word & 0xFF);
		int letter = (word & 0xFF00) >> 8;
		if (letter)
			fRunTitle.push_back(letter);
	}
}

std::string evtRunBuffer::GetRunTitle()
{
	return fRunTitle;
}

