#The modules must be Position Independent Code
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

root_generate_dictionary(ModuleDictionary baseModule.h Caen_General.h Caen_IO_V977.h Mesytec_ADC_MADC32.h XIA_Pixie16.h hribfModule.h XLM_XXV.h LINKDEF ModulesLinkDef.h)

if (${ROOT_VERSION} VERSION_GREATER "6.0")
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ModuleDictionary_rdict.pcm DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()

set(MODULE_SOURCES baseModule.cxx Caen_General.cxx Caen_IO_V977.cxx Mesytec_ADC_MADC32.cxx XIA_Pixie16.cxx hribfModule.cxx XLM_XXV.cxx ModuleDictionary.cxx)

#We build an obect dictionary so that the sources are not built twice for the shared and static libraries.
add_library(ModulesObjects OBJECT ${MODULE_SOURCES})

#Build the static and shared libraries.
add_library(ModulesStatic STATIC $<TARGET_OBJECTS:ModulesObjects>)
add_library(Modules SHARED $<TARGET_OBJECTS:ModulesObjects>)
install(TARGETS Modules DESTINATION ${CMAKE_INSTALL_PREFIX})

