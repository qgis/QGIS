# Standard includes and utils to compile into all tests.
set (util_SRCS)

#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src/auth/oauth2/core
  ${CMAKE_SOURCE_DIR}/src/test
  ${CMAKE_BINARY_DIR}/src/gui/auth
  ${CMAKE_BINARY_DIR}/src/auth/oauth2
)

if(WITH_GUI)
    include_directories(
      ${CMAKE_SOURCE_DIR}/src/auth/oauth2/gui
    )
endif()

# libraries

# because of htonl
if (WIN32)
  set(PLATFORM_LIBRARIES wsock32)
endif()


#############################################################
# Tests:

set(LINKEDLIBRARIES qgis_core authmethod_oauth2_a)
if(WITH_GUI)
  set(LINKEDLIBRARIES ${LINKEDLIBRARIES} qgis_gui)
endif()
if(APPLE)
  set(LINKEDLIBRARIES ${LINKEDLIBRARIES} ${APP_SERVICES_LIBRARY})
endif()

if(WITH_OAUTH2_PLUGIN)
  ADD_QGIS_TEST(testqgsauthoauth2method.cpp MODULE authmethod LINKEDLIBRARIES ${LINKEDLIBRARIES})
endif()
