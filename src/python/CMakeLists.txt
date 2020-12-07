set(QGISPYTHON_SRCS qgispython.cpp qgspythonutilsimpl.cpp)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/core

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/python
)
include_directories(SYSTEM
  ${PYTHON_INCLUDE_PATH}
)

add_library (qgispython SHARED ${QGISPYTHON_SRCS})

set_target_properties(qgispython PROPERTIES
  CLEAN_DIRECT_OUTPUT 1
)

#generate unversioned libs for android
if (NOT ANDROID)
  set_target_properties(qgispython PROPERTIES
    VERSION ${COMPLETE_VERSION}
    SOVERSION ${COMPLETE_VERSION}
  )
endif()

target_link_libraries(qgispython
  qgis_core
  ${PYTHON_LIBRARY}
  ${OPENPTY_LIBRARY}
)

GENERATE_EXPORT_HEADER(
   qgispython
   BASE_NAME PYTHON
   EXPORT_FILE_NAME qgis_python.h
 )

if (APPLE)
  set_target_properties(qgispython PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
endif()

install(TARGETS qgispython
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION ${QGIS_LIB_DIR}
)
