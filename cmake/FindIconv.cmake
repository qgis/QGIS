# CMake module to search for iconv library
#
# If it's found it sets ICONV_FOUND to TRUE
# and following variables are set:
#    ICONV_INCLUDE_DIR
#    ICONV_LIBRARY

# FIND_PATH and FIND_LIBRARY normally search standard locations
# before the specified paths. To search non-standard paths first,
# FIND_* is invoked first with specified paths and NO_DEFAULT_PATH
# and then again with no specified paths to search the default
# locations. When an earlier FIND_* succeeds, subsequent FIND_*s
# searching for the same item do nothing. 
FIND_PATH(ICONV_INCLUDE_DIR iconv.h
  "$ENV{LIB_DIR}/include"
  /usr/local/include
  /usr/include
  #mingw
  c:/msys/local/include
  NO_DEFAULT_PATH
  )
FIND_PATH(ICONV_INCLUDE_DIR iconv.h)

FIND_LIBRARY(ICONV_LIBRARY NAMES iconv PATHS
  "$ENV{LIB_DIR}/lib"
  /usr/local/lib
  /usr/lib
  #mingw
  c:/msys/local/lib
  NO_DEFAULT_PATH
  )

IF (ICONV_INCLUDE_DIR AND ICONV_LIBRARY)
   SET(ICONV_FOUND TRUE)
ENDIF (ICONV_INCLUDE_DIR AND ICONV_LIBRARY)

IF (ICONV_FOUND)
   IF (NOT ICONV_FIND_QUIETLY)
      MESSAGE(STATUS "Found Iconv: ${ICONV_LIBRARY}")
   ENDIF (NOT ICONV_FIND_QUIETLY)
ELSE (ICONV_FOUND)
   IF (ICONV_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find Iconv")
   ENDIF (ICONV_FIND_REQUIRED)
ENDIF (ICONV_FOUND)
