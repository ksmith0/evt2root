set(evt2root_SOURCES msuClassicScaler.cxx msuClassicBuffer.cxx msuEvent.cxx evt2root.cxx)

include_directories(${evt2root_SOURCE_DIR}/include)

add_executable (evt2root ${evt2root_SOURCES})

target_link_libraries (evt2root ${ROOT_LIBRARIES} ${EXTRA_LIBS})

install(TARGETS evt2root DESTINATION ${evt2root_BINARY_DIR})
