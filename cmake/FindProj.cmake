
# CMake module to search for Proj library
#
# If it's found it sets PROJ_FOUND to TRUE
# and following variables are set:
#    PROJ_INCLUDE_DIR
#    PROJ_LIBRARY


FIND_PATH(PROJ_INCLUDE_DIR proj_api.h 
  /usr/local/include 
  /usr/include 
  c:/msys/local/include
  C:/dev/cpp/proj-4.5.0/src
  )

FIND_LIBRARY(PROJ_LIBRARY NAMES proj PATHS 
  /usr/local/lib 
  /usr/lib 
  c:/msys/local/lib
  C:/dev/cpp/proj-4.5.0/src
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
