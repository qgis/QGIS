
FILE (GLOB ICONS *.icns)
INSTALL (FILES ${ICONS}
         DESTINATION "${CMAKE_INSTALL_PREFIX}/${QGIS_BIN_DIR}/QGIS Browser.app/Contents/Resources")
# create alias now, so alias file gets correct icon, delete first if it exists
# could be more than one alias if previous build failed
FILE (GLOB ALIASES "$ENV{DESTDIR}${QGIS_INSTALL_PREFIX}/QGIS Browser.app*")
FOREACH(browserAlias ${ALIASES})
    FILE (REMOVE "${browserAlias}")
ENDFOREACH(browserAlias)
INSTALL (CODE "EXECUTE_PROCESS(COMMAND osascript -e \"tell application \\\"Finder\\\" to make alias file to POSIX file \\\"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${QGIS_BIN_DIR}/QGIS Browser.app\\\" at POSIX file \\\"$ENV{DESTDIR}${QGIS_INSTALL_PREFIX}/\\\"\")")
