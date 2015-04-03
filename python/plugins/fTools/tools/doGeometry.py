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

from PyQt4.QtCore import QObject, SIGNAL, QFile, QThread, QSettings, QVariant
from PyQt4.QtGui import QDialog, QDialogButtonBox, QMessageBox
from qgis.core import QGis, QgsVectorFileWriter, QgsFeature, QgsGeometry, QgsCoordinateTransform, QgsFields, QgsField, QgsFeatureRequest, QgsPoint, QgsDistanceArea
from ui_frmGeometry import Ui_Dialog
import ftools_utils
import voronoi
from sets import Set

class GeometryDialog( QDialog, Ui_Dialog ):
  def __init__( self, iface, function ):
    QDialog.__init__( self, iface.mainWindow() )
    self.iface = iface
    self.setupUi( self )
    self.myFunction = function
    self.buttonOk = self.buttonBox_2.button( QDialogButtonBox.Ok )
    QObject.connect( self.toolOut, SIGNAL( "clicked()" ), self.outFile )
    if self.myFunction == 1:
      QObject.connect( self.inShape, SIGNAL( "currentIndexChanged( QString )" ), self.update )
    elif self.myFunction == 5:
      QObject.connect( self.chkWriteShapefile, SIGNAL( "stateChanged( int )" ), self.updateGui )
      self.updateGui()
    self.manageGui()
    self.success = False
    self.cancel_close = self.buttonBox_2.button( QDialogButtonBox.Close )
    self.progressBar.setValue( 0 )

  def update( self ):
    self.cmbField.clear()
    inputLayer = unicode( self.inShape.currentText() )
    if inputLayer != "":
      changedLayer = ftools_utils.getVectorLayerByName( inputLayer )
      changedField = ftools_utils.getFieldList( changedLayer )
      for f in changedField:
        self.cmbField.addItem( unicode( f.name() ) )
      self.cmbField.addItem( "--- " + self.tr( "Merge all" ) + " ---" )

  def accept( self ):
    if self.inShape.currentText() == "":
      QMessageBox.information( self, self.tr( "Geometry" ),
                               self.tr( "Please specify input vector layer" ) )
    elif self.outShape.text() == "" and self.myFunction != 5:
      QMessageBox.information( self, self.tr( "Geometry" ),
                               self.tr( "Please specify output shapefile" ) )
    elif self.lineEdit.isVisible() and self.lineEdit.value() < 0.00:
      QMessageBox.information( self, self.tr( "Geometry" ),
                               self.tr( "Please specify valid tolerance value" ) )
    elif self.cmbField.isVisible() and self.cmbField.currentText() == "":
      QMessageBox.information( self, self.tr( "Geometry" ),
                               self.tr( "Please specify valid UID field" ) )
    else:
      self.outShape.clear()
      self.geometry( self.inShape.currentText(), self.lineEdit.value(),
                     self.cmbField.currentText() )

  def outFile( self ):
    self.outShape.clear()
    (self.shapefileName, self.encoding) = ftools_utils.saveDialog( self )
    if self.shapefileName is None or self.encoding is None:
      return
    self.outShape.setText( self.shapefileName )

  def manageGui( self ):
    self.lblField.setVisible( False )
    self.cmbField.setVisible( False )

    self.lblCalcType.setVisible( False )
    self.cmbCalcType.setVisible( False )

    self.chkUseSelection.setVisible( False )
    self.chkByFeatures.setVisible( False )

    self.chkWriteShapefile.setVisible( False )
    if self.myFunction == 1: # Singleparts to multipart
      self.setWindowTitle( self.tr( "Singleparts to multipart" ) )
      self.lineEdit.setVisible( False )
      self.label.setVisible( False )
      self.lblOutputShapefile.setText( self.tr( "Output shapefile" ) )
      self.cmbField.setVisible( True )
      self.lblField.setVisible( True )
    elif self.myFunction == 2: # Multipart to singleparts
      self.setWindowTitle( self.tr( "Multipart to singleparts" ) )
      self.lineEdit.setVisible( False )
      self.label.setVisible( False )
      self.lblOutputShapefile.setText( self.tr( "Output shapefile" ) )
    elif self.myFunction == 3: # Extract nodes
      self.setWindowTitle( self.tr( "Extract nodes" ) )
      self.lineEdit.setVisible( False )
      self.label.setVisible( False )
    elif self.myFunction == 4: # Polygons to lines
      self.setWindowTitle( self.tr(  "Polygons to lines" ) )
      self.lblOutputShapefile.setText( self.tr( "Output shapefile" ) )
      self.label_3.setText( self.tr( "Input polygon vector layer" ) )
      self.label.setVisible( False )
      self.lineEdit.setVisible( False )
    elif self.myFunction == 5: # Export/Add geometry columns
      self.setWindowTitle( self.tr(  "Export/Add geometry columns" ) )
      self.lblOutputShapefile.setText( self.tr( "Output shapefile" ) )
      self.label_3.setText( self.tr( "Input vector layer" ) )
      self.label.setVisible( False )
      self.lineEdit.setVisible( False )
      # populate calculation types
      self.lblCalcType.setVisible( True )
      self.cmbCalcType.setVisible( True )
      self.cmbCalcType.addItem( self.tr( "Layer CRS" ) )
      self.cmbCalcType.addItem( self.tr( "Project CRS" ) )
      self.cmbCalcType.addItem( self.tr( "Ellipsoid" ) )

      self.chkWriteShapefile.setVisible( True )
      self.chkWriteShapefile.setChecked( False )
      self.lblOutputShapefile.setVisible( False )
    elif self.myFunction == 7: # Polygon centroids
      self.setWindowTitle( self.tr( "Polygon centroids" ) )
      self.lblOutputShapefile.setText( self.tr( "Output point shapefile" ) )
      self.label_3.setText( self.tr( "Input polygon vector layer" ) )
      self.label.setVisible( False )
      self.lineEdit.setVisible( False )
    else:
      if self.myFunction == 8: # Delaunay triangulation
        self.setWindowTitle( self.tr( "Delaunay triangulation" ) )
        self.label_3.setText( self.tr( "Input point vector layer" ) )
        self.label.setVisible( False )
        self.lineEdit.setVisible( False )
      elif self.myFunction == 10: # Voronoi Polygons
        self.setWindowTitle( self.tr( "Voronoi polygon" ) )
        self.label_3.setText( self.tr( "Input point vector layer" ) )
        self.label.setText( self.tr( "Buffer region" ) )
        self.lineEdit.setSuffix( " %" )
        self.lineEdit.setRange( 0, 100 )
        self.lineEdit.setSingleStep( 5 )
        self.lineEdit.setValue( 0 )
      elif self.myFunction == 11:  # Lines to polygons
        self.setWindowTitle( self.tr(  "Lines to polygons" ) )
        self.lblOutputShapefile.setText( self.tr( "Output shapefile" ) )
        self.label_3.setText( self.tr( "Input line vector layer" ) )
        self.label.setVisible( False )
        self.lineEdit.setVisible( False )
      else: # Polygon from layer extent
        self.setWindowTitle( self.tr( "Polygon from layer extent" ) )
        self.label_3.setText( self.tr( "Input layer" ) )
        self.label.setVisible( False )
        self.lineEdit.setVisible( False )
        self.chkByFeatures.setVisible( True )
        self.chkUseSelection.setVisible( True )
      self.lblOutputShapefile.setText( self.tr( "Output polygon shapefile" ) )
    self.resize( 381, 100 )
    self.populateLayers()

  def updateGui( self ):
    if self.chkWriteShapefile.isChecked():
      self.lineEdit.setEnabled( True )
      self.toolOut.setEnabled( True )
      self.addToCanvasCheck.setEnabled( True )
    else:
      self.lineEdit.setEnabled( False )
      self.toolOut.setEnabled( False )
      self.addToCanvasCheck.setEnabled( False )

  def populateLayers( self ):
    self.inShape.clear()
    if self.myFunction == 3 or self.myFunction == 6:
      myList = ftools_utils.getLayerNames( [ QGis.Polygon, QGis.Line ] )
    elif self.myFunction == 4 or self.myFunction == 7:
      myList = ftools_utils.getLayerNames( [ QGis.Polygon ] )
    elif self.myFunction == 8 or self.myFunction == 10:
      myList = ftools_utils.getLayerNames( [ QGis.Point ] )
    elif self.myFunction == 9:
      myList = ftools_utils.getLayerNames( "all" )
    elif self.myFunction == 11:
      myList = ftools_utils.getLayerNames( [ QGis.Line ] )
    else:
      myList = ftools_utils.getLayerNames( [ QGis.Point, QGis.Line, QGis.Polygon ] )
    self.inShape.addItems( myList )

#1:  Singleparts to multipart
#2:  Multipart to singleparts
#3:  Extract nodes
#4:  Polygons to lines
#5:  Export/Add geometry columns
#6:  Simplify geometries (disabled)
#7:  Polygon centroids
#8:  Delaunay triangulation
#9:  Polygon from layer extent
#10: Voronoi polygons
#11: Lines to polygons

  def geometry( self, myLayer, myParam, myField ):
    if self.myFunction == 9:
      vlayer = ftools_utils.getMapLayerByName( myLayer )
    else:
      vlayer = ftools_utils.getVectorLayerByName( myLayer )

    if ( self.myFunction == 5 and self.chkWriteShapefile.isChecked() ) or self.myFunction != 5:
      check = QFile( self.shapefileName )
      if check.exists():
        if not QgsVectorFileWriter.deleteShapeFile( self.shapefileName ):
          QMessageBox.warning( self, self.tr( "Geometry"),
                               self.tr( "Unable to delete existing shapefile." ) )
          return

    if self.myFunction == 5 and not self.chkWriteShapefile.isChecked():
      self.shapefileName = None
      self.encoding = None

      res = QMessageBox.warning( self, self.tr( "Geometry"),
                                 self.tr( "Currently QGIS doesn't allow simultaneous access from "
                                          "different threads to the same datasource. Make sure your layer's "
                                          "attribute tables are closed. Continue?"),
                                 QMessageBox.Yes | QMessageBox.No )
      if res == QMessageBox.No:
        return

    self.buttonOk.setEnabled( False )
    self.testThread = geometryThread( self.iface.mainWindow(), self, self.myFunction,
                                      vlayer, myParam, myField, self.shapefileName, self.encoding,
                                      self.cmbCalcType.currentIndex(), self.chkWriteShapefile.isChecked(),
                                      self.chkByFeatures.isChecked(), self.chkUseSelection.isChecked() )
    QObject.connect( self.testThread, SIGNAL( "runFinished( PyQt_PyObject )" ), self.runFinishedFromThread )
    QObject.connect( self.testThread, SIGNAL( "runStatus( PyQt_PyObject )" ), self.runStatusFromThread )
    QObject.connect( self.testThread, SIGNAL( "runRange( PyQt_PyObject )" ), self.runRangeFromThread )
    self.cancel_close.setText( self.tr( "Cancel" ) )
    QObject.connect( self.cancel_close, SIGNAL( "clicked()" ), self.cancelThread )
    self.testThread.start()

  def cancelThread( self ):
    self.testThread.stop()
    self.buttonOk.setEnabled( True )

  def runFinishedFromThread( self, success ):
    self.testThread.stop()
    self.buttonOk.setEnabled( True )
    extra = ""
    if success == "math_error":
      QMessageBox.warning( self, self.tr( "Geometry" ),
                           self.tr( "Error processing specified tolerance!\nPlease choose larger tolerance..." ) )
      if not QgsVectorFileWriter.deleteShapeFile( self.shapefileName ):
        QMessageBox.warning( self, self.tr( "Geometry" ),
                             self.tr( "Unable to delete incomplete shapefile." ) )
    elif success == "attr_error":
      QMessageBox.warning( self, self.tr( "Geometry" ),
                           self.tr( "At least two features must have same attribute value!\nPlease choose another field..." ) )
      if not QgsVectorFileWriter.deleteShapeFile( self.shapefileName ):
        QMessageBox.warning( self, self.tr( "Geometry" ),
                             self.tr( "Unable to delete incomplete shapefile." ) )
    else:
      if success == "valid_error":
        extra = self.tr( "One or more features in the output layer may have invalid "
                         + "geometry, please check using the check validity tool\n" )
        success = True
      self.cancel_close.setText( "Close" )
      QObject.disconnect( self.cancel_close, SIGNAL( "clicked()" ), self.cancelThread )
      if success:
        if ( self.myFunction == 5 and self.chkWriteShapefile.isChecked() ) or self.myFunction != 5:
          if self.addToCanvasCheck.isChecked():
            addCanvasCheck = ftools_utils.addShapeToCanvas(unicode(self.shapefileName))
            if not addCanvasCheck:
              QMessageBox.warning( self, self.tr("Geometry"), self.tr( "Error loading output shapefile:\n%s" ) % ( unicode( self.shapefileName ) ))
            self.populateLayers()
          else:
            QMessageBox.information(self, self.tr("Geometry"),self.tr("Created output shapefile:\n%s\n%s" ) % ( unicode( self.shapefileName ), extra ))
        else:
          QMessageBox.information( self, self.tr( "Geometry" ),
                                   self.tr( "Layer '{0}' updated" ).format( self.inShape.currentText() ) )
      else:
        QMessageBox.warning( self, self.tr( "Geometry" ), self.tr( "Error writing output shapefile." ) )

  def runStatusFromThread( self, status ):
    self.progressBar.setValue( status )

  def runRangeFromThread( self, range_vals ):
    self.progressBar.setRange( range_vals[ 0 ], range_vals[ 1 ] )

class geometryThread( QThread ):
  def __init__( self, parentThread, parentObject, function, vlayer, myParam,
                myField, myName, myEncoding, myCalcType, myNewShape, myByFeatures,
                myUseSelection ):
    QThread.__init__( self, parentThread )
    self.parent = parentObject
    self.running = False
    self.myFunction = function
    self.vlayer = vlayer
    self.myParam = myParam
    self.myField = myField
    self.myName = myName
    self.myEncoding = myEncoding
    self.myCalcType = myCalcType
    self.writeShape = myNewShape
    self.byFeatures = myByFeatures
    self.useSelection = myUseSelection

  def run( self ):
    self.running = True
    if self.myFunction == 1: # Singleparts to multipart
      success = self.single_to_multi()
    elif self.myFunction == 2: # Multipart to singleparts
      success = self.multi_to_single()
    elif self.myFunction == 3: # Extract nodes
      success = self.extract_nodes()
    elif self.myFunction == 4: # Polygons to lines
      success = self.polygons_to_lines()
    elif self.myFunction == 5: # Export/Add geometry columns
      success = self.export_geometry_info()
    # note that 6 used to be associated with simplify_geometry
    elif self.myFunction == 7: # Polygon centroids
      success = self.polygon_centroids()
    elif self.myFunction == 8: # Delaunay triangulation
      success = self.delaunay_triangulation()
    elif self.myFunction == 9: # Polygon from layer extent
      if self.byFeatures:
        success = self.feature_extent()
      else:
        success = self.layer_extent()
    elif self.myFunction == 10: # Voronoi Polygons
      success = self.voronoi_polygons()
    elif self.myFunction == 11: # Lines to polygons
      success = self.lines_to_polygons()
    self.emit( SIGNAL( "runFinished( PyQt_PyObject )" ), success )
    self.emit( SIGNAL( "runStatus( PyQt_PyObject )" ), 0 )

  def stop( self ):
    self.running = False

  def single_to_multi( self ):
    vprovider = self.vlayer.dataProvider()
    allValid = True
    geomType = self.singleToMultiGeom( vprovider.geometryType() )
    writer = QgsVectorFileWriter( self.myName, self.myEncoding, vprovider.fields(),
                                  geomType, vprovider.crs() )
    inFeat = QgsFeature()
    outFeat = QgsFeature()
    inGeom = QgsGeometry()
    outGeom = QgsGeometry()
    index = vprovider.fieldNameIndex( self.myField )
    if not index == -1:
      unique = ftools_utils.getUniqueValues( vprovider, int( index ) )
    else:
      unique = [ "" ]
    nFeat = vprovider.featureCount() * len( unique )
    nElement = 0
    self.emit( SIGNAL( "runStatus( PyQt_PyObject )" ), 0 )
    self.emit( SIGNAL( "runRange( PyQt_PyObject )" ), ( 0, nFeat ) )
    merge_all = self.myField == "--- " + self.tr( "Merge all" ) + " ---"
    if not len( unique ) == self.vlayer.featureCount() or merge_all:
      for i in unique:
        # Strip spaces for strings, so "  A " and "A" will be grouped
        # TODO: Make this optional (opt-out to keep it easy for beginners)
        if isinstance( i, basestring ):
          iMod = i.strip()
        else:
          iMod = i
        multi_feature= []
        first = True
        fit = vprovider.getFeatures()
        while fit.nextFeature( inFeat ):
          atMap = inFeat.attributes()
          if not merge_all:
            idVar = atMap[ index ]
            if isinstance( idVar, basestring ):
              idVarMod = idVar.strip()
            else:
              idVarMod = idVar
          else:
            idVarMod = ""
          if idVarMod == iMod or merge_all:
            if first:
              atts = atMap
              first = False
            inGeom = QgsGeometry( inFeat.geometry() )
            vType = inGeom.type()
            feature_list = self.extractAsMulti( inGeom )
            multi_feature.extend( feature_list )
          nElement += 1
          self.emit( SIGNAL( "runStatus( PyQt_PyObject )" ),  nElement )
        if not first:
          outFeat.setAttributes( atts )
          outGeom = QgsGeometry( self.convertGeometry( multi_feature, vType ) )
          if not outGeom.isGeosValid():
            allValid = "valid_error"
          outFeat.setGeometry( outGeom )
          writer.addFeature( outFeat )
      del writer
    else:
      return "attr_error"
    return allValid

  def multi_to_single( self ):
    vprovider = self.vlayer.dataProvider()
    geomType = self.multiToSingleGeom( vprovider.geometryType() )
    writer = QgsVectorFileWriter( self.myName, self.myEncoding, vprovider.fields(),
                                  geomType, vprovider.crs() )
    inFeat = QgsFeature()
    outFeat = QgsFeature()
    inGeom = QgsGeometry()
    nFeat = vprovider.featureCount()
    nElement = 0
    self.emit( SIGNAL( "runStatus( PyQt_PyObject )" ), 0 )
    self.emit( SIGNAL( "runRange( PyQt_PyObject )" ), ( 0, nFeat ) )
    fit = vprovider.getFeatures()
    while fit.nextFeature( inFeat ):
      nElement += 1
      self.emit( SIGNAL( "runStatus( PyQt_PyObject )" ), nElement )
      inGeom = inFeat.geometry()
      atMap = inFeat.attributes()
      featList = self.extractAsSingle( inGeom )
      outFeat.setAttributes( atMap )
      for i in featList:
        outFeat.setGeometry( i )
        writer.addFeature( outFeat )
    del writer
    return True

  def extract_nodes( self ):
    vprovider = self.vlayer.dataProvider()
    writer = QgsVectorFileWriter( self.myName, self.myEncoding, vprovider.fields(),
                                  QGis.WKBPoint, vprovider.crs() )
    inFeat = QgsFeature()
    outFeat = QgsFeature()
    inGeom = QgsGeometry()
    outGeom = QgsGeometry()
    nFeat = vprovider.featureCount()
    nElement = 0
    self.emit( SIGNAL( "runStatus( PyQt_PyObject )" ), 0 )
    self.emit( SIGNAL( "runRange( PyQt_PyObject )" ), ( 0, nFeat ) )
    fit = vprovider.getFeatures()
    while fit.nextFeature( inFeat ):
      nElement += 1
      self.emit( SIGNAL( "runStatus( PyQt_PyObject )" ),  nElement )
      inGeom = inFeat.geometry()
      atMap = inFeat.attributes()
      pointList = ftools_utils.extractPoints( inGeom )
      outFeat.setAttributes( atMap )
      for i in pointList:
        outFeat.setGeometry( outGeom.fromPoint( i ) )
        writer.addFeature( outFeat )
    del writer
    return True

  def polygons_to_lines( self ):
    vprovider = self.vlayer.dataProvider()
    writer = QgsVectorFileWriter( self.myName, self.myEncoding, vprovider.fields(),
                                  QGis.WKBLineString, vprovider.crs() )
    inFeat = QgsFeature()
    outFeat = QgsFeature()
    inGeom = QgsGeometry()
    outGeom = QgsGeometry()
    nFeat = vprovider.featureCount()
    nElement = 0
    self.emit( SIGNAL( "runStatus( PyQt_PyObject )" ), 0)
    self.emit( SIGNAL( "runRange( PyQt_PyObject )" ), ( 0, nFeat ) )

    fit = vprovider.getFeatures()
    while fit.nextFeature( inFeat ):
      nElement += 1
      self.emit( SIGNAL( "runStatus( PyQt_PyObject )" ),  nElement )
      inGeom = inFeat.geometry()
      atMap = inFeat.attributes()
      lineList = self.extractAsLine( inGeom )
      outFeat.setAttributes( atMap )
      for h in lineList:
        outFeat.setGeometry( outGeom.fromPolyline( h ) )
        writer.addFeature( outFeat )
    del writer
    return True

  def lines_to_polygons( self ):
    vprovider = self.vlayer.dataProvider()
    writer = QgsVectorFileWriter( self.myName, self.myEncoding, vprovider.fields(),
                                  QGis.WKBPolygon, vprovider.crs() )
    inFeat = QgsFeature()
    outFeat = QgsFeature()
    nFeat = vprovider.featureCount()
    nElement = 0
    self.emit( SIGNAL( "runStatus( PyQt_PyObject )" ), 0)
    self.emit( SIGNAL( "runRange( PyQt_PyObject )" ), ( 0, nFeat ) )

    fit = vprovider.getFeatures()
    while fit.nextFeature( inFeat ):
      outGeomList = []
      nElement += 1
      self.emit( SIGNAL( "runStatus( PyQt_PyObject )" ),  nElement )
      if inFeat.geometry().isMultipart():
        outGeomList = inFeat.geometry().asMultiPolyline()
      else:
        outGeomList.append( inFeat.geometry().asPolyline() )
      polyGeom = self.remove_bad_lines( outGeomList )
      if len( polyGeom ) != 0:
        outFeat.setGeometry( QgsGeometry.fromPolygon( polyGeom ) )
        atMap = inFeat.attributes()
        outFeat.setAttributes( atMap )
        writer.addFeature( outFeat )
    del writer
    return True

  def export_geometry_info( self ):
    ellips = None
    crs = None
    coordTransform = None

    # calculate with:
    # 0 - layer CRS
    # 1 - project CRS
    # 2 - ellipsoidal
    if self.myCalcType == 2:
      settings = QSettings()
      ellips = settings.value( "/qgis/measure/ellipsoid", "WGS84" )
      crs = self.vlayer.crs().srsid()
    elif self.myCalcType == 1:
      mapCRS = self.parent.iface.mapCanvas().mapRenderer().destinationCrs()
      layCRS = self.vlayer.crs()
      coordTransform = QgsCoordinateTransform( layCRS, mapCRS )

    inFeat = QgsFeature()
    outFeat = QgsFeature()
    inGeom = QgsGeometry()
    nElement = 0

    vprovider = self.vlayer.dataProvider()
    self.emit( SIGNAL( "runStatus( PyQt_PyObject )" ), 0)
    self.emit( SIGNAL( "runRange( PyQt_PyObject )" ), ( 0, vprovider.featureCount() ) )

    ( fields, index1, index2 ) = self.checkMeasurementFields( self.vlayer, not self.writeShape )

    if self.writeShape:
      writer = QgsVectorFileWriter( self.myName, self.myEncoding, fields,
                                    vprovider.geometryType(), vprovider.crs() )

    fit = vprovider.getFeatures()
    while fit.nextFeature(inFeat):
      self.emit( SIGNAL( "runStatus( PyQt_PyObject )" ),  nElement )
      nElement += 1
      inGeom = inFeat.geometry()

      if self.myCalcType == 1:
        inGeom.transform( coordTransform )

      ( attr1, attr2 ) = self.simpleMeasure( inGeom, self.myCalcType, ellips, crs )

      if self.writeShape:
        outFeat.setGeometry( inGeom )
        atMap = inFeat.attributes()
        maxIndex = index1 if index1 > index2 else index2
        if maxIndex >= len(atMap):
          atMap += [ "" ] * ( index2+1 - len(atMap) )
        atMap[ index1 ] = attr1
        if index1 != index2:
          atMap[ index2 ] = attr2
        outFeat.setAttributes( atMap )
        writer.addFeature( outFeat )
      else:
        changeMap = {}
        changeMap[ inFeat.id() ] = {}
        changeMap[ inFeat.id() ][ index1 ] = attr1
        if index1!=index2:
          changeMap[ inFeat.id() ][ index2 ] = attr2
        vprovider.changeAttributeValues( changeMap )
        self.vlayer.updateFields()

    if self.writeShape:
      del writer

    return True

  def polygon_centroids( self ):
    vprovider = self.vlayer.dataProvider()
    writer = QgsVectorFileWriter( self.myName, self.myEncoding, vprovider.fields(),
                                  QGis.WKBPoint, vprovider.crs() )
    inFeat = QgsFeature()
    outFeat = QgsFeature()
    nFeat = vprovider.featureCount()
    nElement = 0
    self.emit( SIGNAL( "runStatus( PyQt_PyObject )" ), 0 )
    self.emit( SIGNAL( "runRange( PyQt_PyObject )" ), ( 0, nFeat ) )
    fit = vprovider.getFeatures()
    while fit.nextFeature( inFeat ):
      nElement += 1
      self.emit( SIGNAL( "runStatus( PyQt_PyObject )" ),  nElement )
      inGeom = inFeat.geometry()
      atMap = inFeat.attributes()
      outGeom = inGeom.centroid()
      if outGeom is None:
        return "math_error"
      outFeat.setAttributes( atMap )
      outFeat.setGeometry( QgsGeometry( outGeom ) )
      writer.addFeature( outFeat )
    del writer
    return True

  def delaunay_triangulation( self ):
    import voronoi
    from sets import Set
    vprovider = self.vlayer.dataProvider()

    fields = QgsFields()
    fields.append( QgsField( "POINTA", QVariant.Double ) )
    fields.append( QgsField( "POINTB", QVariant.Double ) )
    fields.append( QgsField( "POINTC", QVariant.Double ) )

    writer = QgsVectorFileWriter( self.myName, self.myEncoding, fields,
                                  QGis.WKBPolygon, vprovider.crs() )
    inFeat = QgsFeature()
    c = voronoi.Context()
    pts = []
    ptDict = {}
    ptNdx = -1
    fit = vprovider.getFeatures()
    while fit.nextFeature( inFeat ):
      geom = QgsGeometry( inFeat.geometry() )
      point = geom.asPoint()
      x = point.x()
      y = point.y()
      pts.append( ( x, y ) )
      ptNdx +=1
      ptDict[ptNdx] = inFeat.id()
    if len(pts) < 3:
      return False
    uniqueSet = Set( item for item in pts )
    ids = [ pts.index( item ) for item in uniqueSet ]
    sl = voronoi.SiteList( [ voronoi.Site( *i ) for i in uniqueSet ] )
    c.triangulate = True
    voronoi.voronoi( sl, c )
    triangles = c.triangles
    feat = QgsFeature()
    nFeat = len( triangles )
    nElement = 0
    self.emit( SIGNAL( "runStatus( PyQt_PyObject )" ), 0 )
    self.emit( SIGNAL( "runRange( PyQt_PyObject )" ), ( 0, nFeat ) )
    for triangle in triangles:
      indicies = list( triangle )
      indicies.append( indicies[ 0 ] )
      polygon = []
      attrs = []
      step = 0
      for index in indicies:
        vprovider.getFeatures( QgsFeatureRequest().setFilterFid( ptDict[ ids[ index ] ] ) ).nextFeature( inFeat )
        geom = QgsGeometry( inFeat.geometry() )
        point = QgsPoint( geom.asPoint() )
        polygon.append( point )
        if step <= 3:
          attrs.append(ids[ index ] )
        step += 1
      feat.setAttributes(attrs)
      geometry = QgsGeometry().fromPolygon( [ polygon ] )
      feat.setGeometry( geometry )
      writer.addFeature( feat )
      nElement += 1
      self.emit( SIGNAL( "runStatus( PyQt_PyObject )" ), nElement )
    del writer
    return True

  def voronoi_polygons( self ):
    vprovider = self.vlayer.dataProvider()
    writer = QgsVectorFileWriter( self.myName, self.myEncoding, vprovider.fields(),
                                  QGis.WKBPolygon, vprovider.crs() )
    inFeat = QgsFeature()
    outFeat = QgsFeature()
    extent = self.vlayer.extent()
    extraX = extent.height() * ( self.myParam / 100.00 )
    extraY = extent.width() * ( self.myParam / 100.00 )
    height = extent.height()
    width = extent.width()
    c = voronoi.Context()
    pts = []
    ptDict = {}
    ptNdx = -1
    fit = vprovider.getFeatures()
    while fit.nextFeature( inFeat ):
      geom = QgsGeometry( inFeat.geometry() )
      point = geom.asPoint()
      x = point.x() - extent.xMinimum()
      y = point.y() - extent.yMinimum()
      pts.append( ( x, y ) )
      ptNdx +=1
      ptDict[ ptNdx ] = inFeat.id()
    self.vlayer = None
    if len( pts ) < 3:
      return False
    uniqueSet = Set( item for item in pts )
    ids = [ pts.index( item ) for item in uniqueSet ]
    sl = voronoi.SiteList( [ voronoi.Site( i[ 0 ], i[ 1 ], sitenum=j ) for j, i in enumerate( uniqueSet ) ] )
    voronoi.voronoi( sl, c )
    inFeat = QgsFeature()
    nFeat = len( c.polygons )
    nElement = 0
    self.emit( SIGNAL( "runStatus( PyQt_PyObject )" ), 0 )
    self.emit( SIGNAL( "runRange( PyQt_PyObject )" ), ( 0, nFeat ) )
    for site, edges in c.polygons.iteritems():
      vprovider.getFeatures( QgsFeatureRequest().setFilterFid( ptDict[ ids[ site ] ] ) ).nextFeature( inFeat )
      lines = self.clip_voronoi( edges, c, width, height, extent, extraX, extraY )
      geom = QgsGeometry.fromMultiPoint( lines )
      geom = QgsGeometry( geom.convexHull() )
      outFeat.setGeometry( geom )
      outFeat.setAttributes( inFeat.attributes() )
      writer.addFeature( outFeat )
      nElement += 1
      self.emit( SIGNAL( "runStatus( PyQt_PyObject )" ), nElement )
    del writer
    return True

  def clip_voronoi( self, edges, c, width, height, extent, exX, exY ):
    """ Clip voronoi function based on code written for Inkscape
        Copyright (C) 2010 Alvin Penner, penner@vaxxine.com
    """
    def clip_line( x1, y1, x2, y2, w, h, x, y ):
      if x1 < 0 - x and x2 < 0 - x:
        return [ 0, 0, 0, 0 ]
      if x1 > w + x and x2 > w + x:
        return [ 0, 0, 0, 0 ]
      if x1 < 0 - x:
        y1 = ( y1 * x2 - y2 * x1 ) / ( x2 - x1 )
        x1 = 0 - x
      if x2 < 0 - x:
        y2 = ( y1 * x2 - y2 * x1 ) / ( x2 - x1 )
        x2 = 0 - x
      if x1 > w + x:
        y1 = y1 + ( w + x - x1 ) * ( y2 - y1 ) / ( x2 - x1 )
        x1 = w + x
      if x2 > w + x:
        y2 = y1 + ( w + x - x1 ) *( y2 - y1 ) / ( x2 - x1 )
        x2 = w + x
      if y1 < 0 - y and y2 < 0 - y:
        return [ 0, 0, 0, 0 ]
      if y1 > h + y and y2 > h + y:
        return [ 0, 0, 0, 0 ]
      if x1 == x2 and y1 == y2:
        return [ 0, 0, 0, 0 ]
      if y1 < 0 - y:
        x1 = ( x1 * y2 - x2 * y1 ) / ( y2 - y1 )
        y1 = 0 - y
      if y2 < 0 - y:
        x2 = ( x1 * y2 - x2 * y1 ) / ( y2 - y1 )
        y2 = 0 - y
      if y1 > h + y:
        x1 = x1 + ( h + y - y1 ) * ( x2 - x1 ) / ( y2 - y1 )
        y1 = h + y
      if y2 > h + y:
        x2 = x1 + ( h + y - y1) * ( x2 - x1 ) / ( y2 - y1 )
        y2 = h + y
      return [ x1, y1, x2, y2 ]
    lines = []
    hasXMin = False
    hasYMin = False
    hasXMax = False
    hasYMax = False
    for edge in edges:
      if edge[ 1 ] >= 0 and edge[ 2 ] >= 0:       # two vertices
          [ x1, y1, x2, y2 ] = clip_line( c.vertices[ edge[ 1 ] ][ 0 ], c.vertices[ edge[ 1 ] ][ 1 ], c.vertices[ edge[ 2 ] ][ 0 ], c.vertices[ edge[ 2 ] ][ 1 ], width, height, exX, exY )
      elif edge[ 1 ] >= 0:                      # only one vertex
        if c.lines[ edge[ 0 ] ][ 1 ] == 0:      # vertical line
          xtemp = c.lines[ edge[ 0 ] ][ 2 ] / c.lines[ edge[ 0 ] ][ 0 ]
          if c.vertices[ edge[ 1 ] ][ 1 ] > ( height + exY ) / 2:
            ytemp = height + exY
          else:
            ytemp = 0 - exX
        else:
          xtemp = width + exX
          ytemp = ( c.lines[ edge[ 0 ] ][ 2 ] - ( width + exX ) * c.lines[ edge[ 0 ] ][ 0 ] ) / c.lines[ edge[ 0 ] ][ 1 ]
        [ x1, y1, x2, y2 ] = clip_line( c.vertices[ edge[ 1 ] ][ 0 ], c.vertices[ edge[ 1 ] ][ 1 ], xtemp, ytemp, width, height, exX, exY )
      elif edge[ 2 ] >= 0:                       # only one vertex
        if c.lines[ edge[ 0 ] ][ 1 ] == 0:       # vertical line
          xtemp = c.lines[ edge[ 0 ] ][ 2 ] / c.lines[ edge[ 0 ] ][ 0 ]
          if c.vertices[ edge[ 2 ] ][ 1 ] > ( height + exY ) / 2:
            ytemp = height + exY
          else:
            ytemp = 0.0 - exY
        else:
          xtemp = 0.0 - exX
          ytemp = c.lines[ edge[ 0 ] ][ 2 ] / c.lines[ edge[ 0 ] ][ 1 ]
        [ x1, y1, x2, y2 ] = clip_line( xtemp, ytemp, c.vertices[ edge[ 2 ] ][ 0 ], c.vertices[ edge[ 2 ] ][ 1 ], width, height, exX, exY )
      if x1 or x2 or y1 or y2:
        lines.append( QgsPoint( x1 + extent.xMinimum(), y1 + extent.yMinimum() ) )
        lines.append( QgsPoint( x2 + extent.xMinimum(), y2 + extent.yMinimum() ) )
        if 0 - exX in ( x1, x2 ):
          hasXMin = True
        if 0 - exY in ( y1, y2 ):
          hasYMin = True
        if height + exY in ( y1, y2 ):
          hasYMax = True
        if width + exX in ( x1, x2 ):
          hasXMax = True
    if hasXMin:
      if hasYMax:
        lines.append( QgsPoint( extent.xMinimum() - exX, height + extent.yMinimum() + exY ) )
      if hasYMin:
        lines.append( QgsPoint( extent.xMinimum() - exX, extent.yMinimum() - exY ) )
    if hasXMax:
      if hasYMax:
        lines.append( QgsPoint( width + extent.xMinimum() + exX, height + extent.yMinimum() + exY ) )
      if hasYMin:
        lines.append( QgsPoint( width + extent.xMinimum() + exX, extent.yMinimum() - exY ) )
    return lines

  def layer_extent( self ):
    self.emit( SIGNAL( "runStatus( PyQt_PyObject )" ), 0 )
    self.emit( SIGNAL( "runRange( PyQt_PyObject )" ), ( 0, 0 ) )

    fields = QgsFields()
    fields.append( QgsField( "MINX", QVariant.Double ) )
    fields.append( QgsField( "MINY", QVariant.Double ) )
    fields.append( QgsField( "MAXX", QVariant.Double ) )
    fields.append( QgsField( "MAXY", QVariant.Double ) )
    fields.append( QgsField( "CNTX", QVariant.Double ) )
    fields.append( QgsField( "CNTY", QVariant.Double ) )
    fields.append( QgsField( "AREA", QVariant.Double ) )
    fields.append( QgsField( "PERIM", QVariant.Double ) )
    fields.append( QgsField( "HEIGHT", QVariant.Double ) )
    fields.append( QgsField( "WIDTH", QVariant.Double ) )

    writer = QgsVectorFileWriter( self.myName, self.myEncoding, fields,
                                  QGis.WKBPolygon, self.vlayer.crs() )
    rect = self.vlayer.extent()
    minx = rect.xMinimum()
    miny = rect.yMinimum()
    maxx = rect.xMaximum()
    maxy = rect.yMaximum()
    height = rect.height()
    width = rect.width()
    cntx = minx + ( width / 2.0 )
    cnty = miny + ( height / 2.0 )
    area = width * height
    perim = ( 2 * width ) + (2 * height )
    rect = [ QgsPoint( minx, miny ),
             QgsPoint( minx, maxy ),
             QgsPoint( maxx, maxy ),
             QgsPoint( maxx, miny ),
             QgsPoint( minx, miny ) ]
    geometry = QgsGeometry().fromPolygon( [ rect ] )
    feat = QgsFeature()
    feat.setGeometry( geometry )
    feat.setAttributes( [ minx,
                          miny,
                          maxx,
                          maxy,
                          cntx,
                          cnty,
                          area,
                          perim,
                          height,
                          width ] )
    writer.addFeature( feat )
    self.emit( SIGNAL( "runRange( PyQt_PyObject )" ), ( 0, 100 ) )
    self.emit( SIGNAL( "runStatus( PyQt_PyObject )" ),  0 )
    del writer

    return True

  def feature_extent( self ):
    vprovider = self.vlayer.dataProvider()
    self.emit( SIGNAL( "runStatus( PyQt_PyObject )" ), 0 )

    fields = QgsFields()
    fields.append( QgsField( "MINX", QVariant.Double ) )
    fields.append( QgsField( "MINY", QVariant.Double ) )
    fields.append( QgsField( "MAXX", QVariant.Double ) )
    fields.append( QgsField( "MAXY", QVariant.Double ) )
    fields.append( QgsField( "CNTX", QVariant.Double ) )
    fields.append( QgsField( "CNTY", QVariant.Double ) )
    fields.append( QgsField( "AREA", QVariant.Double ) )
    fields.append( QgsField( "PERIM", QVariant.Double ) )
    fields.append( QgsField( "HEIGHT", QVariant.Double ) )
    fields.append( QgsField( "WIDTH", QVariant.Double ) )

    writer = QgsVectorFileWriter( self.myName, self.myEncoding, fields,
                                  QGis.WKBPolygon, self.vlayer.crs() )
    inFeat = QgsFeature()
    outFeat = QgsFeature()
    nElement = 0

    if self.useSelection:
      self.emit( SIGNAL( "runRange( PyQt_PyObject )" ), (0, self.vlayer.selectedFeatureCount() ) )
      for inFeat in self.vlayer.selectedFeatures():
        self.emit( SIGNAL( "runStatus( PyQt_PyObject )" ),  nElement )
        nElement += 1

        rect = inFeat.geometry().boundingBox()
        minx = rect.xMinimum()
        miny = rect.yMinimum()
        maxx = rect.xMaximum()
        maxy = rect.yMaximum()
        height = rect.height()
        width = rect.width()
        cntx = minx + ( width / 2.0 )
        cnty = miny + ( height / 2.0 )
        area = width * height
        perim = ( 2 * width ) + ( 2 * height )
        rect = [ QgsPoint( minx, miny ),
                 QgsPoint( minx, maxy ),
                 QgsPoint( maxx, maxy ),
                 QgsPoint( maxx, miny ),
                 QgsPoint( minx, miny ) ]
        geometry = QgsGeometry().fromPolygon( [ rect ] )

        outFeat.setGeometry( geometry )
        outFeat.setAttributes( [ minx,
                                 miny,
                                 maxx,
                                 maxy,
                                 cntx,
                                 cnty,
                                 area,
                                 perim,
                                 height,
                                 width ] )
        writer.addFeature( outFeat )
    else:
      self.emit( SIGNAL( "runRange( PyQt_PyObject )" ), ( 0, vprovider.featureCount() ) )
      fit = vprovider.getFeatures()
      while fit.nextFeature( inFeat ):
        self.emit( SIGNAL( "runStatus( PyQt_PyObject )" ),  nElement )
        nElement += 1

        rect = inFeat.geometry().boundingBox()
        minx = rect.xMinimum()
        miny = rect.yMinimum()
        maxx = rect.xMaximum()
        maxy = rect.yMaximum()
        height = rect.height()
        width = rect.width()
        cntx = minx + ( width / 2.0 )
        cnty = miny + ( height / 2.0 )
        area = width * height
        perim = ( 2 * width ) + ( 2 * height )
        rect = [ QgsPoint( minx, miny ),
                 QgsPoint( minx, maxy ),
                 QgsPoint( maxx, maxy ),
                 QgsPoint( maxx, miny ),
                 QgsPoint( minx, miny ) ]
        geometry = QgsGeometry().fromPolygon( [ rect ] )

        outFeat.setGeometry( geometry )
        outFeat.setAttributes( [ minx,
                                 miny,
                                 maxx,
                                 maxy,
                                 cntx,
                                 cnty,
                                 area,
                                 perim,
                                 height,
                                 width ] )
        writer.addFeature( outFeat )

    del writer
    return True

  def simpleMeasure( self, inGeom, calcType, ellips, crs ):
    if inGeom.wkbType() in ( QGis.WKBPoint, QGis.WKBPoint25D ):
      pt = inGeom.asPoint()
      attr1 = pt.x()
      attr2 = pt.y()
    elif inGeom.wkbType() in ( QGis.WKBMultiPoint, QGis.WKBMultiPoint25D ):
      pt = inGeom.asMultiPoint()
      attr1 = pt[ 0 ].x()
      attr2 = pt[ 0 ].y()
    else:
      measure = QgsDistanceArea()

      if calcType == 2:
        measure.setSourceCrs( crs )
        measure.setEllipsoid( ellips )
        measure.setEllipsoidalMode( True )

      attr1 = measure.measure( inGeom )
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

  def doubleFieldIndex( self, name, desc, fieldList ):
    i = 0
    for f in fieldList:
      if name == f.name().upper():
        return (i, fieldList )
      i += 1

    fieldList.append( QgsField( name, QVariant.Double, "double precision", 21, 6, desc ) )
    return ( len(fieldList)-1, fieldList )

  def checkMeasurementFields( self, vlayer, add ):
    vprovider = vlayer.dataProvider()
    geomType = vlayer.geometryType()
    fieldList = vprovider.fields()

    idx = len(fieldList)

    if geomType == QGis.Polygon:
      (index1, fieldList) = self.doubleFieldIndex( "AREA", self.tr( "Polygon area" ), fieldList )
      (index2, fieldList) = self.doubleFieldIndex( "PERIMETER", self.tr( "Polygon perimeter" ), fieldList )
    elif geomType == QGis.Line:
      (index1, fieldList) = self.doubleFieldIndex( "LENGTH", self.tr( "Line length" ), fieldList )
      index2 = index1
    else:
      (index1, fieldList) = self.doubleFieldIndex( "XCOORD", self.tr( "Point x ordinate" ), fieldList )
      (index2, fieldList) = self.doubleFieldIndex( "YCOORD", self.tr( "Point y ordinate" ), fieldList )

    if add and idx<len(fieldList):
      newFields = []
      for i in range(idx,len(fieldList)):
        newFields.append(fieldList[i])
      vprovider.addAttributes( newFields )

    return ( fieldList, index1, index2 )

  def extractAsLine( self, geom ):
    multi_geom = QgsGeometry()
    temp_geom = []
    if geom.type() == 2:
      if geom.isMultipart():
        multi_geom = geom.asMultiPolygon()
        for i in multi_geom:
          temp_geom.extend( i )
      else:
        multi_geom = geom.asPolygon()
        temp_geom = multi_geom
      return temp_geom
    else:
      return []

  def remove_bad_lines( self, lines ):
    temp_geom = []
    if len( lines ) == 1:
      if len( lines[ 0 ] ) > 2:
        temp_geom = lines
      else:
        temp_geom = []
    else:
      temp_geom = [ elem for elem in lines if len( elem ) > 2 ]
    return temp_geom

  def singleToMultiGeom( self, wkbType ):
    try:
      if wkbType in ( QGis.WKBPoint, QGis.WKBMultiPoint,
                      QGis.WKBPoint25D, QGis.WKBMultiPoint25D ):
        return QGis.WKBMultiPoint
      elif wkbType in ( QGis.WKBLineString, QGis.WKBMultiLineString,
                        QGis.WKBMultiLineString25D, QGis.WKBLineString25D ):
        return QGis.WKBMultiLineString
      elif wkbType in ( QGis.WKBPolygon, QGis.WKBMultiPolygon,
                        QGis.WKBMultiPolygon25D, QGis.WKBPolygon25D ):
        return QGis.WKBMultiPolygon
      else:
        return QGis.WKBUnknown
    except Exception, err:
      print str( err )

  def multiToSingleGeom( self, wkbType ):
    try:
      if wkbType in ( QGis.WKBPoint, QGis.WKBMultiPoint,
                      QGis.WKBPoint25D, QGis.WKBMultiPoint25D ):
        return QGis.WKBPoint
      elif wkbType in ( QGis.WKBLineString, QGis.WKBMultiLineString,
                        QGis.WKBMultiLineString25D, QGis.WKBLineString25D ):
        return QGis.WKBLineString
      elif wkbType in ( QGis.WKBPolygon, QGis.WKBMultiPolygon,
                        QGis.WKBMultiPolygon25D, QGis.WKBPolygon25D ):
        return QGis.WKBPolygon
      else:
        return QGis.WKBUnknown
    except Exception, err:
      print str( err )

  def extractAsSingle( self, geom ):
    multi_geom = QgsGeometry()
    temp_geom = []
    if geom.type() == 0:
      if geom.isMultipart():
        multi_geom = geom.asMultiPoint()
        for i in multi_geom:
          temp_geom.append( QgsGeometry().fromPoint ( i ) )
      else:
        temp_geom.append( geom )
    elif geom.type() == 1:
      if geom.isMultipart():
        multi_geom = geom.asMultiPolyline()
        for i in multi_geom:
          temp_geom.append( QgsGeometry().fromPolyline( i ) )
      else:
        temp_geom.append( geom )
    elif geom.type() == 2:
      if geom.isMultipart():
        multi_geom = geom.asMultiPolygon()
        for i in multi_geom:
          temp_geom.append( QgsGeometry().fromPolygon( i ) )
      else:
        temp_geom.append( geom )
    return temp_geom

  def extractAsMulti( self, geom ):
    if geom.type() == 0:
      if geom.isMultipart():
        return geom.asMultiPoint()
      else:
        return [ geom.asPoint() ]
    elif geom.type() == 1:
      if geom.isMultipart():
        return geom.asMultiPolyline()
      else:
        return [ geom.asPolyline() ]
    else:
      if geom.isMultipart():
        return geom.asMultiPolygon()
      else:
        return [ geom.asPolygon() ]

  def convertGeometry( self, geom_list, vType ):
    if vType == 0:
      return QgsGeometry().fromMultiPoint( geom_list )
    elif vType == 1:
      return QgsGeometry().fromMultiPolyline( geom_list )
    else:
      return QgsGeometry().fromMultiPolygon( geom_list )
