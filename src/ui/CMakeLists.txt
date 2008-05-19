
# TODO: delete from SVN
#   qgslinestyledialog.ui
#   qgslinestylewidget.ui
#   qgsmarkerdialog.ui
#   qgspatterndialog.ui
#   qgspointstylewidget.ui
#   qgsvectorsymbologywidget.ui

SET(QGIS_UIS
  qgisappbase.ui
  qgsabout.ui
  qgsaddattrdialogbase.ui
  qgsattributeactiondialogbase.ui
  qgsattributedialogbase.ui
  qgsattributetablebase.ui
  qgsbookmarksbase.ui
  qgscomposerbase.ui
  qgscomposerlabelbase.ui
  qgscomposermapbase.ui
  qgscomposerpicturebase.ui
  qgscomposerscalebarbase.ui
  qgscomposervectorlegendbase.ui
  qgscompositionbase.ui
  qgscontinuouscolordialogbase.ui
  qgscustomprojectiondialogbase.ui
  qgsdbsourceselectbase.ui
  qgsdelattrdialogbase.ui
  qgsdetaileditemwidgetbase.ui
  qgsfillstylewidgetbase.ui
  qgsgeomtypedialogbase.ui
  qgsgraduatedsymboldialogbase.ui
  qgshelpviewerbase.ui
  qgsidentifyresultsbase.ui
  qgslabeldialogbase.ui
  qgslayerprojectionselectorbase.ui
  qgsludialogbase.ui
  qgsmeasurebase.ui
  qgsmessageviewer.ui
  qgsnewconnectionbase.ui
  qgsnewhttpconnectionbase.ui
  qgsoptionsbase.ui
  qgspastetransformationsbase.ui
  qgspgquerybuilderbase.ui
  qgspluginmanagerbase.ui
  qgsprojectionselectorbase.ui
  qgsprojectpropertiesbase.ui
  qgsrasterlayerpropertiesbase.ui
  qgsserversourceselectbase.ui
  qgssinglesymboldialogbase.ui
  qgssnappingdialogbase.ui
  qgsuniquevaluedialogbase.ui
  qgsvectorlayerpropertiesbase.ui
)

IF (PYTHON_FOUND)
  SET (QGIS_UIS ${QGIS_UIS} ../ui/qgspythondialog.ui)
ENDIF (PYTHON_FOUND)

QT4_WRAP_UI(QGIS_UIS_H ${QGIS_UIS})

ADD_CUSTOM_TARGET(ui ALL DEPENDS ${QGIS_UIS_H})
