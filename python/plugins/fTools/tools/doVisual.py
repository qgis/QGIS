from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from ui_frmVisual import Ui_Dialog
import ftools_utils
import math

class VisualDialog( QDialog, Ui_Dialog ):
  def __init__( self, iface, function ):
    QDialog.__init__( self )
    self.iface = iface
    self.setupUi( self )
    self.myFunction = function
    if self.myFunction == 2 or self.myFunction == 3:
      QObject.connect( self.inShape, SIGNAL( "currentIndexChanged(QString)" ), self.update )
    self.manageGui()
    self.cancel_close = self.buttonBox_2.button( QDialogButtonBox.Close )
    self.progressBar.setValue( 0 )
    
  def keyPressEvent( self, e ):
    '''
    Reimplemented key press event:
    '''
    if ( e.modifiers() == Qt.ControlModifier or e.modifiers() == Qt.MetaModifier ) and e.key() == Qt.Key_C:
      selection = self.lstUnique.selectedItems()
      items = QString()
      for item in selection:
        items.append( item.text() + "\n" )
      if not items.isEmpty():
        clip_board = QApplication.clipboard()
        clip_board.setText( items )
    else:
      QDialog.keyPressEvent( self, e )

  def update( self ):
    self.cmbField.clear()
    inputLayer = unicode( self.inShape.currentText() )
    if inputLayer != "":
      changedLayer = ftools_utils.getVectorLayerByName( inputLayer )
      changedField = changedLayer.dataProvider().fields()
      # for Basic statistics (with or without selection)
      if self.myFunction == 3:
        if changedLayer.selectedFeatureCount() != 0:
          self.useSelected.setCheckState( Qt.Checked )
        else:
          self.useSelected.setCheckState( Qt.Unchecked )
      # add all fields in combobox because now we can work with text fields too
      for i in changedField:
        if self.myFunction == 3:
          if changedField[i].type() == QVariant.Int or changedField[i].type() == QVariant.Double:
            self.cmbField.addItem( unicode( changedField[i].name() ) )
        else:
          self.cmbField.addItem( unicode( changedField[i].name() ) )
        self.cmbField.addItem( unicode( changedField[i].name() ) )
        
  def accept( self ):
    if self.inShape.currentText() == "":
      QMessageBox.information( self, "Error!", self.tr( "Please specify input vector layer" ) )
    elif self.cmbField.isVisible() and self.cmbField.currentText() == "":
      QMessageBox.information( self, "Error!", self.tr( "Please specify input field" ) )
    else:
      self.visual( self.inShape.currentText(), self.cmbField.currentText(), self.useSelected.checkState() )
  
  def manageGui( self ):
    if self.myFunction == 1: # Check geometry validity
      self.setWindowTitle( self.tr( "Check geometry validity" ) )
      self.cmbField.setVisible( False )
      self.label.setVisible( False )
      self.useSelected.setVisible( False )
      self.label_2.setText( self.tr( "Geometry errors" ) )
      self.label_4.setText( self.tr( "Total encountered errors" ) )
    elif self.myFunction == 2: # List unique values
      self.setWindowTitle( self.tr( "List unique values" ) )
      self.label_2.setText( self.tr( "Unique values" ) )
      self.label_4.setText(self.tr(  "Total unique values" ) )
      self.useSelected.setVisible( False )
    elif self.myFunction == 3: # Basic statistics
      self.setWindowTitle( self.tr( "Basics statistics" ) )
      self.label_2.setText( self.tr( "Statistics output" ) )
      self.label_4.setVisible( False )
      self.lstCount.setVisible( False )
      self.resize( 381, 400 )
    elif self.myFunction == 4: # Nearest neighbour analysis
      self.setWindowTitle( self.tr( "Nearest neighbour analysis" ) )
      self.cmbField.setVisible( False )
      self.label.setVisible( False )
      self.useSelected.setVisible( False )
      self.label_2.setText( self.tr( "Nearest neighbour statistics" ) )
      self.label_4.setVisible( False )
      self.lstCount.setVisible( False )
      self.resize( 381, 200 )
    self.inShape.clear()
    if self.myFunction == 1:
      myList = ftools_utils.getLayerNames( [ QGis.Polygon ] )
    elif self.myFunction == 4:
      myList = ftools_utils.getLayerNames( [ QGis.Point ] )
    else:
      myList = ftools_utils.getLayerNames( [ QGis.Point, QGis.Line, QGis.Polygon ] )    
    self.inShape.addItems( myList )
    return

#1:  Check geometry
#2:  List unique values
#3:  Basic statistics
#4:  Nearest neighbour analysis
  def visual( self,  myLayer, myField, mySelection ):
    vlayer = ftools_utils.getVectorLayerByName( myLayer )
    self.lstUnique.clear()
    self.lstCount.clear()
    self.testThread = visualThread( self.iface.mainWindow(), self, self.myFunction, vlayer, myField, mySelection )
    QObject.connect( self.testThread, SIGNAL( "runFinished(PyQt_PyObject)" ), self.runFinishedFromThread )
    QObject.connect( self.testThread, SIGNAL( "runStatus(PyQt_PyObject)" ), self.runStatusFromThread )
    QObject.connect( self.testThread, SIGNAL( "runRange(PyQt_PyObject)" ), self.runRangeFromThread )
    self.cancel_close.setText( "Cancel" )
    QObject.connect( self.cancel_close, SIGNAL( "clicked()" ), self.cancelThread )
    self.testThread.start()
    return True

  def cancelThread( self ):
    self.testThread.stop()
    
  def runFinishedFromThread( self, output ):
    self.testThread.stop()
    self.lstUnique.addItems( output[ 0 ] )
    self.lstCount.insert( unicode( output[ 1 ] ) )
    self.cancel_close.setText( "Close" )
    QObject.disconnect( self.cancel_close, SIGNAL( "clicked()" ), self.cancelThread )
    return True
    
  def runStatusFromThread( self, status ):
    self.progressBar.setValue( status )
        
  def runRangeFromThread( self, range_vals ):
    self.progressBar.setRange( range_vals[ 0 ], range_vals[ 1 ] )
    
class visualThread( QThread ):
  def __init__( self, parentThread, parentObject, function, vlayer, myField, mySelection ):
    QThread.__init__( self, parentThread )
    self.parent = parentObject
    self.running = False
    self.myFunction = function
    self.vlayer = vlayer
    self.myField = myField
    self.mySelection = mySelection
#        self.total = 0
#        self.currentCount = 0

  def run( self ):
    self.running = True
    if self.myFunction == 1: # Check geometry
      ( lst, cnt ) = self.check_geometry( self.vlayer )
    elif self.myFunction == 2: # List unique values
      ( lst, cnt ) = self.list_unique_values( self.vlayer, self.myField )
    elif self.myFunction == 3: # Basic statistics
      ( lst, cnt ) = self.basic_statistics( self.vlayer, self.myField )
    elif self.myFunction == 4: # Nearest neighbour analysis
      ( lst, cnt ) = self.nearest_neighbour_analysis( self.vlayer )
    self.emit( SIGNAL( "runFinished(PyQt_PyObject)" ), ( lst, cnt ) )
    self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0 )

  def stop(self):
    self.running = False        

  def list_unique_values( self, vlayer, myField ):
    vprovider = vlayer.dataProvider()
    allAttrs = vprovider.attributeIndexes()
    vprovider.select( allAttrs )
    fields = vprovider.fields()
    index = vprovider.fieldNameIndex( myField )
    unique = ftools_utils.getUniqueValues( vprovider, int( index ) )
    lstUnique = []
    nFeat = len( unique )
    nElement = 0
    self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0 )
    self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
    for item in unique:
      nElement += 1
      self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
      lstUnique.append(item.toString().trimmed())
    lstCount = len( unique )
    return ( lstUnique, lstCount )

  def basic_statistics( self, vlayer, myField ):
    vprovider = vlayer.dataProvider()
    allAttrs = vprovider.attributeIndexes()
    vprovider.select( allAttrs )
    fields = vprovider.fields()
    index = vprovider.fieldNameIndex( myField )
    feat = QgsFeature()
    sumVal = 0.0
    meanVal = 0.0
    nVal = 0.0
    values = []
    first = True
    nElement = 0
    # determine selected field type
    if ftools_utils.getFieldType( vlayer, myField ) == 'String':
      fillVal = 0
      emptyVal = 0
      if self.mySelection: # only selected features
        selection = vlayer.selectedFeatures()
        nFeat = vlayer.selectedFeatureCount()
        self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0 )
        self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
        for f in selection:
          atMap = f.attributeMap()
          lenVal = float( len( atMap[ index ].toString() ) )
          if first:
            minVal = lenVal
            maxVal = lenVal
            first = False
          else:
            if lenVal < minVal: minVal = lenVal
            if lenVal > maxVal: maxVal = lenVal
          if lenVal != 0.00:
            fillVal += 1
          else:
            emptyVal += 1
          values.append( lenVal )
          sumVal = sumVal + lenVal
          nElement += 1
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
      else: # there is no selection, process the whole layer
        nFeat = vprovider.featureCount()
        self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0 )
        self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
        while vprovider.nextFeature( feat ):
          atMap = feat.attributeMap()
          lenVal = float( len( atMap[ index ].toString() ) )
          if first:
            minVal = lenVal
            maxVal = lenVal
            first = False
          else:
            if lenVal < minVal: minVal = lenVal
            if lenVal > maxVal: maxVal = lenVal
          if lenVal != 0.00:
            fillVal += 1
          else:
            emptyVal += 1
          values.append( lenVal )
          sumVal = sumVal + lenVal
          nElement += 1
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
      nVal= float( len( values ) )
      if nVal > 0.00:
        meanVal = sumVal / nVal
      lstStats = []
      lstStats.append( QCoreApplication.translate( "statResult", "Max. len.      : " ) + unicode( maxVal ) )
      lstStats.append( QCoreApplication.translate( "statResult", "Min. len.       : " ) + unicode( minVal ) )
      lstStats.append( QCoreApplication.translate( "statResult", "Mean. len     : " ) + unicode( meanVal ) )
      lstStats.append( QCoreApplication.translate( "statResult", "Filled             : " ) + unicode( fillVal ) )
      lstStats.append( QCoreApplication.translate( "statResult", "Empty           : " ) + unicode( emptyVal ) )
      lstStats.append( QCoreApplication.translate( "statResult", "N                   : " ) + unicode( nVal ) )
      return ( lstStats, [] )
    else: # numeric field
      stdVal = 0
      cvVal = 0
      if self.mySelection: # only selected features
        selection = vlayer.selectedFeatures()
        nFeat = vlayer.selectedFeatureCount()
        self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0 )
        self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
        for f in selection:
          atMap = f.attributeMap()
          value = float( atMap[ index ].toDouble()[ 0 ] )
          if first:
            minVal = value
            maxVal = value
            first = False
          else:
            if value < minVal: minVal = value
            if value > maxVal: maxVal = value
          values.append( value )
          sumVal = sumVal + value
          nElement += 1
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
      else: # there is no selection, process the whole layer
        nFeat = vprovider.featureCount()
        self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0 )
        self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
        while vprovider.nextFeature( feat ):
          atMap = feat.attributeMap()
          value = float( atMap[ index ].toDouble()[ 0 ] )
          if first:
            minVal = value
            maxVal = value
            first = False
          else:
            if value < minVal: minVal = value
            if value > maxVal: maxVal = value
          values.append( value )
          sumVal = sumVal + value
          nElement += 1
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
      nVal= float( len( values ) )
      if nVal > 0.00:
        meanVal = sumVal / nVal
        if meanVal != 0.00:
          for val in values:
            stdVal += ( ( val - meanVal ) * ( val - meanVal ) )
          stdVal = math.sqrt( stdVal / nVal )
          cvVal = stdVal / meanVal
      lstStats = []
      lstStats.append( "Mean    : " + unicode( meanVal ) )
      lstStats.append( "StdDev : " + unicode( stdVal ) )
      lstStats.append( "Sum     : " + unicode( sumVal) )
      lstStats.append( "Min     : " + unicode( minVal ) )
      lstStats.append( "Max     : " + unicode( maxVal ) )
      lstStats.append( "N         : " + unicode( nVal ) )
      lstStats.append( "CV       : " + unicode( cvVal ) )
      return ( lstStats, [] )

  def nearest_neighbour_analysis( self, vlayer ):
    vprovider = vlayer.dataProvider()
    allAttrs = vprovider.attributeIndexes()
    vprovider.select( allAttrs )
    feat = QgsFeature()
    neighbour = QgsFeature()
    sumDist = 0.00
    distance = QgsDistanceArea()
    A = vlayer.extent()
    A = float( A.width() * A.height() )
    index = ftools_utils.createIndex( vprovider )
    vprovider.rewind()
    nFeat = vprovider.featureCount()
    nElement = 0
    self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0 )
    self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
    while vprovider.nextFeature( feat ):
      neighbourID = index.nearestNeighbor( feat.geometry().asPoint(), 2 )[ 1 ]
      vprovider.featureAtId( neighbourID, neighbour, True, [] )
      nearDist = distance.measureLine( neighbour.geometry().asPoint(), feat.geometry().asPoint() )
      sumDist += nearDist
      nElement += 1
      self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
    nVal = vprovider.featureCount()
    do = float( sumDist) / nVal
    de = float( 0.5 / math.sqrt( nVal / A ) )
    d = float( do / de )
    SE = float( 0.26136 / math.sqrt( ( nVal * nVal ) / A ) )
    zscore = float( ( do - de ) / SE )
    lstStats = []
    lstStats.append( self.tr( "Observed mean distance : " ) + "         " + unicode( do ) )
    lstStats.append( self.tr( "Expected mean distance : " ) + "        " + unicode( de ) )
    lstStats.append( self.tr( "Nearest neighbour index : " ) + "        " + unicode( d ) )
    lstStats.append( "N :           " + unicode( nVal ) )
    lstStats.append( "Z-Score :           " + unicode( zscore ) )
    return ( lstStats, [] )

  def check_geometry( self, vlayer ):
    vprovider = vlayer.dataProvider()
    allAttrs = vprovider.attributeIndexes()
    vprovider.select( allAttrs )
    feat = QgsFeature()
    geom = QgsGeometry()
    count = 0
    lstErrors = []
    nFeat = vprovider.featureCount()
    nElement = 0
    self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0 )
    self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
    while vprovider.nextFeature( feat ):
      geom = QgsGeometry( feat.geometry() )
      nElement += 1
      self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
      if geom.isMultipart():
        polygons = geom.asMultiPolygon()
        for polygon in polygons:
          if not self.isHoleNested( polygon ):
            lstErrors.append( self.tr( "Feature %1 contains an unnested hole" ).arg( unicode( feat.id() ) ) )
            count += 1
          if not self.isPolygonClosed( polygon ):
            lstErrors.append( self.tr( "Feature %1 is not closed" ).arg( unicode( feat.id() ) ) )
            count += 1
          if self.isSelfIntersecting( polygon ):
            lstErrors.append( self.tr( "Feature %1 is self intersecting" ).arg( unicode( feat.id() ) ) )
            count += 1
          if not self.isCorrectOrientation( polygon ):
            lstErrors.append( self.tr( "Feature %1 has incorrect node ordering" ).arg( unicode( feat.id() ) ) )
            count += 1
          
      else:
        geom = geom.asPolygon()
        if not self.isHoleNested( geom ):
          lstErrors.append( self.tr( "Feature %1 contains an unnested hole" ).arg( unicode( feat.id() ) ) )
          count += 1
        if not self.isPolygonClosed( geom ):
          lstErrors.append( self.tr( "Feature %1 is not closed" ).arg( unicode( feat.id() ) ) )
          count += 1
        if self.isSelfIntersecting( geom ):
          lstErrors.append( self.tr( "Feature %1 is self intersecting" ).arg( unicode( feat.id() ) ) )
          count += 1
        if not self.isCorrectOrientation( geom ):
          lstErrors.append( self.tr( "Feature %1 has incorrect node ordering" ).arg( unicode( feat.id() ) ) )
          count += 1
    return ( lstErrors, count )

  def isHoleNested( self, polygon ):
    if len( polygon ) <= 1:
      return True
    else:
      outer = polygon[ 0 ]
      for i in polygon[ 1: len( polygon ) ]:
        if not self.arePointsInside( i, outer ):
          return False
    return True

  def arePointsInside( self, inner, outer ):
    outer = QgsGeometry().fromPolygon( [ outer ] )
    for j in inner:
      if not outer.contains(j):
        return False
    return True

  def isPolygonClosed( self, polygon ):
    for i in polygon:
      first = i[ 0 ]
      last = i[ len( i )-1 ]
      if not first == last:
        return False
    return True

  def isSelfIntersecting( self, polygon ):
    for h in polygon:
      count = 0
      size = range( 0, len (h )- 1 )
      for i in size:
        count = 0
        for j in size:
          if QgsGeometry().fromPolyline( [ h[ i ], h[ i + 1 ] ] ).intersects( QgsGeometry().fromPolyline( [ h[ j ], h[ j + 1 ] ] ) ):
            count += 1
        if count > 3:
          return True
    return False

  def isCorrectOrientation( self, polygon ):
    outer = True
    for h in polygon:
      if outer:
        outer = False
        if not self.isClockwise( h ):
          return False
      else:
        if self.isClockwise(h):
          return False
    return True

  def isClockwise( self, temp ):
    area = 0
    for pt in range( 0, len( temp ) -1 ):
      area += ( temp[ pt ].x() * temp[ pt + 1 ].y() - temp[ pt + 1 ].x() * temp[ pt ].y() )
    area = area / 2
    if area <= 0:
      return True
    else:
      return False
