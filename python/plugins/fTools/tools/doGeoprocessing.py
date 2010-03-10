# -*- coding: utf-8 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *
from ui_frmGeoprocessing import Ui_Dialog
import ftools_utils
import sys

class GeoprocessingDialog( QDialog, Ui_Dialog ):
  def __init__( self, iface, function ):
    QDialog.__init__( self )
    self.iface = iface
    self.setupUi( self )
    self.myFunction = function
    QObject.connect( self.btnBrowse, SIGNAL( "clicked()" ), self.outFile )
    QObject.connect( self.inShapeA, SIGNAL( "currentIndexChanged(QString)" ), self.checkA )
    QObject.connect( self.inShapeB, SIGNAL( "currentIndexChanged(QString)" ), self.checkB )
    if function == 4 or function == 1 or function == 2:
      QObject.connect( self.inShapeA, SIGNAL( "currentIndexChanged(QString)" ), self.update )
    self.manageGui()
    self.success = False
    self.cancel_close = self.buttonBox_2.button( QDialogButtonBox.Close )
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
      for i in changedField:
        self.attrib.addItem( unicode( changedField[i].name() ) )
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
          self.useSelectedB.checkState() )
        else:
          if self.param.isEnabled() and self.param.isVisible():
            parameter = float( self.param.text() )
          else:
            parameter = None
          self.geoprocessing( self.inShapeA.currentText(), self.inShapeB.currentText(), 
          parameter, self.mergeOutput.checkState(), self.useSelectedA.checkState(), self.useSelectedB.checkState() )
  
  def outFile( self ):
    self.outShape.clear()
    ( self.shapefileName, self.encoding ) = ftools_utils.saveDialog( self )
    if self.shapefileName is None or self.encoding is None:
      return
    self.outShape.setText( QString( self.shapefileName ) )
  
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
      self.setWindowTitle( self.tr( "Dissolve" ) )
    else:
      self.rdoBuffer.hide()
      self.param.hide()
      self.label_4.hide()
      self.rdoField.hide()
      self.attrib.hide()
      self.mergeOutput.hide()
      if self.myFunction == 3: # Difference
        self.label_2.setText( self.tr( "Difference layer" ) )
        self.setWindowTitle( self.tr( "Difference" ) )
      elif self.myFunction == 5: # Intersect
        self.label_2.setText( self.tr( "Intersect layer" ) )
        self.setWindowTitle( self.tr( "Intersect" ) )
      elif self.myFunction == 7: # Symetrical difference
        self.label_2.setText( self.tr( "Difference layer" ) )
        self.setWindowTitle( self.tr( "Symetrical difference" ) )
      elif self.myFunction == 8: # Clip
        self.label_2.setText( self.tr( "Clip layer" ) )
        self.setWindowTitle( self.tr( "Clip" ) )
      else: # Union
        self.label_2.setText( self.tr( "Union layer" ) )
        self.setWindowTitle( self.tr( "Union" ) )
    self.resize(381, 100)
    myListA = []
    myListB = []
    self.inShapeA.clear()
    self.inShapeB.clear()
    
    if self.myFunction == 5 or self.myFunction == 8 or self.myFunction == 3:
      myListA = ftools_utils.getLayerNames( [ QGis.Point, QGis.Line, QGis.Polygon ] )
      myListB = ftools_utils.getLayerNames( [ QGis.Polygon ] )
    elif self.myFunction == 7 or self.myFunction == 6:
      myListA = ftools_utils.getLayerNames( [ QGis.Point, QGis.Line, QGis.Polygon ] ) # added points and lines to test
      myListB = ftools_utils.getLayerNames( [ QGis.Point, QGis.Line, QGis.Polygon ] ) # added points and lines to test
    elif self.myFunction == 4:
      myListA = ftools_utils.getLayerNames( [ QGis.Polygon ] )
      myListB = []
    else:
      myListA = ftools_utils.getLayerNames( [ QGis.Point, QGis.Line, QGis.Polygon ] )
      myListB = ftools_utils.getLayerNames( [ QGis.Point, QGis.Line, QGis.Polygon ] )
    self.inShapeA.addItems( myListA )
    self.inShapeB.addItems( myListB )
    return

#1: Buffer
#2: Convex Hull
#3: Difference
#4: Dissolve
#5: Intersection
#6: Union
#7: Symetrical Difference
#8: Clip

  def geoprocessing( self,  myLayerA,  myLayerB,  myParam,  myMerge, mySelectionA, mySelectionB ):
    check = QFile( self.shapefileName )
    if check.exists():
      if not QgsVectorFileWriter.deleteShapeFile( self.shapefileName ):
        QMessageBox.warning( self, self.tr("Geoprocessing"), self.tr( "Unable to delete existing shapefile." ) )
        return
    self.testThread = geoprocessingThread( self.iface.mainWindow(), self, self.myFunction, myLayerA, 
    myLayerB, myParam, myMerge, mySelectionA, mySelectionB, self.shapefileName, self.encoding )
    QObject.connect( self.testThread, SIGNAL( "runFinished(PyQt_PyObject)" ), self.runFinishedFromThread )
    QObject.connect( self.testThread, SIGNAL( "runStatus(PyQt_PyObject)" ), self.runStatusFromThread )
    QObject.connect( self.testThread, SIGNAL( "runRange(PyQt_PyObject)" ), self.runRangeFromThread )
    self.cancel_close.setText( self.tr("Cancel") )
    QObject.connect( self.cancel_close, SIGNAL( "clicked()" ), self.cancelThread )
    self.testThread.start()
    return True
  
  def cancelThread( self ):
    self.testThread.stop()
  
  def runFinishedFromThread( self, results ):
    self.testThread.stop()
    self.cancel_close.setText( self.tr("Close") )
    QObject.disconnect( self.cancel_close, SIGNAL( "clicked()" ), self.cancelThread )
    if not results[2] or not results[1] or not results [0]:
      out_text = self.tr( "\nWarnings:" )
      end_text = self.tr( "\nSome output geometries may be missing or invalid.\n\nWould you like to add the new layer anyway?" )
    else:
      out_text = "\n"
      end_text = self.tr( "\n\nWould you like to add the new layer to the TOC?" )
    if not results[2]:
      out_text = out_text + self.tr( "\nInput CRS error: Different input coordinate reference systems detected, results may not be as expected.")
    if not results[1]:
      out_text = out_text + self.tr( "\nFeature geometry error: One or more output features ignored due to invalid geometry.")
    if not results[0]:
      out_text = out_text + self.tr( "\nGEOS geoprocessing error: One or more input features have invalid geometry.")
    else:
      out_text = ""
    addToTOC = QMessageBox.question( self, self.tr("Geoprocessing"), self.tr( "Created output shapefile:\n%1\n\n%2%3" ).arg( unicode( self.shapefileName ) ).arg( out_text ).arg( end_text ), QMessageBox.Yes, QMessageBox.No, QMessageBox.NoButton )
    if addToTOC == QMessageBox.Yes:
      if not ftools_utils.addShapeToCanvas( unicode( self.shapefileName ) ):
          QMessageBox.warning( self, self.tr("Geoprocessing"), self.tr( "Error loading output shapefile:\n%1" ).arg( unicode( self.shapefileName ) ))
    
  def runStatusFromThread( self, status ):
    self.progressBar.setValue( status )
  
  def runRangeFromThread( self, range_vals ):
    self.progressBar.setRange( range_vals[ 0 ], range_vals[ 1 ] )
  
class geoprocessingThread( QThread ):
  def __init__( self, parentThread, parentObject, function, myLayerA, myLayerB, 
  myParam, myMerge, mySelectionA, mySelectionB, myName, myEncoding ):
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
    self.myName = myName
    self.myEncoding = myEncoding
  
  def run( self ):
    self.running = True
    self.vlayerA = ftools_utils.getVectorLayerByName( self.myLayerA )
    if self.myFunction == 1 or self.myFunction == 2 or self.myFunction == 4:
      ( self.myParam, useField ) = self.checkParameter( self.vlayerA, self.myParam )
      if not self.myParam is None:
        if self.myFunction == 1:
          geos, feature, match = self.buffering( useField )
        elif self.myFunction == 2:
          geos, feature, match = self.convex_hull( useField )
        elif self.myFunction == 4:
          geos, feature, match = self.dissolve( useField )
    else:
      self.vlayerB = ftools_utils.getVectorLayerByName( self.myLayerB )
      if self.myFunction == 3:
        geos, feature, match = self.difference()
      elif self.myFunction == 5:
        geos, feature, match = self.intersect()
      elif self.myFunction == 6:
        geos, feature, match = self.union()
      elif self.myFunction == 7:
        geos, feature, match = self.symetrical_difference()
      elif self.myFunction == 8:
        geos, feature, match = self.clip()
    self.emit( SIGNAL( "runFinished(PyQt_PyObject)" ), (geos, feature, match) )
    self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0 )
  
  def stop(self):
    self.running = False
  
  def buffering( self, useField ):
    GEOS_EXCEPT = True
    FEATURE_EXCEPT = True
    vproviderA = self.vlayerA.dataProvider()
    allAttrs = vproviderA.attributeIndexes()
    vproviderA.select( allAttrs )
    fields = vproviderA.fields()
    writer = QgsVectorFileWriter( self.myName, self.myEncoding, 
    fields, QGis.WKBPolygon, vproviderA.crs() )
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
          atMap = inFeat.attributeMap()
          if useField:
            value = atMap[ self.myParam ].doDouble()[ 0 ]
          else:
            value = self.myParam
          inGeom = QgsGeometry( inFeat.geometry() )
          try:
            outGeom = inGeom.buffer( float( value ), 5 )
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
          atMap = inFeat.attributeMap()
          if useField:
            value = atMap[ self.myParam ].toDouble()[ 0 ]
          else:
            value = self.myParam
          inGeom = QgsGeometry( inFeat.geometry() )
          try:
            outGeom = inGeom.buffer( float( value ), 5 )
            try:
              outFeat.setGeometry( outGeom )
              outFeat.setAttributeMap( atMap )
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
        while vproviderA.nextFeature( inFeat ):
          atMap = inFeat.attributeMap()
          if useField:
            value = atMap[ self.myParam ].toDouble()[ 0 ]
          else:
            value = self.myParam
          inGeom = QgsGeometry( inFeat.geometry() )
          try:
            outGeom = inGeom.buffer( float( value ), 5 )
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
        vproviderA.rewind()
        while vproviderA.nextFeature( inFeat ):
          atMap = inFeat.attributeMap()
          if useField:
            value = atMap[ self.myParam ].toDouble()[ 0 ]
          else:
            value = self.myParam
          inGeom = QgsGeometry( inFeat.geometry() )
          try:
            outGeom = inGeom.buffer( float( value ), 5 )
            try:
              outFeat.setGeometry( outGeom )
              outFeat.setAttributeMap( atMap )
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
    return GEOS_EXCEPT, FEATURE_EXCEPT, True
  
  def convex_hull(self, useField ):
    GEOS_EXCEPT = True
    FEATURE_EXCEPT = True
    vproviderA = self.vlayerA.dataProvider()
    allAttrsA = vproviderA.attributeIndexes()
    vproviderA.select(allAttrsA)
    fields = vproviderA.fields()
    writer = QgsVectorFileWriter( self.myName, self.myEncoding, 
    fields, QGis.WKBPolygon, vproviderA.crs() )
    inFeat = QgsFeature()
    outFeat = QgsFeature()
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
          hull = []
          first = True
          outID = 0
          for inFeat in selectionA:
            atMap = inFeat.attributeMap()
            idVar = atMap[ self.myParam ]
            if idVar.toString().trimmed() == i.toString().trimmed():
              if first:
                outID = idVar
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
              outFeat.addAttribute( 0, QVariant( outID ) )
              outFeat.addAttribute( 1, QVariant( area ) )
              outFeat.addAttribute( 2, QVariant( perim ) )
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
          writer.addFeature( outFeat )
        except:
          GEOS_EXCEPT = False
    # there is no selection in input layer
    else:
      rect = self.vlayerA.extent()
      nFeat = vproviderA.featureCount()
      if useField:
        unique = ftools_utils.getUniqueValues( vproviderA, self.myParam )
        nFeat = nFeat * len( unique )
        self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0 )
        self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
        for i in unique:
          hull = []
          first = True
          outID = 0
          vproviderA.select( allAttrsA )#, rect )
          vproviderA.rewind()
          while vproviderA.nextFeature( inFeat ):
            atMap = inFeat.attributeMap()
            idVar = atMap[ self.myParam ]
            if idVar.toString().trimmed() == i.toString().trimmed():
              if first:
                outID = idVar
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
              outFeat.addAttribute( 0, QVariant( outID ) )
              outFeat.addAttribute( 1, QVariant( area ) )
              outFeat.addAttribute( 2, QVariant( perim ) )
              writer.addFeature( outFeat )
            except:
              GEOS_EXCEPT = False
              continue
      else:
        self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0 )
        self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
        hull = []
        vproviderA.rewind()
        vproviderA.select(allAttrsA)
        while vproviderA.nextFeature( inFeat ):
          inGeom = QgsGeometry( inFeat.geometry() )
          points = ftools_utils.extractPoints( inGeom )
          hull.extend( points )
          nElement += 1
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
        tmpGeom = QgsGeometry( outGeom.fromMultiPoint( hull ) )
        try:
          outGeom = tmpGeom.convexHull()
          outFeat.setGeometry( outGeom )
          writer.addFeature( outFeat )
        except:
          GEOS_EXCEPT = False
    del writer
    return GEOS_EXCEPT, FEATURE_EXCEPT, True
  
  def dissolve( self, useField ):
    GEOS_EXCEPT = True
    FEATURE_EXCEPT = True
    vproviderA = self.vlayerA.dataProvider()
    allAttrsA = vproviderA.attributeIndexes()
    fields = vproviderA.fields()
    writer = QgsVectorFileWriter( self.myName, self.myEncoding, 
    fields, vproviderA.geometryType(), vproviderA.crs() )
    inFeat = QgsFeature()
    outFeat = QgsFeature()
    vproviderA.rewind()
    nElement = 0
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
            attrs = inFeat.attributeMap()
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
        outFeat.setAttributeMap( attrs )
        writer.addFeature( outFeat )
      else:
        unique = vproviderA.uniqueValues( int( self.myParam ) )
        nFeat = nFeat * len( unique )
        self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0 )
        self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
        for item in unique:
          first = True
          add = False
          vproviderA.select( allAttrsA )
          vproviderA.rewind()
          for inFeat in selectionA:
            nElement += 1
            self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
            atMap = inFeat.attributeMap()
            tempItem = atMap[ self.myParam ]
            if tempItem.toString().trimmed() == item.toString().trimmed():
              add = True
              if first:
                QgsGeometry( inFeat.geometry() )
                tmpInGeom = QgsGeometry( inFeat.geometry() )
                outFeat.setGeometry( tmpInGeom )
                first = False
                attrs = inFeat.attributeMap()
              else:
                tmpInGeom = QgsGeometry( inFeat.geometry() )
                tmpOutGeom = QgsGeometry( outFeat.geometry() )
                try:
                  tmpOutGeom = QgsGeometry( tmpOutGeom.combine( tmpInGeom ) )
                  outFeat.setGeometry( tmpOutGeom )
                except:
                  GEOS_EXCEPT = False
                  add = False
          if add:
            outFeat.setAttributeMap( attrs )
            writer.addFeature( outFeat )
    # there is no selection in input layer
    else:
      nFeat = vproviderA.featureCount()
      if not useField:
        self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0)
        self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
        first = True
        while vproviderA.nextFeature( inFeat ):
          nElement += 1
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ),  nElement )
          if first:
            attrs = inFeat.attributeMap()
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
        outFeat.setAttributeMap( attrs )
        writer.addFeature( outFeat )
      else:
        unique = vproviderA.uniqueValues( int( self.myParam ) )
        nFeat = nFeat * len( unique )
        self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0)
        self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
        for item in unique:
          first = True
          add = True
          vproviderA.select( allAttrsA )
          vproviderA.rewind()
          while vproviderA.nextFeature( inFeat ):
            nElement += 1
            self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ),  nElement )
            atMap = inFeat.attributeMap()
            tempItem = atMap[ self.myParam ]
            if tempItem.toString().trimmed() == item.toString().trimmed():
              if first:
                QgsGeometry( inFeat.geometry() )
                tmpInGeom = QgsGeometry( inFeat.geometry() )
                outFeat.setGeometry( tmpInGeom )
                first = False
                attrs = inFeat.attributeMap()
              else:
                tmpInGeom = QgsGeometry( inFeat.geometry() )
                tmpOutGeom = QgsGeometry( outFeat.geometry() )
                try:
                  tmpOutGeom = QgsGeometry( tmpOutGeom.combine( tmpInGeom ) )
                  outFeat.setGeometry( tmpOutGeom )
                except:
                  GEOS_EXCEPT = False
                  add = False
          if add:
            outFeat.setAttributeMap( attrs )
            writer.addFeature( outFeat )
    del writer
    return GEOS_EXCEPT, FEATURE_EXCEPT, True
  
  def difference( self ):
    GEOS_EXCEPT = True
    FEATURE_EXCEPT = True
    vproviderA = self.vlayerA.dataProvider()
    allAttrsA = vproviderA.attributeIndexes()
    vproviderA.select( allAttrsA )
    vproviderB = self.vlayerB.dataProvider()
    allAttrsB = vproviderB.attributeIndexes()
    vproviderB.select( allAttrsB )
    fields = vproviderA.fields()
    if vproviderA.crs() == vproviderB.crs(): crs_match = True
    else: crs_match = False
    writer = QgsVectorFileWriter( self.myName, self.myEncoding, 
    fields, vproviderA.geometryType(), vproviderA.crs() )
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
          add = True
          geom = QgsGeometry( inFeatA.geometry() )
          diff_geom = QgsGeometry( geom )
          atMap = inFeatA.attributeMap()
          intersects = index.intersects( geom.boundingBox() )
          for id in intersects:
            # is intersect feature in selection
            if id in selectionB:
              vproviderB.featureAtId( int( id ), inFeatB , True, allAttrsB )
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
              outFeat.setAttributeMap( atMap )
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
          atMap = inFeatA.attributeMap()
          intersects = index.intersects( geom.boundingBox() )
          for id in intersects:
            vproviderB.featureAtId( int( id ), inFeatB , True, allAttrsB )
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
              outFeat.setAttributeMap( atMap )
              writer.addFeature( outFeat )
            except:
              FEATURE_EXCEPT = False
              continue
    # there is no selection in input layer
    else:
      nFeat = vproviderA.featureCount()
      self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0 )
      self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
      vproviderA.rewind()
      # we have selection in overlay layer
      if self.mySelectionB:
        selectionB = self.vlayerB.selectedFeaturesIds()
        while vproviderA.nextFeature( inFeatA ):
          nElement += 1
          add = True
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
          geom = QgsGeometry( inFeatA.geometry() )
          diff_geom = QgsGeometry( geom )
          atMap = inFeatA.attributeMap()
          intersects = index.intersects( geom.boundingBox() )
          for id in intersects:
            # now check if id in selection
            if id in selectionB:
              vproviderB.featureAtId( int( id ), inFeatB , True, allAttrsB )
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
              outFeat.setAttributeMap( atMap )
              writer.addFeature( outFeat )
            except:
              FEATURE_EXCEPT = False
              continue
      # we have no selection in overlay layer
      else:
        while vproviderA.nextFeature( inFeatA ):
          nElement += 1
          add = True
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
          geom = QgsGeometry( inFeatA.geometry() )
          diff_geom = QgsGeometry( geom )
          atMap = inFeatA.attributeMap()
          intersects = index.intersects( geom.boundingBox() )
          for id in intersects:
            vproviderB.featureAtId( int( id ), inFeatB , True, allAttrsB )
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
              outFeat.setAttributeMap( atMap )
              writer.addFeature( outFeat )
            except:
              FEATURE_EXCEPT = False
              continue
    del writer
    return GEOS_EXCEPT, FEATURE_EXCEPT, crs_match
  
  def intersect( self ):
    GEOS_EXCEPT = True
    FEATURE_EXCEPT = True
    vproviderA = self.vlayerA.dataProvider()
    allAttrsA = vproviderA.attributeIndexes()
    vproviderA.select( allAttrsA )
    vproviderB = self.vlayerB.dataProvider()
    allAttrsB = vproviderB.attributeIndexes()
    vproviderB.select( allAttrsB )
    if vproviderA.crs() == vproviderB.crs(): crs_match = True
    else: crs_match = False
    fields = ftools_utils.combineVectorFields( self.vlayerA, self.vlayerB )
    writer = QgsVectorFileWriter( self.myName, self.myEncoding, 
    fields, vproviderA.geometryType(), vproviderA.crs() )
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
          atMapA = inFeatA.attributeMap()
          intersects = index.intersects( geom.boundingBox() )
          for id in intersects:
            if id in selectionB:
              vproviderB.featureAtId( int( id ), inFeatB , True, allAttrsB )
              tmpGeom = QgsGeometry( inFeatB.geometry() )
              try:
                if geom.intersects( tmpGeom ):
                  atMapB = inFeatB.attributeMap()
                  int_geom = QgsGeometry( geom.intersection( tmpGeom ) )
                  if int_geom.wkbType() == 7:
                    int_com = geom.combine( tmpGeom )
                    int_sym = geom.symDifference( tmpGeom )
                    int_geom = QgsGeometry( int_com.difference( int_sym ) )
                  try:
                    outFeat.setGeometry( int_geom )
                    outFeat.setAttributeMap( ftools_utils.combineVectorAttributes( atMapA, atMapB ) )
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
          atMapA = inFeatA.attributeMap()
          intersects = index.intersects( geom.boundingBox() )
          for id in intersects:
            vproviderB.featureAtId( int( id ), inFeatB , True, allAttrsB )
            tmpGeom = QgsGeometry( inFeatB.geometry() )
            try:
              if geom.intersects( tmpGeom ):
                atMapB = inFeatB.attributeMap()
                int_geom = QgsGeometry( geom.intersection( tmpGeom ) )
                if int_geom.wkbType() == 7:
                  int_com = geom.combine( tmpGeom )
                  int_sym = geom.symDifference( tmpGeom )
                  int_geom = QgsGeometry( int_com.difference( int_sym ) )
                try:
                  outFeat.setGeometry( int_geom )
                  outFeat.setAttributeMap( ftools_utils.combineVectorAttributes( atMapA, atMapB ) )
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
      vproviderA.rewind()
      # we have selection in overlay layer
      if self.mySelectionB:
        selectionB = self.vlayerB.selectedFeaturesIds()
        while vproviderA.nextFeature( inFeatA ):
          nElement += 1
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
          geom = QgsGeometry( inFeatA.geometry() )
          atMapA = inFeatA.attributeMap()
          intersects = index.intersects( geom.boundingBox() )
          for id in intersects:
            if id in selectionB:
              vproviderB.featureAtId( int( id ), inFeatB , True, allAttrsB )
              tmpGeom = QgsGeometry( inFeatB.geometry() )
              try:
                if geom.intersects( tmpGeom ):
                  atMapB = inFeatB.attributeMap()
                  int_geom = QgsGeometry( geom.intersection( tmpGeom ) )
                  if int_geom.wkbType() == 7:
                    int_com = geom.combine( tmpGeom )
                    int_sym = geom.symDifference( tmpGeom )
                    int_geom = QgsGeometry( int_com.difference( int_sym ) )
                  try:
                    outFeat.setGeometry( int_geom )
                    outFeat.setAttributeMap( ftools_utils.combineVectorAttributes( atMapA, atMapB ) )
                    writer.addFeature( outFeat )
                  except:
                    FEATURE_EXCEPT = False
                    continue
              except:
                GEOS_EXCEPT = False
                break
      # we have no selection in overlay layer
      else:
        while vproviderA.nextFeature( inFeatA ):
          nElement += 1
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
          geom = QgsGeometry( inFeatA.geometry() )
          atMapA = inFeatA.attributeMap()
          intersects = index.intersects( geom.boundingBox() )
          for id in intersects: 
            vproviderB.featureAtId( int( id ), inFeatB , True, allAttrsB )
            tmpGeom = QgsGeometry( inFeatB.geometry() )
            try:
              if geom.intersects( tmpGeom ):
                atMapB = inFeatB.attributeMap()
                int_geom = QgsGeometry( geom.intersection( tmpGeom ) )
                if int_geom.wkbType() == 7:
                  int_com = geom.combine( tmpGeom )
                  int_sym = geom.symDifference( tmpGeom )
                  int_geom = QgsGeometry( int_com.difference( int_sym ) )
                try:
                  outFeat.setGeometry( int_geom )
                  outFeat.setAttributeMap( ftools_utils.combineVectorAttributes( atMapA, atMapB ) )
                  writer.addFeature( outFeat )
                except:
                  FEATURE_EXCEPT = False
                  continue
            except:
              GEOS_EXCEPT = False
              break
    del writer
    return GEOS_EXCEPT, FEATURE_EXCEPT, crs_match
  
  def union( self ):
    GEOS_EXCEPT = True
    FEATURE_EXCEPT = True
    vproviderA = self.vlayerA.dataProvider()
    allAttrsA = vproviderA.attributeIndexes()
    vproviderA.select( allAttrsA )
    vproviderB = self.vlayerB.dataProvider()
    allAttrsB = vproviderB.attributeIndexes()
    vproviderB.select( allAttrsB )
    if vproviderA.crs() == vproviderB.crs(): crs_match = True
    else: crs_match = False
    fields = ftools_utils.combineVectorFields( self.vlayerA, self.vlayerB )
    writer = QgsVectorFileWriter( self.myName, self.myEncoding, 
    fields, vproviderA.geometryType(), vproviderA.crs() )
    inFeatA = QgsFeature()
    inFeatB = QgsFeature()
    outFeat = QgsFeature()
    indexA = ftools_utils.createIndex( vproviderB )
    indexB = ftools_utils.createIndex( vproviderA )
    nFeat = vproviderA.featureCount() * vproviderB.featureCount()
    nElement = 0
    self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0)
    self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
    vproviderA.rewind()
    while vproviderA.nextFeature( inFeatA ):
      self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
      nElement += 1
      found = False
      geom = QgsGeometry( inFeatA.geometry() )
      diffGeom = QgsGeometry( inFeatA.geometry() )
      atMapA = inFeatA.attributeMap()
      intersects = indexA.intersects( geom.boundingBox() )
      if len( intersects ) <= 0:
        try:
          outFeat.setGeometry( geom )
          outFeat.setAttributeMap( atMapA )
          writer.addFeature( outFeat )
        except:
          FEATURE_EXCEPT = False
          continue
      else:
        for id in intersects:
          vproviderB.featureAtId( int( id ), inFeatB , True, allAttrsB )
          atMapB = inFeatB.attributeMap()
          tmpGeom = QgsGeometry( inFeatB.geometry() )
          try:
            if geom.intersects( tmpGeom ):
              found = True
              diff_geom = QgsGeometry( geom.difference( tmpGeom ) )
              int_geom = QgsGeometry( geom.intersection( tmpGeom ) )
              if int_geom.wkbType() == 7:
                int_com = geom.combine( tmpGeom )
                int_sym = geom.symDifference( tmpGeom )
                int_geom = QgsGeometry( int_com.difference( int_sym ) )
              try:
                outFeat.setGeometry( int_geom )
                outFeat.setAttributeMap( ftools_utils.combineVectorAttributes( atMapA, atMapB ) )
                writer.addFeature( outFeat )
              except:
                FEATURE_EXCEPT = False
                continue
            else:
              try:
                outFeat.setGeometry( geom )
                outFeat.setAttributeMap( atMapA )
                writer.addFeature( outFeat )
              except:
                FEATURE_EXCEPT = False
                continue
          except:
            GEOS_EXCEPT = False
            found = False
            continue
        if found:
          try:
            outFeat.setGeometry( diff_geom )
            outFeat.setAttributeMap( atMapA )
            writer.addFeature( outFeat )
          except:
            FEATURE_EXCEPT = False
            continue
    length = len( vproviderA.fields().values() )
    vproviderB.rewind()
    while vproviderB.nextFeature( inFeatA ):
      add = True
      geom = QgsGeometry( inFeatA.geometry() )
      diff_geom = QgsGeometry( geom )
      atMap = inFeatA.attributeMap().values()
      atMap = dict( zip( range( length, length + len( atMap ) ), atMap ) )
      intersects = indexB.intersects( geom.boundingBox() )
      if len(intersects) <= 0:
        try:
          outFeat.setGeometry( diff_geom )
          outFeat.setAttributeMap( atMap )
          writer.addFeature( outFeat )
        except:
          FEATURE_EXCEPT = False
          continue
      else:
        for id in intersects:
          vproviderA.featureAtId( int( id ), inFeatB , True, allAttrsA )
          atMapB = inFeatB.attributeMap()
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
            outFeat.setAttributeMap( atMap )
            writer.addFeature( outFeat )
          except:
            FEATURE_EXCEPT = False
            continue
      self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ),  nElement )
      nElement += 1
    del writer
    return GEOS_EXCEPT, FEATURE_EXCEPT, crs_match
  
  def symetrical_difference( self ):
    GEOS_EXCEPT = True
    FEATURE_EXCEPT = True
    vproviderA = self.vlayerA.dataProvider()
    allAttrsA = vproviderA.attributeIndexes()
    vproviderA.select( allAttrsA )
    vproviderB = self.vlayerB.dataProvider()
    allAttrsB = vproviderB.attributeIndexes()
    vproviderB.select( allAttrsB )
    if vproviderA.crs() == vproviderB.crs(): crs_match = True
    else: crs_match = False
    fields = ftools_utils.combineVectorFields( self.vlayerA, self.vlayerB )
    writer = QgsVectorFileWriter( self.myName, self.myEncoding, 
    fields, vproviderA.geometryType(), vproviderA.crs() )
    inFeatA = QgsFeature()
    inFeatB = QgsFeature()
    outFeat = QgsFeature()
    indexA = ftools_utils.createIndex( vproviderB )
    indexB = ftools_utils.createIndex( vproviderA )
    nFeat = vproviderA.featureCount() * vproviderB.featureCount()
    nElement = 0
    self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0)
    self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
    vproviderA.rewind()
    while vproviderA.nextFeature( inFeatA ):
      nElement += 1
      self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
      add = True
      geom = QgsGeometry( inFeatA.geometry() )
      diff_geom = QgsGeometry( geom )
      atMapA = inFeatA.attributeMap()
      intersects = indexA.intersects( geom.boundingBox() )
      for id in intersects:
        vproviderB.featureAtId( int( id ), inFeatB , True, allAttrsB )
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
          outFeat.setAttributeMap( atMapA )
          writer.addFeature( outFeat )
        except:
          FEATURE_EXCEPT = False
          continue
    length = len( vproviderA.fields().values() )
    vproviderB.rewind()
    while vproviderB.nextFeature( inFeatA ):
      nElement += 1
      self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
      add = True
      geom = QgsGeometry( inFeatA.geometry() )
      diff_geom = QgsGeometry( geom )
      atMap = inFeatA.attributeMap().values()
      atMap = dict( zip( range( length, length + len( atMap ) ), atMap ) )
      intersects = indexB.intersects( geom.boundingBox() )
      for id in intersects:
        vproviderA.featureAtId( int( id ), inFeatB , True, allAttrsA )
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
          outFeat.setAttributeMap( atMap )
          writer.addFeature( outFeat )
        except:
          FEATURE_EXCEPT = False
          continue
    del writer
    return GEOS_EXCEPT, FEATURE_EXCEPT, crs_match
  
  def clip( self ):
    GEOS_EXCEPT = True
    FEATURE_EXCEPT = True
    vproviderA = self.vlayerA.dataProvider()
    allAttrsA = vproviderA.attributeIndexes()
    vproviderA.select( allAttrsA )
    vproviderB = self.vlayerB.dataProvider()
    allAttrsB = vproviderB.attributeIndexes()
    vproviderB.select( allAttrsB )
    if vproviderA.crs() == vproviderB.crs(): crs_match = True
    else: crs_match = False
    fields = vproviderA.fields()
    writer = QgsVectorFileWriter( self.myName, self.myEncoding, 
    fields, vproviderA.geometryType(), vproviderA.crs() )
    inFeatA = QgsFeature()
    inFeatB = QgsFeature()
    outFeat = QgsFeature()
    index = ftools_utils.createIndex( vproviderB )
    vproviderA.rewind()
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
          atMap = inFeatA.attributeMap()
          intersects = index.intersects( geom.boundingBox() )
          found = False
          first = True
          for id in intersects:
            if id in selectionB:
              vproviderB.featureAtId( int( id ), inFeatB , True, allAttrsB )
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
              if new_geom.wkbType() == 7:
                int_com = QgsGeometry( geom.combine( cur_geom ) )
                int_sym = QgsGeometry( geom.symDifference( cur_geom ) )
                new_geom = QgsGeometry( int_com.difference( int_sym ) )
              try:
                outFeat.setGeometry( new_geom )
                outFeat.setAttributeMap( atMap )
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
          atMap = inFeatA.attributeMap()
          intersects = index.intersects( geom.boundingBox() )
          found = False
          first = True
          for id in intersects:
            vproviderB.featureAtId( int( id ), inFeatB , True, allAttrsB )
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
              if new_geom.wkbType() == 7:
                int_com = QgsGeometry( geom.combine( cur_geom ) )
                int_sym = QgsGeometry( geom.symDifference( cur_geom ) )
                new_geom = QgsGeometry( int_com.difference( int_sym ) )
              try:
                outFeat.setGeometry( new_geom )
                outFeat.setAttributeMap( atMap )
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
        while vproviderA.nextFeature( inFeatA ):
          nElement += 1
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
          geom = QgsGeometry( inFeatA.geometry() )
          atMap = inFeatA.attributeMap()
          intersects = index.intersects( geom.boundingBox() )
          found = False
          first = True
          for id in intersects:
            if id in selectionB:
              vproviderB.featureAtId( int( id ), inFeatB , True, allAttrsB )
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
              if new_geom.wkbType() == 7:
                int_com = QgsGeometry( geom.combine( cur_geom ) )
                int_sym = QgsGeometry( geom.symDifference( cur_geom ) )
                new_geom = QgsGeometry( int_com.difference( int_sym ) )
              try:
                outFeat.setGeometry( new_geom )
                outFeat.setAttributeMap( atMap )
                writer.addFeature( outFeat )
              except:
                FEAT_EXCEPT = False
                continue
            except:
              GEOS_EXCEPT = False
              continue          
      # we have no selection in overlay layer
      else:
        while vproviderA.nextFeature( inFeatA ):
          nElement += 1
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
          geom = QgsGeometry( inFeatA.geometry() )
          atMap = inFeatA.attributeMap()
          intersects = index.intersects( geom.boundingBox() )
          first = True
          found = False
          if len( intersects ) > 0:
            for id in intersects:
              vproviderB.featureAtId( int( id ), inFeatB , True, allAttrsB )
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
                if new_geom.wkbType() == 7:
                  int_com = QgsGeometry( geom.combine( cur_geom ) )
                  int_sym = QgsGeometry( geom.symDifference( cur_geom ) )
                  new_geom = QgsGeometry( int_com.difference( int_sym ) )
                try:
                  outFeat.setGeometry( new_geom )
                  outFeat.setAttributeMap( atMap )
                  writer.addFeature( outFeat )
                except:
                  FEAT_EXCEPT = False
                  continue
              except:
                GEOS_EXCEPT = False
                continue              
    del writer
    return GEOS_EXCEPT, FEATURE_EXCEPT, crs_match

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
