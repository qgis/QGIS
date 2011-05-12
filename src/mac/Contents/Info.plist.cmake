SET(CMAKE_BACKWARDS_COMPATIBILITY "2.4")

# See if we have git installed
FIND_PROGRAM (GIT git)

# Read the revision if installed, else set to "unknown"
IF (GIT)
  EXEC_PROGRAM (${GIT} ${SOURCE_DIR} ARGS "log -n1 --pretty=%h" OUTPUT_VARIABLE REVISION)
ELSE (GIT)
  SET (REVISION unknown)
ENDIF (GIT)

# Create Info.plist
CONFIGURE_FILE (${CURRENT_SOURCE_DIR}/Info.plist.in
                ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)
