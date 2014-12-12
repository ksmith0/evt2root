root_generate_dictionary(ExpEventDictionary eventData.h eventScaler.h LINKDEF ExpEventLinkDef.h)

add_library(ExpEvent SHARED eventData.cxx eventScaler.cxx ExpEventDictionary.cxx)
add_library(ExpEvent_Static STATIC eventData.cxx eventScaler.cxx ExpEventDictionary.cxx)

#Following line necessary for Mac OSX
target_link_libraries(ExpEvent ${ROOT_LIBRARIES})

install(TARGETS ExpEvent DESTINATION ${CMAKE_INSTALL_PREFIX})
