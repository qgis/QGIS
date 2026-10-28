########################################################
# Files

BISON_TARGET(QgsCql2Parser qgscql2parser.yy ${CMAKE_CURRENT_BINARY_DIR}/qgscql2parser.cpp COMPILE_FLAGS "-p cql2_")
FLEX_TARGET(QgsCql2Lexer qgscql2lexer.ll ${CMAKE_CURRENT_BINARY_DIR}/qgscql2lexer.cpp)
ADD_FLEX_BISON_DEPENDENCY(QgsCql2Lexer QgsCql2Parser)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/qgscql2parser.cpp PROPERTIES COMPILE_FLAGS "-Wno-unused-but-set-variable")
  set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/qgscql2lexer.cpp PROPERTIES COMPILE_FLAGS "-Wno-implicit-fallthrough")
endif()

set (WFS3_SRCS
  ${CMAKE_SOURCE_DIR}/external/inja/inja/inja.hpp
  qgswfs3.cpp
  qgswfs3handlers.cpp

  ${FLEX_QgsCql2Lexer_OUTPUTS}
  ${BISON_QgsCql2Parser_OUTPUTS}
)

########################################################
# Build

add_library (wfs3 MODULE ${WFS3_SRCS})

include_directories(
  ${CMAKE_SOURCE_DIR}/src/server
  ${CMAKE_SOURCE_DIR}/src/server/services
  ${CMAKE_SOURCE_DIR}/src/server/services/wfs3

  ${CMAKE_BINARY_DIR}/src/python
  ${CMAKE_BINARY_DIR}/src/analysis
  ${CMAKE_BINARY_DIR}/src/server

  ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(wfs3
  qgis_core
  qgis_server
)

########################################################
# Install

install(TARGETS wfs3
    RUNTIME DESTINATION ${QGIS_SERVER_MODULE_DIR}
    LIBRARY DESTINATION ${QGIS_SERVER_MODULE_DIR}
)
