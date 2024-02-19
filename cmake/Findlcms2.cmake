# Denton: Found in LibMNG sources
#
# - Find LCMS version 2 library
#
#  LCMS2_INCLUDE_DIR    - Where to find lcms2.h, etc.
#  LCMS2_LIBRARIES      - Libraries to link against to use LCMS2.
#  LCMS2_FOUND          - If false, do not try to use LCMS2.
#
# also defined, but not for general use are
#  LCMS2_LIBRARY        - Where to find the LCMS2 library.

FIND_PATH(LCMS2_INCLUDE_DIR lcms2.h PATHS /usr/include /usr/local/include /opt/include /opt/local/include)

SET(LCMS2_NAMES ${LCMS2_NAMES} lcms2 liblcms2 liblcms2_static)

FIND_LIBRARY(LCMS2_LIBRARY NAMES ${LCMS2_NAMES} )

MARK_AS_ADVANCED(LCMS2_INCLUDE_DIR LCMS2_LIBRARY)

# handle the QUIETLY and REQUIRED arguments and set LCMS2_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(lcms2  DEFAULT_MSG  LCMS2_LIBRARY  LCMS2_INCLUDE_DIR)

IF(LCMS2_FOUND)
  SET(LCMS2_INCLUDE_DIRS ${LCMS2_INCLUDE_DIR})
  SET(LCMS2_LIBRARIES ${LCMS2_LIBRARY} )
ENDIF(LCMS2_FOUND)
