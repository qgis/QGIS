
SET(QGIS_GUI_SRCS
qgisgui.cpp
qgisinterface.cpp
qgscolorbutton.cpp
qgscursors.cpp
qgsencodingfiledialog.cpp
qgsfiledropedit.cpp
qgslayerprojectionselector.cpp
qgsmapcanvas.cpp
qgsmapcanvasitem.cpp
qgsmapcanvasmap.cpp
qgsmapcanvassnapper.cpp
qgsmapoverviewcanvas.cpp
qgsmaptool.cpp
qgsmaptoolemitpoint.cpp
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
qgsmapoverviewcanvas.h
qgsmessageviewer.h
qgsmaptoolemitpoint.h
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
  ${QT_QTGUI_LIBRARY} 
  ${QT_QTXML_LIBRARY} 
  ${QT_QTSVG_LIBRARY} 
  ${QT_QTNETWORK_LIBRARY} 
  ${QT_QTMAIN_LIBRARY} 
  ${QT_QT3SUPPORT_LIBRARY} 
  ${SQLITE3_LIBRARIES}
  qgis_core
)

#work aroud for the fact that qt3 support on mac is linked ot qsql
IF (APPLE)
  TARGET_LINK_LIBRARIES(qgis_gui
    ${QT_QTSQL_LIBRARY} 
    )
ENDIF (APPLE)


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
qgslayerprojectionselector.h
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
)


INSTALL(CODE "MESSAGE(\"Installing GUI headers...\")")
INSTALL(FILES ${QGIS_GUI_HDRS} ${QGIS_GUI_MOC_HDRS} DESTINATION ${QGIS_INCLUDE_DIR})

