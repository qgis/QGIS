# for installing headers

include_directories(SYSTEM
  ${QT_INCLUDE_DIR}
)

set(QGIS_TEST_HDRS
  qgstest.h
)

add_library(qgis_test ${LIBRARY_TYPE}
  ${QGIS_TEST_HDRS}
)

# require c++17
target_compile_features(qgis_test PRIVATE cxx_std_17)

target_include_directories(qgis_test PUBLIC
  ${CMAKE_SOURCE_DIR}/src/test

  ${CMAKE_BINARY_DIR}/src/test
)

GENERATE_EXPORT_HEADER(
   qgis_test
   BASE_NAME TEST
   EXPORT_FILE_NAME qgis_test.h
)

target_link_libraries(qgis_test qgis_core ${QT_VERSION_BASE}::Test)

set(QGIS_TEST_HDRS ${QGIS_TEST_HDRS} ${CMAKE_CURRENT_BINARY_DIR}/qgis_test.h)

install(FILES ${QGIS_TEST_HDRS} DESTINATION ${QGIS_INCLUDE_DIR})
