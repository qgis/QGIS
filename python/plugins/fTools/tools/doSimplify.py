# -*- coding: utf-8 -*-
#-----------------------------------------------------------
#
# fTools
# Copyright (C) 2008-2011  Carson Farmer
# EMAIL: carson.farmer (at) gmail.com
# WEB  : http://www.ftools.ca/fTools.html
#
# A collection of data management and analysis tools for vector data
#
#-----------------------------------------------------------
#
# licensed under the terms of GNU GPL 2
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
#---------------------------------------------------------------------

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from qgis.core import *
from qgis.gui import *

import ftools_utils

from ui_frmSimplify import Ui_Dialog

class Dialog( QDialog, Ui_Dialog ):
  def __init__( self, iface, function ):
    QDialog.__init__( self, iface.mainWindow() )
    self.setupUi( self )
    self.iface = iface
    self.myFunction = function

    self.workThread = None

    if self.myFunction == 2:
      self.setWindowTitle( self.tr( "Densify geometries" ) )
      self.lblTolerance.setText( self.tr( "Vertices to add" ) )
      self.spnTolerance.setDecimals( 0 )
      self.spnTolerance.setMinimum( 1 )
      self.spnTolerance.setSingleStep( 1 )
      self.spnTolerance.setValue( 1.0 )

    self.btnOk = self.buttonBox.button( QDialogButtonBox.Ok )
    self.btnClose = self.buttonBox.button( QDialogButtonBox.Close )

    QObject.connect( self.chkWriteShapefile, SIGNAL( "stateChanged( int )" ), self.updateGui )
    QObject.connect( self.btnSelectOutputFile, SIGNAL( "clicked()" ), self.selectOutputFile )

    self.populateLayers()

  def populateLayers( self ):
    layers = ftools_utils.getLayerNames( [ QGis.Polygon, QGis.Line ] )
    self.cmbInputLayer.clear()
    self.cmbInputLayer.addItems( layers )

  def updateGui( self ):
    if self.chkWriteShapefile.isChecked():
      self.edOutputFile.setEnabled( True )
      self.btnSelectOutputFile.setEnabled( True )
      self.chkAddToCanvas.setEnabled( True )
    else:
      self.edOutputFile.setEnabled( False )
      self.btnSelectOutputFile.setEnabled( False )
      self.chkAddToCanvas.setEnabled( False )

    self.encoding = None

  def selectOutputFile( self ):
    self.edOutputFile.clear()
    ( self.shapeFileName, self.encoding ) = ftools_utils.saveDialog( self )
    if self.shapeFileName is None or self.encoding is None:
      return
    self.edOutputFile.setText( QString( self.shapeFileName ) )

  def accept( self ):
    vLayer = ftools_utils.getVectorLayerByName( self.cmbInputLayer.currentText() )

    self.btnOk.setEnabled( False )

    if self.chkWriteShapefile.isChecked():
      outFileName = self.edOutputFile.text()
      outFile = QFile( outFileName )
      if outFile.exists():
        if not QgsVectorFileWriter.deleteShapeFile( outFileName ):
          QMessageBox.warning( self, self.tr( "Delete error" ),
                               self.tr( "Can't delete file %1" ).arg( outFileName ) )
          return

      self.workThread = GeomThread( self.myFunction, vLayer, self.chkUseSelection.isChecked(),
                                    self.spnTolerance.value(), True, outFileName, self.encoding )
    else:
      res = QMessageBox.warning( self, self.tr( "Warning"),
                                 self.tr( "Currently QGIS doesn't allow simultaneous access from \
                                 different threads to the same datasource. Make sure your layer's \
                                 attribute tables are closed. Continue?"),
                                 QMessageBox.Yes | QMessageBox.No )
      if res == QMessageBox.No:
        return

      self.workThread = GeomThread( self.myFunction, vLayer, self.chkUseSelection.isChecked(),
                                    self.spnTolerance.value(), False, None, None )

    QObject.connect( self.workThread, SIGNAL( "rangeCalculated( PyQt_PyObject )" ), self.setProgressRange )
    QObject.connect( self.workThread, SIGNAL( "featureProcessed()" ), self.featureProcessed )
    QObject.connect( self.workThread, SIGNAL( "processingFinished( PyQt_PyObject )" ), self.processFinished )
    QObject.connect( self.workThread, SIGNAL( "processingInterrupted()" ), self.processInterrupted )

    self.btnClose.setText( self.tr( "Cancel" ) )
    QObject.disconnect( self.buttonBox, SIGNAL( "rejected()" ), self.reject )
    QObject.connect( self.btnClose, SIGNAL( "clicked()" ), self.stopProcessing )

    self.workThread.start()

  def setProgressRange( self, maximum ):
    self.progressBar.setRange( 0, maximum )

  def featureProcessed( self ):
    self.progressBar.setValue( self.progressBar.value() + 1 )

  def processFinished( self, pointsCount ):
    self.stopProcessing()

    if self.myFunction == 1:
      QMessageBox.information( self, self.tr( "Simplify results" ),
                               self.tr( "There were %1 vertices in original dataset which\nwere reduced to %2 vertices after simplification" )
                               .arg( pointsCount[ 0 ] )
                               .arg( pointsCount[ 1 ] ) )

    self.restoreGui()

    if self.chkAddToCanvas.isEnabled() and self.chkAddToCanvas.isChecked():
      if not ftools_utils.addShapeToCanvas( unicode( self.shapeFileName ) ):
        QMessageBox.warning( self, self.tr( "Error" ),
                             self.tr( "Error loading output shapefile:\n%1" )
                             .arg( unicode( self.shapeFileName ) ) )
      self.populateLayers()

    QMessageBox.information( self, self.tr( "Finished" ), self.tr( "Processing completed." ) )
    self.iface.mapCanvas().refresh()

  def processInterrupted( self ):
    self.restoreGui()

  def stopProcessing( self ):
    if self.workThread != None:
      self.workThread.stop()
      self.workThread = None

  def restoreGui( self ):
    self.progressBar.setValue( 0 )
    QObject.connect( self.buttonBox, SIGNAL( "rejected()" ), self.reject )
    self.btnClose.setText( self.tr( "Close" ) )
    self.btnOk.setEnabled( True )

def geomVertexCount( geometry ):
  geomType = geometry.type()
  if geomType == QGis.Line:
    if geometry.isMultipart():
      pointsList = geometry.asMultiPolyline()
      points = sum( pointsList, [] )
    else:
      points = geometry.asPolyline()
    return len( points )
  elif geomType == QGis.Polygon:
    if geometry.isMultipart():
      polylinesList = geometry.asMultiPolygon()
      polylines = sum( polylinesList, [] )
    else:
      polylines = geometry.asPolygon()
    points = []
    for l in polylines:
      points.extend( l )
    return len( points )
  else:
    return None

def densify( polyline, pointsNumber ):
  output = []
  if pointsNumber != 1:
    multiplier = 1.0 / float( pointsNumber + 1 )
  else:
    multiplier = 1
  for i in xrange( len( polyline ) - 1 ):
    p1 = polyline[ i ]
    p2 = polyline[ i + 1 ]
    output.append( p1 )
    for j in xrange( pointsNumber ):
      delta = multiplier * ( j + 1 )
      x = p1.x() + delta * ( p2.x() - p1.x() )
      y = p1.y() + delta * ( p2.y() - p1.y() )
      output.append( QgsPoint( x, y ) )
      if j + 1 == pointsNumber:
        break
  output.append( polyline[ len( polyline ) - 1 ] )
  return output

def densifyGeometry( geometry, pointsNumber, isPolygon ):
  output = []
  if isPolygon:
    if geometry.isMultipart():
      polygons = geometry.asMultiPolygon()
      for poly in polygons:
        p = []
        for ring in poly:
          p.append( densify( ring, pointsNumber ) )
        output.append( p )
      return QgsGeometry.fromMultiPolygon( output )
    else:
      rings = geometry.asPolygon()
      for ring in rings:
        output.append( densify( ring, pointsNumber ) )
      return QgsGeometry.fromPolygon( output )
  else:
    if geometry.isMultipart():
      lines = geometry.asMultiPolyline()
      for points in lines:
        output.append( densify( points, pointsNumber ) )
      return QgsGeometry.fromMultiPolyline( output )
    else:
      points = geometry.asPolyline()
      output = densify( points, pointsNumber )
      return QgsGeometry.fromPolyline( output )

class GeomThread( QThread ):
  def __init__( self, function, inputLayer, useSelection, tolerance, writeShape, shapePath, shapeEncoding ):
    QThread.__init__( self, QThread.currentThread() )
    self.inputLayer = inputLayer
    self.useSelection = useSelection
    self.tolerance = tolerance
    self.writeShape = writeShape
    self.outputFileName = shapePath
    self.outputEncoding = shapeEncoding
    self.myFunction = function

    self.mutex = QMutex()
    self.stopMe = 0

  def run( self ):
    if self.myFunction == 1:
      self.runSimplify()
    else:
      self.runDensify()

  def runSimplify( self ):
    self.mutex.lock()
    self.stopMe = 0
    self.mutex.unlock()

    interrupted = False

    shapeFileWriter = None

    pointsBefore = 0
    pointsAfter = 0

    if self.writeShape:
      vProvider = self.inputLayer.dataProvider()
      allAttrs = vProvider.attributeIndexes()
      vProvider.select( allAttrs )
      shapeFields = vProvider.fields()
      crs = vProvider.crs()
      wkbType = self.inputLayer.wkbType()
      if not crs.isValid():
        crs = None
      shapeFileWriter = QgsVectorFileWriter( self.outputFileName, self.outputEncoding, shapeFields, wkbType, crs )
      featureId = 0
      if self.useSelection:
        selection = self.inputLayer.selectedFeatures()
        self.emit( SIGNAL( "rangeCalculated( PyQt_PyObject )" ), len( selection ) )
        for f in selection:
          featGeometry = QgsGeometry( f.geometry() )
          attrMap = f.attributeMap()

          pointsBefore += geomVertexCount( featGeometry )
          newGeometry = featGeometry.simplify( self.tolerance )
          pointsAfter += geomVertexCount( newGeometry )

          feature = QgsFeature()
          feature.setGeometry( newGeometry )
          feature.setAttributeMap( attrMap )
          shapeFileWriter.addFeature( feature )
          featureId += 1
          self.emit( SIGNAL( "featureProcessed()" ) )

          self.mutex.lock()
          s = self.stopMe
          self.mutex.unlock()
          if s == 1:
            interrupted = True
            break
      else:
        self.emit( SIGNAL( "rangeCalculated( PyQt_PyObject )" ), vProvider.featureCount() )
        f = QgsFeature()
        while vProvider.nextFeature( f ):
          featGeometry = QgsGeometry( f.geometry() )
          attrMap = f.attributeMap()

          pointsBefore += geomVertexCount( featGeometry )
          newGeometry = featGeometry.simplify( self.tolerance )
          pointsAfter += geomVertexCount( newGeometry )

          feature = QgsFeature()
          feature.setGeometry( newGeometry )
          feature.setAttributeMap( attrMap )
          shapeFileWriter.addFeature( feature )
          featureId += 1
          self.emit( SIGNAL( "featureProcessed()" ) )

          self.mutex.lock()
          s = self.stopMe
          self.mutex.unlock()
          if s == 1:
            interrupted = True
            break
    else: # modify existing shapefile
      if not self.inputLayer.isEditable():
        self.inputLayer.startEditing()
      self.inputLayer.beginEditCommand( QString( "Simplify line(s)" ) )
      if self.useSelection:
        selection = self.inputLayer.selectedFeatures()
        self.emit( SIGNAL( "rangeCalculated( PyQt_PyObject )" ), len( selection ) )
        for f in selection:
          featureId = f.id()
          featGeometry = QgsGeometry( f.geometry() )

          pointsBefore += geomVertexCount( featGeometry )
          newGeometry = featGeometry.simplify( self.tolerance )
          pointsAfter += geomVertexCount( newGeometry )

          self.inputLayer.changeGeometry( featureId, newGeometry )
          self.emit( SIGNAL( "featureProcessed()" ) )

          self.mutex.lock()
          s = self.stopMe
          self.mutex.unlock()
          if s == 1:
            interrupted = True
            break
      else:
        vProvider = self.inputLayer.dataProvider()
        self.emit( SIGNAL( "rangeCalculated( PyQt_PyObject )" ), vProvider.featureCount() )
        f = QgsFeature()
        while vProvider.nextFeature( f ):
          featureId = f.id()
          featGeometry = QgsGeometry( f.geometry() )

          pointsBefore += geomVertexCount( featGeometry )
          newGeometry = featGeometry.simplify( self.tolerance )
          pointsAfter += geomVertexCount( newGeometry )

          self.inputLayer.changeGeometry( featureId, newGeometry )
          self.emit( SIGNAL( "featureProcessed()" ) )

          self.mutex.lock()
          s = self.stopMe
          self.mutex.unlock()
          if s == 1:
            interrupted = True
            break

    # cleanup
    if self.inputLayer.isEditable():
      self.inputLayer.endEditCommand()

    if shapeFileWriter != None:
      del shapeFileWriter

    if not interrupted:
      self.emit( SIGNAL( "processingFinished( PyQt_PyObject )" ), ( pointsBefore, pointsAfter ) )
    else:
      self.emit( SIGNAL( "processingInterrupted()" ) )

  def runDensify( self ):
    self.mutex.lock()
    self.stopMe = 0
    self.mutex.unlock()

    interrupted = False

    shapeFileWriter = None

    isPolygon = self.inputLayer.geometryType() == QGis.Polygon

    if self.writeShape:
      # prepare writer
      vProvider = self.inputLayer.dataProvider()
      allAttrs = vProvider.attributeIndexes()
      vProvider.select( allAttrs )
      shapeFields = vProvider.fields()
      crs = vProvider.crs()
      wkbType = self.inputLayer.wkbType()
      if not crs.isValid():
        crs = None
      shapeFileWriter = QgsVectorFileWriter( self.outputFileName, self.outputEncoding, shapeFields, wkbType, crs )
      featureId = 0

      if self.useSelection:
        selection = self.inputLayer.selectedFeatures()
        self.emit( SIGNAL( "rangeCalculated( PyQt_PyObject )" ), len( selection ) )
        for f in selection:
          featGeometry = QgsGeometry( f.geometry() )
          attrMap = f.attributeMap()
          newGeometry = densifyGeometry( featGeometry, int( self.tolerance ), isPolygon )

          feature = QgsFeature()
          feature.setGeometry( newGeometry )
          feature.setAttributeMap( attrMap )
          shapeFileWriter.addFeature( feature )
          featureId += 1
          self.emit( SIGNAL( "featureProcessed()" ) )

          self.mutex.lock()
          s = self.stopMe
          self.mutex.unlock()
          if s == 1:
            interrupted = True
            break
      else:
        self.emit( SIGNAL( "rangeCalculated( PyQt_PyObject )" ), vProvider.featureCount() )
        f = QgsFeature()
        while vProvider.nextFeature( f ):
          featGeometry = QgsGeometry( f.geometry() )
          attrMap = f.attributeMap()
          newGeometry = densifyGeometry( featGeometry, int( self.tolerance ), isPolygon )

          feature = QgsFeature()
          feature.setGeometry( newGeometry )
          feature.setAttributeMap( attrMap )
          shapeFileWriter.addFeature( feature )
          featureId += 1
          self.emit( SIGNAL( "featureProcessed()" ) )

          self.mutex.lock()
          s = self.stopMe
          self.mutex.unlock()
          if s == 1:
            interrupted = True
            break
    else: # modify existing shapefile
      if not self.inputLayer.isEditable():
        self.inputLayer.startEditing()

      self.inputLayer.beginEditCommand( QString( "Densify line(s)" ) )

      if self.useSelection:
        selection = self.inputLayer.selectedFeatures()
        self.emit( SIGNAL( "rangeCalculated( PyQt_PyObject )" ), len( selection ) )
        for f in selection:
          featureId = f.id()
          featGeometry = QgsGeometry( f.geometry() )
          newGeometry = densifyGeometry( featGeometry, int( self.tolerance ), isPolygon )

          self.inputLayer.changeGeometry( featureId, newGeometry )
          self.emit( SIGNAL( "featureProcessed()" ) )

          self.mutex.lock()
          s = self.stopMe
          self.mutex.unlock()
          if s == 1:
            interrupted = True
            break
      else:
        vProvider = self.inputLayer.dataProvider()
        self.emit( SIGNAL( "rangeCalculated( PyQt_PyObject )" ), vProvider.featureCount() )
        f = QgsFeature()
        while vProvider.nextFeature( f ):
          featureId = f.id()
          featGeometry = QgsGeometry( f.geometry() )
          newGeometry = densifyGeometry( featGeometry, int( self.tolerance ), isPolygon )

          self.inputLayer.changeGeometry( featureId, newGeometry )
          self.emit( SIGNAL( "featureProcessed()" ) )

          self.mutex.lock()
          s = self.stopMe
          self.mutex.unlock()
          if s == 1:
            interrupted = True
            break

    # cleanup
    if self.inputLayer.isEditable():
      self.inputLayer.endEditCommand()

    if shapeFileWriter != None:
      del shapeFileWriter

    if not interrupted:
      self.emit( SIGNAL( "processingFinished( PyQt_PyObject )" ), None )
    else:
      self.emit( SIGNAL( "processingInterrupted()" ) )

  def stop( self ):
    self.mutex.lock()
    self.stopMe = 1
    self.mutex.unlock()

    QThread.wait( self )
