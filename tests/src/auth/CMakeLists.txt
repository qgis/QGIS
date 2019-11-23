# Standard includes and utils to compile into all tests.
SET (util_SRCS)

#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src/auth/oauth2
  ${CMAKE_SOURCE_DIR}/src/auth/oauth2/qjsonwrapper
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/auth
  ${CMAKE_SOURCE_DIR}/src/test
  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/gui/auth
  ${CMAKE_BINARY_DIR}/src/auth/oauth2
)

INCLUDE_DIRECTORIES(SYSTEM
  ${GDAL_INCLUDE_DIR}
  ${PROJ_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
  ${SQLITE3_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
  ${O2_INCLUDE_DIR}
)

# libraries

# because of htonl
IF (WIN32)
  SET(PLATFORM_LIBRARIES wsock32)
ENDIF (WIN32)

MACRO (ADD_QGIS_TEST testname testsrc)
  ADD_EXECUTABLE(qgis_${testname} ${testsrc} ${util_SRCS})
  IF(APPLE)
    TARGET_LINK_LIBRARIES(qgis_${testname} ${APP_SERVICES_LIBRARY})
  ENDIF(APPLE)
  TARGET_LINK_LIBRARIES(qgis_${testname}
    ${Qt5Core_LIBRARIES}
    ${Qt5Xml_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${Qt5Test_LIBRARIES}
    ${Qt5Network_LIBRARIES}
    ${PROJ_LIBRARY}
    ${GEOS_LIBRARY}
    ${GDAL_LIBRARY}
    ${OPTIONAL_QTWEBKIT}
    ${PROJ_LIBRARY}
    ${GEOS_LIBRARY}
    ${GDAL_LIBRARY}
    qgis_core
    qgis_gui
    )
  ADD_TEST(qgis_${testname} ${CMAKE_CURRENT_BINARY_DIR}/../../../output/bin/qgis_${testname} -maxwarnings 10000)
ENDMACRO (ADD_QGIS_TEST)

#############################################################
# Tests:

IF(WITH_OAUTH2_PLUGIN)
  ADD_QGIS_TEST(authoauthmethodtest testqgsauthoauth2method.cpp)
  ADD_DEPENDENCIES(qgis_authoauthmethodtest oauth2authmethod_static)
  TARGET_LINK_LIBRARIES(qgis_authoauthmethodtest oauth2authmethod_static)
ENDIF()
