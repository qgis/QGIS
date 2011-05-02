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
    self.buttonOk = self.buttonBox_2.button( QDialogButtonBox.Ok )
    self.progressBar.setValue( 0 )
    self.partProgressBar.setValue( 0 )
    self.partProgressBar.setVisible( False )

  def keyPressEvent( self, e ):
    '''
    Reimplemented key press event:
    '''
    if ( e.modifiers() == Qt.ControlModifier or e.modifiers() == Qt.MetaModifier ) and e.key() == Qt.Key_C:
      #selection = self.tblUnique.selectedItems()
      items = QString()
      if self.myFunction in ( 1, 2 ):
        for rec in range( self.tblUnique.rowCount() ):
          items.append( self.tblUnique.item( rec, 0 ).text() + "\n" )
      else:
        for rec in range( self.tblUnique.rowCount() ):
          items.append( self.tblUnique.item( rec, 0 ).text() + ":" + self.tblUnique.item( rec, 1 ).text() + "\n" )
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
        self.cmbField.addItem( unicode( changedField[i].name() ) )

  def accept( self ):
    if self.inShape.currentText() == "":
      QMessageBox.information( self, self.tr("Error!"), self.tr( "Please specify input vector layer" ) )
    elif self.cmbField.isVisible() and self.cmbField.currentText() == "":
      QMessageBox.information( self, self.tr("Error!"), self.tr( "Please specify input field" ) )
    else:
      self.visual( self.inShape.currentText(), self.cmbField.currentText(), self.useSelected.checkState() )

  def manageGui( self ):
    if self.myFunction == 2: # List unique values
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
    if self.myFunction == 4:
      myList = ftools_utils.getLayerNames( [ QGis.Point ] )
    else:
      myList = ftools_utils.getLayerNames( [ QGis.Point, QGis.Line, QGis.Polygon ] )
    self.inShape.addItems( myList )
    return

#1:  Check geometry (disabled)
#2:  List unique values
#3:  Basic statistics
#4:  Nearest neighbour analysis
  def visual( self,  myLayer, myField, mySelection ):
    vlayer = ftools_utils.getVectorLayerByName( myLayer )
    self.tblUnique.clearContents()
    self.tblUnique.setRowCount( 0 )
    self.lstCount.clear()
    self.buttonOk.setEnabled( False )

    self.testThread = visualThread( self.iface.mainWindow(), self, self.myFunction, vlayer, myField, mySelection )
    QObject.connect( self.testThread, SIGNAL( "runFinished(PyQt_PyObject)" ), self.runFinishedFromThread )
    QObject.connect( self.testThread, SIGNAL( "runStatus(PyQt_PyObject)" ), self.runStatusFromThread )
    QObject.connect( self.testThread, SIGNAL( "runRange(PyQt_PyObject)" ), self.runRangeFromThread )
    QObject.connect( self.testThread, SIGNAL( "runPartRange(PyQt_PyObject)" ), self.runPartRangeFromThread )
    QObject.connect( self.testThread, SIGNAL( "runPartStatus(PyQt_PyObject)" ), self.runPartStatusFromThread )
    self.cancel_close.setText( self.tr("Cancel") )
    QObject.connect( self.cancel_close, SIGNAL( "clicked()" ), self.cancelThread )

    QApplication.setOverrideCursor( Qt.WaitCursor )
    self.testThread.start()
    return True

  def cancelThread( self ):
    self.testThread.stop()
    QApplication.restoreOverrideCursor()
    self.buttonOk.setEnabled( True )

  def runFinishedFromThread( self, output ):
    self.testThread.stop()
    QApplication.restoreOverrideCursor()
    self.buttonOk.setEnabled( True )
    result = output[ 0 ]
    numRows = len( result )
    self.tblUnique.setRowCount( numRows )
    if self.myFunction in ( 1, 2 ):
      self.tblUnique.setColumnCount( 1 )
      for rec in range( numRows ):
        item = QTableWidgetItem( result[ rec ] )
        self.tblUnique.setItem( rec, 0, item )
    else:
      self.tblUnique.setColumnCount( 2 )
      for rec in range( numRows ):
        tmp = result[ rec ].split( ":" )
        item = QTableWidgetItem( tmp[ 0 ] )
        self.tblUnique.setItem( rec, 0, item )
        item = QTableWidgetItem( tmp[ 1 ] )
        self.tblUnique.setItem( rec, 1, item )
      self.tblUnique.setHorizontalHeaderLabels( [ self.tr("Parameter"), self.tr("Value") ] )
      self.tblUnique.horizontalHeader().setResizeMode( 1, QHeaderView.ResizeToContents )
      self.tblUnique.horizontalHeader().show()
    self.tblUnique.horizontalHeader().setResizeMode( 0, QHeaderView.Stretch )
    self.tblUnique.resizeRowsToContents()

    self.lstCount.insert( unicode( output[ 1 ] ) )
    self.cancel_close.setText( "Close" )
    QObject.disconnect( self.cancel_close, SIGNAL( "clicked()" ), self.cancelThread )
    return True

  def runStatusFromThread( self, status ):
    self.progressBar.setValue( status )

  def runRangeFromThread( self, range_vals ):
    self.progressBar.setRange( range_vals[ 0 ], range_vals[ 1 ] )

  def runPartStatusFromThread( self, status ):
    self.partProgressBar.setValue( status )
    if status >= self.part_max:
      self.partProgressBar.setVisible( False )

  def runPartRangeFromThread( self, range_vals ):
    self.part_max = range_vals[ 1 ]
    self.partProgressBar.setVisible( True )
    self.partProgressBar.setRange( range_vals[ 0 ], range_vals[ 1 ] )

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
    # note that 1 used to be associated with check_geometry
    if self.myFunction == 2: # List unique values
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
    if nFeat > 0:
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
    if ftools_utils.getFieldType( vlayer, myField ) in (
    'String', 'varchar', 'char', 'text'):
      fillVal = 0
      emptyVal = 0
      if self.mySelection: # only selected features
        selection = vlayer.selectedFeatures()
        nFeat = vlayer.selectedFeatureCount()
        if nFeat > 0:
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
	if nFeat > 0:
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0 )
          self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
        vprovider.select( allAttrs )
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
      if nVal > 0:
        meanVal = sumVal / nVal
        lstStats = []
        lstStats.append( self.tr( "Max. len:" ) + unicode( maxVal ) )
        lstStats.append( self.tr( "Min. len:" ) + unicode( minVal ) )
        lstStats.append( self.tr( "Mean. len:" ) + unicode( meanVal ) )
        lstStats.append( self.tr( "Filled:" ) + unicode( fillVal ) )
        lstStats.append( self.tr( "Empty:" ) + unicode( emptyVal ) )
        lstStats.append( self.tr( "N:" ) + unicode( nVal ) )
        return ( lstStats, [] )
      else:
        return ( ["Error:No features selected!"], [] )
    else: # numeric field
      stdVal = 0.00
      cvVal = 0.00
      rangeVal = 0.00
      medianVal = 0.00
      maxVal = 0.00
      minVal = 0.00
      if self.mySelection: # only selected features
        selection = vlayer.selectedFeatures()
        nFeat = vlayer.selectedFeatureCount()
        uniqueVal = ftools_utils.getUniqueValuesCount( vlayer, index, True )
        if nFeat > 0:
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
        uniqueVal = ftools_utils.getUniqueValuesCount( vlayer, index, False )
	if nFeat > 0:
          self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0 )
          self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
        vprovider.select( allAttrs )
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
        rangeVal = maxVal - minVal
        meanVal = sumVal / nVal
        if meanVal != 0.00:
          for val in values:
            stdVal += ( ( val - meanVal ) * ( val - meanVal ) )
          stdVal = math.sqrt( stdVal / nVal )
          cvVal = stdVal / meanVal
        if nVal > 1:
            lstVal = values
            lstVal.sort()
            if ( nVal % 2 ) == 0:
                medianVal = 0.5 * ( lstVal[ int( ( nVal - 1 ) / 2 ) ] + lstVal[ int( ( nVal ) / 2 ) ] )
            else:
                medianVal = lstVal[ int( ( nVal + 1 ) / 2 - 1 ) ]
        lstStats = []
        lstStats.append( self.tr( "Mean:" ) + unicode( meanVal ) )
        lstStats.append( self.tr( "StdDev:" ) + unicode( stdVal ) )
        lstStats.append( self.tr( "Sum:" ) + unicode( sumVal) )
        lstStats.append( self.tr( "Min:" ) + unicode( minVal ) )
        lstStats.append( self.tr( "Max:" ) + unicode( maxVal ) )
        lstStats.append( self.tr( "N:" ) + unicode( nVal ) )
        lstStats.append( self.tr( "CV:" ) + unicode( cvVal ) )
        lstStats.append( self.tr( "Number of unique values:" ) + unicode( uniqueVal ) )
        lstStats.append( self.tr( "Range:" ) + unicode( rangeVal ) )
        lstStats.append( self.tr( "Median:" ) + unicode( medianVal ) )
        return ( lstStats, [] )
      else:
        return ( ["Error:No features selected!"], [] )

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
    if nFeat > 0:
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
    lstStats.append( self.tr( "Observed mean distance:" ) + unicode( do ) )
    lstStats.append( self.tr( "Expected mean distance:" ) + unicode( de ) )
    lstStats.append( self.tr( "Nearest neighbour index:" ) + unicode( d ) )
    lstStats.append( self.tr( "N:" ) + unicode( nVal ) )
    lstStats.append( self.tr( "Z-Score:" ) + unicode( zscore ) )
    return ( lstStats, [] )
