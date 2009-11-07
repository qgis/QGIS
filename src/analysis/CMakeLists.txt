

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
  raster/qgsslopefilter.cpp
  raster/qgsaspectfilter.cpp
  raster/qgstotalcurvaturefilter.cpp
  vector/qgsgeometryanalyzer.cpp
  vector/qgszonalstatistics.cpp
)

SET(QGIS_ANALYSIS_MOC_HDRS
)

QT4_WRAP_CPP(QGIS_ANALYSIS_MOC_SRCS ${QGIS_ANALYSIS_MOC_HDRS})


INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_CURRENT_SOURCE_DIR}/../core/
  ${CMAKE_CURRENT_SOURCE_DIR}/../core/renderer
  interpolation
  ${PROJ_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
)

IF (WIN32)
  IF (MSVC)
    ADD_DEFINITIONS("-DANALYSIS_EXPORT=__declspec(dllexport)")
  ELSE (MSVC)
    ADD_DEFINITIONS("-UANALYSIS_EXPORT \"-DANALYSIS_EXPORT=__declspec(dllexport)\"")
  ENDIF (MSVC)
ENDIF (WIN32)


#############################################################
# qgis_analysis library

ADD_LIBRARY(qgis_analysis SHARED ${QGIS_ANALYSIS_SRCS} ${QGIS_ANALYSIS_MOC_SRCS} )

ADD_DEPENDENCIES(qgis_analysis qgis_core)

SET_TARGET_PROPERTIES(qgis_analysis PROPERTIES VERSION ${COMPLETE_VERSION} SOVERSION ${COMPLETE_VERSION})

# because of htonl
IF (WIN32)
  SET(PLATFORM_LIBRARIES wsock32)
  IF (MSVC)
    #needed for linking to gdal which needs odbc
    SET(PLATFORM_LIBRARIES ${PLATFORM_LIBRARIES} odbc32 odbccp32)
  ENDIF (MSVC)
ENDIF (WIN32)

TARGET_LINK_LIBRARIES(qgis_analysis
  qgis_core
  ${QT_QTCORE_LIBRARY} 
  ${QT_QTGUI_LIBRARY} 
  ${QT_QTMAIN_LIBRARY} 
  ${PROJ_LIBRARY}
  ${GEOS_LIBRARY}
  ${GDAL_LIBRARY}
  ${PLATFORM_LIBRARIES}
)

IF (APPLE)
  SET_TARGET_PROPERTIES(qgis_analysis PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE )
ENDIF (APPLE)

INSTALL(TARGETS qgis_analysis
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION ${QGIS_LIB_DIR}
  ARCHIVE DESTINATION ${QGIS_LIB_DIR})


# Added by Tim to install headers

SET(QGIS_ANALYSIS_HDRS vector/qgsgeometryanalyzer.h vector/qgszonalstatistics.h
)

INSTALL(CODE "MESSAGE(\"Installing ANALYSIS headers...\")")
INSTALL(FILES ${QGIS_ANALYSIS_HDRS} ${QGIS_ANALYSIS_MOC_HDRS} DESTINATION ${QGIS_INCLUDE_DIR}) 
