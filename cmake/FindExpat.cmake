
# CMake module to search for Expat library
# (library for parsing XML files)
#
# If it's found it sets EXPAT_FOUND to TRUE
# and following variables are set:
#    EXPAT_INCLUDE_DIR
#    EXPAT_LIBRARY


FIND_PATH(EXPAT_INCLUDE_DIR expat.h /usr/local/include /usr/include c:/msys/local/include)

FIND_LIBRARY(EXPAT_LIBRARY NAMES expat PATHS /usr/local/lib /usr/lib c:/msys/local/lib)

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
