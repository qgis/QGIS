cmake_minimum_required(VERSION 3.22.0)
project(lazperf_qgis CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(lazperf SHARED
lazperf/charbuf.cpp
lazperf/filestream.cpp
lazperf/header.cpp
lazperf/lazperf.cpp
lazperf/readers.cpp
lazperf/writers.cpp
lazperf/vlr.cpp
lazperf/detail/field_byte10.cpp
lazperf/detail/field_byte14.cpp
lazperf/detail/field_gpstime10.cpp
lazperf/detail/field_nir14.cpp
lazperf/detail/field_point10.cpp
lazperf/detail/field_point14.cpp
lazperf/detail/field_rgb10.cpp
lazperf/detail/field_rgb14.cpp
)

target_include_directories(lazperf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(QGis::lazperf ALIAS lazperf)

install(TARGETS lazperf
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION ${QGIS_LIB_DIR}
  ARCHIVE DESTINATION ${QGIS_LIB_DIR})