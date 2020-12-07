# Standard includes and utils to compile into all tests.
set (util_SRCS)

#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
include_directories(
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

include_directories(SYSTEM
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
if (WIN32)
  set(PLATFORM_LIBRARIES wsock32)
endif()

macro (ADD_QGIS_TEST testname testsrc)
  add_executable(qgis_${testname} ${testsrc} ${util_SRCS})
  if(APPLE)
    target_link_libraries(qgis_${testname} ${APP_SERVICES_LIBRARY})
  endif()
  target_link_libraries(qgis_${testname}
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
  add_test(qgis_${testname} ${CMAKE_CURRENT_BINARY_DIR}/../../../output/bin/qgis_${testname} -maxwarnings 10000)
endmacro (ADD_QGIS_TEST)

#############################################################
# Tests:

if(WITH_OAUTH2_PLUGIN)
  ADD_QGIS_TEST(authoauthmethodtest testqgsauthoauth2method.cpp)
  add_dependencies(qgis_authoauthmethodtest oauth2authmethod_static)
  target_link_libraries(qgis_authoauthmethodtest oauth2authmethod_static)
endif()
