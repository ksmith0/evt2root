#include "listDataBuffer.h"

listDataBuffer::listDataBuffer(int bufferSize, int bufferHeaderSize,
	int wordSize) :
	mainBuffer(bufferHeaderSize,bufferSize,wordSize)
{

}

listDataBuffer::~listDataBuffer() {

}
