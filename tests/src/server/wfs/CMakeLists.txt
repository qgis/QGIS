#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
include_directories(${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src/test
  ${CMAKE_SOURCE_DIR}/src/server
  ${CMAKE_SOURCE_DIR}/src/server/services
  ${CMAKE_SOURCE_DIR}/src/server/services/wfs

  ${CMAKE_BINARY_DIR}/src/server

  ${CMAKE_CURRENT_BINARY_DIR}
)

#note for tests we should not include the moc of our
#qtests in the executable file list as the moc is
#directly included in the sources
#and should not be compiled twice. Trying to include
#them in will cause an error at build time

#No relinking and full RPATH for the install tree
#See: http://www.cmake.org/Wiki/CMake_RPATH_handling#No_relinking_and_full_RPATH_for_the_install_tree
set(MODULE_WFS_SRCS
  ${CMAKE_SOURCE_DIR}/src/server/services/wfs/qgswfsgetfeature.cpp
)

set(MODULE_WFS_HDRS
)

qt_wrap_cpp(MODULE_WFS_MOC_SRCS ${MODULE_WFS_HDRS})

#############################################################
# Tests:

set(TESTS
  test_qgsserver_wfs_getfeature.cpp
)

foreach(TESTSRC ${TESTS})
    ADD_QGIS_TEST(${TESTSRC} MODULE server LINKEDLIBRARIES qgis_server wfs_static)
endforeach(TESTSRC)
