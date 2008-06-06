SET(CMAKE_BACKWARDS_COMPATIBILITY "2.4")

# See if we have svn installed
FIND_PROGRAM(SVNVERSION svnversion)

# Read the revision if installed, else set to "unknown"
IF (SVNVERSION)
  EXEC_PROGRAM (${SVNVERSION} ARGS ${SOURCE_DIR} OUTPUT_VARIABLE REVISION)
ELSE (SVNVERSION)
  SET (REVISION unknown)
ENDIF (SVNVERSION)

# Create Info.plist
CONFIGURE_FILE (${CURRENT_SOURCE_DIR}/Info.plist.in
                ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)
