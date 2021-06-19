########################################################
# Files

if(MSVC)
  add_definitions(-DWIN32_LEAN_AND_MEAN)
endif(MSVC)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(PDAL_SRCS
  qgspdaldataitems.cpp
  qgspdalprovider.cpp
  qgspdaleptgenerationtask.cpp
  ${CMAKE_SOURCE_DIR}/external/untwine/api/QgisUntwine.cpp
)

set(PDAL_HDRS
  qgspdaldataitems.h
  qgspdalprovider.h
  qgspdaleptgenerationtask.h
  ${CMAKE_SOURCE_DIR}/external/untwine/api/QgisUntwine.hpp
)

if (WITH_GUI)
  set(PDAL_GUI_SRCS
    qgspdalprovidergui.cpp
    qgspdaldataitemguiprovider.cpp
  )

  set(PDAL_GUI_HDRS
    qgspdalprovidergui.h
    qgspdaldataitemguiprovider.h
  )
endif()

set(UNTWINE_SRCS
  ${CMAKE_SOURCE_DIR}/external/untwine/bu/BuPyramid.cpp
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

  ${CMAKE_SOURCE_DIR}/external/untwine/untwine/MapFile.cpp
  ${CMAKE_SOURCE_DIR}/external/untwine/untwine/ProgressWriter.cpp
  ${CMAKE_SOURCE_DIR}/external/untwine/untwine/ThreadPool.cpp
  ${CMAKE_SOURCE_DIR}/external/untwine/untwine/Untwine.cpp
)

set(UNTWINE_HDRS
  ${CMAKE_SOURCE_DIR}/external/untwine/bu/BuPyramid.hpp
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
  ${CMAKE_SOURCE_DIR}/external/untwine/untwine/FileDimInfo.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/untwine/GridKey.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/untwine/MapFile.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/untwine/Point.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/untwine/ProgressWriter.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/untwine/ThreadPool.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/untwine/VoxelKey.hpp
)

configure_file(${CMAKE_SOURCE_DIR}/external/untwine/untwine/Config.hpp.in ${CMAKE_BINARY_DIR}/untwine/Config.hpp)

set(UNTWINE_INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/external/untwine/untwine
  ${CMAKE_SOURCE_DIR}/external/untwine/epf
  ${CMAKE_SOURCE_DIR}/external/untwine/bu
  ${CMAKE_SOURCE_DIR}/untwine/api
  ${CMAKE_BINARY_DIR}/untwine
)

########################################################
# Build

include_directories(
  ${CMAKE_SOURCE_DIR}/external/untwine/api
)

include_directories(SYSTEM
  ${PDAL_INCLUDE_DIR}
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
target_include_directories(untwine PRIVATE ${UNTWINE_INCLUDE_DIRS})

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
    ${Qt5Xml_LIBRARIES}
    ${Qt5Core_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${Qt5Network_LIBRARIES}
    ${Qt5Sql_LIBRARIES}
    ${Qt5Concurrent_LIBRARIES}
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
  ${Qt5Xml_LIBRARIES}
  ${Qt5Core_LIBRARIES}
  ${Qt5Svg_LIBRARIES}
  ${Qt5Network_LIBRARIES}
  ${Qt5Sql_LIBRARIES}
  ${Qt5Concurrent_LIBRARIES}
  qgis_core
)

if (WITH_GUI)
  add_library (provider_pdal_gui_a STATIC ${PDAL_GUI_SRCS} ${PDAL_GUI_HDRS})

  # require c++17
  target_compile_features(provider_pdal_gui_a PRIVATE cxx_std_17)

  target_link_libraries (provider_pdal_gui_a
    ${PDAL_LIBRARIES}
    ${Qt5Xml_LIBRARIES}
    ${Qt5Core_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${Qt5Network_LIBRARIES}
    ${Qt5Sql_LIBRARIES}
    ${Qt5Concurrent_LIBRARIES}
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
