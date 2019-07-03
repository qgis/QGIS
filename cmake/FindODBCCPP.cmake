# Find odbc-cpp-wrapper
# ~~~~~~~~~~~~~~~
# CMake module to search for C++ Wrapper for ODBC from:
#    https://github.com/SAP/odbc-cpp-wrapper

FIND_PACKAGE(ODBC REQUIRED)

IF (NOT ODBCCPP_INCLUDE_DIR)
  FIND_PATH(ODBCCPP_INCLUDE_DIR odbc/Environment.h
    PATHS
     /usr/local/include 
     /usr/include 
     c:/msys/local/include
     "$ENV{LIB_DIR}/include"
     $ENV{INCLUDE}
     "$ENV{ODBCCPP_PATH}/include"
  )
ENDIF (NOT ODBCCPP_INCLUDE_DIR)

IF (NOT ODBCCPP_LIBRARY)
  FIND_LIBRARY(ODBCCPP_LIBRARY ODBCCPP
    PATHS
     /usr/lib    
     /usr/local/lib
     c:/msys/local/lib
     "$ENV{LIB_DIR}/lib"
     $ENV{LIB}
     "$ENV{ODBCCPP_PATH}/lib"
  )
ENDIF (NOT ODBCCPP_LIBRARY)

IF (ODBCCPP_INCLUDE_DIR AND ODBCCPP_LIBRARY)
   SET(ODBCCPP_FOUND TRUE)
ENDIF (ODBCCPP_INCLUDE_DIR AND ODBCCPP_LIBRARY)

IF (ODBCCPP_FOUND)
 IF (NOT ODBCCPP_FIND_QUIETLY)
    MESSAGE(STATUS "Found odbc-cpp: ${ODBCCPP_LIBRARY}")
 ENDIF (NOT ODBCCPP_FIND_QUIETLY)
ELSE (ODBCCPP_FOUND)
   IF (ODBCCPP_FIND_REQUIRED)
     MESSAGE(FATAL_ERROR "Could not find odbc-cpp library")
   ELSE (ODBCCPP_FIND_REQUIRED)
     IF (NOT ODBCCPP_FIND_QUIETLY)
        MESSAGE(STATUS "Could not find odbc-cpp library")
     ENDIF (NOT ODBCCPP_FIND_QUIETLY)
   ENDIF (ODBCCPP_FIND_REQUIRED)
ENDIF (ODBCCPP_FOUND)
