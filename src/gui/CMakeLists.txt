
SET(QGIS_GUI_SRCS
qgisgui.cpp
qgisinterface.cpp
qgscolorbutton.cpp
qgscursors.cpp
qgsencodingfiledialog.cpp
qgslayerprojectionselector.cpp
qgsmapcanvas.cpp
qgsmapcanvasitem.cpp
qgsmapcanvasmap.cpp
qgsmapoverviewcanvas.cpp
qgsmaptool.cpp
qgsmaptoolpan.cpp
qgsmaptoolzoom.cpp
qgsmessageviewer.cpp
qgsprojectionselector.cpp
qgsrubberband.cpp
qgsvertexmarker.cpp
)

SET(QGIS_GUI_MOC_HDRS
qgisinterface.h
qgsencodingfiledialog.h
qgslayerprojectionselector.h
qgsmapcanvas.h
qgsmapcanvasitem.h
qgsmapoverviewcanvas.h
qgsprojectionselector.h
)

QT4_WRAP_CPP(QGIS_GUI_MOC_SRCS ${QGIS_GUI_MOC_HDRS})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ../core
     ../core/raster ../core/renderer ../core/symbology
     ${SQLITE3_INCLUDE_DIR}
     ${CMAKE_CURRENT_BINARY_DIR}
     ${CMAKE_CURRENT_BINARY_DIR}/../ui
)

IF (WIN32)
  IF (MSVC)
    ADD_DEFINITIONS("-DGUI_EXPORT=__declspec(dllexport)")
  ELSE (MSVC)
    ADD_DEFINITIONS("-UGUI_EXPORT \"-DGUI_EXPORT=__declspec(dllexport)\"")
  ENDIF (MSVC)
ENDIF (WIN32)


#############################################################
# qgis_gui library


ADD_LIBRARY(qgis_gui SHARED ${QGIS_GUI_SRCS} ${QGIS_GUI_MOC_SRCS})

# make sure that UI files will be processed first
ADD_DEPENDENCIES(qgis_gui ui)

TARGET_LINK_LIBRARIES(qgis_gui
  ${QT_LIBRARIES}
  ${SQLITE3_LIBRARIES}
  qgis_core
)

INSTALL(TARGETS qgis_gui
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION lib)
