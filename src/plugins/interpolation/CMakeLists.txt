########################################################
# Files

SET (INTERPOLATION_SRCS
     qgsinterpolationplugin.cpp
     qgsgridfilewriter.cpp
     qgsidwinterpolator.cpp
     qgsidwinterpolatordialog.cpp
     qgsinterpolationdialog.cpp
     qgsinterpolator.cpp
     qgsinterpolatordialog.cpp
     qgstininterpolator.cpp
     qgstininterpolatordialog.cpp
     DualEdgeTriangulation.cc
     HalfEdge.cc
     Line3D.cc
     LinTriangleInterpolator.cc
     MathUtils.cc
     Node.cc
     Point3D.cc
     TriangleInterpolator.cc
     Triangulation.cc
     Vector3D.cc
)

SET (INTERPOLATION_UIS 
    qgsidwinterpolatordialogbase.ui
    qgsinterpolationdialogbase.ui
    qgstininterpolatordialogbase.ui
    )

SET (INTERPOLATION_MOC_HDRS
     qgsinterpolationplugin.h
     qgstininterpolatordialog.h
     qgsidwinterpolatordialog.h
     qgsinterpolationdialog.h
)

########################################################
# Build

QT4_WRAP_UI (INTERPOLATION_UIS_H  ${INTERPOLATION_UIS})

QT4_WRAP_CPP (INTERPOLATION_MOC_SRCS  ${INTERPOLATION_MOC_HDRS})

ADD_LIBRARY (interpolationplugin MODULE ${INTERPOLATION_SRCS} ${INTERPOLATION_MOC_SRCS} ${INTERPOLATION_RCC_SRCS} ${INTERPOLATION_UIS_H})

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ../../core ../../core/raster ../../core/renderer ../../core/symbology
     ../../gui
     ..
     .
)

TARGET_LINK_LIBRARIES(interpolationplugin
  qgis_core
  qgis_gui
)


########################################################
# Install

INSTALL(TARGETS interpolationplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})