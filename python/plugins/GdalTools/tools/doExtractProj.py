# -*- coding: utf-8 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

from ui_dialogExtractProjection import Ui_GdalToolsDialog as Ui_Dialog
import GdalTools_utils as Utils

import os.path

try:
  from osgeo import gdal
  from osgeo import osr
except ImportError, e:
  error_str = e.args[ 0 ]
  error_mod = error_str.replace( "No module named ", "" )
  if req_mods.has_key( error_mod ):
    error_str = error_str.replace( error_mod, req_mods[error_mod] )
  raise ImportError( error_str )

class GdalToolsDialog( QDialog, Ui_Dialog ):
  def __init__( self, iface ):
    QDialog.__init__( self )
    self.setupUi( self )
    self.iface = iface

    self.inSelector.setType( self.inSelector.FILE )

    self.recurseCheck.hide()

    self.okButton = self.buttonBox.button( QDialogButtonBox.Ok )
    self.cancelButton = self.buttonBox.button( QDialogButtonBox.Cancel )

    self.connect( self.inSelector, SIGNAL( "selectClicked()" ), self.fillInputFileEdit )
    self.connect( self.batchCheck, SIGNAL( "stateChanged( int )" ), self.switchToolMode )

  def switchToolMode( self ):
    self.recurseCheck.setVisible( self.batchCheck.isChecked() )

    self.inSelector.clear()

    if self.batchCheck.isChecked():
      self.inFileLabel = self.label.text()
      self.label.setText( QCoreApplication.translate( "GdalTools", "&Input directory" ) )

      QObject.disconnect( self.inSelector, SIGNAL( "selectClicked()" ), self.fillInputFileEdit )
      QObject.connect( self.inSelector, SIGNAL( "selectClicked()" ), self.fillInputDir )
    else:
      self.label.setText( self.inFileLabel )

      QObject.connect( self.inSelector, SIGNAL( "selectClicked()" ), self.fillInputFileEdit )
      QObject.disconnect( self.inSelector, SIGNAL( "selectClicked()" ), self.fillInputDir )

  def fillInputFileEdit( self ):
    lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
    inputFile = Utils.FileDialog.getOpenFileName( self, self.tr( "Select the file to analyse" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter )
    if inputFile.isEmpty():
      return
    Utils.FileFilter.setLastUsedRasterFilter( lastUsedFilter )
    self.inSelector.setFilename( inputFile )

  def fillInputDir( self ):
    inputDir = Utils.FileDialog.getExistingDirectory( self, self.tr( "Select the input directory with files to Assign projection" ))
    if inputDir.isEmpty():
      return
    self.inSelector.setFilename( inputDir )

  def reject( self ):
    QDialog.reject( self )

  def accept( self ):
    self.inFiles = None
    if self.batchCheck.isChecked():
      self.inFiles = Utils.getRasterFiles( self.inSelector.filename(), self.recurseCheck.isChecked() )
    else:
      self.inFiles = [ self.inSelector.filename() ]

    self.progressBar.setRange( 0, len( self.inFiles ) )

    QApplication.setOverrideCursor( QCursor( Qt.WaitCursor ) )
    self.okButton.setEnabled( False )

    self.extractor = ExtractThread( self.inFiles, self.prjCheck.isChecked() )
    QObject.connect( self.extractor, SIGNAL( "fileProcessed()" ), self.updateProgress )
    QObject.connect( self.extractor, SIGNAL( "processFinished()" ), self.processingFinished )
    QObject.connect( self.extractor, SIGNAL( "processInterrupted()" ), self.processingInterrupted )

    QObject.disconnect( self.buttonBox, SIGNAL( "rejected()" ), self.reject )
    QObject.connect( self.buttonBox, SIGNAL( "rejected()" ), self.stopProcessing )

    self.extractor.start()

  def updateProgress( self ):
    self.progressBar.setValue( self.progressBar.value() + 1 )

  def processingFinished( self ):
    self.stopProcessing()

  def processingInterrupted( self ):
    self.restoreGui()

  def stopProcessing( self ):
    if self.extractor != None:
      self.extractor.stop()
      self.extractor = None

    self.restoreGui()

  def restoreGui( self ):
    self.progressBar.setRange( 0, 100 )
    self.progressBar.setValue( 0 )

    QApplication.restoreOverrideCursor()

    QObject.disconnect( self.buttonBox, SIGNAL( "rejected()" ), self.stopProcessing )
    QObject.connect( self.buttonBox, SIGNAL( "rejected()" ), self.reject )

    self.okButton.setEnabled( True )

# ----------------------------------------------------------------------

def extractProjection( filename, createPrj ):
  raster = gdal.Open( unicode( filename ) )

  crs = raster.GetProjection()
  geotransform = raster.GetGeoTransform()

  raster = None

  outFileName = os.path.splitext( unicode( filename ) )[0]

  # create prj file requested and if projection available
  if crs != "" and createPrj:
    # convert CRS into ESRI format
    tmp = osr.SpatialReference()
    tmp.ImportFromWkt( crs )
    tmp.MorphToESRI()
    crs = tmp.ExportToWkt()
    tmp = None

    prj = open( outFileName + '.prj', 'wt' )
    prj.write( crs )
    prj.close()

  # create wld file
  wld = open( outFileName + '.wld', 'wt')
  wld.write( "%0.8f\n" % geotransform[1] )
  wld.write( "%0.8f\n" % geotransform[4] )
  wld.write( "%0.8f\n" % geotransform[2] )
  wld.write( "%0.8f\n" % geotransform[5] )
  wld.write( "%0.8f\n" % (geotransform[0] + 0.5 * geotransform[1] + 0.5 * geotransform[2] ) )
  wld.write( "%0.8f\n" % (geotransform[3] + 0.5 * geotransform[4] + 0.5 * geotransform[5] ) )
  wld.close()

class ExtractThread( QThread ):
  def __init__( self, files, needPrj ):
    QThread.__init__( self, QThread.currentThread() )
    self.inFiles = files
    self.needPrj = needPrj

    self.mutex = QMutex()
    self.stopMe = 0

  def run( self ):
    self.mutex.lock()
    self.stopMe = 0
    self.mutex.unlock()

    interrupted = False

    for f in self.inFiles:
      extractProjection( f, self.needPrj )
      self.emit( SIGNAL( "fileProcessed()" ) )

      self.mutex.lock()
      s = self.stopMe
      self.mutex.unlock()
      if s == 1:
        interrupted = True
        break

    if not interrupted:
      self.emit( SIGNAL( "processFinished()" ) )
    else:
      self.emit( SIGNAL( "processIterrupted()" ) )

  def stop( self ):
    self.mutex.lock()
    self.stopMe = 1
    self.mutex.unlock()

    QThread.wait( self )
