root_generate_dictionary("msuEventData.h;msuScalerData.h" "MSUEventLinkDef.h" "MSUEventDictionary.cxx" "${ROOT_INCLUDE_DIR};${evt2root_SOURCE_DIR}/MSUEvent")
add_library(MSUEvent SHARED msuEventData.cxx msuScalerData.cxx MSUEventDictionary.cxx)
add_library(MSUEventStatic STATIC msuEventData.cxx msuScalerData.cxx MSUEventDictionary.cxx)
install(TARGETS MSUEvent DESTINATION ${evt2root_BINARY_DIR})
