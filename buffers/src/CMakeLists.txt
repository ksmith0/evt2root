set (BUFFERS_SOURCES mainBuffer.cxx moduleBuffer.cxx nsclClassicBuffer.cxx nsclUSBBuffer.cxx nsclRingBuffer.cxx hribfBuffer.cxx maestroChnBuffer.cxx fastListBuffer.cxx)

add_library(Buffers STATIC ${BUFFERS_SOURCES})
target_link_libraries(Buffers Storage)
