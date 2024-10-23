########################################################
# Files

if(MSVC)
  set(PDAL_LIBRARIES ${PDAL_LIBRARIES} ws2_32)
endif(MSVC)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(PDAL_SRCS
  qgspdalprovider.cpp
  qgspdalindexingtask.cpp
  ${CMAKE_SOURCE_DIR}/external/untwine/api/QgisUntwine.cpp
)

set(PDAL_HDRS
  qgspdalprovider.h
  qgspdalindexingtask.h
  ${CMAKE_SOURCE_DIR}/external/untwine/api/QgisUntwine.hpp
)

if (WITH_GUI)
  set(PDAL_GUI_SRCS
    qgspdalprovidergui.cpp
  )

  set(PDAL_GUI_HDRS
    qgspdalprovidergui.h
  )
endif()

set(UNTWINE_SRCS
  ${CMAKE_SOURCE_DIR}/external/untwine/bu/BuPyramid.cpp
  ${CMAKE_SOURCE_DIR}/external/untwine/bu/CopcSupport.cpp
  ${CMAKE_SOURCE_DIR}/external/untwine/bu/OctantInfo.cpp
  ${CMAKE_SOURCE_DIR}/external/untwine/bu/Processor.cpp
  ${CMAKE_SOURCE_DIR}/external/untwine/bu/PyramidManager.cpp
  ${CMAKE_SOURCE_DIR}/external/untwine/bu/Stats.cpp

  ${CMAKE_SOURCE_DIR}/external/untwine/epf/BufferCache.cpp
  ${CMAKE_SOURCE_DIR}/external/untwine/epf/Cell.cpp
  ${CMAKE_SOURCE_DIR}/external/untwine/epf/Epf.cpp
  ${CMAKE_SOURCE_DIR}/external/untwine/epf/FileProcessor.cpp
  ${CMAKE_SOURCE_DIR}/external/untwine/epf/Grid.cpp
  ${CMAKE_SOURCE_DIR}/external/untwine/epf/Reprocessor.cpp
  ${CMAKE_SOURCE_DIR}/external/untwine/epf/Writer.cpp

  ${CMAKE_SOURCE_DIR}/external/untwine/untwine/ProgressWriter.cpp
  ${CMAKE_SOURCE_DIR}/external/untwine/untwine/ThreadPool.cpp
  ${CMAKE_SOURCE_DIR}/external/untwine/untwine/Untwine.cpp
)

set(UNTWINE_HDRS
  ${CMAKE_SOURCE_DIR}/external/untwine/bu/BuPyramid.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/bu/CopcSupport.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/bu/FileInfo.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/bu/OctantInfo.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/bu/PointAccessor.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/bu/Processor.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/bu/PyramidManager.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/bu/Stats.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/bu/VoxelInfo.hpp

  ${CMAKE_SOURCE_DIR}/external/untwine/epf/BufferCache.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/epf/Cell.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/epf/Epf.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/epf/EpfTypes.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/epf/FileProcessor.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/epf/Grid.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/epf/Reprocessor.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/epf/Writer.hpp

  ${CMAKE_SOURCE_DIR}/external/untwine/untwine/Common.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/untwine/FatalError.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/untwine/FileDimInfo.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/untwine/GridKey.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/untwine/Point.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/untwine/ProgressWriter.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/untwine/ThreadPool.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/untwine/VoxelKey.hpp
)

# MINGW must come before WIN32 test
# APPLE must come before UNIX test
if (MINGW)
    set(UNTWINE_OS_DIR ${CMAKE_SOURCE_DIR}/external/untwine/untwine/mingw)
elseif (WIN32)
    set(UNTWINE_OS_DIR ${CMAKE_SOURCE_DIR}/external/untwine/untwine/windows)
elseif (APPLE)
    set(UNTWINE_OS_DIR ${CMAKE_SOURCE_DIR}/external/untwine/untwine/osx)
elseif (UNIX)
    set(UNTWINE_OS_DIR ${CMAKE_SOURCE_DIR}/external/untwine/untwine/generic)
else()
    message(FATAL_ERROR "OS not supported")
endif()

configure_file(${CMAKE_SOURCE_DIR}/external/untwine/untwine/Config.hpp.in ${CMAKE_BINARY_DIR}/untwine/Config.hpp)

set(UNTWINE_INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/external/untwine/untwine
  ${CMAKE_SOURCE_DIR}/external/untwine/epf
  ${CMAKE_SOURCE_DIR}/external/untwine/bu
  ${CMAKE_SOURCE_DIR}/external/untwine
  ${CMAKE_SOURCE_DIR}/untwine/api
  ${CMAKE_BINARY_DIR}/untwine
)

if (NOT LazPerf_FOUND)
  set(UNTWINE_SRCS
    ${UNTWINE_SRCS}
    ${CMAKE_SOURCE_DIR}/external/lazperf/charbuf.cpp
    ${CMAKE_SOURCE_DIR}/external/lazperf/filestream.cpp
    ${CMAKE_SOURCE_DIR}/external/lazperf/header.cpp
    ${CMAKE_SOURCE_DIR}/external/lazperf/lazperf.cpp
    ${CMAKE_SOURCE_DIR}/external/lazperf/readers.cpp
    ${CMAKE_SOURCE_DIR}/external/lazperf/vlr.cpp
    ${CMAKE_SOURCE_DIR}/external/lazperf/writers.cpp
    ${CMAKE_SOURCE_DIR}/external/lazperf/detail/field_byte10.cpp
    ${CMAKE_SOURCE_DIR}/external/lazperf/detail/field_byte14.cpp
    ${CMAKE_SOURCE_DIR}/external/lazperf/detail/field_gpstime10.cpp
    ${CMAKE_SOURCE_DIR}/external/lazperf/detail/field_nir14.cpp
    ${CMAKE_SOURCE_DIR}/external/lazperf/detail/field_point10.cpp
    ${CMAKE_SOURCE_DIR}/external/lazperf/detail/field_point14.cpp
    ${CMAKE_SOURCE_DIR}/external/lazperf/detail/field_rgb10.cpp
    ${CMAKE_SOURCE_DIR}/external/lazperf/detail/field_rgb14.cpp
  )
endif()

set(PDAL_WRENCH_SRCS
  ${CMAKE_SOURCE_DIR}/external/pdal_wrench/main.cpp
  ${CMAKE_SOURCE_DIR}/external/pdal_wrench/alg.cpp
  ${CMAKE_SOURCE_DIR}/external/pdal_wrench/boundary.cpp
  ${CMAKE_SOURCE_DIR}/external/pdal_wrench/clip.cpp
  ${CMAKE_SOURCE_DIR}/external/pdal_wrench/density.cpp
  ${CMAKE_SOURCE_DIR}/external/pdal_wrench/info.cpp
  ${CMAKE_SOURCE_DIR}/external/pdal_wrench/merge.cpp
  ${CMAKE_SOURCE_DIR}/external/pdal_wrench/thin.cpp
  ${CMAKE_SOURCE_DIR}/external/pdal_wrench/to_raster.cpp
  ${CMAKE_SOURCE_DIR}/external/pdal_wrench/to_raster_tin.cpp
  ${CMAKE_SOURCE_DIR}/external/pdal_wrench/to_vector.cpp
  ${CMAKE_SOURCE_DIR}/external/pdal_wrench/translate.cpp
  ${CMAKE_SOURCE_DIR}/external/pdal_wrench/utils.cpp
  ${CMAKE_SOURCE_DIR}/external/pdal_wrench/vpc.cpp

  ${CMAKE_SOURCE_DIR}/external/pdal_wrench/tile/tile.cpp
  ${CMAKE_SOURCE_DIR}/external/pdal_wrench/tile/BufferCache.cpp
  ${CMAKE_SOURCE_DIR}/external/pdal_wrench/tile/Cell.cpp
  ${CMAKE_SOURCE_DIR}/external/pdal_wrench/tile/FileProcessor.cpp
  ${CMAKE_SOURCE_DIR}/external/pdal_wrench/tile/Las.cpp
  ${CMAKE_SOURCE_DIR}/external/pdal_wrench/tile/TileGrid.cpp
  ${CMAKE_SOURCE_DIR}/external/pdal_wrench/tile/ThreadPool.cpp
  ${CMAKE_SOURCE_DIR}/external/pdal_wrench/tile/Writer.cpp
)

########################################################
# Build

include_directories(
  ${CMAKE_SOURCE_DIR}/external/untwine/api
)

add_executable(untwine ${UNTWINE_SRCS} ${UNTWINE_HDRS})
target_compile_features(untwine PRIVATE cxx_std_17)

set_target_properties(untwine PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/${QGIS_LIBEXEC_SUBDIR}
)

target_link_libraries (untwine
  ${PDAL_LIBRARIES}
  Threads::Threads
)
target_include_directories(untwine PRIVATE ${UNTWINE_OS_DIR} ${UNTWINE_INCLUDE_DIRS})

if (LazPerf_FOUND)
  # Use system laz-perf
  target_include_directories(untwine PRIVATE ${LazPerf_INCLUDE_DIR})
else()
  target_include_directories(untwine PRIVATE ${CMAKE_SOURCE_DIR}/external)
endif()

if (PDAL_2_5_OR_HIGHER)
  add_executable(pdal_wrench ${PDAL_WRENCH_SRCS})
  target_compile_features(pdal_wrench PRIVATE cxx_std_17)

  set_target_properties(pdal_wrench PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/${QGIS_LIBEXEC_SUBDIR}
  )

  target_include_directories(pdal_wrench PRIVATE
    ${CMAKE_SOURCE_DIR}/external
  )

  target_link_libraries (pdal_wrench PRIVATE
    ${PDAL_LIBRARIES}
    GDAL::GDAL
    Threads::Threads
  )

  install(TARGETS pdal_wrench
    RUNTIME DESTINATION ${QGIS_LIBEXEC_DIR}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif()

add_library (provider_pdal MODULE ${PDAL_SRCS} ${PDAL_HDRS} ${PDAL_GUI_SRCS} ${PDAL_GUI_HDRS})

target_compile_features(provider_pdal PRIVATE cxx_std_17)

target_link_libraries (provider_pdal
  qgis_core
  ${PDAL_LIBRARIES}
  Threads::Threads
)

if (WITH_GUI)
  target_link_libraries (provider_pdal
    ${PDAL_LIBRARIES}
    ${QT_VERSION_BASE}::Xml
    ${QT_VERSION_BASE}::Core
    ${QT_VERSION_BASE}::Svg
    ${QT_VERSION_BASE}::Network
    ${QT_VERSION_BASE}::Sql
    ${QT_VERSION_BASE}::Concurrent
    qgis_gui
  )
  add_dependencies(provider_pdal ui)
endif()

# static library
add_library (provider_pdal_a STATIC ${PDAL_SRCS} ${PDAL_HDRS})

# require c++17
target_compile_features(provider_pdal_a PRIVATE cxx_std_17)

target_link_libraries (provider_pdal_a
  ${PDAL_LIBRARIES}
  ${QT_VERSION_BASE}::Xml
  ${QT_VERSION_BASE}::Core
  ${QT_VERSION_BASE}::Svg
  ${QT_VERSION_BASE}::Network
  ${QT_VERSION_BASE}::Sql
  ${QT_VERSION_BASE}::Concurrent
  qgis_core
)

if (WITH_GUI)
  add_library (provider_pdal_gui_a STATIC ${PDAL_GUI_SRCS} ${PDAL_GUI_HDRS})

  # require c++17
  target_compile_features(provider_pdal_gui_a PRIVATE cxx_std_17)

  target_link_libraries (provider_pdal_gui_a
    ${PDAL_LIBRARIES}
    ${QT_VERSION_BASE}::Xml
    ${QT_VERSION_BASE}::Core
    ${QT_VERSION_BASE}::Svg
    ${QT_VERSION_BASE}::Network
    ${QT_VERSION_BASE}::Sql
    ${QT_VERSION_BASE}::Concurrent
    qgis_gui
  )
  add_dependencies(provider_pdal_gui_a ui)
endif()

install(TARGETS provider_pdal
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
# override default path where built files are put to allow running qgis without installing

install(TARGETS untwine
  RUNTIME DESTINATION ${QGIS_LIBEXEC_DIR}
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
