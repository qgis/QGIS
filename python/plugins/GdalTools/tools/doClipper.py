# -*- coding: utf-8 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

from ui_widgetClipper import Ui_GdalToolsWidget as Ui_Widget
from widgetPluginBase import GdalToolsBasePluginWidget as BasePluginWidget
import GdalTools_utils as Utils

class GdalToolsDialog(QWidget, Ui_Widget, BasePluginWidget):

  def __init__(self, iface):
      QWidget.__init__(self)
      self.iface = iface
      self.canvas = self.iface.mapCanvas()

      self.clipper = ClipperSelector(self.canvas)

      self.setupUi(self)
      BasePluginWidget.__init__(self, self.iface, "gdal_merge.py", None, self.iface.mainWindow())

      self.outputFormat = Utils.fillRasterOutputFormat()

      self.inputFiles = QStringList()
      self.warningDialog = QErrorMessage(self)

      self.setParamsStatus(
        [
          (self.outputFileEdit, SIGNAL("textChanged(const QString &)")), 
          (self.noDataSpin, SIGNAL("valueChanged(int)"), self.noDataCheck),
          (self.pctCheck, SIGNAL("stateChanged(int)"))
        ]
      )

      self.connect(self.selectOutputFileButton, SIGNAL("clicked()"), self.fillOutputFileEdit)
      self.connect(self.clipper, SIGNAL("clippingRectangleCreated()"), self.fillCoords)
      self.connect(self.x1CoordEdit, SIGNAL("textChanged(const QString &)"), self.coordsChanged)
      self.connect(self.x2CoordEdit, SIGNAL("textChanged(const QString &)"), self.coordsChanged)
      self.connect(self.y1CoordEdit, SIGNAL("textChanged(const QString &)"), self.coordsChanged)
      self.connect(self.y2CoordEdit, SIGNAL("textChanged(const QString &)"), self.coordsChanged)
      self.connect(self.clipper, SIGNAL("deactivated()"), self.pauseClipping)
      self.connect(self.btnEnableClip, SIGNAL("clicked()"), self.startClipping)

  def show_(self):
      self.connect(self.canvas, SIGNAL("layersChanged()"), self.fillInputFiles)
      self.btnEnableClip.setVisible(False)
      BasePluginWidget.show_(self)

      self.fillInputFiles()
      self.fillCoords()

  def onClosing(self):
      self.disconnect(self.canvas, SIGNAL("layersChanged()"), self.fillInputFiles)
      self.stopClipping()
      BasePluginWidget.onClosing(self)

  def stopClipping(self):
      self.isClippingStarted = False
      self.canvas.unsetMapTool(self.clipper)
      self.clipper.reset()
      self.btnEnableClip.setVisible(False)

  def startClipping(self):
      self.canvas.setMapTool(self.clipper)
      self.isClippingStarted = True
      self.btnEnableClip.setVisible(False)
      self.coordsChanged()

  def pauseClipping(self):
      if not self.isClippingStarted:
        return

      self.btnEnableClip.setVisible(True)

  def fillInputFiles(self):
      self.inputFiles = QStringList()

      # for each active layer
      for i in range(self.canvas.layerCount()): 
        layer = self.canvas.layer(i) 
        # only raster layers, but not WMS ones
        if layer.type() == layer.RasterLayer and ( not layer.usesProvider() ):
            # do not use the output file as input
            if layer.source() == self.outputFileEdit.text():
              continue

            self.inputFiles << layer.source()

      if self.inputFiles.isEmpty():
        self.stopClipping()

        if self.isVisible() and self.warningDialog.isHidden():
          msg = QString( self.tr("No active raster layers. You must add almost one raster layer to continue.") )
          self.warningDialog.showMessage(msg)
      else:
        self.warningDialog.hide()
        self.startClipping()

      self.checkRun()

  def isCoordsValid(self):
      return not ( self.x1CoordEdit.text().isEmpty() or \
             self.x2CoordEdit.text().isEmpty() or \
             self.y1CoordEdit.text().isEmpty() or \
             self.y2CoordEdit.text().isEmpty() )

  def coordsChanged(self):
      if not self.isCoordsValid():
        self.clipper.setClippingRectangle(None)
      else:
        point1 = QgsPoint( float(self.x1CoordEdit.text()), float(self.y1CoordEdit.text()) )
        point2 = QgsPoint( float(self.x2CoordEdit.text()), float(self.y2CoordEdit.text()) )
        rect = QgsRectangle(point1, point2)

        self.clipper.setClippingRectangle(rect)

      self.checkRun()

  def fillCoords(self):
      rect = self.clipper.clippingRectangle()
      if rect != None:
        self.x1CoordEdit.setText( str(rect.xMinimum()) )
        self.x2CoordEdit.setText( str(rect.xMaximum()) )
        self.y1CoordEdit.setText( str(rect.yMaximum()) )
        self.y2CoordEdit.setText( str(rect.yMinimum()) )
      else:
        self.x1CoordEdit.clear()
        self.x2CoordEdit.clear()
        self.y1CoordEdit.clear()
        self.y2CoordEdit.clear()

      self.checkRun()

  def checkRun(self):
      self.someValueChanged()

      self.x1CoordEdit.setEnabled( not self.inputFiles.isEmpty() )
      self.x2CoordEdit.setEnabled( not self.inputFiles.isEmpty() )
      self.y1CoordEdit.setEnabled( not self.inputFiles.isEmpty() )
      self.y2CoordEdit.setEnabled( not self.inputFiles.isEmpty() )

      self.base.enableRun( not self.inputFiles.isEmpty() and self.isCoordsValid() )

  def fillOutputFileEdit(self):
      lastUsedFilter = Utils.FileFilter.lastUsedRasterFilter()
      outputFile = Utils.FileDialog.getSaveFileName(self, self.tr( "Select the raster file to save the results to" ), Utils.FileFilter.allRastersFilter(), lastUsedFilter)
      if outputFile.isEmpty():
        return
      Utils.FileFilter.setLastUsedRasterFilter(lastUsedFilter)

      self.outputFormat = Utils.fillRasterOutputFormat(lastUsedFilter, outputFile)

      # do not use the output file as input
      if self.inputFiles.contains(outputFile):
        self.inputFiles.removeAt( self.inputFiles.indexOf(outputFile) )
      self.outputFileEdit.setText(outputFile)

  def getArguments(self):
      arguments = QStringList()
      if self.noDataCheck.isChecked():
        arguments << "-n"
        arguments << str(self.noDataSpin.value())
      if self.pctCheck.isChecked():
        arguments << "-pct"
      if self.isCoordsValid():
        rect = self.clipper.clippingRectangle()
        if rect != None:
          arguments << "-ul_lr"
          arguments << str(rect.xMinimum())
          arguments << str(rect.yMaximum())
          arguments << str(rect.xMaximum())
          arguments << str(rect.yMinimum())
      if not self.outputFileEdit.text().isEmpty():
        arguments << "-of"
        arguments << self.outputFormat
      if not self.outputFileEdit.text().isEmpty():
        arguments << "-o"
        arguments << self.outputFileEdit.text()
      arguments << self.inputFiles
      return arguments

  def getOutputFileName(self):
      return self.outputFileEdit.text()

  def addLayerIntoCanvas(self, fileInfo):
      self.iface.addRasterLayer(fileInfo.filePath())


class ClipperSelector(QgsMapToolEmitPoint):
  def __init__(self, canvas):
      self.canvas = canvas
      QgsMapToolEmitPoint.__init__(self, self.canvas)

      self.rubberBand = QgsRubberBand( self.canvas, True )	# true, its a polygon
      self.rubberBand.setColor( Qt.red )
      self.rubberBand.setWidth( 1 )

      self.isEmittingPoint = False

      self.startPoint = self.endPoint = None

  def reset(self):
      self.isEmittingPoint = False
      self.rubberBand.reset( True )	# true, its a polygon

  def canvasPressEvent(self, e):
      self.startPoint = self.toMapCoordinates( e.pos() )
      self.endPoint = self.startPoint
      self.isEmittingPoint = True

      self.showRect(self.startPoint, self.endPoint)

  def canvasReleaseEvent(self, e):
      self.isEmittingPoint = False
      self.emit( SIGNAL("clippingRectangleCreated()") )

  def canvasMoveEvent(self, e):
      if not self.isEmittingPoint:
        return

      self.endPoint = self.toMapCoordinates( e.pos() )
      self.showRect(self.startPoint, self.endPoint)

  def showRect(self, startPoint, endPoint):
      self.rubberBand.reset( True )	# true, it's a polygon

      if startPoint.x() == endPoint.x() or startPoint.y() == endPoint.y():
        return

      point1 = QgsPoint(startPoint.x(), startPoint.y())
      point2 = QgsPoint(startPoint.x(), endPoint.y())
      point3 = QgsPoint(endPoint.x(), endPoint.y())
      point4 = QgsPoint(endPoint.x(), startPoint.y())

      self.rubberBand.addPoint( point1, False )
      self.rubberBand.addPoint( point2, False )
      self.rubberBand.addPoint( point3, False )
      self.rubberBand.addPoint( point4, True )	# true to update canvas
      self.rubberBand.show()

  def clippingRectangle(self):
      if self.startPoint == None or self.endPoint == None:
        return None
      elif self.startPoint.x() == self.endPoint.x() or self.startPoint.y() == self.endPoint.y():
        return None

      return QgsRectangle(self.startPoint, self.endPoint)

  def setClippingRectangle(self, rect):
      if rect == None:
        self.reset()
        return

      self.startPoint = QgsPoint(rect.xMaximum(), rect.yMaximum())
      self.endPoint = QgsPoint(rect.xMinimum(), rect.yMinimum())
      self.showRect(self.startPoint, self.endPoint)

  def deactivate(self):
      QgsMapTool.deactivate(self)
      self.emit(SIGNAL("deactivated()"))

