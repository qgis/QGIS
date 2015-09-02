# Find EXIV2
# ~~~~~~~~~~
# CMake module to search for EXIV2 library
#
# If it's found it sets EXIV2_FOUND to TRUE
# and following variables are set:
#    EXIV2_INCLUDE_DIR
#    EXIV2_LIBRARY
#


FIND_PATH(EXIV2_INCLUDE_DIR exiv2/exiv2.hpp /usr/local/include /usr/include)
FIND_LIBRARY(EXIV2_LIBRARY NAMES exiv2 PATHS /usr/local/lib /usr/lib)

IF (EXIV2_INCLUDE_DIR AND EXIV2_LIBRARY)
    SET(EXIV2_FOUND TRUE)
    MESSAGE(STATUS "Found exiv2: ${EXIV2_LIBRARY}")
ELSE (EXIV2_INCLUDE_DIR AND EXIV2_LIBRARY)
    MESSAGE(EXIV2_INCLUDE_DIR=${EXIV2_INCLUDE_DIR})
    MESSAGE(EXIV2_LIBRARY=${EXIV2_LIBRARY})
    MESSAGE(FATAL_ERROR "Could not find exiv2")
ENDIF (EXIV2_INCLUDE_DIR AND EXIV2_LIBRARY)
