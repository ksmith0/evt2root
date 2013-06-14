root_generate_dictionary("eventData.h;eventScaler.h" "ExpEventLinkDef.h" "ExpEventDictionary.cxx" "${ROOT_INCLUDE_DIR};${evt2root_SOURCE_DIR}/ExpEvent")
add_library(ExpEvent SHARED eventData.cxx eventScaler.cxx ExpEventDictionary.cxx)
add_library(ExpEventStatic STATIC eventData.cxx eventScaler.cxx ExpEventDictionary.cxx)
install(TARGETS ExpEvent DESTINATION ${evt2root_BINARY_DIR})
