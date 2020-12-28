########################################################
# Files


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
  ${CMAKE_SOURCE_DIR}/external/untwine/bu/BuTypes.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/bu/FileInfo.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/bu/OctantInfo.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/bu/PointAccessor.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/bu/Processor.hpp
  ${CMAKE_SOURCE_DIR}/external/untwine/bu/PyramidManager.hpp
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
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/pointcloud
  ${CMAKE_SOURCE_DIR}/src/core/providers/ept
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/auth
  ${CMAKE_SOURCE_DIR}/src/gui/codeeditors
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann
  ${CMAKE_SOURCE_DIR}/external/untwine/api

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/ui
)

include_directories(SYSTEM
  ${PDAL_INCLUDE_DIR}
)

add_executable(untwine ${UNTWINE_SRCS} ${UNTWINE_HDRS})
set_target_properties(untwine PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/${QGIS_LIBEXEC_SUBDIR}
)

target_link_libraries (untwine
  ${PDAL_LIBRARIES}
  Threads::Threads
)
target_include_directories(untwine PRIVATE ${UNTWINE_INCLUDE_DIRS})

add_library (pdalprovider MODULE ${PDAL_SRCS} ${PDAL_HDRS} ${PDAL_GUI_SRCS} ${PDAL_GUI_HDRS})
target_link_libraries (pdalprovider
  ${PDAL_LIBRARIES}
  Threads::Threads
)

if (WITH_GUI)
  target_link_libraries (pdalprovider
    ${PDAL_LIBRARIES}
    ${Qt5Xml_LIBRARIES}
    ${Qt5Core_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${Qt5Network_LIBRARIES}
    ${Qt5Sql_LIBRARIES}
    ${Qt5Concurrent_LIBRARIES}
  )
  target_link_libraries(pdalprovider qgis_gui)
  add_dependencies(pdalprovider ui)
endif()

# static library
add_library (pdalprovider_a STATIC ${PDAL_SRCS} ${PDAL_HDRS})
target_link_libraries (pdalprovider_a
  ${PDAL_LIBRARIES}
  ${Qt5Xml_LIBRARIES}
  ${Qt5Core_LIBRARIES}
  ${Qt5Svg_LIBRARIES}
  ${Qt5Network_LIBRARIES}
  ${Qt5Sql_LIBRARIES}
  ${Qt5Concurrent_LIBRARIES}
)

if (WITH_GUI)
  add_library (pdalprovider_gui_a STATIC ${PDAL_GUI_SRCS} ${PDAL_GUI_HDRS})
  target_link_libraries (pdalprovider_gui_a
    ${PDAL_LIBRARIES}
    ${Qt5Xml_LIBRARIES}
    ${Qt5Core_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${Qt5Network_LIBRARIES}
    ${Qt5Sql_LIBRARIES}
    ${Qt5Concurrent_LIBRARIES}
  )
  target_link_libraries(pdalprovider_gui_a qgis_gui)
  add_dependencies(pdalprovider_gui_a ui)
endif()

install(TARGETS pdalprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
# override default path where built files are put to allow running qgis without installing

install(TARGETS untwine
  RUNTIME DESTINATION ${QGIS_LIBEXEC_DIR}
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
