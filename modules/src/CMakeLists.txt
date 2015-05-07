
root_generate_dictionary(ModuleDictionary baseModule.h Caen_General.h Caen_IO_V977.h Mesytec_ADC_MADC32.h XIA_Pixie16.h hribfModule.h LINKDEF ModulesLinkDef.h)

set(MODULE_SOURCES baseModule.cxx Caen_General.cxx Caen_IO_V977.cxx Mesytec_ADC_MADC32.cxx XIA_Pixie16.cxx hribfModule.cxx ModuleDictionary.cxx)

add_library(ModulesStatic STATIC ${MODULE_SOURCES})
add_library(Modules SHARED ${MODULE_SOURCES})
install(TARGETS Modules DESTINATION ${CMAKE_INSTALL_PREFIX})

