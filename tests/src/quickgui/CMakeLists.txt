# Standard includes and utils to compile into all tests.

#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}

    ${CMAKE_SOURCE_DIR}/src/native
    ${CMAKE_SOURCE_DIR}/src/quickgui
    ${CMAKE_SOURCE_DIR}/src/test

    ${CMAKE_BINARY_DIR}/src/native
    ${CMAKE_BINARY_DIR}/src/quickgui
)

include_directories(SYSTEM
    ${PROJ_INCLUDE_DIR}
    ${GEOS_INCLUDE_DIR}
    ${LIBZIP_INCLUDE_DIRS}
    ${SPATIALINDEX_INCLUDE_DIR}
    ${GDAL_INCLUDE_DIR}
    ${EXPAT_INCLUDE_DIR}
    ${SQLITE3_INCLUDE_DIR}
    ${SPATIALITE_INCLUDE_DIR}
    ${QCA_INCLUDE_DIR}
    ${QTKEYCHAIN_INCLUDE_DIR}
)


#############################################################
# Tests:

add_qgis_test(testqgsquickmapsettings.cpp MODULE quick LINKEDLIBRARIES qgis_quick)

#############################################################
# Add also test application
add_subdirectory(app)
