# Standard includes and utils to compile into all tests.
set (util_SRCS)

#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
include_directories(${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/tests/core #for render checker class
  ${CMAKE_SOURCE_DIR}/src/test
  ${CMAKE_BINARY_DIR}/src/ui
  ${CMAKE_BINARY_DIR}/src/native
  ${CMAKE_CURRENT_BINARY_DIR}
)
include_directories(SYSTEM
  ${QT_INCLUDE_DIR}
  ${${QT_VERSION_BASE}_3DEXTRA_INCLUDE_DIR}
)


add_subdirectory(sandbox)

set(TESTS
  testqgs3dcameracontroller.cpp
  testqgs3dmaterial.cpp
  testqgs3drendering.cpp
  testqgs3dsymbolregistry.cpp
  testqgs3dutils.cpp
  testqgsaabb.cpp
  testqgsgltf3dutils.cpp
  testqgslayout3dmap.cpp
  testqgsmaterialregistry.cpp
  testqgsmesh3drendering.cpp
  testqgspointcloud3drendering.cpp
  testqgstessellator.cpp
  testqgstilingscheme.cpp
)


foreach(TESTSRC ${TESTS})
  add_qgis_test(${TESTSRC} MODULE 3d LINKEDLIBRARIES qgis_3d)
endforeach(TESTSRC)
