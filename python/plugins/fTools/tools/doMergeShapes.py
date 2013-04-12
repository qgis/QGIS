# -*- coding: utf-8 -*-

"""
***************************************************************************
    doMergeShapes.py - merge multiple shapefile into one
     --------------------------------------
    Date                 : 30-Mar-2010
    Copyright            : (C) 2010 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from qgis.core import *
from qgis.gui import *

import ftools_utils

from ui_frmMergeShapes import Ui_Dialog

class Dialog( QDialog, Ui_Dialog ):
  def __init__( self, iface ):
    QDialog.__init__( self, iface.mainWindow() )
    self.setupUi( self )
    self.iface = iface

    self.mergeThread = None
    self.inputFiles = None
    self.outFileName = None
    self.inEncoding = None

    self.btnOk = self.buttonBox.button( QDialogButtonBox.Ok )
    self.btnClose = self.buttonBox.button( QDialogButtonBox.Close )

    QObject.connect( self.btnSelectDir, SIGNAL( "clicked()" ), self.inputDir )
    QObject.connect( self.btnSelectFile, SIGNAL( "clicked()" ), self.outFile )
    QObject.connect( self.chkListMode, SIGNAL( "stateChanged( int )" ), self.changeMode )
    QObject.connect( self.leOutShape, SIGNAL( "editingFinished()" ), self.updateOutFile )

  def inputDir( self ):
    settings = QSettings()
    lastDir = settings.value( "/fTools/lastShapeDir", "." ).toString()
    inDir = QFileDialog.getExistingDirectory( self,
              self.tr( "Select directory with shapefiles to merge" ),
              lastDir )

    if inDir.isEmpty():
      return

    workDir = QDir( inDir )
    workDir.setFilter( QDir.Files | QDir.NoSymLinks | QDir.NoDotAndDotDot )
    nameFilter = QStringList() << "*.shp" << "*.SHP"
    workDir.setNameFilters( nameFilter )
    self.inputFiles = workDir.entryList()
    if self.inputFiles.count() == 0:
      QMessageBox.warning( self, self.tr( "No shapefiles found" ),
        self.tr( "There are no shapefiles in this directory. Please select another one." ) )
      self.inputFiles = None
      return

    settings.setValue( "/fTools/lastShapeDir", inDir )

    self.progressFiles.setRange( 0, self.inputFiles.count() )
    self.leInputDir.setText( inDir )

  def outFile( self ):
    ( self.outFileName, self.encoding ) = ftools_utils.saveDialog( self )
    if self.outFileName is None or self.encoding is None:
      return
    self.leOutShape.setText( self.outFileName )

  def inputFile( self ):
    ( files, self.inEncoding ) =  ftools_utils.openDialog( self, dialogMode="ManyFiles" )
    if files is None or self.inEncoding is None:
      self.inputFiles = None
      return

    self.inputFiles = QStringList()
    for f in files:
      fileName = QFileInfo( f ).fileName()
      self.inputFiles.append( fileName )

    self.progressFiles.setRange( 0, self.inputFiles.count() )
    self.leInputDir.setText( files.join( ";" ) )

  def changeMode( self ):
    if self.chkListMode.isChecked():
      self.label.setText( self.tr( "Input files" ) )
      QObject.disconnect( self.btnSelectDir, SIGNAL( "clicked()" ), self.inputDir )
      QObject.connect( self.btnSelectDir, SIGNAL( "clicked()" ), self.inputFile )
      self.lblGeometry.setEnabled( False )
      self.cmbGeometry.setEnabled( False )
    else:
      self.label.setText( self.tr( "Input directory" ) )
      QObject.disconnect( self.btnSelectDir, SIGNAL( "clicked()" ), self.inputFile )
      QObject.connect( self.btnSelectDir, SIGNAL( "clicked()" ), self.inputDir )
      self.lblGeometry.setEnabled( True )
      self.cmbGeometry.setEnabled( True )

  def updateOutFile( self ):
    self.outFileName = self.leOutShape.text()
    settings = QSettings()
    self.outEncoding = settings.value( "/UI/encoding" ).toString()

  def reject( self ):
    QDialog.reject( self )

  def accept( self ):
    if self.inputFiles is None:
      workDir = QDir( self.leInputDir.text() )
      workDir.setFilter( QDir.Files | QDir.NoSymLinks | QDir.NoDotAndDotDot )
      nameFilter = QStringList() << "*.shp" << "*.SHP"
      workDir.setNameFilters( nameFilter )
      self.inputFiles = workDir.entryList()
      if self.inputFiles.count() == 0:
        QMessageBox.warning( self, self.tr( "No shapefiles found" ),
          self.tr( "There are no shapefiles in this directory. Please select another one." ) )
        self.inputFiles = None
        return

    if self.outFileName is None:
      QMessageBox.warning( self, self.tr( "No output file" ),
        self.tr( "Please specify output file." ) )
      return

    if self.chkListMode.isChecked():
      files = self.leInputDir.text().split( ";" )
      baseDir = QFileInfo( files[ 0 ] ).absolutePath()
    else:
      baseDir = self.leInputDir.text()
      # look for shapes with specified geometry type
      self.inputFiles = ftools_utils.getShapesByGeometryType( baseDir, self.inputFiles, self.cmbGeometry.currentIndex() )
      if self.inputFiles is None:
        QMessageBox.warning( self, self.tr( "No shapefiles found" ),
          self.tr( "There are no shapefiles with the given geometry type. Please select an available geometry type." ) )
        return
      self.progressFiles.setRange( 0, self.inputFiles.count() )

    outFile = QFile( self.outFileName )
    if outFile.exists():
      if not QgsVectorFileWriter.deleteShapeFile( self.outFileName ):
        QMessageBox.warning( self, self.tr( "Delete error" ), self.tr( "Can't delete file %1" ).arg( self.outFileName ) )
        return

    if self.inEncoding == None:
      self.inEncoding = "System"

    self.btnOk.setEnabled( False )

    self.mergeThread = ShapeMergeThread( baseDir, self.inputFiles, self.inEncoding, self.outFileName, self.encoding )
    QObject.connect( self.mergeThread, SIGNAL( "rangeChanged( PyQt_PyObject )" ), self.setFeatureProgressRange )
    QObject.connect( self.mergeThread, SIGNAL( "checkStarted()" ), self.setFeatureProgressFormat )
    QObject.connect( self.mergeThread, SIGNAL( "checkFinished()" ), self.resetFeatureProgressFormat )
    QObject.connect( self.mergeThread, SIGNAL( "fileNameChanged( PyQt_PyObject )" ), self.setShapeProgressFormat )
    QObject.connect( self.mergeThread, SIGNAL( "featureProcessed()" ), self.featureProcessed )
    QObject.connect( self.mergeThread, SIGNAL( "shapeProcessed()" ), self.shapeProcessed )
    QObject.connect( self.mergeThread, SIGNAL( "processingFinished()" ), self.processingFinished )
    QObject.connect( self.mergeThread, SIGNAL( "processingInterrupted()" ), self.processingInterrupted )

    self.btnClose.setText( self.tr( "Cancel" ) )
    QObject.disconnect( self.buttonBox, SIGNAL( "rejected()" ), self.reject )
    QObject.connect( self.btnClose, SIGNAL( "clicked()" ), self.stopProcessing )

    self.mergeThread.start()

  def setFeatureProgressRange( self, maximum ):
    self.progressFeatures.setRange( 0, maximum )
    self.progressFeatures.setValue( 0 )

  def setFeatureProgressFormat( self ):
    self.progressFeatures.setFormat( "Checking files: %p% ")

  def resetFeatureProgressFormat( self ):
    self.progressFeatures.setFormat( "%p% ")

  def featureProcessed( self ):
    self.progressFeatures.setValue( self.progressFeatures.value() + 1 )

  def setShapeProgressFormat( self, fileName ):
    self.progressFiles.setFormat( "%p% " + fileName )

  def shapeProcessed( self ):
    self.progressFiles.setValue( self.progressFiles.value() + 1 )

  def processingFinished( self ):
    self.stopProcessing()

    if self.chkAddToCanvas.isChecked():
      if not ftools_utils.addShapeToCanvas( unicode( self.outFileName ) ):
        QMessageBox.warning( self, self.tr( "Merging" ),
                             self.tr( "Error loading output shapefile:\n%1" )
                             .arg( unicode( self.outFileName ) ) )

    self.restoreGui()

  def processingInterrupted( self ):
    self.restoreGui()

  def stopProcessing( self ):
    if self.mergeThread != None:
      self.mergeThread.stop()
      self.mergeThread = None

  def restoreGui( self ):
    self.progressFiles.setFormat( "%p%" )
    self.progressFeatures.setRange( 0, 100 )
    self.progressFeatures.setValue( 0 )
    self.progressFiles.setValue( 0 )
    QObject.connect( self.buttonBox, SIGNAL( "rejected()" ), self.reject )
    self.btnClose.setText( self.tr( "Close" ) )
    self.btnOk.setEnabled( True )

class ShapeMergeThread( QThread ):
  def __init__( self, dir, shapes, inputEncoding, outputFileName, outputEncoding ):
    QThread.__init__( self, QThread.currentThread() )
    self.baseDir = dir
    self.shapes = shapes
    self.inputEncoding = inputEncoding
    self.outputFileName = outputFileName
    self.outputEncoding = outputEncoding

    self.mutex = QMutex()
    self.stopMe = 0

  def run( self ):
    self.mutex.lock()
    self.stopMe = 0
    self.mutex.unlock()

    interrupted = False

    # create attribute list with uniquie fields
    # from all selected layers
    mergedFields = []
    self.emit( SIGNAL( "rangeChanged( PyQt_PyObject )" ), len( self.shapes ) )
    self.emit( SIGNAL( "checkStarted()" ) )

    shapeIndex = 0
    fieldMap = {}
    for fileName in self.shapes:
      layerPath = QFileInfo( self.baseDir + "/" + fileName ).absoluteFilePath()
      newLayer = QgsVectorLayer( layerPath, QFileInfo( layerPath ).baseName(), "ogr" )
      if not newLayer.isValid():
        continue

      vprovider = newLayer.dataProvider()

      fieldIndex = 0
      for layerField in vprovider.fields():
        fieldFound = False
        for mergedField in mergedFields:
          if mergedField.name() == layerField.name() and mergedField.type() == layerField.type():
            fieldFound = True
            break

        if not fieldFound:
          if not fieldMap.has_key(shapeIndex):
            fieldMap[shapeIndex]={}

          fieldMap[shapeIndex][fieldIndex] = len(mergedFields)

          mergedFields.append( layerField )

        fieldIndex += 1

      shapeIndex += 1
      self.emit( SIGNAL( "featureProcessed()" ) )
    self.emit( SIGNAL( "checkFinished()" ) )

    # get information about shapefiles
    layerPath = QFileInfo( self.baseDir + "/" + self.shapes[ 0 ] ).absoluteFilePath()
    newLayer = QgsVectorLayer( layerPath, QFileInfo( layerPath ).baseName(), "ogr" )
    self.crs = newLayer.crs()
    self.geom = newLayer.wkbType()
    vprovider = newLayer.dataProvider()

    fields = QgsFields()
    for f in mergedFields:
      fields.append(f)

    writer = QgsVectorFileWriter( self.outputFileName, self.outputEncoding,
                 fields, self.geom, self.crs )

    shapeIndex = 0
    for fileName in self.shapes:
      layerPath = QFileInfo( self.baseDir + "/" + fileName ).absoluteFilePath()
      newLayer = QgsVectorLayer( layerPath, QFileInfo( layerPath ).baseName(), "ogr" )
      if not newLayer.isValid():
        continue
      vprovider = newLayer.dataProvider()
      vprovider.setEncoding( self.inputEncoding )
      layerFields = vprovider.fields()
      nFeat = vprovider.featureCount()
      self.emit( SIGNAL( "rangeChanged( PyQt_PyObject )" ), nFeat )
      self.emit( SIGNAL( "fileNameChanged( PyQt_PyObject )" ), fileName )
      inFeat = QgsFeature()
      outFeat = QgsFeature()
      inGeom = QgsGeometry()
      fit = vprovider.getFeatures()
      while fit.nextFeature( inFeat ):
        mergedAttrs = [QVariant()] * len(mergedFields)

        # fill available attributes with values
        fieldIndex = 0
        for v in inFeat.attributes():
          if fieldMap.has_key(shapeIndex) and fieldMap[shapeIndex].has_key(fieldIndex):
            mergedAttrs[ fieldMap[shapeIndex][fieldIndex] ] = v
          fieldIndex += 1

        inGeom = QgsGeometry( inFeat.geometry() )
        outFeat.setGeometry( inGeom )
        outFeat.setAttributes( mergedAttrs )
        writer.addFeature( outFeat )
        self.emit( SIGNAL( "featureProcessed()" ) )

      self.emit( SIGNAL( "shapeProcessed()" ) )
      self.mutex.lock()
      s = self.stopMe
      self.mutex.unlock()

      if s == 1:
        interrupted = True
        break

      shapeIndex += 1

    del writer

    if not interrupted:
      self.emit( SIGNAL( "processingFinished()" ) )
    else:
      self.emit( SIGNAL( "processingInterrupted()" ) )

  def stop( self ):
    self.mutex.lock()
    self.stopMe = 1
    self.mutex.unlock()

    QThread.wait( self )
