root_generate_dictionary(ExpEventDictionary eventData.h eventScaler.h LINKDEF ExpEventLinkDef.h)

#root_generate_dictionary("eventData.h;eventScaler.h" "ExpEventLinkDef.h" "ExpEventDictionary.cxx" "${ROOT_INCLUDE_DIR};${evt2root_SOURCE_DIR}/ExpEvent")
add_library(ExpAnalysis SHARED eventData.cxx eventScaler.cxx ExpEventDictionary.cxx)
add_library(ExpEventStatic STATIC eventData.cxx eventScaler.cxx ExpEventDictionary.cxx)
install(TARGETS ExpAnalysis DESTINATION ${CMAKE_INSTALL_PREFIX})
#target_link_libraries (ExpEvent ${ROOT_LIBRARIES})
