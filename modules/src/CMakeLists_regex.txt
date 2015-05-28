#Configure file to force reconfiguration when evt_config.h updated.
CONFIGURE_FILE("${PROJECT_SOURCE_DIR}/include/evt_config.h" "generated/evt_config.h")

#Look for modules configured by user
#Get all string out of evt_config.h with MODULE(*) 
file(STRINGS "${PROJECT_SOURCE_DIR}/include/evt_config.h" MODULES REGEX "^[ \t]*MODULE(.*)")
#Check if we found something
if (NOT "${MODULES}" STREQUAL "")
	#Strip string to get class names
	STRING(REGEX REPLACE "[ \t]*MODULE\\(" "" MODULES "${MODULES}") 	#Strip MODULE(
	STRING(REGEX REPLACE "\\)[ \t]*" "" MODULES "${MODULES}") 			#Strip )
	STRING(REGEX REPLACE "[ \t]*\\\\[ \t]*" "" MODULES "${MODULES}") 	#Split on \
	
	#Loop over class names to build list of sources and headers
	foreach(item ${MODULES})
		string(STRIP item ${item})
		message("-- Found module: ${item}")
		list(APPEND MODULE_SOURCES "${item}.cxx")
		#Header list expanded in module.h
		list(APPEND MODULE_HEADERS "#include \"${item}.h\"")
	endforeach(item)

	#Remove duplicates
	LIST(REMOVE_DUPLICATES MODULE_HEADERS)
	LIST(REMOVE_DUPLICATES MODULE_SOURCES)

	#Convert Header list to string
	STRING(REPLACE ";" "\n" MODULE_HEADERS "${MODULE_HEADERS}")
else()
	message(FATAL_ERROR "No module(s) found! Did you setup evt_config.h?" )
endif()

#Configure modules.h file with module headers
CONFIGURE_FILE("../include/modules.h.in" "${PROJECT_BINARY_DIR}/generated/modules.h" @ONLY)
#include the generated module.h header file
include_directories("${PROJECT_BINARY_DIR}/generated")

list(APPEND MODULE_SOURCES baseModule.cxx)

add_library(Modules STATIC ${MODULE_SOURCES})
