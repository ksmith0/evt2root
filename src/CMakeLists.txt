set(evt2root_SOURCES evt2root.cxx)
set(evtDump_SOURCES evtDump.cxx)

add_executable (evt2root ${evt2root_SOURCES})
add_executable (evtDump ${evtDump_SOURCES})

target_link_libraries (evt2root ${EXTRA_LIBS} ${ROOT_LIBRARIES})
target_link_libraries (evtDump ${EXTRA_LIBS} ${ROOT_LIBRARIES})

install(TARGETS evt2root DESTINATION ${CMAKE_INSTALL_PREFIX})
install(TARGETS evtDump DESTINATION ${CMAKE_INSTALL_PREFIX})
