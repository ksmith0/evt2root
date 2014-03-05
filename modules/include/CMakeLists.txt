#Look for modules configured by user
#Get all string out of evt_config.h with MODULE(*) 
file(STRINGS ${PROJECT_SOURCE_DIR}/include/evt_config.h MODULES REGEX "^[ \t]*MODULE(.*)")
#Check if we found something
if (NOT "${MODULES}" STREQUAL "")
	#Strip the strip to just get the class names
	STRING(STRIP "${MODULES}" MODULES) 													#Strip space
	STRING(REGEX REPLACE "MODULE\\(" "" MODULES ${MODULES}) 						#Strip MODULE(
	STRING(REGEX REPLACE "\\)" "" MODULES ${MODULES}) 								#Strip )

	#Create list of sources
	STRING(REGEX REPLACE "[ \t]*\\[ \t]*" ".cxx;" MODULE_SOURCES ${MODULES}) #Swap " / " with ;
	SET(MODULE_SOURCES "${MODULE_SOURCES}.cxx")										#Add trailing ;

	#Create list of headers
	STRING(REGEX REPLACE "[ \t]*\\[ \t]*" ".h\"\n #include \"" MODULE_HEADERS ${MODULES}) #Swap " / " with ;
	SET(MODULE_HEADERS "#include \"${MODULE_HEADERS}")										#Add leading '#include "'
	SET(MODULE_HEADERS "${MODULE_HEADERS}.h\"")										#Add trailing '.h"'
	message("Loading the following modules: ${MODULE_SOURCES}")
	message("hey ${MODULE_HEADERS}")
else()
	message(FATAL_ERROR "No module(s) found! Did you setup evt_config.h?" )
endif()

#Configure modules.h file
#CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/modules.h",
CONFIGURE_FILE("modules.h.in" "modules.h" @ONLY)
