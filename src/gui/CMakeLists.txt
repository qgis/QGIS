
SET(QGIS_GUI_SRCS
qgisgui.cpp
qgisinterface.cpp
qgscolorbutton.cpp
qgscomposerview.cpp
qgscursors.cpp
qgsdetaileditemdelegate.cpp
qgsdetaileditemwidget.cpp
qgsdetaileditemdata.cpp
qgsencodingfiledialog.cpp
qgsfiledropedit.cpp
qgsgenericprojectionselector.cpp
qgsmapcanvas.cpp
qgsmapcanvasitem.cpp
qgsmapcanvasmap.cpp
qgsmapcanvassnapper.cpp
qgsmapoverviewcanvas.cpp
qgsmaptip.cpp
qgsmaptool.cpp
qgsmaptoolemitpoint.cpp
qgsmaptoolpan.cpp
qgsmaptoolzoom.cpp
qgsmessageviewer.cpp
qgsprojectionselector.cpp
qgsquickprint.cpp
qgsrubberband.cpp
qgsvertexmarker.cpp
)

SET(QGIS_GUI_MOC_HDRS
qgscomposerview.h
qgsdetaileditemdelegate.h
qgsdetaileditemwidget.h
qgisinterface.h
qgsencodingfiledialog.h
qgsgenericprojectionselector.h
qgsmapcanvas.h
qgsmapoverviewcanvas.h
qgsmaptoolemitpoint.h
qgsmessageviewer.h
qgsprojectionselector.h
qgsquickprint.h
)

QT4_WRAP_CPP(QGIS_GUI_MOC_SRCS ${QGIS_GUI_MOC_HDRS})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ../core
     ../core/composer ../core/raster ../core/renderer ../core/symbology
     ${CMAKE_CURRENT_BINARY_DIR}
     ${CMAKE_CURRENT_BINARY_DIR}/../ui
     ${GEOS_INCLUDE_DIR}
)
IF (WITH_INTERNAL_SQLITE3)
  INCLUDE_DIRECTORIES(../core/sqlite3)
ELSE (WITH_INTERNAL_SQLITE3)
  INCLUDE_DIRECTORIES(${SQLITE3_INCLUDE_DIR})
ENDIF (WITH_INTERNAL_SQLITE3)

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

SET_TARGET_PROPERTIES(qgis_gui PROPERTIES 
			VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}
			SOVERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR})

# make sure that UI files will be processed first
ADD_DEPENDENCIES(qgis_gui ui)

TARGET_LINK_LIBRARIES(qgis_gui
  ${QT_QTGUI_LIBRARY} 
  ${QT_QTXML_LIBRARY} 
  ${QT_QTSVG_LIBRARY} 
  ${QT_QTNETWORK_LIBRARY} 
  ${QT_QTMAIN_LIBRARY} 
  ${SQLITE3_LIBRARIES}
  qgis_core
)

INSTALL(TARGETS qgis_gui
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION ${QGIS_LIB_DIR}
  ARCHIVE DESTINATION ${QGIS_LIB_DIR})

# for installing headers

SET(QGIS_GUI_HDRS
qgisgui.h
qgisinterface.h
qgscolorbutton.h
qgscursors.h
qgsencodingfiledialog.h
qgsfiledropedit.h
qgsgenericprojectionselector.h
qgsmapcanvas.h
qgsmapcanvasitem.h
qgsmapcanvasmap.h
qgsmapcanvassnapper.h
qgsmapoverviewcanvas.h
qgsmaptool.h
qgsmaptoolemitpoint.h
qgsmaptoolpan.h
qgsmaptoolzoom.h
qgsmessageviewer.h
qgsprojectionselector.h
qgsrubberband.h
qgsvertexmarker.h
qgsmaptip.h

${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsdetaileditemwidgetbase.h
${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsgenericprojectionselectorbase.h
${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsmessageviewer.h
${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsprojectionselectorbase.h
)


INSTALL(CODE "MESSAGE(\"Installing GUI headers...\")")
INSTALL(FILES ${QGIS_GUI_HDRS} ${QGIS_GUI_MOC_HDRS} DESTINATION ${QGIS_INCLUDE_DIR})

