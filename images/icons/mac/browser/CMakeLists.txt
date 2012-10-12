
FILE (GLOB ICONS *.icns)
INSTALL (FILES ${ICONS}
         DESTINATION "${CMAKE_INSTALL_PREFIX}/${QGIS_BIN_DIR}/QGIS Browser.app/Contents/Resources")

# create alias now, so alias file gets correct icon, delete first if any exist
SET (BROWSERNAME "QGIS Browser")
INSTALL (CODE "EXECUTE_PROCESS(COMMAND /usr/bin/find \"$ENV{DESTDIR}${QGIS_INSTALL_PREFIX}\" -name \"${BROWSERNAME}*\" -maxdepth 1 -type f -delete)")
INSTALL (CODE "EXECUTE_PROCESS(COMMAND /usr/bin/osascript -e \"tell application \\\"Finder\\\" to make alias file to POSIX file \\\"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${QGIS_BIN_DIR}/${BROWSERNAME}.app\\\" at POSIX file \\\"$ENV{DESTDIR}${QGIS_INSTALL_PREFIX}/\\\"\")")
