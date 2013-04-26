# Install script for directory: /afs/crc.nd.edu/user/s/spectro/evt2root/src

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr/local")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "0")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  IF(EXISTS "$ENV{DESTDIR}/afs/crc.nd.edu/user/s/spectro/evt2root/build/evt2root" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/afs/crc.nd.edu/user/s/spectro/evt2root/build/evt2root")
    FILE(RPATH_CHECK
         FILE "$ENV{DESTDIR}/afs/crc.nd.edu/user/s/spectro/evt2root/build/evt2root"
         RPATH "")
  ENDIF()
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/afs/crc.nd.edu/user/s/spectro/evt2root/build/evt2root")
FILE(INSTALL DESTINATION "/afs/crc.nd.edu/user/s/spectro/evt2root/build" TYPE EXECUTABLE FILES "/afs/crc.nd.edu/user/s/spectro/evt2root/build/src/evt2root")
  IF(EXISTS "$ENV{DESTDIR}/afs/crc.nd.edu/user/s/spectro/evt2root/build/evt2root" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/afs/crc.nd.edu/user/s/spectro/evt2root/build/evt2root")
    FILE(RPATH_REMOVE
         FILE "$ENV{DESTDIR}/afs/crc.nd.edu/user/s/spectro/evt2root/build/evt2root")
    IF(CMAKE_INSTALL_DO_STRIP)
      EXECUTE_PROCESS(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/afs/crc.nd.edu/user/s/spectro/evt2root/build/evt2root")
    ENDIF(CMAKE_INSTALL_DO_STRIP)
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

