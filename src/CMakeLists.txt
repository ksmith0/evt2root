set(evt2root_SOURCES nsclScalerBuffer.cxx nsclBuffer.cxx nsclRunBuffer.cxx nsclEventBuffer.cxx evt2root.cxx)
set(evtDump_SOURCES nsclScalerBuffer.cxx nsclBuffer.cxx nsclRunBuffer.cxx nsclEventBuffer.cxx evtDump.cxx)

add_executable (evt2root ${evt2root_SOURCES})
add_executable (evtDump ${evtDump_SOURCES})

target_link_libraries (evt2root ${EXTRA_LIBS} ${ROOT_LIBRARIES})
target_link_libraries (evtDump ${EXTRA_LIBS} ${ROOT_LIBRARIES})

install(TARGETS evt2root evtDump DESTINATION ${CMAKE_INSTALL_PREFIX})
