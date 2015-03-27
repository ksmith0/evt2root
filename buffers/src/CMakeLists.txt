set (BUFFERS_SOURCES mainBuffer.cxx nsclClassicBuffer.cxx nsclUSBBuffer.cxx nsclRingBuffer.cxx hribfBuffer.cxx maestroChnBuffer.cxx)

add_library(Buffers STATIC ${BUFFERS_SOURCES})
