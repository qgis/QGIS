
# TODO: delete qgsvectorsymbologywidget.cpp and qgsvectorsymbologywidget.h from SVN

SET(QGIS_APP_SRCS
  main.cpp
  qgisapp.cpp
  qgisappinterface.cpp
  qgsabout.cpp
  qgsaddattrdialog.cpp
  qgsattributeactiondialog.cpp
  qgsattributedialog.cpp
  qgsattributetable.cpp
  qgsattributetabledisplay.cpp
  qgsbookmarkitem.cpp
  qgsbookmarks.cpp
  qgsclipboard.cpp
  qgscontinuouscolordialog.cpp
  qgscustomprojectiondialog.cpp
  qgsdelattrdialog.cpp
  qgsfillstylewidget.cpp
  qgsgeomtypedialog.cpp
  qgsgraduatedsymboldialog.cpp
  qgshelpviewer.cpp
  qgsidentifyresults.cpp
  qgslabeldialog.cpp
  qgslinestyledialog.cpp
  qgslinestylewidget.cpp
  qgsludialog.cpp
  qgsmaptooladdfeature.cpp
  qgsmaptooladdvertex.cpp
  qgsmaptooladdisland.cpp
  qgsmaptooladdring.cpp
  qgsmaptoolcapture.cpp
  qgsmaptooldeletevertex.cpp
  qgsmaptooledit.cpp
  qgsmaptoolidentify.cpp
  qgsmaptoolmovefeature.cpp
  qgsmaptoolmovevertex.cpp
  qgsmaptoolselect.cpp
  qgsmaptoolvertexedit.cpp
  qgsmarkerdialog.cpp
  qgsmeasuredialog.cpp
  qgsmeasuretool.cpp
  qgsnewhttpconnection.cpp
  qgsnumericsortlistviewitem.cpp
  qgsoptions.cpp
  qgspastetransformations.cpp
  qgspatterndialog.cpp
  qgspluginitem.cpp
  qgspluginmanager.cpp
  qgspluginmetadata.cpp
  qgspluginregistry.cpp
  qgspointstylewidget.cpp
  qgsprojectproperties.cpp
  qgsrasterlayerproperties.cpp
  qgssearchquerybuilder.cpp
  qgsserversourceselect.cpp
  qgssinglesymboldialog.cpp
  qgssnappingdialog.cpp
  qgsuniquevaluedialog.cpp
  qgsvectorlayerproperties.cpp

  composer/qgscomposer.cpp
  composer/qgscomposeritem.cpp
  composer/qgscomposerlabel.cpp
  composer/qgscomposerpicture.cpp
  composer/qgscomposermap.cpp
  composer/qgscomposerscalebar.cpp
  composer/qgscomposervectorlegend.cpp
  composer/qgscomposerview.cpp
  composer/qgscomposition.cpp

  legend/qgslegendgroup.cpp
  legend/qgslegend.cpp
  legend/qgslegenditem.cpp
  legend/qgslegendlayer.cpp
  legend/qgslegendlayerfile.cpp
  legend/qgslegendlayerfilegroup.cpp
  legend/qgslegendpropertygroup.cpp
  legend/qgslegendpropertyitem.cpp
  legend/qgslegendsymbologygroup.cpp
  legend/qgslegendsymbologyitem.cpp
  legend/qgslegendvectorsymbologyitem.cpp
  )


SET (QGIS_APP_MOC_HDRS
  qgisapp.h
  qgisappinterface.h
  qgsabout.h
  qgsaddattrdialog.h
  qgsattributeactiondialog.h
  qgsattributedialog.h
  qgsattributetabledisplay.h
  qgsattributetable.h
  qgsbookmarks.h
  qgscontinuouscolordialog.h
  qgscustomprojectiondialog.h
  qgsdelattrdialog.h
  qgsfillstylewidget.h
  qgsgeomtypedialog.h
  qgsgraduatedsymboldialog.h
  qgshelpviewer.h
  qgsidentifyresults.h
  qgslabeldialog.h
  qgslinestyledialog.h
  qgslinestylewidget.h
  qgsludialog.h
  qgsmaplayerinterface.h
  qgsmaptooledit.h
  qgsmaptoolidentify.h
  qgsmarkerdialog.h
  qgsmeasuredialog.h
  qgsnewhttpconnection.h
  qgsoptions.h
  qgspastetransformations.h
  qgspatterndialog.h
  qgspluginmanager.h
  qgspointstylewidget.h
  qgsprojectproperties.h
  qgsrasterlayerproperties.h
  qgssearchquerybuilder.h
  qgsserversourceselect.h
  qgssinglesymboldialog.h
  qgssnappingdialog.h
  qgsuniquevaluedialog.h
  qgsvectorlayerproperties.h

  composer/qgscomposer.h
  composer/qgscomposerlabel.h
  composer/qgscomposerpicture.h
  composer/qgscomposermap.h
  composer/qgscomposerscalebar.h
  composer/qgscomposervectorlegend.h
  composer/qgscomposerview.h
  composer/qgscomposition.h

  legend/qgslegend.h
  legend/qgslegendlayer.h
  legend/qgslegendlayerfile.h
  )

IF (POSTGRES_FOUND)
  SET (QGIS_APP_SRCS ${QGIS_APP_SRCS}
    qgsdbsourceselect.cpp
    qgsnewconnection.cpp
    qgspgquerybuilder.cpp
    )
  SET (QGIS_APP_MOC_HDRS ${QGIS_APP_MOC_HDRS}
    qgsdbsourceselect.h
    qgsnewconnection.h
    qgspgquerybuilder.h
    )
ENDIF (POSTGRES_FOUND)

# Python support
IF (PYTHON_FOUND)
  SET (QGIS_APP_SRCS ${QGIS_APP_SRCS} qgspythondialog.cpp qgspythonutils.cpp)
  SET (QGIS_APP_MOC_HDRS ${QGIS_APP_MOC_HDRS} qgspythondialog.h)
ENDIF (PYTHON_FOUND)

QT4_WRAP_CPP(QGIS_APP_MOC_SRCS ${QGIS_APP_MOC_HDRS})



IF (WIN32)

  IF (MSVC)
    SET (QGIS_APP_SRCS ${QGIS_APP_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/qgis_win32.rc)
  ELSE(MSVC)

    # Icon for windows MINGW
    # Note: can't include .rc directly to source files
    # as it's ignored when used MinGW

    IF (NOT WINDRES)
      FIND_PROGRAM(WINDRES windres)
      IF (NOT WINDRES)
        MESSAGE(FATAL_ERROR "windres not found - aborting")
      ENDIF (NOT WINDRES)
    ENDIF (NOT WINDRES)
    #############################################################
    # application icon
    # resource compilation for MinGW
    ADD_CUSTOM_COMMAND( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/icon.o
      COMMAND ${WINDRES} -I${CMAKE_CURRENT_SOURCE_DIR} -i${CMAKE_CURRENT_SOURCE_DIR}/qgis_win32.rc 
      -o ${CMAKE_CURRENT_BINARY_DIR}/icon.o )
    SET(QGIS_APP_SRCS ${QGIS_APP_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/icon.o)
  ENDIF (MSVC)
ENDIF (WIN32)



INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR} composer legend
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/../ui
  ../core
  ../core/raster ../core/renderer ../core/symbology
  ../gui
  ../plugins
  ${PROJ_INCLUDE_DIR}
  ${SQLITE3_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  )

IF (POSTGRES_FOUND)
  INCLUDE_DIRECTORIES(${POSTGRES_INCLUDE_DIR})
ENDIF (POSTGRES_FOUND)

IF (PYTHON_FOUND)
  INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})
ENDIF (PYTHON_FOUND)

#############

IF (WIN32)
  ADD_EXECUTABLE(qgis WIN32 ${QGIS_APP_SRCS} ${QGIS_APP_MOC_SRCS})
ELSE (WIN32)
  ADD_EXECUTABLE(qgis ${QGIS_APP_SRCS} ${QGIS_APP_MOC_SRCS})
ENDIF (WIN32)

TARGET_LINK_LIBRARIES(qgis
  ${QT_LIBRARIES}
  qgis_core
  qgis_gui
  )
IF (MSVC)
  #This resolves issues like cant find winmain etc
  TARGET_LINK_LIBRARIES(qgis qtmain)
ENDIF (MSVC)

SET_TARGET_PROPERTIES(qgis PROPERTIES
  INSTALL_RPATH ${QGIS_LIB_DIR}
  INSTALL_RPATH_USE_LINK_PATH true
  )

IF (PYTHON_FOUND)
  TARGET_LINK_LIBRARIES(qgis ${PYTHON_LIBRARIES})
ENDIF (PYTHON_FOUND)

IF (POSTGRES_FOUND)
  TARGET_LINK_LIBRARIES (qgis ${POSTGRES_LIBRARY})
ENDIF (POSTGRES_FOUND)

IF (APPLE)
  # For Mac OS X, the executable must be at the root of the bundle's executable folder
  INSTALL(TARGETS qgis RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
ELSE (APPLE)
  INSTALL(TARGETS qgis RUNTIME DESTINATION ${QGIS_BIN_DIR})
ENDIF (APPLE)


########## Install headers for developers to use against qgis libs
#commented out since app classes dont go into any lib there is little point
#INSTALL(FILES ${CMAKE_BINARY_DIR}/src/ui/ui_qgsmeasurebase.h DESTINATION ${QGIS_INCLUDE_DIR})
#INSTALL(FILES ${QGIS_APP_MOC_HDRS} DESTINATION ${QGIS_INCLUDE_DIR})
