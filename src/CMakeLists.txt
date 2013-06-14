set(evt2root_SOURCES msuClassicScaler.cxx msuClassicBuffer.cxx evtRunBuffer.cxx msuEvent.cxx evt2root.cxx)
set(evtDump_SOURCES msuClassicScaler.cxx msuClassicBuffer.cxx evtRunBuffer.cxx msuEvent.cxx evtDump.cxx)

include_directories(${evt2root_SOURCE_DIR}/include)

add_executable (evt2root ${evt2root_SOURCES})
add_executable (evtDump ${evtDump_SOURCES})

target_link_libraries (evt2root ${EXTRA_LIBS} ${ROOT_LIBRARIES})
target_link_libraries (evtDump ${EXTRA_LIBS} ${ROOT_LIBRARIES})

install(TARGETS evt2root DESTINATION ${evt2root_BINARY_DIR})
install(TARGETS evtDump DESTINATION ${evt2root_BINARY_DIR})
