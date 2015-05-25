SUBDIRS(network)

#############################################################
# sources

SET(QGIS_ANALYSIS_SRCS
  interpolation/qgsgridfilewriter.cpp
  interpolation/qgsidwinterpolator.cpp
  interpolation/qgsinterpolator.cpp
  interpolation/qgstininterpolator.cpp
  interpolation/Bezier3D.cc
  interpolation/CloughTocherInterpolator.cc
  interpolation/DualEdgeTriangulation.cc
  interpolation/HalfEdge.cc
  interpolation/Line3D.cc
  interpolation/LinTriangleInterpolator.cc
  interpolation/MathUtils.cc
  interpolation/NormVecDecorator.cc
  interpolation/Node.cc
  interpolation/ParametricLine.cc
  interpolation/Point3D.cc
  interpolation/TriangleInterpolator.cc
  interpolation/Triangulation.cc
  interpolation/TriDecorator.cc
  interpolation/Vector3D.cc
  raster/qgsninecellfilter.cpp
  raster/qgsruggednessfilter.cpp
  raster/qgsderivativefilter.cpp
  raster/qgshillshadefilter.cpp
  raster/qgsslopefilter.cpp
  raster/qgsaspectfilter.cpp
  raster/qgstotalcurvaturefilter.cpp
  raster/qgsrelief.cpp
  raster/qgsrastercalcnode.cpp
  raster/qgsrastercalculator.cpp
  raster/qgsrastermatrix.cpp
  vector/mersenne-twister.cpp
  vector/qgsgeometryanalyzer.cpp
  vector/qgspointsample.cpp
  vector/qgstransectsample.cpp
  vector/qgszonalstatistics.cpp
  vector/qgsoverlayanalyzer.cpp

  openstreetmap/qgsosmbase.cpp
  openstreetmap/qgsosmdatabase.cpp
  openstreetmap/qgsosmdownload.cpp
  openstreetmap/qgsosmimport.cpp
)

SET(QGIS_ANALYSIS_MOC_HDRS
  openstreetmap/qgsosmdownload.h
  openstreetmap/qgsosmimport.h
)

INCLUDE_DIRECTORIES(${SPATIALITE_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${SQLITE3_INCLUDE_DIR})
INCLUDE_DIRECTORIES(BEFORE raster)

ADD_FLEX_FILES_PREFIX(QGIS_ANALYSIS_SRCS raster raster/qgsrastercalclexer.ll)

ADD_BISON_FILES_PREFIX(QGIS_ANALYSIS_SRCS raster raster/qgsrastercalcparser.yy)

IF (PEDANTIC AND MSVC)
  # disable warnings
  # 4065 switch with default and no case
  # 4127 conditional expression is constant
  # 4702 unreachable code
  SET_SOURCE_FILES_PROPERTIES(
    ${CMAKE_BINARY_DIR}/src/analysis/qgsrastercalcparser.cpp
    PROPERTIES COMPILE_FLAGS "-wd4065 -wd4127 -wd4702"
  )
ENDIF (PEDANTIC AND MSVC)

IF ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  SET_SOURCE_FILES_PROPERTIES(
    interpolation/qgstininterpolator.cpp
    interpolation/NormVecDecorator.cc
    interpolation/CloughTocherInterpolator.cc
    PROPERTIES COMPILE_FLAGS "-Wno-overloaded-virtual"
  )
ENDIF ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")

QT4_WRAP_CPP(QGIS_ANALYSIS_MOC_SRCS ${QGIS_ANALYSIS_MOC_HDRS})

# install headers

SET(QGIS_ANALYSIS_HDRS
  raster/qgsaspectfilter.h
  raster/qgsderivativefilter.h
  raster/qgshillshadefilter.h
  raster/qgsninecellfilter.h
  raster/qgsrastercalculator.h
  raster/qgsrelief.h
  raster/qgsruggednessfilter.h
  raster/qgsslopefilter.h
  raster/qgsrastermatrix.h
  raster/qgsrastercalcnode.h
  raster/qgstotalcurvaturefilter.h

  vector/qgsgeometryanalyzer.h
  vector/qgszonalstatistics.h
  vector/qgsoverlayanalyzer.h

  interpolation/qgsinterpolator.h
  interpolation/qgsgridfilewriter.h
  interpolation/qgsidwinterpolator.h
  interpolation/qgstininterpolator.h
  interpolation/Bezier3D.h
  interpolation/ParametricLine.h
  interpolation/CloughTocherInterpolator.h
  interpolation/TriangleInterpolator.h
  interpolation/Point3D.h
  interpolation/Vector3D.h
  interpolation/DualEdgeTriangulation.h
  interpolation/Node.h
  interpolation/TriDecorator.h
  interpolation/Triangulation.h
  interpolation/HalfEdge.h
  interpolation/LinTriangleInterpolator.h
  interpolation/NormVecDecorator.h
  interpolation/Line3D.h

  openstreetmap/qgsosmbase.h
  openstreetmap/qgsosmdatabase.h
  openstreetmap/qgsosmdownload.h
  openstreetmap/qgsosmimport.h
)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../core/
  ${CMAKE_CURRENT_SOURCE_DIR}/../core/geometry
  ${CMAKE_CURRENT_SOURCE_DIR}/../core/raster
  ${CMAKE_CURRENT_SOURCE_DIR}/../core/symbology-ng
  interpolation
  ${PROJ_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
)

IF (NOT MSVC)
  ADD_DEFINITIONS("-UANALYSIS_EXPORT")
  IF ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
  ENDIF()
ENDIF (NOT MSVC)
ADD_DEFINITIONS("\"-DANALYSIS_EXPORT=${DLLEXPORT}\"")

#############################################################
# qgis_analysis library

ADD_LIBRARY(qgis_analysis SHARED ${QGIS_ANALYSIS_SRCS} ${QGIS_ANALYSIS_MOC_SRCS} ${QGIS_ANALYSIS_HDRS})

IF(NOT APPLE)
  INSTALL(FILES ${QGIS_ANALYSIS_HDRS} DESTINATION ${QGIS_INCLUDE_DIR})
ELSE(NOT APPLE)
  SET_TARGET_PROPERTIES(qgis_analysis PROPERTIES
    # no moc headers, messes up PROPERTIES syntax
    CLEAN_DIRECT_OUTPUT 1
    FRAMEWORK 1
    FRAMEWORK_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}"
    MACOSX_FRAMEWORK_INFO_PLIST "${CMAKE_SOURCE_DIR}/mac/framework.info.plist.in"
    MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${COMPLETE_VERSION}
    MACOSX_FRAMEWORK_IDENTIFIER org.qgis.qgis2_analysis
    BUILD_WITH_INSTALL_RPATH TRUE
    PUBLIC_HEADER "${QGIS_ANALYSIS_HDRS}"
    LINK_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}"
  )
ENDIF(NOT APPLE)

#generate unversioned libs for android
IF (NOT ANDROID)
  SET_TARGET_PROPERTIES(qgis_analysis PROPERTIES
    VERSION ${COMPLETE_VERSION}
    SOVERSION ${COMPLETE_VERSION}
    )
ENDIF (NOT ANDROID)

TARGET_LINK_LIBRARIES(qgis_analysis qgis_core)


# install

INSTALL(TARGETS qgis_analysis
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION ${QGIS_LIB_DIR}
  ARCHIVE DESTINATION ${QGIS_LIB_DIR}
  FRAMEWORK DESTINATION ${QGIS_FW_SUBDIR}
  PUBLIC_HEADER DESTINATION ${QGIS_INCLUDE_DIR})

# Mac dev frameworks

IF (APPLE AND QGIS_MACAPP_INSTALL_DEV)
  INSTALL(TARGETS qgis_analysis FRAMEWORK DESTINATION ${QGIS_MACAPP_DEV_PREFIX})
  INSTALL(CODE "EXECUTE_PROCESS(COMMAND install_name_tool -id \"${QGIS_MACAPP_DEV_PREFIX}/qgis_analysis.framework/Versions/${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}/qgis_analysis\" \"$ENV{DESTDIR}${QGIS_MACAPP_DEV_PREFIX}/qgis_analysis.framework/qgis_analysis\")")
  INSTALL(CODE "EXECUTE_PROCESS(COMMAND install_name_tool -change \"${CMAKE_INSTALL_NAME_DIR}/qgis_core.framework/Versions/${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}/qgis_core\" \"${QGIS_MACAPP_DEV_PREFIX}/qgis_core.framework/Versions/${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}/qgis_core\" \"$ENV{DESTDIR}${QGIS_MACAPP_DEV_PREFIX}/qgis_analysis.framework/qgis_analysis\")")
ENDIF (APPLE AND QGIS_MACAPP_INSTALL_DEV)
