# -*- coding: utf-8 -*-

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from qgis.core import *
from qgis.gui import *

import ftools_utils

from ui_frmMergeShapes import Ui_Dialog

class Dialog( QDialog, Ui_Dialog ):
  def __init__( self, iface ):
    QDialog.__init__( self )
    self.setupUi( self )
    self.iface = iface

    self.mergeThread = None
    self.inputFiles = None

    self.btnOk = self.buttonBox.button( QDialogButtonBox.Ok )
    self.btnClose = self.buttonBox.button( QDialogButtonBox.Close )

    QObject.connect( self.btnSelectDir, SIGNAL( "clicked()" ), self.inputDir )
    QObject.connect( self.btnSelectFile, SIGNAL( "clicked()" ), self.outFile )
    QObject.connect( self.chkListMode, SIGNAL( "stateChanged( int )" ), self.changeMode )

  def inputDir( self ):
    inDir = QFileDialog.getExistingDirectory( self,
              self.tr( "Select directory with shapefiles to merge" ),
              "." )

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

    self.progressFiles.setRange( 0, self.inputFiles.count() )
    self.leInputDir.setText( inDir )

  def outFile( self ):
    ( self.outFileName, self.encoding ) = ftools_utils.saveDialog( self )
    if self.outFileName is None or self.encoding is None:
      return
    self.leOutShape.setText( self.outFileName )

  def inputFile( self ):
    files = QFileDialog.getOpenFileNames( self, self.tr( "Select files to merge" ), ".", "Shapefiles(*.shp *.SHP)"  )
    if files.isEmpty():
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
    else:
      self.label.setText( self.tr( "Input directory" ) )
      QObject.disconnect( self.btnSelectDir, SIGNAL( "clicked()" ), self.inputFile )
      QObject.connect( self.btnSelectDir, SIGNAL( "clicked()" ), self.inputDir )

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

      self.progressFiles.setRange( 0, self.inputFiles.count() )

    outFile = QFile( self.outFileName )
    if outFile.exists():
      if not QgsVectorFileWriter.deleteShapeFile( self.outFileName ):
        QMessageBox.warning( self, self.tr( "Delete error" ), self.tr( "Can't delete file %1" ).arg( outFileName ) )
        return

    if self.chkListMode.isChecked():
      files = self.leInputDir.text().split( ";" )
      baseDir = QFileInfo( files[ 0 ] ).absolutePath()
    else:
      baseDir = self.leInputDir.text()

    QApplication.setOverrideCursor( QCursor( Qt.WaitCursor ) )
    self.btnOk.setEnabled( False )

    self.mergeThread = ShapeMergeThread( baseDir, self.inputFiles, self.outFileName, self.encoding )
    QObject.connect( self.mergeThread, SIGNAL( "rangeChanged( PyQt_PyObject )" ), self.setProgressRange )
    QObject.connect( self.mergeThread, SIGNAL( "featureProcessed()" ), self.featureProcessed )
    QObject.connect( self.mergeThread, SIGNAL( "shapeProcessed()" ), self.shapeProcessed )
    QObject.connect( self.mergeThread, SIGNAL( "processingFinished()" ), self.processingFinished )
    QObject.connect( self.mergeThread, SIGNAL( "processingInterrupted()" ), self.processingInterrupted )

    self.btnClose.setText( self.tr( "Cancel" ) )
    QObject.disconnect( self.buttonBox, SIGNAL( "rejected()" ), self.reject )
    QObject.connect( self.btnClose, SIGNAL( "clicked()" ), self.stopProcessing )

    self.mergeThread.start()

  def setProgressRange( self, max ):
    self.progressFeatures.setRange( 0, max )
    self.progressFeatures.setValue( 0 )

  def featureProcessed( self ):
    self.progressFeatures.setValue( self.progressFeatures.value() + 1 )

  def shapeProcessed( self ):
    self.progressFiles.setValue( self.progressFiles.value() + 1 )

  def processingFinished( self ):
    self.stopProcessing()

    if self.chkAddToCanvas.isChecked():
      if not ftools_utils.addShapeToCanvas( unicode( self.outFileName ) ):
        QMessageBox.warning( self, self.tr( "Merging" ), self.tr( "Error loading output shapefile:\n%1" ).arg( unicode( self.outFileName ) ) )

    self.restoreGui()

  def processingInterrupted( self ):
    self.restoreGui()

  def stopProcessing( self ):
    if self.mergeThread != None:
      self.mergeThread.stop()
      self.mergeThread = None

  def restoreGui( self ):
    self.progressFeatures.setValue( 0 )
    self.progressFiles.setValue( 0 )
    QApplication.restoreOverrideCursor()
    QObject.connect( self.buttonBox, SIGNAL( "rejected()" ), self.reject )
    self.btnClose.setText( self.tr( "Close" ) )
    self.btnOk.setEnabled( True )

class ShapeMergeThread( QThread ):
  def __init__( self, dir, shapes, outputFileName, outputEncoding ):
    QThread.__init__( self, QThread.currentThread() )
    self.baseDir = dir
    self.shapes = shapes
    self.outputFileName = outputFileName
    self.outputEncoding = outputEncoding

    self.mutex = QMutex()
    self.stopMe = 0

  def run( self ):
    self.mutex.lock()
    self.stopMe = 0
    self.mutex.unlock()

    interrupted = False

    # get information about shapefiles
    layerPath = QFileInfo( self.baseDir + "/" + self.shapes[ 0 ] ).absoluteFilePath()
    newLayer = QgsVectorLayer( layerPath, QFileInfo( layerPath ).baseName(), "ogr" )
    self.crs = newLayer.srs()
    self.geom = newLayer.wkbType()
    vprovider = newLayer.dataProvider()
    self.fields = vprovider.fields()

    writer = QgsVectorFileWriter( self.outputFileName, self.outputEncoding,
             self.fields, self.geom, self.crs )

    for fileName in self.shapes:
      layerPath = QFileInfo( self.baseDir + "/" + fileName ).absoluteFilePath()
      newLayer = QgsVectorLayer( layerPath, QFileInfo( layerPath ).baseName(), "ogr" )
      vprovider = newLayer.dataProvider()
      allAttrs = vprovider.attributeIndexes()
      vprovider.select( allAttrs )
      nFeat = vprovider.featureCount()
      self.emit( SIGNAL( "rangeChanged( PyQt_PyObject )" ), nFeat )
      inFeat = QgsFeature()
      outFeat = QgsFeature()
      inGeom = QgsGeometry()
      while vprovider.nextFeature( inFeat ):
        atMap = inFeat.attributeMap()
        inGeom = QgsGeometry( inFeat.geometry() )
        outFeat.setGeometry( inGeom )
        outFeat.setAttributeMap( atMap )
        writer.addFeature( outFeat )
        self.emit( SIGNAL( "featureProcessed()" ) )

      self.emit( SIGNAL( "shapeProcessed()" ) )
      self.mutex.lock()
      s = self.stopMe
      self.mutex.unlock()
      if s == 1:
        interrupted = True
        break

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
