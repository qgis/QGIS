
# CMake module to search for Expat library
# (library for parsing XML files)
#
# If it's found it sets EXPAT_FOUND to TRUE
# and following variables are set:
#    EXPAT_INCLUDE_DIR
#    EXPAT_LIBRARY

# Normally there is no need to specify /usr/... paths because 
# cmake will look there automatically. However the NO_DEFAULT_PATH
# prevents this behaviour allowing you to use no standard file
# locations in preference over standard ones. Note in this case
# you then need to explicitly add /usr and /usr/local prefixes
# to the search list. This applies both to FIND_PATH and FIND_LIBRARY
FIND_PATH(EXPAT_INCLUDE_DIR expat.h 
  "$ENV{LIB_DIR}/include/"
  "$ENV{LIB_DIR}/include/expat"
  /Users/tim/dev/universal_libs/include
  /usr/local/include 
  /usr/include 
  c:/msys/local/include
  NO_DEFAULT_PATH
  )
#libexpat needed for msvc version
FIND_LIBRARY(EXPAT_LIBRARY NAMES expat libexpat PATHS 
  "$ENV{LIB_DIR}/lib"
  /usr/local/lib 
  /usr/lib 
  c:/msys/local/lib
  NO_DEFAULT_PATH
  )

IF (EXPAT_INCLUDE_DIR AND EXPAT_LIBRARY)
   SET(EXPAT_FOUND TRUE)
ENDIF (EXPAT_INCLUDE_DIR AND EXPAT_LIBRARY)


IF (EXPAT_FOUND)

   IF (NOT EXPAT_FIND_QUIETLY)
      MESSAGE(STATUS "Found Expat: ${EXPAT_LIBRARY}")
   ENDIF (NOT EXPAT_FIND_QUIETLY)

ELSE (EXPAT_FOUND)

   IF (EXPAT_FIND_REQUIRED)
     MESSAGE(FATAL_ERROR "Could not find Expat")
   ELSE (EXPAT_FIND_REQUIRED)
     IF (NOT EXPAT_FIND_QUIETLY)
        MESSAGE(STATUS "Could not find Expat")
     ENDIF (NOT EXPAT_FIND_QUIETLY)
   ENDIF (EXPAT_FIND_REQUIRED)

ENDIF (EXPAT_FOUND)
