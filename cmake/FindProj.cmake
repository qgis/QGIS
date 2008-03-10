
# CMake module to search for Proj library
#
# If it's found it sets PROJ_FOUND to TRUE
# and following variables are set:
#    PROJ_INCLUDE_DIR
#    PROJ_LIBRARY


# Normally there is no need to specify /usr/... paths because 
# cmake will look there automatically. However the NO_DEFAULT_PATH
# prevents this behaviour allowing you to use no standard file
# locations in preference over standard ones. Note in this case
# you then need to explicitly add /usr and /usr/local prefixes
# to the search list. This applies both to FIND_PATH and FIND_LIBRARY
FIND_PATH(PROJ_INCLUDE_DIR proj_api.h 
  "$ENV{LIB_DIR}/include/proj"
  "$ENV{LIB_DIR}/include"
  /usr/local/include 
  /usr/include 
  #mingw
  c:/msys/local/include
  NO_DEFAULT_PATH
  )

FIND_LIBRARY(PROJ_LIBRARY NAMES proj PATHS 
  "$ENV{LIB_DIR}/lib"
  /usr/local/lib 
  /usr/lib 
  c:/msys/local/lib
  NO_DEFAULT_PATH
  )

IF (PROJ_INCLUDE_DIR AND PROJ_LIBRARY)
   SET(PROJ_FOUND TRUE)
ENDIF (PROJ_INCLUDE_DIR AND PROJ_LIBRARY)


IF (PROJ_FOUND)

   IF (NOT PROJ_FIND_QUIETLY)
      MESSAGE(STATUS "Found Proj: ${PROJ_LIBRARY}")
   ENDIF (NOT PROJ_FIND_QUIETLY)

ELSE (PROJ_FOUND)

   IF (PROJ_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find Proj")
   ENDIF (PROJ_FIND_REQUIRED)

ENDIF (PROJ_FOUND)
