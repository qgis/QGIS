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
# Geoprocessing functions adapted from 'Geoprocessing Plugin',
# (C) 2008 by Dr. Horst Duester, Stefan Ziegler
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
from ui_frmGeoprocessing import Ui_Dialog
import ftools_utils
import sys

class GeoprocessingDialog( QDialog, Ui_Dialog ):
  def __init__( self, iface, function ):
    QDialog.__init__( self, iface.mainWindow() )
    self.iface = iface
    self.setupUi( self )
    self.param.setValidator(QDoubleValidator(self.param))
    self.myFunction = function
    QObject.connect( self.btnBrowse, SIGNAL( "clicked()" ), self.outFile )
    QObject.connect( self.inShapeA, SIGNAL( "currentIndexChanged(QString)" ), self.checkA )
    QObject.connect( self.inShapeB, SIGNAL( "currentIndexChanged(QString)" ), self.checkB )
    if function == 4 or function == 1 or function == 2:
      QObject.connect( self.inShapeA, SIGNAL( "currentIndexChanged(QString)" ), self.update )
    self.manageGui()
    self.success = False
    self.cancel_close = self.buttonBox_2.button( QDialogButtonBox.Close )
    self.buttonOk = self.buttonBox_2.button( QDialogButtonBox.Ok )
    self.progressBar.setValue (0 )

  def checkA( self ):
    inputLayer = unicode( self.inShapeA.currentText() )
    if inputLayer != "":
      changedLayer = ftools_utils.getVectorLayerByName( inputLayer )
      if changedLayer.selectedFeatureCount() != 0:
        self.useSelectedA.setCheckState( Qt.Checked )
      else:
        self.useSelectedA.setCheckState( Qt.Unchecked )

  def checkB( self ):
    inputLayer = unicode( self.inShapeB.currentText() )
    if inputLayer != "":
      changedLayer = ftools_utils.getVectorLayerByName( inputLayer )
      if changedLayer.selectedFeatureCount() != 0:
        self.useSelectedB.setCheckState( Qt.Checked )
      else:
        self.useSelectedB.setCheckState( Qt.Unchecked )

  def update( self ):
    self.attrib.clear()
    inputLayer = unicode( self.inShapeA.currentText() )
    if inputLayer != "":
      changedLayer = ftools_utils.getVectorLayerByName( inputLayer )
      changedField = changedLayer.dataProvider().fields()
      for f in changedField:
        self.attrib.addItem( unicode( f.name() ) )
      if self.myFunction == 4:
        self.attrib.addItem( "--- " + self.tr( "Dissolve all" ) + " ---" )

  def accept( self ):
    if self.inShapeA.currentText() == "":
      QMessageBox.warning( self, self.tr("Geoprocessing"), self.tr( "Please specify an input layer" ) )
    elif self.inShapeB.isVisible() and self.inShapeB.currentText() == "":
      QMessageBox.warning( self, self.tr("Geoprocessing"), self.tr( "Please specify a difference/intersect/union layer" ) )
    elif self.param.isEnabled() and self.param.isVisible() and self.param.text() == "":
      QMessageBox.warning( self, self.tr("Geoprocessing"), self.tr( "Please specify valid buffer value" ) )
    elif self.attrib.isEnabled() and self.attrib.isVisible() and self.attrib.currentText() == "":
      QMessageBox.warning( self, self.tr("Geoprocessing"), self.tr( "Please specify dissolve field" ) )
    elif self.outShape.text() == "":
      QMessageBox.warning( self, self.tr("Geoprocessing"), self.tr( "Please specify output shapefile" ) )
    else:
      changedLayerA = ftools_utils.getVectorLayerByName( self.inShapeA.currentText() )
      changedLayerB = ftools_utils.getVectorLayerByName( self.inShapeB.currentText() )
      # check for selection in layer A
      if self.useSelectedA.isChecked() and changedLayerA.selectedFeatureCount() == 0:
        QMessageBox.warning( self, self.tr("Geoprocessing"), self.tr( "No features selected, please uncheck 'Use selected' or make a selection" ) )
      # check for selection in layer B
      elif self.inShapeB.isVisible() and self.useSelectedB.isChecked() and changedLayerB.selectedFeatureCount() == 0:
        QMessageBox.warning( self, self.tr("Geoprocessing"), self.tr( "No features selected, please uncheck 'Use selected' or make a selection" ) )
      else:
        self.outShape.clear()
        if self.attrib.isEnabled():
          self.geoprocessing( self.inShapeA.currentText(), self.inShapeB.currentText(),
          unicode( self.attrib.currentText() ), self.mergeOutput.checkState(), self.useSelectedA.checkState(),
          self.useSelectedB.checkState(), self.spnSegments.value() )
        else:
          if self.param.isEnabled() and self.param.isVisible():
            parameter = float( self.param.text() )
          else:
            parameter = None
          self.geoprocessing( self.inShapeA.currentText(), self.inShapeB.currentText(),
          parameter, self.mergeOutput.checkState(), self.useSelectedA.checkState(), self.useSelectedB.checkState(), self.spnSegments.value() )

  def outFile( self ):
    self.outShape.clear()
    ( self.shapefileName, self.encoding ) = ftools_utils.saveDialog( self )
    if self.shapefileName is None or self.encoding is None:
      return
    self.outShape.setText( self.shapefileName )

  def manageGui( self ):
    if self.myFunction == 1:  # Buffer
      self.label_2.hide()
      self.inShapeB.hide()
      self.useSelectedB.hide()
      self.label_4.hide()
      self.setWindowTitle( self.tr( "Buffer(s)" ) )
    elif self.myFunction == 2: # Convex hull
      self.label_2.hide()
      self.inShapeB.hide()
      self.useSelectedB.hide()
      self.rdoBuffer.setText( self.tr( "Create single minimum convex hull" ) )
      self.rdoField.setText( self.tr( "Create convex hulls based on input field" ) )
      self.label_4.hide()
      self.param.hide()
      self.lblSegments.hide()
      self.spnSegments.hide()
      self.setWindowTitle( self.tr( "Convex hull(s)" ) )
      self.mergeOutput.hide()
    elif self.myFunction == 4: # Dissolve
      self.label_2.hide()
      self.inShapeB.hide()
      self.useSelectedB.hide()
      self.rdoBuffer.hide()
      self.attrib.setEnabled( True )
      self.param.hide()
      self.rdoField.hide()
      self.mergeOutput.hide()
      self.lblSegments.hide()
      self.spnSegments.hide()
      self.setWindowTitle( self.tr( "Dissolve" ) )
    else:
      self.rdoBuffer.hide()
      self.param.hide()
      self.label_4.hide()
      self.rdoField.hide()
      self.attrib.hide()
      self.mergeOutput.hide()
      self.lblSegments.hide()
      self.spnSegments.hide()
      if self.myFunction == 3: # Difference
        self.label_2.setText( self.tr( "Difference layer" ) )
        self.setWindowTitle( self.tr( "Difference" ) )
      elif self.myFunction == 5: # Intersect
        self.label_2.setText( self.tr( "Intersect layer" ) )
        self.setWindowTitle( self.tr( "Intersect" ) )
      elif self.myFunction == 7: # Symmetrical difference
        self.label_2.setText( self.tr( "Difference layer" ) )
        self.setWindowTitle( self.tr( "Symmetrical difference" ) )
        self.useSelectedA.hide()
        self.useSelectedB.hide()
      elif self.myFunction == 8: # Clip
        self.label_2.setText( self.tr( "Clip layer" ) )
        self.setWindowTitle( self.tr( "Clip" ) )
      else: # Union
        self.label_2.setText( self.tr( "Union layer" ) )
        self.setWindowTitle( self.tr( "Union" ) )
        self.useSelectedA.hide()
        self.useSelectedB.hide()
    self.resize(381, 100)
    self.populateLayers()

  def populateLayers( self ):
    myListA = []
    myListB = []
    self.inShapeA.clear()
    self.inShapeB.clear()

    if self.myFunction == 4:
      myListA = ftools_utils.getLayerNames( [ QGis.Line, QGis.Polygon ] )
      myListB = []
    else:
      myListA = ftools_utils.getLayerNames( [ QGis.Point, QGis.Line, QGis.Polygon ] )
      myListB = ftools_utils.getLayerNames( [ QGis.Point, QGis.Line, QGis.Polygon ] )
    self.inShapeA.addItems( myListA )
    self.inShapeB.addItems( myListB )

#1: Buffer
#2: Convex Hull
#3: Difference
#4: Dissolve
#5: Intersection
#6: Union
#7: Symetrical Difference
#8: Clip

  def geoprocessing( self,  myLayerA,  myLayerB,  myParam,  myMerge, mySelectionA, mySelectionB, mySegments ):
    check = QFile( self.shapefileName )
    if check.exists():
      if not QgsVectorFileWriter.deleteShapeFile( self.shapefileName ):
        QMessageBox.warning( self, self.tr("Geoprocessing"), self.tr( "Unable to delete existing shapefile." ) )
        return
    self.buttonOk.setEnabled( False )
    self.testThread = geoprocessingThread( self.iface.mainWindow(), self, self.myFunction, myLayerA,
    myLayerB, myParam, myMerge, mySelectionA, mySelectionB, mySegments, self.shapefileName, self.encoding )
    QObject.connect( self.testThread, SIGNAL( "runFinished(PyQt_PyObject)" ), self.runFinishedFromThread )
    QObject.connect( self.testThread, SIGNAL( "runStatus(PyQt_PyObject)" ), self.runStatusFromThread )
    QObject.connect( self.testThread, SIGNAL( "runRange(PyQt_PyObject)" ), self.runRangeFromThread )
    self.cancel_close.setText( self.tr("Cancel") )
    QObject.connect( self.cancel_close, SIGNAL( "clicked()" ), self.cancelThread )
    self.testThread.start()
    return True

  def cancelThread( self ):
    self.testThread.stop()
    self.buttonOk.setEnabled( True )

  def runFinishedFromThread( self, results ):
    self.testThread.stop()
    self.buttonOk.setEnabled( True )
    self.cancel_close.setText( self.tr("Close") )
    QObject.disconnect( self.cancel_close, SIGNAL( "clicked()" ), self.cancelThread )
    out_text = ""
    if results[3] is not None:
      QMessageBox.warning( self, self.tr( "Geoprocessing" ),
                  self.tr( "No output created. File creation error:\n%s" ) % ( results[3] ) )
      return
    if (not results[2] is None and not results[2]) or not results[1] or not results [0]:
      out_text = self.tr( "\nWarnings:" )
      end_text = self.tr( "\nSome output geometries may be missing or invalid.\n\nWould you like to add the new layer anyway?" )
    else:
      out_text = ""
      end_text = ""
    if not results[2] is None:
      if not results[2]:
        out_text = out_text + self.tr( "\nInput CRS error: Different input coordinate reference systems detected, results may not be as expected.")
    else:
      out_text = out_text + self.tr( "\nInput CRS error: One or more input layers missing coordinate reference information, results may not be as expected.")
    if not results[1]:
      out_text = out_text + self.tr( "\nFeature geometry error: One or more output features ignored due to invalid geometry.")
    if not results[0]:
      out_text = out_text + self.tr( "\nGEOS geoprocessing error: One or more input features have invalid geometry.")
    if self.addToCanvasCheck.isChecked():
      addCanvasCheck = ftools_utils.addShapeToCanvas(unicode(self.shapefileName))
      if not addCanvasCheck:
        QMessageBox.warning( self, self.tr("Geoprocessing"), self.tr( "Error loading output shapefile:\n%s" ) % ( unicode( self.shapefileName ) ))
      self.populateLayers()
    else:
      QMessageBox.information(self, self.tr("Geoprocessing"),self.tr("Created output shapefile:\n%s\n%s%s" ) % ( unicode( self.shapefileName ), out_text, end_text ))

  def runStatusFromThread( self, status ):
    self.progressBar.setValue( status )

  def runRangeFromThread( self, range_vals ):
    self.progressBar.setRange( range_vals[ 0 ], range_vals[ 1 ] )

class geoprocessingThread( QThread ):
  def __init__( self, parentThread, parentObject, function, myLayerA, myLayerB,
  myParam, myMerge, mySelectionA, mySelectionB, mySegments, myName, myEncoding ):
    QThread.__init__( self, parentThread )
    self.parent = parentObject
    self.running = False
    self.myFunction = function
    self.myLayerA = myLayerA
    self.myLayerB = myLayerB
    self.myParam = myParam
    self.myMerge = myMerge
    self.mySelectionA = mySelectionA
    self.mySelectionB = mySelectionB
    self.mySegments = int( mySegments )
    self.myName = myName
    self.myEncoding = myEncoding

  def run( self ):
    self.running = True
    self.vlayerA = ftools_utils.getVectorLayerByName( self.myLayerA )
    error = None
    if self.myFunction == 1 or self.myFunction == 2 or self.myFunction == 4:
      ( self.myParam, useField ) = self.checkParameter( self.vlayerA, self.myParam )
      if not self.myParam is None:
        if self.myFunction == 1:
          geos, feature, match, error = self.buffering( useField )
        elif self.myFunction == 2:
          geos, feature, match, error = self.convex_hull( useField )
        elif self.myFunction == 4:
          geos, feature, match, error = self.dissolve( useField )
    else:
      self.vlayerB = ftools_utils.getVectorLayerByName( self.myLayerB )
      if self.myFunction == 3:
        geos, feature, match, error = self.difference()
      elif self.myFunction == 5:
        geos, feature, match, error = self.intersect()
      elif self.myFunction == 6:
        geos, feature, match, error = self.union()
      elif self.myFunction == 7:
        geos, feature, match, error = self.symetrical_difference()
      elif self.myFunction == 8:
        geos, feature, match, error = self.clip()
    self.emit( SIGNAL( "runFinished(PyQt_PyObject)" ), (geos, feature, match, error) )
    self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0 )

  def stop(self):
    self.running = False

  def buffering( self, useField ):
    GEOS_EXCEPT = True
    FEATURE_EXCEPT = True
    vproviderA = self.vlayerA.dataProvider()

    writer = QgsVectorFileWriter( self.myName, self.myEncoding, vproviderA.fields(),
                                  QGis.WKBPolygon, vproviderA.crs() )
    # check if writer was created properly, if not, return with error
    if writer.hasError():
      return GEOS_EXCEPT, FEATURE_EXCEPT, True, writer.errorMessage()

    outFeat = QgsFeature()
    inFeat = QgsFeature()
    inGeom = QgsGeometry()
    outGeom = QgsGeometry()
    nElement = 0

    # there is selection in input layer
    if self.mySelectionA:
      nFeat = self.vlayerA.selectedFeatureCount()
      selectionA = self.vlayerA.selectedFeatures()
      self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0 )
      self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
      # with dissolve
      if self.myMerge:
        first = True
        for inFeat in selectionA:
          atMap = inFeat.attributes()
          if useField:
            value = atMap[ self.myParam ].doDouble()[ 0 ]
          else:
            value = self.myParam
          inGeom = QgsGeometry( inFeat.geometry() )
          try:
            outGeom = inGeom.buffer( float( value ), self.mySegments )
            if first:
              tempGeom = QgsGeometry( outGeom )
              first = False
            else:
              try:
                tempGeom = tempGeom.combine( outGeom )
              except:
                GEOS_EXCEPT = False
                continue
          except:
            GEOS_EXCEPT = False
            continue
          nElement += 1
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
        try:
          outFeat.setGeometry( tempGeom )
          writer.addFeature( outFeat )
        except:
          FEATURE_EXCEPT = False
      # without dissolve
      else:
        for inFeat in selectionA:
          atMap = inFeat.attributes()
          if useField:
            value = atMap[ self.myParam ]
          else:
            value = self.myParam
          inGeom = QgsGeometry( inFeat.geometry() )
          try:
            outGeom = inGeom.buffer( float( value ), self.mySegments )
            if not outGeom.isGeosEmpty():
              try:
                outFeat.setGeometry( outGeom )
                outFeat.setAttributes( atMap )
                writer.addFeature( outFeat )
              except:
                FEATURE_EXCEPT = False
                continue
          except:
            GEOS_EXCEPT = False
            continue
          nElement += 1
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
    # there is no selection in input layer
    else:
      nFeat = vproviderA.featureCount()
      self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0 )
      self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
      # with dissolve
      if self.myMerge:
        first = True
        fit = vproviderA.getFeatures()
        while fit.nextFeature( inFeat ):
          atMap = inFeat.attributes()
          if useField:
            value = atMap[ self.myParam ]
          else:
            value = self.myParam
          inGeom = QgsGeometry( inFeat.geometry() )
          try:
            outGeom = inGeom.buffer( float( value ), self.mySegments )
            if first:
              tempGeom = QgsGeometry( outGeom )
              first = False
            else:
              try:
                tempGeom = tempGeom.combine( outGeom )
              except:
                GEOS_EXCEPT = False
                continue
          except:
            GEOS_EXCEPT = False
            continue
          nElement += 1
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
        try:
          outFeat.setGeometry( tempGeom )
          writer.addFeature( outFeat )
        except:
          FEATURE_EXCEPT = False
      # without dissolve
      else:
        fit = vproviderA.getFeatures()
        while fit.nextFeature( inFeat ):
          atMap = inFeat.attributes()
          if useField:
            value = atMap[ self.myParam ]
          else:
            value = self.myParam
          inGeom = QgsGeometry( inFeat.geometry() )
          try:
            outGeom = inGeom.buffer( float( value ), self.mySegments )
            if not outGeom.isGeosEmpty():
              try:
                outFeat.setGeometry( outGeom )
                outFeat.setAttributes( atMap )
                writer.addFeature( outFeat )
              except:
                FEATURE_EXCEPT = False
                continue
          except:
            GEOS_EXCEPT = False
            continue
          nElement += 1
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
    del writer
    return GEOS_EXCEPT, FEATURE_EXCEPT, True, None

  def convex_hull(self, useField ):
    GEOS_EXCEPT = True
    FEATURE_EXCEPT = True
    vproviderA = self.vlayerA.dataProvider()
    #
    outFeatFields = QgsFields()
    if useField:
      importedField = vproviderA.fields().at( self.myParam )
      importedFieldName = importedField.name( )
    #
    outFeatFields.extend( vproviderA.fields() )
    # creating area and perimeter fields
    areaField = QgsField("area", QVariant.Double)
    perimField = QgsField("perim", QVariant.Double)
    # appending fields
    outFeatFields.append(areaField)
    outFeatFields.append(perimField)
    #
    writer = QgsVectorFileWriter( self.myName, self.myEncoding, outFeatFields,
                                  QGis.WKBPolygon, vproviderA.crs() )
    if writer.hasError():
      return GEOS_EXCEPT, FEATURE_EXCEPT, True, writer.errorMessage()

    inFeat = QgsFeature()
    outFeat = QgsFeature()
    # set feature fields
    outFeat.setFields(outFeatFields)
    #
    inGeom = QgsGeometry()
    outGeom = QgsGeometry()
    nElement = 0

    # there is selection in input layer
    if self.mySelectionA:
      nFeat = self.vlayerA.selectedFeatureCount()
      selectionA = self.vlayerA.selectedFeatures()
      if useField:
        unique = ftools_utils.getUniqueValues( vproviderA, self.myParam )
        nFeat = nFeat * len( unique )
        self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0 )
        self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
        for i in unique:
          first = True
          hull = []
          for inFeat in selectionA:
            atMap = inFeat.attributes()
            idVar = atMap[ self.myParam ]
            if idVar == i:
              if first:
                firstFeature = QgsFeature( inFeat )
                first = False
              inGeom = QgsGeometry( inFeat.geometry() )
              points = ftools_utils.extractPoints( inGeom )
              hull.extend( points )
            nElement += 1
            self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
          if len( hull ) >= 3:
            tmpGeom = QgsGeometry( outGeom.fromMultiPoint( hull ) )
            try:
              outGeom = tmpGeom.convexHull()
              outFeat.setGeometry( outGeom )
              (area, perim) = self.simpleMeasure( outGeom )
              for f in firstFeature.fields():
                outFeat.setAttribute( f.name( ), firstFeature.attribute( f.name( ) ) )
              outFeat.setAttribute( "area", area )
              outFeat.setAttribute( "perim", perim )
              writer.addFeature( outFeat )
            except:
              GEOS_EXCEPT = False
              continue
      else:
        self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0 )
        self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
        hull = []
        for inFeat in selectionA:
          inGeom = QgsGeometry( inFeat.geometry() )
          points = ftools_utils.extractPoints( inGeom )
          hull.extend( points )
          nElement += 1
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
        tmpGeom = QgsGeometry( outGeom.fromMultiPoint( hull ) )
        try:
          outGeom = tmpGeom.convexHull()
          outFeat.setGeometry( outGeom )
          (area, perim) = self.simpleMeasure( outGeom )
          for f in inFeat.fields():
            outFeat.setAttribute( f.name( ), inFeat.attribute( f.name( ) ) )
          outFeat.setAttribute( "area", area )
          outFeat.setAttribute( "perim", perim )
          writer.addFeature( outFeat )
        except:
          GEOS_EXCEPT = False
    # there is no selection in input layer
    else:
      nFeat = vproviderA.featureCount()
      if useField:
        unique = ftools_utils.getUniqueValues( vproviderA, self.myParam )
        nFeat = nFeat * len( unique )
        self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0 )
        self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )

        for i in unique:
          first = True
          hull = []
          fitA = vproviderA.getFeatures()
          while fitA.nextFeature( inFeat ):
            idVar = inFeat.attribute( importedFieldName ) 
            if idVar == i:
              if first:
                firstFeature = QgsFeature( inFeat )
                first = False
              inGeom = QgsGeometry( inFeat.geometry() )
              points = ftools_utils.extractPoints( inGeom )
              hull.extend( points )
            nElement += 1
            self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
          if len( hull ) >= 3:
            tmpGeom = QgsGeometry( outGeom.fromMultiPoint( hull ) )
            try:
              outGeom = tmpGeom.convexHull()
              outFeat.setGeometry( outGeom )
              (area, perim) = self.simpleMeasure( outGeom )
              for f in firstFeature.fields():
                outFeat.setAttribute( f.name( ), firstFeature.attribute( f.name( ) ) )
              outFeat.setAttribute( "area", area )
              outFeat.setAttribute( "perim", perim )
              writer.addFeature( outFeat )
            except:
              GEOS_EXCEPT = False
              continue
      else:
        self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0 )
        self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
        hull = []
        fitA = vproviderA.getFeatures()
        while fitA.nextFeature( inFeat ):
          inGeom = QgsGeometry( inFeat.geometry() )
          points = ftools_utils.extractPoints( inGeom )
          hull.extend( points )
          nElement += 1
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
        tmpGeom = QgsGeometry( outGeom.fromMultiPoint( hull ) )
        try:
          outGeom = tmpGeom.convexHull()
          outFeat.setGeometry( outGeom )
          (area, perim) = self.simpleMeasure( outGeom )
          for f in inFeat.fields():
            outFeat.setAttribute( f.name( ), inFeat.attribute( f.name( ) ) )
          outFeat.setAttribute( "area", area )
          outFeat.setAttribute( "perim", perim )
          writer.addFeature( outFeat )
        except:
          GEOS_EXCEPT = False
    del writer
    return GEOS_EXCEPT, FEATURE_EXCEPT, True, None

  def dissolve( self, useField ):
    GEOS_EXCEPT = True
    FEATURE_EXCEPT = True
    vproviderA = self.vlayerA.dataProvider()

    writer = QgsVectorFileWriter( self.myName, self.myEncoding, vproviderA.fields(),
                                  vproviderA.geometryType(), vproviderA.crs() )
    if writer.hasError():
      return GEOS_EXCEPT, FEATURE_EXCEPT, True, writer.errorMessage()

    inFeat = QgsFeature()
    outFeat = QgsFeature()
    nElement = 0
    attrs = None

    # there is selection in input layer
    if self.mySelectionA:
      nFeat = self.vlayerA.selectedFeatureCount()
      selectionA = self.vlayerA.selectedFeatures()
      if not useField:
        self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0 )
        self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
        first = True
        for inFeat in selectionA:
          nElement += 1
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
          if first:
            attrs = inFeat.attributes()
            tmpInGeom = QgsGeometry( inFeat.geometry() )
            outFeat.setGeometry( tmpInGeom )
            first = False
          else:
            tmpInGeom = QgsGeometry( inFeat.geometry() )
            tmpOutGeom = QgsGeometry( outFeat.geometry() )
            try:
              tmpOutGeom = QgsGeometry( tmpOutGeom.combine( tmpInGeom ) )
              outFeat.setGeometry( tmpOutGeom )
            except:
              GEOS_EXCEPT = False
              continue
        outFeat.setAttributes( attrs )
        writer.addFeature( outFeat )
      else:
        self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0)
        self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )

        outFeats = {}
        attrs = {}

        for inFeat in selectionA:
          nElement += 1
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ),  nElement )
          atMap = inFeat.attributes()
          tempItem = unicode(atMap[self.myParam]).strip()

          if not (tempItem in outFeats):
            outFeats[tempItem] = QgsGeometry(inFeat.geometry())
            attrs[tempItem] = atMap
          else:
            try:
              outFeats[tempItem] = outFeats[tempItem].combine(inFeat.geometry())
            except:
              GEOS_EXCEPT = False
              continue
        for k in outFeats.keys():
          feature = QgsFeature()
          feature.setAttributes(attrs[k])
          feature.setGeometry(outFeats[k])
          writer.addFeature( feature )
    # there is no selection in input layer
    else:
      nFeat = vproviderA.featureCount()
      if not useField:
        self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0)
        self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
        first = True
        fitA = vproviderA.getFeatures()
        while fitA.nextFeature( inFeat ):
          nElement += 1
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ),  nElement )
          if first:
            attrs = inFeat.attributes()
            tmpInGeom = QgsGeometry( inFeat.geometry() )
            outFeat.setGeometry( tmpInGeom )
            first = False
          else:
            tmpInGeom = QgsGeometry( inFeat.geometry() )
            tmpOutGeom = QgsGeometry( outFeat.geometry() )
            try:
              tmpOutGeom = QgsGeometry( tmpOutGeom.combine( tmpInGeom ) )
              outFeat.setGeometry( tmpOutGeom )
            except:
              GEOS_EXCEPT = False
              continue
        outFeat.setAttributes( attrs )
        writer.addFeature( outFeat )
      else:
        self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0)
        self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )

        outFeats = {}
        attrs = {}

        fitA = vproviderA.getFeatures()
        while fitA.nextFeature( inFeat ):
          nElement += 1
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ),  nElement )
          atMap = inFeat.attributes()
          tempItem = unicode(atMap[self.myParam]).strip()

          if not (tempItem in outFeats):
            outFeats[tempItem] = QgsGeometry(inFeat.geometry())
            attrs[tempItem] = atMap
          else:
            try:
              outFeats[tempItem] = outFeats[tempItem].combine(inFeat.geometry())
            except:
              GEOS_EXCEPT = False
              continue
        for k in outFeats.keys():
          feature = QgsFeature()
          feature.setAttributes(attrs[k])
          feature.setGeometry(outFeats[k])
          writer.addFeature( feature )
    del writer
    return GEOS_EXCEPT, FEATURE_EXCEPT, True, None

  def difference( self ):
    GEOS_EXCEPT = True
    FEATURE_EXCEPT = True
    vproviderA = self.vlayerA.dataProvider()
    vproviderB = self.vlayerB.dataProvider()

    # check for crs compatibility
    crsA = vproviderA.crs()
    crsB = vproviderB.crs()
    if not crsA.isValid() or not crsB.isValid():
        crs_match = None
    else:
        crs_match = crsA == crsB

    writer = QgsVectorFileWriter( self.myName, self.myEncoding, vproviderA.fields(),
                                  vproviderA.geometryType(), vproviderA.crs() )
    if writer.hasError():
      return GEOS_EXCEPT, FEATURE_EXCEPT, crs_match, writer.errorMessage()

    inFeatA = QgsFeature()
    inFeatB = QgsFeature()
    outFeat = QgsFeature()
    nElement = 0

    index = ftools_utils.createIndex( vproviderB )

    # there is selection in input layer
    if self.mySelectionA:
      nFeat = self.vlayerA.selectedFeatureCount()
      selectionA = self.vlayerA.selectedFeatures()
      self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0)
      self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
      # we have selection in overlay layer
      if self.mySelectionB:
        selectionB = self.vlayerB.selectedFeaturesIds()
        for inFeatA in selectionA:
          nElement += 1
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
          add = True
          geom = QgsGeometry( inFeatA.geometry() )
          diff_geom = QgsGeometry( geom )
          atMap = inFeatA.attributes()
          intersects = index.intersects( geom.boundingBox() )
          for id in intersects:
            # is intersect feature in selection
            if id in selectionB:
              vproviderB.getFeatures( QgsFeatureRequest().setFilterFid( int( id ) ) ).nextFeature( inFeatB )
              tmpGeom = QgsGeometry( inFeatB.geometry() )
              try:
                if diff_geom.intersects( tmpGeom ):
                  diff_geom = QgsGeometry( diff_geom.difference( tmpGeom ) )
              except:
                GEOS_EXCEPT = False
                add = False
                break
          if add:
            try:
              outFeat.setGeometry( diff_geom )
              outFeat.setAttributes( atMap )
              writer.addFeature( outFeat )
            except:
              FEATURE_EXCEPT = False
              continue
      # we have no selection in overlay layer
      else:
        for inFeatA in selectionA:
          nElement += 1
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
          add = True
          geom = QgsGeometry( inFeatA.geometry() )
          diff_geom = QgsGeometry( geom )
          atMap = inFeatA.attributes()
          intersects = index.intersects( geom.boundingBox() )
          for id in intersects:
            vproviderB.getFeatures( QgsFeatureRequest().setFilterFid( int( id ) ) ).nextFeature( inFeatB )
            tmpGeom = QgsGeometry( inFeatB.geometry() )
            try:
              if diff_geom.intersects( tmpGeom ):
                diff_geom = QgsGeometry( diff_geom.difference( tmpGeom ) )
            except:
              GEOS_EXCEPT = False
              add = False
              break
          if add:
            try:
              outFeat.setGeometry( diff_geom )
              outFeat.setAttributes( atMap )
              writer.addFeature( outFeat )
            except:
              FEATURE_EXCEPT = False
              continue
    # there is no selection in input layer
    else:
      nFeat = vproviderA.featureCount()
      self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0 )
      self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
      # we have selection in overlay layer
      if self.mySelectionB:
        selectionB = self.vlayerB.selectedFeaturesIds()
        fitA = vproviderA.getFeatures()
        while fitA.nextFeature( inFeatA ):
          nElement += 1
          add = True
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
          geom = QgsGeometry( inFeatA.geometry() )
          diff_geom = QgsGeometry( geom )
          atMap = inFeatA.attributes()
          intersects = index.intersects( geom.boundingBox() )
          for id in intersects:
            # now check if id in selection
            if id in selectionB:
              vproviderB.getFeatures( QgsFeatureRequest().setFilterFid( int( id ) ) ).nextFeature( inFeatB )
              tmpGeom = QgsGeometry( inFeatB.geometry() )
              try:
                if diff_geom.intersects( tmpGeom ):
                  diff_geom = QgsGeometry( diff_geom.difference( tmpGeom ) )
              except:
                GEOS_EXCEPT = False
                add = False
                break
          if add:
            try:
              outFeat.setGeometry( diff_geom )
              outFeat.setAttributes( atMap )
              writer.addFeature( outFeat )
            except:
              FEATURE_EXCEPT = False
              continue
      # we have no selection in overlay layer
      else:
        fitA = vproviderA.getFeatures()
        while fitA.nextFeature( inFeatA ):
          nElement += 1
          add = True
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
          geom = QgsGeometry( inFeatA.geometry() )
          diff_geom = QgsGeometry( geom )
          atMap = inFeatA.attributes()
          intersects = index.intersects( geom.boundingBox() )
          for id in intersects:
            vproviderB.getFeatures( QgsFeatureRequest().setFilterFid( int( id ) ) ).nextFeature( inFeatB )
            tmpGeom = QgsGeometry( inFeatB.geometry() )
            try:
              if diff_geom.intersects( tmpGeom ):
                diff_geom = QgsGeometry( diff_geom.difference( tmpGeom ) )
            except:
              GEOS_EXCEPT = False
              add = False
              break
          if add:
            try:
              outFeat.setGeometry( diff_geom )
              outFeat.setAttributes( atMap )
              writer.addFeature( outFeat )
            except:
              FEATURE_EXCEPT = False
              continue
    del writer
    return GEOS_EXCEPT, FEATURE_EXCEPT, crs_match, None

  def intersect( self ):
    GEOS_EXCEPT = True
    FEATURE_EXCEPT = True
    vproviderA = self.vlayerA.dataProvider()
    vproviderB = self.vlayerB.dataProvider()

    # check for crs compatibility
    crsA = vproviderA.crs()
    crsB = vproviderB.crs()
    if not crsA.isValid() or not crsB.isValid():
        crs_match = None
    else:
        crs_match = crsA == crsB
    fields = ftools_utils.combineVectorFields( self.vlayerA, self.vlayerB )
    writer = QgsVectorFileWriter( self.myName, self.myEncoding, fields,
                                  vproviderA.geometryType(), vproviderA.crs() )
    if writer.hasError():
      return GEOS_EXCEPT, FEATURE_EXCEPT, crs_match, writer.errorMessage()

    inFeatA = QgsFeature()
    inFeatB = QgsFeature()
    outFeat = QgsFeature()
    nElement = 0

    index = ftools_utils.createIndex( vproviderB )

    # there is selection in input layer
    if self.mySelectionA:
      nFeat = self.vlayerA.selectedFeatureCount()
      selectionA = self.vlayerA.selectedFeatures()
      self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0)
      self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
      # we have selection in overlay layer
      if self.mySelectionB:
        selectionB = self.vlayerB.selectedFeaturesIds()
        for inFeatA in selectionA:
          nElement += 1
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
          geom = QgsGeometry( inFeatA.geometry() )
          atMapA = inFeatA.attributes()
          intersects = index.intersects( geom.boundingBox() )
          for id in intersects:
            if id in selectionB:
              vproviderB.getFeatures( QgsFeatureRequest().setFilterFid( int( id ) ) ).nextFeature( inFeatB )
              tmpGeom = QgsGeometry( inFeatB.geometry() )
              try:
                if geom.intersects( tmpGeom ):
                  atMapB = inFeatB.attributes()
                  int_geom = QgsGeometry( geom.intersection( tmpGeom ) )
                  if int_geom.wkbType() == 0:
                    int_com = geom.combine( tmpGeom )
                    int_sym = geom.symDifference( tmpGeom )
                    int_geom = QgsGeometry( int_com.difference( int_sym ) )
                  try:
                    # Geometry list: prevents writing error
                    # in geometries of different types
                    # produced by the intersection
                    # fix #3549
                    gList = ftools_utils.getGeomType( geom.wkbType() )
                    if int_geom.wkbType() in gList:
                      outFeat.setGeometry( int_geom )
                      outFeat.setAttributes( atMapA + atMapB )
                      writer.addFeature( outFeat )
                  except:
                    FEATURE_EXCEPT = False
                    continue
              except:
                GEOS_EXCEPT = False
                break
      # we don't have selection in overlay layer
      else:
        for inFeatA in selectionA:
          nElement += 1
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
          geom = QgsGeometry( inFeatA.geometry() )
          atMapA = inFeatA.attributes()
          intersects = index.intersects( geom.boundingBox() )
          for id in intersects:
            vproviderB.getFeatures( QgsFeatureRequest().setFilterFid( int( id ) ) ).nextFeature( inFeatB )
            tmpGeom = QgsGeometry( inFeatB.geometry() )
            try:
              if geom.intersects( tmpGeom ):
                atMapB = inFeatB.attributes()
                int_geom = QgsGeometry( geom.intersection( tmpGeom ) )
                if int_geom.wkbType() == 0:
                  int_com = geom.combine( tmpGeom )
                  int_sym = geom.symDifference( tmpGeom )
                  int_geom = QgsGeometry( int_com.difference( int_sym ) )
                try:
                  gList = ftools_utils.getGeomType( geom.wkbType() )
                  if int_geom.wkbType() in gList:
                    outFeat.setGeometry( int_geom )
                    outFeat.setAttributes( atMapA + atMapB )
                    writer.addFeature( outFeat )
                except:
                  EATURE_EXCEPT = False
                  continue
            except:
              GEOS_EXCEPT = False
              break
    # there is no selection in input layer
    else:
      nFeat = vproviderA.featureCount()
      self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0)
      self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
      # we have selection in overlay layer
      if self.mySelectionB:
        selectionB = self.vlayerB.selectedFeaturesIds()
        fitA = vproviderA.getFeatures()
        while fitA.nextFeature( inFeatA ):
          nElement += 1
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
          geom = QgsGeometry( inFeatA.geometry() )
          atMapA = inFeatA.attributes()
          intersects = index.intersects( geom.boundingBox() )
          for id in intersects:
            if id in selectionB:
              vproviderB.getFeatures( QgsFeatureRequest().setFilterFid( int( id ) ) ).nextFeature( inFeatB )
              tmpGeom = QgsGeometry( inFeatB.geometry() )
              try:
                if geom.intersects( tmpGeom ):
                  atMapB = inFeatB.attributes()
                  int_geom = QgsGeometry( geom.intersection( tmpGeom ) )
                  if int_geom.wkbType() == 0:
                    int_com = geom.combine( tmpGeom )
                    int_sym = geom.symDifference( tmpGeom )
                    int_geom = QgsGeometry( int_com.difference( int_sym ) )
                  try:
                    gList = ftools_utils.getGeomType( geom.wkbType() )
                    if int_geom.wkbType() in gList:
                      outFeat.setGeometry( int_geom )
                      outFeat.setAttributes( atMapA + atMapB )
                      writer.addFeature( outFeat )
                  except:
                    FEATURE_EXCEPT = False
                    continue
              except:
                GEOS_EXCEPT = False
                break
      # we have no selection in overlay layer
      else:
        fitA = vproviderA.getFeatures()
        while fitA.nextFeature( inFeatA ):
          nElement += 1
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
          geom = QgsGeometry( inFeatA.geometry() )
          atMapA = inFeatA.attributes()
          intersects = index.intersects( geom.boundingBox() )
          for id in intersects:
            vproviderB.getFeatures( QgsFeatureRequest().setFilterFid( int( id ) ) ).nextFeature( inFeatB )
            tmpGeom = QgsGeometry( inFeatB.geometry() )
            try:
              if geom.intersects( tmpGeom ):
                atMapB = inFeatB.attributes()
                int_geom = QgsGeometry( geom.intersection( tmpGeom ) )
                if int_geom.wkbType() == 0:
                  int_com = geom.combine( tmpGeom )
                  int_sym = geom.symDifference( tmpGeom )
                  int_geom = QgsGeometry( int_com.difference( int_sym ) )

                try:
                  gList = ftools_utils.getGeomType( geom.wkbType() )
                  if int_geom.wkbType() in gList:
                    outFeat.setGeometry( int_geom )
                    outFeat.setAttributes( atMapA + atMapB )
                    writer.addFeature( outFeat )
                except:
                  FEATURE_EXCEPT = False
                  continue
            except:
              GEOS_EXCEPT = False
              break
    del writer
    return GEOS_EXCEPT, FEATURE_EXCEPT, crs_match, None

  def union( self ):
    GEOS_EXCEPT = True
    FEATURE_EXCEPT = True
    vproviderA = self.vlayerA.dataProvider()
    vproviderB = self.vlayerB.dataProvider()

    # check for crs compatibility
    crsA = vproviderA.crs()
    crsB = vproviderB.crs()
    if not crsA.isValid() or not crsB.isValid():
        crs_match = None
    else:
        crs_match = crsA == crsB

    fields = ftools_utils.combineVectorFields( self.vlayerA, self.vlayerB )
    writer = QgsVectorFileWriter( self.myName, self.myEncoding, fields,
                                  vproviderA.geometryType(), vproviderA.crs() )
    if writer.hasError():
      return GEOS_EXCEPT, FEATURE_EXCEPT, crs_match, writer.errorMessage()

    inFeatA = QgsFeature()
    inFeatB = QgsFeature()
    outFeat = QgsFeature()
    indexA = ftools_utils.createIndex( vproviderB )
    indexB = ftools_utils.createIndex( vproviderA )

    nFeat = vproviderA.featureCount() + vproviderB.featureCount()
    self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0)
    self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )

    count = 0
    nElement = 0

    fitA = vproviderA.getFeatures()
    while fitA.nextFeature( inFeatA ):
      self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
      nElement += 1
      found = False
      geom = QgsGeometry( inFeatA.geometry() )
      diff_geom = QgsGeometry( geom )
      atMapA = inFeatA.attributes()
      intersects = indexA.intersects( geom.boundingBox() )
      if len( intersects ) < 1:
        try:
          outFeat.setGeometry( geom )
          outFeat.setAttributes( atMapA )
          writer.addFeature( outFeat )
        except:
          # this really shouldn't happen, as we
          # haven't edited the input geom at all
          FEATURE_EXCEPT = False
      else:
        for id in intersects:
          count += 1
          vproviderB.getFeatures( QgsFeatureRequest().setFilterFid( int( id ) ) ).nextFeature( inFeatB )
          atMapB = inFeatB.attributes()
          tmpGeom = QgsGeometry( inFeatB.geometry() )
          try:
            if geom.intersects( tmpGeom ):
              found = True
              int_geom = geom.intersection( tmpGeom )

              if int_geom is None:
                # There was a problem creating the intersection
                GEOS_EXCEPT = False
                int_geom = QgsGeometry()
              else:
                int_geom = QgsGeometry(int_geom)

              if diff_geom.intersects( tmpGeom ):
                diff_geom = diff_geom.difference( tmpGeom )
                if diff_geom is None:
                  # It's possible there was an error here?
                  diff_geom = QgsGeometry()
                else:
                  diff_geom = QgsGeometry(diff_geom)

              if int_geom.wkbType() == 0:
                # intersection produced different geometry types
                temp_list = int_geom.asGeometryCollection()
                for i in temp_list:
                  if i.type() == geom.type():
                    int_geom = QgsGeometry( i )
                    try:
                      outFeat.setGeometry( int_geom )
                      outFeat.setAttributes( atMapA + atMapB )
                      writer.addFeature( outFeat )
                    except Exception, err:
                      FEATURE_EXCEPT = False
              else:
                # Geometry list: prevents writing error
                # in geometries of different types
                # produced by the intersection
                # fix #3549
                gList = ftools_utils.getGeomType( geom.wkbType() )
                if int_geom.wkbType() in gList:
                  try:
                    outFeat.setGeometry( int_geom )
                    outFeat.setAttributes( atMapA + atMapB )
                    writer.addFeature( outFeat )
                  except Exception, err:
                    FEATURE_EXCEPT = False
            else:
              # this only happends if the bounding box
              # intersects, but the geometry doesn't
              try:
                outFeat.setGeometry( geom )
                outFeat.setAttributes( atMapA )
                writer.addFeature( outFeat )
              except:
                # also shoudn't ever happen
                FEATURE_EXCEPT = False
          except Exception, err:
            GEOS_EXCEPT = False
            found = False

        if found:
          try:
            if diff_geom.wkbType() == 0:
              temp_list = diff_geom.asGeometryCollection()
              for i in temp_list:
                if i.type() == geom.type():
                    diff_geom = QgsGeometry( i )
            outFeat.setGeometry( diff_geom )
            outFeat.setAttributes( atMapA )
            writer.addFeature( outFeat )
          except Exception, err:
            FEATURE_EXCEPT = False

    length = len( vproviderA.fields() )
    atMapA = [None] * length

    fitB = vproviderB.getFeatures()
    while fitB.nextFeature( inFeatB ):
      add = False
      geom = QgsGeometry( inFeatB.geometry() )
      diff_geom = QgsGeometry( geom )
      atMap = atMapA + inFeatB.attributes()
      intersects = indexB.intersects( geom.boundingBox() )

      if len(intersects) < 1:
        try:
          outFeat.setGeometry( geom )
          outFeat.setAttributes( atMap )
          writer.addFeature( outFeat )
        except Exception, err:
          FEATURE_EXCEPT = False
      else:
        for id in intersects:
          vproviderA.getFeatures( QgsFeatureRequest().setFilterFid( int( id ) ) ).nextFeature( inFeatA )
          tmpGeom = QgsGeometry( inFeatA.geometry() )

          try:
            if diff_geom.intersects( tmpGeom ):
              add = True
              diff_geom = QgsGeometry( diff_geom.difference( tmpGeom ) )
            else:
              # this only happends if the bounding box
              # intersects, but the geometry doesn't
              outFeat.setGeometry( diff_geom )
              outFeat.setAttributes( atMap )
              writer.addFeature( outFeat )
          except Exception, err:
            add = False
            GEOS_EXCEPT = False

        if add:
          try:
            outFeat.setGeometry( diff_geom )
            outFeat.setAttributes( atMap )
            writer.addFeature( outFeat )
          except Exception, err:
            FEATURE_EXCEPT = False

      self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ),  nElement )
      nElement += 1
    del writer
    return GEOS_EXCEPT, FEATURE_EXCEPT, crs_match, None

  def symetrical_difference( self ):
    GEOS_EXCEPT = True
    FEATURE_EXCEPT = True
    vproviderA = self.vlayerA.dataProvider()
    vproviderB = self.vlayerB.dataProvider()

    # check for crs compatibility
    crsA = vproviderA.crs()
    crsB = vproviderB.crs()
    if not crsA.isValid() or not crsB.isValid():
        crs_match = None
    else:
        crs_match = crsA == crsB

    fields = ftools_utils.combineVectorFields( self.vlayerA, self.vlayerB )
    writer = QgsVectorFileWriter( self.myName, self.myEncoding, fields,
                                  vproviderA.geometryType(), vproviderA.crs() )
    if writer.hasError():
      return GEOS_EXCEPT, FEATURE_EXCEPT, crs_match, writer.errorMessage()

    inFeatA = QgsFeature()
    inFeatB = QgsFeature()
    outFeat = QgsFeature()

    indexA = ftools_utils.createIndex( vproviderB )
    indexB = ftools_utils.createIndex( vproviderA )

    nFeat = vproviderA.featureCount() * vproviderB.featureCount()
    nElement = 0
    self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0)
    self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
    fitA = vproviderA.getFeatures()
    while fitA.nextFeature( inFeatA ):
      nElement += 1
      self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
      add = True
      geom = QgsGeometry( inFeatA.geometry() )
      diff_geom = QgsGeometry( geom )
      atMapA = inFeatA.attributes()
      intersects = indexA.intersects( geom.boundingBox() )
      for id in intersects:
        vproviderB.getFeatures( QgsFeatureRequest().setFilterFid( int( id ) ) ).nextFeature( inFeatB )
        tmpGeom = QgsGeometry( inFeatB.geometry() )
        try:
          if diff_geom.intersects( tmpGeom ):
            diff_geom = QgsGeometry( diff_geom.difference( tmpGeom ) )
        except:
          add = False
          GEOS_EXCEPT = False
          break
      if add:
        try:
          outFeat.setGeometry( diff_geom )
          outFeat.setAttributes( atMapA )
          writer.addFeature( outFeat )
        except:
          FEATURE_EXCEPT = False
          continue

    length = len( vproviderA.fields() )

    fitB = vproviderB.getFeatures()
    while fitB.nextFeature( inFeatA ):
      nElement += 1
      self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
      add = True
      geom = QgsGeometry( inFeatA.geometry() )
      diff_geom = QgsGeometry( geom )
      atMap = inFeatA.attributes()
      atMap = dict( zip( range( length, length + len( atMap ) ), atMap ) )
      intersects = indexB.intersects( geom.boundingBox() )
      for id in intersects:
        vproviderA.getFeatures( QgsFeatureRequest().setFilterFid( int( id ) ) ).nextFeature( inFeatB )
        tmpGeom = QgsGeometry( inFeatB.geometry() )
        try:
          if diff_geom.intersects( tmpGeom ):
            diff_geom = QgsGeometry( diff_geom.difference( tmpGeom ) )
        except:
          add = False
          GEOS_EXCEPT = False
          break
      if add:
        try:
          outFeat.setGeometry( diff_geom )
          outFeat.setAttributes( atMap )
          writer.addFeature( outFeat )
        except:
          FEATURE_EXCEPT = False
          continue
    del writer
    return GEOS_EXCEPT, FEATURE_EXCEPT, crs_match, None

  def clip( self ):
    GEOS_EXCEPT = True
    FEATURE_EXCEPT = True
    vproviderA = self.vlayerA.dataProvider()
    vproviderB = self.vlayerB.dataProvider()

    # check for crs compatibility
    crsA = vproviderA.crs()
    crsB = vproviderB.crs()
    if not crsA.isValid() or not crsB.isValid():
        crs_match = None
    else:
        crs_match = crsA == crsB

    writer = QgsVectorFileWriter( self.myName, self.myEncoding, vproviderA.fields(),
                                  vproviderA.geometryType(), vproviderA.crs() )
    if writer.hasError():
      return GEOS_EXCEPT, FEATURE_EXCEPT, crs_match, writer.errorMessage()

    inFeatA = QgsFeature()
    inFeatB = QgsFeature()
    outFeat = QgsFeature()

    index = ftools_utils.createIndex( vproviderB )

    nElement = 0

    # there is selection in input layer
    if self.mySelectionA:
      nFeat = self.vlayerA.selectedFeatureCount()
      selectionA = self.vlayerA.selectedFeatures()
      self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0)
      self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
      # we have selection in overlay layer
      if self.mySelectionB:
        selectionB = self.vlayerB.selectedFeaturesIds()
        for inFeatA in selectionA:
          nElement += 1
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
          geom = QgsGeometry( inFeatA.geometry() )
          int_geom = QgsGeometry( geom )
          atMap = inFeatA.attributes()
          intersects = index.intersects( geom.boundingBox() )
          found = False
          first = True
          for id in intersects:
            if id in selectionB:
              vproviderB.getFeatures( QgsFeatureRequest().setFilterFid( int( id ) ) ).nextFeature( inFeatB )
              tmpGeom = QgsGeometry( inFeatB.geometry() )
              if tmpGeom.intersects( geom ):
                found = True
                if first:
                  outFeat.setGeometry( QgsGeometry( tmpGeom ) )
                  first = False
                else:
                  try:
                    cur_geom = QgsGeometry( outFeat.geometry() )
                    new_geom = QgsGeometry( cur_geom.combine( tmpGeom ) )
                    outFeat.setGeometry( QgsGeometry( new_geom ) )
                  except:
                    GEOS_EXCEPT = False
                    break
          if found:
            try:
              cur_geom = QgsGeometry( outFeat.geometry() )
              new_geom = QgsGeometry( geom.intersection( cur_geom ) )
              if new_geom.wkbType() == 0:
                int_com = QgsGeometry( geom.combine( cur_geom ) )
                int_sym = QgsGeometry( geom.symDifference( cur_geom ) )
                new_geom = QgsGeometry( int_com.difference( int_sym ) )
              try:
                outFeat.setGeometry( new_geom )
                outFeat.setAttributes( atMap )
                writer.addFeature( outFeat )
              except:
                FEAT_EXCEPT = False
                continue
            except:
              GEOS_EXCEPT = False
              continue
      # we have no selection in overlay layer
      else:
        for inFeatA in selectionA:
          nElement += 1
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
          geom = QgsGeometry( inFeatA.geometry() )
          atMap = inFeatA.attributes()
          intersects = index.intersects( geom.boundingBox() )
          found = False
          first = True
          for id in intersects:
            vproviderB.getFeatures( QgsFeatureRequest().setFilterFid( int( id ) ) ).nextFeature( inFeatB )
            tmpGeom = QgsGeometry( inFeatB.geometry() )
            if tmpGeom.intersects( geom ):
              found = True
              if first:
                outFeat.setGeometry( QgsGeometry( tmpGeom ) )
                first = False
              else:
                try:
                  cur_geom = QgsGeometry( outFeat.geometry() )
                  new_geom = QgsGeometry( cur_geom.combine( tmpGeom ) )
                  outFeat.setGeometry( QgsGeometry( new_geom ) )
                except:
                  GEOS_EXCEPT = False
                  break
          if found:
            try:
              cur_geom = QgsGeometry( outFeat.geometry() )
              new_geom = QgsGeometry( geom.intersection( cur_geom ) )
              if new_geom.wkbType() == 0:
                int_com = QgsGeometry( geom.combine( cur_geom ) )
                int_sym = QgsGeometry( geom.symDifference( cur_geom ) )
                new_geom = QgsGeometry( int_com.difference( int_sym ) )
              try:
                outFeat.setGeometry( new_geom )
                outFeat.setAttributes( atMap )
                writer.addFeature( outFeat )
              except:
                FEAT_EXCEPT = False
                continue
            except:
              GEOS_EXCEPT = False
              continue
    # there is no selection in input layer
    else:
      nFeat = vproviderA.featureCount()
      self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0)
      self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
      # we have selection in overlay layer
      if self.mySelectionB:
        selectionB = self.vlayerB.selectedFeaturesIds()
        fitA = vproviderA.getFeatures()
        while fitA.nextFeature( inFeatA ):
          nElement += 1
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
          geom = QgsGeometry( inFeatA.geometry() )
          atMap = inFeatA.attributes()
          intersects = index.intersects( geom.boundingBox() )
          found = False
          first = True
          for id in intersects:
            if id in selectionB:
              vproviderB.getFeatures( QgsFeatureRequest().setFilterFid( int( id ) ) ).nextFeature( inFeatB )
              tmpGeom = QgsGeometry( inFeatB.geometry() )
              if tmpGeom.intersects( geom ):
                found = True
                if first:
                  outFeat.setGeometry( QgsGeometry( tmpGeom ) )
                  first = False
                else:
                  try:
                    cur_geom = QgsGeometry( outFeat.geometry() )
                    new_geom = QgsGeometry( cur_geom.combine( tmpGeom ) )
                    outFeat.setGeometry( QgsGeometry( new_geom ) )
                  except:
                    GEOS_EXCEPT = False
                    break
          if found:
            try:
              cur_geom = QgsGeometry( outFeat.geometry() )
              new_geom = QgsGeometry( geom.intersection( cur_geom ) )
              if new_geom.wkbType() == 0:
                int_com = QgsGeometry( geom.combine( cur_geom ) )
                int_sym = QgsGeometry( geom.symDifference( cur_geom ) )
                new_geom = QgsGeometry( int_com.difference( int_sym ) )
              try:
                outFeat.setGeometry( new_geom )
                outFeat.setAttributes( atMap )
                writer.addFeature( outFeat )
              except:
                FEAT_EXCEPT = False
                continue
            except:
              GEOS_EXCEPT = False
              continue
      # we have no selection in overlay layer
      else:
        fitA = vproviderA.getFeatures()
        while fitA.nextFeature( inFeatA ):
          nElement += 1
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
          geom = QgsGeometry( inFeatA.geometry() )
          atMap = inFeatA.attributes()
          intersects = index.intersects( geom.boundingBox() )
          first = True
          found = False
          if len( intersects ) > 0:
            for id in intersects:
              vproviderB.getFeatures( QgsFeatureRequest().setFilterFid( int( id ) ) ).nextFeature( inFeatB )
              tmpGeom = QgsGeometry( inFeatB.geometry() )
              if tmpGeom.intersects( geom ):
                found = True
                if first:
                  outFeat.setGeometry( QgsGeometry( tmpGeom ) )
                  first = False
                else:
                  try:
                    cur_geom = QgsGeometry( outFeat.geometry() )
                    new_geom = QgsGeometry( cur_geom.combine( tmpGeom ) )
                    outFeat.setGeometry( QgsGeometry( new_geom ) )
                  except:
                    GEOS_EXCEPT = False
                    break
            if found:
              try:
                cur_geom = QgsGeometry( outFeat.geometry() )
                new_geom = QgsGeometry( geom.intersection( cur_geom ) )
                if new_geom.wkbType() == 0:
                  int_com = QgsGeometry( geom.combine( cur_geom ) )
                  int_sym = QgsGeometry( geom.symDifference( cur_geom ) )
                  new_geom = QgsGeometry( int_com.difference( int_sym ) )
                try:
                  outFeat.setGeometry( new_geom )
                  outFeat.setAttributes( atMap )
                  writer.addFeature( outFeat )
                except:
                  FEAT_EXCEPT = False
                  continue
              except:
                GEOS_EXCEPT = False
                continue
    del writer
    return GEOS_EXCEPT, FEATURE_EXCEPT, crs_match, None

  def checkParameter( self, layer, param ):
    if self.myFunction == 1:
      if type( param ) == unicode:
        check = layer.dataProvider().fieldNameIndex( param )
        if check == -1:
          return ( None, False )
        else:
          return ( check, True )
      else:
        if type( param ) == float or type( param ) == int:
          return ( param, False )
        else:
          return ( None, False )
    elif self.myFunction == 2:
      if not param is None:
        if type( param ) == unicode:
          check = layer.dataProvider().fieldNameIndex( param )
          if check == -1:
            return ( None, False )
          else:
            return ( check, True )
        else:
          return ( None, False )
      else:
        return ( True, False )
    elif self.myFunction == 4:
      if type( param ) == unicode:
        check = layer.dataProvider().fieldNameIndex( param )
        if check == -1:
          return ( check, False )
        else:
          return ( check, True )
      else:
        return ( None, False )

  def simpleMeasure( self, inGeom ):
    if inGeom.wkbType() == QGis.WKBPoint:
      pt = QgsPoint()
      pt = inGeom.asPoint()
      attr1 = pt.x()
      attr2 = pt.y()
    else:
      measure = QgsDistanceArea()
      attr1 = measure.measure(inGeom)
      if inGeom.type() == QGis.Polygon:
        attr2 = self.perimMeasure( inGeom, measure )
      else:
        attr2 = attr1
    return ( attr1, attr2 )

  def perimMeasure( self, inGeom, measure ):
    value = 0.00
    if inGeom.isMultipart():
      poly = inGeom.asMultiPolygon()
      for k in poly:
        for j in k:
          value = value + measure.measureLine( j )
    else:
      poly = inGeom.asPolygon()
      for k in poly:
        value = value + measure.measureLine( k )
    return value
