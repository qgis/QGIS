if(NOT MSVC)
  add_subdirectory(wms)
  add_subdirectory(wfs)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src/server
  ${CMAKE_SOURCE_DIR}/src/test

  ${CMAKE_BINARY_DIR}/src/server
)

#############################################################
# Tests:

set(TESTS
  testqgsserverquerystringparameter.cpp
)

foreach(TESTSRC ${TESTS})
    add_qgis_test(${TESTSRC} MODULE server LINKEDLIBRARIES qgis_server)
endforeach(TESTSRC)
