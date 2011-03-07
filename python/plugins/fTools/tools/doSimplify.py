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
  def __init__( self, iface ):
    QDialog.__init__( self )
    self.setupUi( self )
    self.iface = iface

    self.simplifyThread = None

    self.okButton = self.buttonBox.button( QDialogButtonBox.Ok )
    self.closeButton = self.buttonBox.button( QDialogButtonBox.Close )

    QObject.connect( self.writeShapefileCheck, SIGNAL( "stateChanged( int )" ), self.updateGui )
    QObject.connect( self.btnSelectOutputFile, SIGNAL( "clicked()" ), self.selectOutputFile )

    self.manageGui()

  def manageGui( self ):
    layers = ftools_utils.getLayerNames( [ QGis.Polygon, QGis.Line ] )
    self.cmbInputLayer.addItems( layers )

  def updateGui( self ):
    if self.writeShapefileCheck.isChecked():
      self.outputFileEdit.setEnabled( True )
      self.btnSelectOutputFile.setEnabled( True )
      self.addToCanvasCheck.setEnabled( True )
    else:
      self.outputFileEdit.setEnabled( False )
      self.btnSelectOutputFile.setEnabled( False )
      self.addToCanvasCheck.setEnabled( False )
    self.encoding = None

  def selectOutputFile( self ):
    self.outputFileEdit.clear()
    (self.shapefileName, self.encoding) = ftools_utils.saveDialog(self)
    if self.shapefileName is None or self.encoding is None:
      return
    self.outputFileEdit.setText(QString(self.shapefileName))

  def accept( self ):
    vLayer = ftools_utils.getVectorLayerByName( self.cmbInputLayer.currentText() )

    QApplication.setOverrideCursor( Qt.WaitCursor )
    self.okButton.setEnabled( False )

    if self.writeShapefileCheck.isChecked():
      outFileName = self.outputFileEdit.text()
      outFile = QFile( outFileName )
      if outFile.exists():
        if not QgsVectorFileWriter.deleteShapeFile( outFileName ):
          QmessageBox.warning( self, self.tr( "Delete error" ), self.tr( "Can't delete file %1" ).arg( outFileName ) )
          return
      self.simplifyThread = GeneralizationThread( vLayer, self.useSelectionCheck.isChecked(), self.toleranceSpin.value(), True, outFileName, self.encoding )
    else:
      self.simplifyThread = GeneralizationThread( vLayer, self.useSelectionCheck.isChecked(), self.toleranceSpin.value(), False, None, None  )
    QObject.connect( self.simplifyThread, SIGNAL( "rangeCalculated( PyQt_PyObject )" ), self.setProgressRange )
    QObject.connect( self.simplifyThread, SIGNAL( "featureProcessed()" ), self.featureProcessed )
    QObject.connect( self.simplifyThread, SIGNAL( "generalizationFinished( PyQt_PyObject )" ), self.generalizationFinished )
    QObject.connect( self.simplifyThread, SIGNAL( "generalizationInterrupted()" ), self.generalizationInterrupted )

    self.closeButton.setText( self.tr( "Cancel" ) )
    QObject.disconnect( self.buttonBox, SIGNAL( "rejected()" ), self.reject )
    QObject.connect( self.closeButton, SIGNAL( "clicked()" ), self.stopProcessing )

    self.simplifyThread.start()

  def setProgressRange( self, max ):
    self.progressBar.setRange( 0, max )

  def featureProcessed( self ):
    self.progressBar.setValue( self.progressBar.value() + 1 )

  def generalizationFinished( self, pointsCount ):
    self.stopProcessing()

    QMessageBox.information( self,
                             self.tr( "Simplify results" ),
                             self.tr( "There were %1 vertices in original dataset which\nwere reduced to %2 vertices after simplification" )
                             .arg( pointsCount[ 0 ] )
                             .arg( pointsCount[ 1 ] ) )

    self.restoreGui()
    if self.addToCanvasCheck.isEnabled() and self.addToCanvasCheck.isChecked():
      if not ftools_utils.addShapeToCanvas( unicode( self.shapefileName ) ):
        QMessageBox.warning( self, self.tr( "Merging" ),
                             self.tr( "Error loading output shapefile:\n%1" )
                             .arg( unicode( self.shapefileName ) ) )

    self.iface.mapCanvas().refresh()
    #self.restoreGui()

  def generalizationInterrupted( self ):
    self.restoreGui()

  def stopProcessing( self ):
    if self.simplifyThread != None:
      self.simplifyThread.stop()
      self.simplifyThread = None

  def restoreGui( self ):
    self.progressBar.setValue( 0 )
    QApplication.restoreOverrideCursor()
    QObject.connect( self.buttonBox, SIGNAL( "rejected()" ), self.reject )
    self.closeButton.setText( self.tr( "Close" ) )
    self.okButton.setEnabled( True )

def geomVertexCount( geometry ):
  geomType = geometry.type()
  if geomType == 1: # line
    points = geometry.asPolyline()
    return len( points )
  elif geomType == 2: # polygon
    polylines = geometry.asPolygon()
    points = []
    for l in polylines:
      points.extend( l )
    return len( points )
  else:
    return None

class GeneralizationThread( QThread ):
  def __init__( self, inputLayer, useSelection, tolerance, writeShape, shapePath, shapeEncoding ):
    QThread.__init__( self, QThread.currentThread() )
    self.inputLayer = inputLayer
    self.useSelection = useSelection
    self.tolerance = tolerance
    self.writeShape = writeShape
    self.outputFileName = shapePath
    self.outputEncoding = shapeEncoding

    self.shapeFileWriter = None
    self.pointsBefore = 0
    self.pointsAfter = 0

    self.mutex = QMutex()
    self.stopMe = 0

  def run( self ):
    self.mutex.lock()
    self.stopMe = 0
    self.mutex.unlock()

    interrupted = False

    shapeFileWriter = None

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
          self.pointsBefore += geomVertexCount( featGeometry )
          newGeometry = featGeometry.simplify( self.tolerance )
          self.pointsAfter += geomVertexCount( newGeometry )
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
          self.pointsBefore += geomVertexCount( featGeometry )
          newGeometry = featGeometry.simplify( self.tolerance )
          self.pointsAfter += geomVertexCount( newGeometry )
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
          self.pointsBefore += geomVertexCount( featGeometry )
          newGeometry = featGeometry.simplify( self.tolerance )
          self.pointsAfter += geomVertexCount( newGeometry )
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
          self.pointsBefore += geomVertexCount( featGeometry )
          newGeometry = featGeometry.simplify( self.tolerance )
          self.pointsAfter += geomVertexCount( newGeometry )
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
      self.emit( SIGNAL( "generalizationFinished( PyQt_PyObject )" ), ( self.pointsBefore, self.pointsAfter ) )
    else:
      self.emit( SIGNAL( "generalizationInterrupted()" ) )

  def stop( self ):
    self.mutex.lock()
    self.stopMe = 1
    self.mutex.unlock()

    QThread.wait( self )
