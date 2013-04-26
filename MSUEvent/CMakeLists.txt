root_generate_dictionary("msuEventData.h;msuRingScalerData.h;msuScalerData.h" "MSUEventLinkDef.h" "MSUEventDictionary.cxx" "${ROOT_INCLUDE_DIR};${evt2root_SOURCE_DIR}/MSUEvent")
add_library(MSUEvent msuEventData.cxx msuRingScalerData.cxx msuScalerData.cxx MSUEventDictionary.cxx)
