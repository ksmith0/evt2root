set (STORAGE_SOURCES RootStorageManager.cxx)

add_library(Storage STATIC ${STORAGE_SOURCES})
target_link_libraries(Storage ${ROOT_LIBRARIES})

