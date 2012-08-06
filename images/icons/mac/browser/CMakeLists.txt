
FILE (GLOB ICONS *.icns)
INSTALL (FILES ${ICONS}
         DESTINATION "${CMAKE_INSTALL_PREFIX}/${QGIS_BIN_DIR}/QGIS Browser.app/Contents/Resources")
# create alias now, so alias file gets correct icon, delete first if it exists
SET (BROWSER_ALIAS "$ENV{DESTDIR}${QGIS_INSTALL_PREFIX}/QGIS Browser.app")
IF (EXISTS ${BROWSER_ALIAS})
    FILE (REMOVE ${BROWSER_ALIAS})
ENDIF ()
INSTALL (CODE "EXECUTE_PROCESS(COMMAND osascript -e \"tell application \\\"Finder\\\" to make alias file to POSIX file \\\"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${QGIS_BIN_DIR}/QGIS Browser.app\\\" at POSIX file \\\"$ENV{DESTDIR}${QGIS_INSTALL_PREFIX}/\\\"\")")
