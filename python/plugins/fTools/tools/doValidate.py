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

from PyQt4.QtCore import SIGNAL, Qt, QObject, QSettings, QByteArray, QThread, QVariant
from PyQt4.QtGui import QDialog, QDialogButtonBox, QAbstractItemView, QApplication, QMessageBox, QHeaderView, QTableWidgetItem
from qgis.core import QGis, QgsPoint, QgsFeature, QgsFeatureRequest, QgsGeometry, QgsFields, QgsField, QgsVectorFileWriter
from qgis.gui import QgsVertexMarker

from ui_frmVisual import Ui_Dialog
import ftools_utils


class MarkerErrorGeometry():
  def __init__(self, mapCanvas):
    self.__canvas = mapCanvas
    self.__marker = None

  def __del__(self):
    self.reset()

  def __createMarker(self, point):
    self.__marker = QgsVertexMarker(self.__canvas)
    self.__marker.setCenter(point)
    self.__marker.setIconType(QgsVertexMarker.ICON_X)
    self.__marker.setPenWidth(3)

  def setGeom(self, p):
    if self.__marker is not None:
      self.reset()
    if self.__marker is None:
      self.__createMarker(p)
    else:
      self.__marker.setCenter(p)

  def reset(self):
    if self.__marker is not None:
      self.__canvas.scene().removeItem(self.__marker)
      del self.__marker
      self.__marker = None

class ValidateDialog( QDialog, Ui_Dialog ):
  def __init__(self, iface):
    QDialog.__init__(self, iface.mainWindow())
    self.iface = iface
    self.setupUi(self)
    # self.setModal(False) # we want to be able to interact with the featuresmc.extent().width()
    # self.setWindowFlags( Qt.SubWindow )
    # adjust user interface
    self.setWindowTitle( self.tr( "Check geometry validity" ) )
    self.cmbField.setVisible( False )
    self.label.setVisible( False )
    self.useSelected.setVisible( True )
    self.label_2.setText( self.tr( "Geometry errors" ) )
    self.label_4.setText( self.tr( "Total encountered errors" ) )
    self.partProgressBar.setVisible( False )
    self.tblUnique.setSelectionMode(QAbstractItemView.SingleSelection)
    self.tblUnique.setSelectionBehavior(QAbstractItemView.SelectRows)
    # populate list of available layers
    myList = ftools_utils.getLayerNames( [ QGis.Point, QGis.Line, QGis.Polygon ] )
    self.connect(self.tblUnique, SIGNAL("currentItemChanged(QTableWidgetItem*, QTableWidgetItem*)" ), self.zoomToError)
    self.inShape.addItems( myList )
    self.buttonBox_2.setOrientation(Qt.Horizontal)
    self.cancel_close = self.buttonBox_2.button(QDialogButtonBox.Close)
    self.buttonOk = self.buttonBox_2.button(QDialogButtonBox.Ok)
    self.progressBar.setValue(0)
    self.storedScale = self.iface.mapCanvas().scale()
    # create marker for error
    self.marker = MarkerErrorGeometry(self.iface.mapCanvas())

    settings = QSettings()
    self.restoreGeometry( settings.value("/fTools/ValidateDialog/geometry", QByteArray(), type=QByteArray) )

    QObject.connect( self.browseShpError, SIGNAL( "clicked()" ), self.outFile )
    QObject.connect( self.ckBoxShpError, SIGNAL( "stateChanged( int )" ), self.updateGui )
    self.updateGui()

  def closeEvent(self, e):
    settings = QSettings()
    settings.setValue( "/fTools/ValidateDialog/geometry", self.saveGeometry())
    QDialog.closeEvent(self, e)
    del self.marker

  def keyPressEvent( self, e ):
    if ( e.modifiers() == Qt.ControlModifier or
         e.modifiers() == Qt.MetaModifier ) and \
       e.key() == Qt.Key_C:
      items = ""
      for row in range( self.tblUnique.rowCount() ):
        items += self.tblUnique.item( row, 0 ).text() + "," + self.tblUnique.item( row, 1 ).text() + "\n"
      if items:
        clip_board = QApplication.clipboard()
        clip_board.setText( items )
    else:
      QDialog.keyPressEvent( self, e )

  def accept( self ):
    if self.inShape.currentText() == "":
      QMessageBox.information( self, self.tr("Error!"), self.tr( "Please specify input vector layer" ) )
    elif self.cmbField.isVisible() and self.cmbField.currentText() == "":
      QMessageBox.information( self, self.tr("Error!"), self.tr( "Please specify input field" ) )
    elif self.ckBoxShpError.isChecked() and self.lineEditShpError.text() == "":
      QMessageBox.information( self, self.tr( "Error!" ), self.tr( "Please specify output shapefile" ) )
    else:
      self.vlayer = ftools_utils.getVectorLayerByName( self.inShape.currentText() )
      self.validate( self.useSelected.checkState() )

  def updateGui( self ):
    if self.ckBoxShpError.isChecked():
      self.lineEditShpError.setEnabled( True )
      self.browseShpError.setEnabled( True )
      self.addToCanvasCheck.setEnabled(True)
      self.tblUnique.setEnabled( False )
      self.lstCount.setEnabled( False )
      self.label_2.setEnabled( False )
      self.label_4.setEnabled( False )
      self.label_5.setEnabled( False )
    else:
      self.lineEditShpError.setEnabled( False )
      self.browseShpError.setEnabled( False )
      self.addToCanvasCheck.setEnabled(False)
      self.tblUnique.setEnabled( True )
      self.lstCount.setEnabled( True )
      self.label_2.setEnabled( True )
      self.label_4.setEnabled( True )
      self.label_5.setEnabled( True )

  def outFile( self ):
    self.lineEditShpError.clear()
    (self.shapefileName, self.encoding) = ftools_utils.saveDialog( self )
    if self.shapefileName is None or self.encoding is None:
      return
    self.lineEditShpError.setText( self.shapefileName )

  def zoomToError(self, curr, prev):
      if curr is None:
        return
      row = curr.row() # if we clicked in the first column, we want the second
      item = self.tblUnique.item(row, 1)

      mc = self.iface.mapCanvas()
      mc.zoomToPreviousExtent()

      e = item.data(Qt.UserRole)

      if type(e)==QgsPoint:
        e = mc.mapRenderer().layerToMapCoordinates( self.vlayer, e )

        self.marker.setGeom(e)

        rect = mc.extent()
        rect.scale( 0.5, e )

      else:
        self.marker.reset()

        ft = QgsFeature()
        fid = int(self.tblUnique.item(row, 0).text())
        if not self.vlayer.getFeatures( QgsFeatureRequest().setFilterFid( fid ) ).nextFeature( ft ):
          return

        rect = mc.mapRenderer().layerExtentToOutputExtent( self.vlayer, ft.geometry().boundingBox() )
        rect.scale( 1.05 )

      # Set the extent to our new rectangle
      mc.setExtent(rect)
      # Refresh the map
      mc.refresh()

  def validate( self, mySelection ):
    if not self.ckBoxShpError.isChecked():
      self.tblUnique.clearContents()
      self.tblUnique.setRowCount( 0 )
      self.lstCount.clear()
      self.shapefileName = None
      self.encoding = None

    self.buttonOk.setEnabled( False )

    self.testThread = validateThread( self.iface.mainWindow(), self, self.vlayer, mySelection, self.shapefileName, self.encoding, self.ckBoxShpError.isChecked() )
    QObject.connect( self.testThread, SIGNAL( "runFinished(PyQt_PyObject)" ), self.runFinishedFromThread )
    QObject.connect( self.testThread, SIGNAL( "runStatus(PyQt_PyObject)" ), self.runStatusFromThread )
    QObject.connect( self.testThread, SIGNAL( "runRange(PyQt_PyObject)" ), self.runRangeFromThread )
    self.cancel_close.setText( self.tr("Cancel") )
    QObject.connect( self.cancel_close, SIGNAL( "clicked()" ), self.cancelThread )
    QApplication.setOverrideCursor( Qt.WaitCursor )
    self.testThread.start()
    return True

  def reject(self):
    # Remove Marker
    self.marker.reset()
    QDialog.reject(self)

  def cancelThread( self ):
    self.testThread.stop()
    QApplication.restoreOverrideCursor()
    self.buttonOk.setEnabled( True )

  def runFinishedFromThread( self, success ):
    self.testThread.stop()
    QApplication.restoreOverrideCursor()
    self.buttonOk.setEnabled( True )
    if success == "writeShape":
      extra = ""
      if self.addToCanvasCheck.isChecked():
        addCanvasCheck = ftools_utils.addShapeToCanvas(unicode(self.shapefileName))
        if not addCanvasCheck:
          QMessageBox.warning( self, self.tr("Geometry"), self.tr( "Error loading output shapefile:\n%s" ) % ( unicode( self.shapefileName ) ))
      else:
        QMessageBox.information(self, self.tr("Geometry"),self.tr("Created output shapefile:\n%s\n%s" ) % ( unicode( self.shapefileName ), extra))
    else:
      self.tblUnique.setColumnCount( 2 )
      count = 0
      for rec in success:
        if len(rec[1]) < 1:
          continue
        for err in rec[1]:  # for each error we find
          self.tblUnique.insertRow(count)
          fidItem = QTableWidgetItem( str(rec[0]) )
          self.tblUnique.setItem( count, 0, fidItem )
          message = err.what()
          errItem = QTableWidgetItem( message )
          if err.hasWhere():  # if there is a location associated with the error
            errItem.setData(Qt.UserRole, err.where())
          self.tblUnique.setItem( count, 1, errItem )
          count += 1
      self.tblUnique.setHorizontalHeaderLabels( [ self.tr("Feature"), self.tr("Error(s)") ] )
      self.tblUnique.horizontalHeader().setResizeMode( 0, QHeaderView.ResizeToContents )
      self.tblUnique.horizontalHeader().show()
      self.tblUnique.horizontalHeader().setResizeMode( 1, QHeaderView.Stretch )
      self.tblUnique.resizeRowsToContents()
      self.lstCount.insert(str(count))
    self.cancel_close.setText( "Close" )
    QObject.disconnect( self.cancel_close, SIGNAL( "clicked()" ), self.cancelThread )
    return True

  def runStatusFromThread( self, status ):
    self.progressBar.setValue( status )

  def runRangeFromThread( self, range_vals ):
    self.progressBar.setRange( range_vals[ 0 ], range_vals[ 1 ] )

class validateThread( QThread ):
  def __init__( self, parentThread, parentObject, vlayer, mySelection, myName, myEncoding, myNewShape ):
    QThread.__init__( self, parentThread )
    self.parent = parentObject
    self.running = False
    self.vlayer = vlayer
    self.mySelection = mySelection
    self.myName = myName
    self.myEncoding = myEncoding
    self.writeShape = myNewShape

  def run( self ):
    self.running = True
    success = self.check_geometry( self.vlayer )
    self.emit( SIGNAL( "runFinished(PyQt_PyObject)" ), success )

  def stop(self):
    self.running = False

  def check_geometry( self, vlayer ):
    lstErrors = []
    if self.mySelection:
      layer = vlayer.selectedFeatures()
      nFeat = len(layer)
    else:
      layer = []
      ft = QgsFeature()
      fit = vlayer.getFeatures( QgsFeatureRequest().setSubsetOfAttributes([]) )
      while fit.nextFeature(ft):
        layer.append( QgsFeature(ft) )
      nFeat = len(layer)
    nElement = 0
    if nFeat > 0:
      self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0 )
      self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
    for feat in layer:
      if not self.running:
        return list()
      geom = QgsGeometry(feat.geometry()) # ger reference to geometry
      self.emit(SIGNAL("runStatus(PyQt_PyObject)"), nElement)
      nElement += 1
      # Check Add error
      if not geom.isGeosEmpty():
        lstErrors.append((feat.id(), list(geom.validateGeometry())))
    self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nFeat )

    if self.writeShape:
      fields = QgsFields()
      fields.append( QgsField( "FEAT_ID", QVariant.Int ) )
      fields.append( QgsField( "ERROR", QVariant.String ) )

      writer = QgsVectorFileWriter( self.myName, self.myEncoding, fields,
                                    QGis.WKBPoint, vlayer.crs() )
      for rec in lstErrors:
        if len(rec[1]) < 1:
          continue
        for err in rec[1]:
          fidItem = str(rec[0])
          message = err.what()
          if err.hasWhere():
            locErr = err.where()
            xP = locErr.x()
            yP = locErr.y()
            myPoint = QgsPoint( xP, yP )
            geometry = QgsGeometry().fromPoint( myPoint )
            ft = QgsFeature()
            ft.setGeometry( geometry )
            ft.setAttributes( [ fidItem, message ] )
            writer.addFeature( ft )
      del writer
      return "writeShape"
    else:
      return lstErrors
