
# CMake module to search for Sqlite3 library
#
# If it's found it sets SQLITE3_FOUND to TRUE
# and following variables are set:
#    SQLITE3_INCLUDE_DIR
#    SQLITE3_LIBRARY


# Normally there is no need to specify /usr/... paths because 
# cmake will look there automatically. However the NO_DEFAULT_PATH
# prevents this behaviour allowing you to use no standard file
# locations in preference over standard ones. Note in this case
# you then need to explicitly add /usr and /usr/local prefixes
# to the search list. This applies both to FIND_PATH and FIND_LIBRARY
FIND_PATH(SQLITE3_INCLUDE_DIR sqlite3.h 
  "$ENV{LIB_DIR}/include"
  /usr/local/include 
  /usr/include 
  #mingw
  c:/msys/local/include
  NO_DEFAULT_PATH
  )

FIND_LIBRARY(SQLITE3_LIBRARY NAMES sqlite3 PATHS 
  "$ENV{LIB_DIR}/lib"
  /usr/local/lib 
  /usr/lib 
  c:/msys/local/lib
  NO_DEFAULT_PATH
  )

IF (SQLITE3_INCLUDE_DIR AND SQLITE3_LIBRARY)
   SET(SQLITE3_FOUND TRUE)
ENDIF (SQLITE3_INCLUDE_DIR AND SQLITE3_LIBRARY)


IF (SQLITE3_FOUND)

   IF (NOT SQLITE3_FIND_QUIETLY)
      MESSAGE(STATUS "Found Sqlite3: ${SQLITE3_LIBRARY}")
   ENDIF (NOT SQLITE3_FIND_QUIETLY)

ELSE (SQLITE3_FOUND)

   IF (SQLITE3_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find Sqlite3")
   ENDIF (SQLITE3_FIND_REQUIRED)

ENDIF (SQLITE3_FOUND)
