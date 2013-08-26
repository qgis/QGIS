CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0 FATAL_ERROR)

cmake_policy(VERSION 2.6.0)

PROJECT(spawn-fcgi)
SET(PACKAGE_NAME ${CMAKE_PROJECT_NAME})
SET(PACKAGE_VERSION 1.6.4)

SET(CMAKE_MAN_DIR "share/man" CACHE STRING
  "Install location for man pages (relative to prefix).")
MARK_AS_ADVANCED(CMAKE_MAN_DIR)

# Set Default build type to RelWithDebInfo
IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")

ADD_SUBDIRECTORY(src)

INSTALL(FILES spawn-fcgi.1 DESTINATION ${CMAKE_MAN_DIR}/man1)
