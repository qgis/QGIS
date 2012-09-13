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
import ftools_utils
from qgis.core import *
from ui_frmVectorGrid import Ui_Dialog
import math

class Dialog(QDialog, Ui_Dialog):
    def __init__(self, iface):
        QDialog.__init__(self, iface.mainWindow())
        self.iface = iface
        self.setupUi(self)
        QObject.connect(self.toolOut, SIGNAL("clicked()"), self.outFile)
        QObject.connect(self.spnX, SIGNAL("valueChanged(double)"), self.offset)
        #QObject.connect(self.inShape, SIGNAL("currentIndexChanged(QString)"), self.updateInput)
        QObject.connect(self.btnUpdate, SIGNAL("clicked()"), self.updateLayer)
        QObject.connect(self.btnCanvas, SIGNAL("clicked()"), self.updateCanvas)
        QObject.connect(self.chkAlign, SIGNAL("toggled(bool)"), self.chkAlignToggled)
        self.buttonOk = self.buttonBox_2.button( QDialogButtonBox.Ok )
        self.setWindowTitle(self.tr("Vector grid"))
        self.xMin.setValidator(QDoubleValidator(self.xMin))
        self.xMax.setValidator(QDoubleValidator(self.xMax))
        self.yMin.setValidator(QDoubleValidator(self.yMin))
        self.yMax.setValidator(QDoubleValidator(self.yMax))
        self.populateLayers()

    def populateLayers( self ):
        self.inShape.clear()
        layermap = QgsMapLayerRegistry.instance().mapLayers()
        for name, layer in layermap.iteritems():
            self.inShape.addItem( unicode( layer.name() ) )
            if layer == self.iface.activeLayer():
                self.inShape.setCurrentIndex( self.inShape.count() -1 )

    def offset(self, value):
        if self.chkLock.isChecked():
            self.spnY.setValue(value)

    def updateLayer( self ):
        mLayerName = self.inShape.currentText()
        if not mLayerName == "":
            mLayer = ftools_utils.getMapLayerByName( unicode( mLayerName ) )
            # get layer extents
            boundBox = mLayer.extent()
            # if "align extents and resolution..." button is checked
            if self.chkAlign.isChecked():
                if not mLayer.type() == QgsMapLayer.RasterLayer:
                    QMessageBox.information(self, self.tr("Vector grid"), self.tr("Please select a raster layer"))
                else:
                    dx = math.fabs(boundBox.xMaximum()-boundBox.xMinimum()) / mLayer.width()
                    dy = math.fabs(boundBox.yMaximum()-boundBox.yMinimum()) / mLayer.height()
                    self.spnX.setValue(dx)
                    self.spnY.setValue(dy)
            self.updateExtents( boundBox )

    def updateCanvas( self ):
        canvas = self.iface.mapCanvas()
        boundBox = canvas.extent()
        # if "align extents and resolution..." button is checked
        if self.chkAlign.isChecked():
            mLayerName = self.inShape.currentText()
            if not mLayerName == "":
                mLayer = ftools_utils.getMapLayerByName( unicode( mLayerName ) )
                if not mLayer.type() == QgsMapLayer.RasterLayer:
                    QMessageBox.information(self, self.tr("Vector grid"), self.tr("Please select a raster layer"))
                else:
                    # get extents and pixel size
                    xMin = boundBox.xMinimum()
                    yMin = boundBox.yMinimum()
                    xMax = boundBox.xMaximum()
                    yMax = boundBox.yMaximum()
                    boundBox2 = mLayer.extent()
                    dx = math.fabs(boundBox2.xMaximum()-boundBox2.xMinimum()) / mLayer.width()
                    dy = math.fabs(boundBox2.yMaximum()-boundBox2.yMinimum()) / mLayer.height()
                    # get pixels from the raster that are closest to the desired extent
                    newXMin = self.getClosestPixel( boundBox2.xMinimum(), boundBox.xMinimum(), dx, True )
                    newXMax = self.getClosestPixel( boundBox2.xMaximum(), boundBox.xMaximum(), dx, False )
                    newYMin = self.getClosestPixel( boundBox2.yMinimum(), boundBox.yMinimum(), dy, True )
                    newYMax = self.getClosestPixel( boundBox2.yMaximum(), boundBox.yMaximum(), dy, False )
                    # apply new values if found all min/max
                    if newXMin is not None and newXMax is not None and newYMin is not None and newYMax is not None:
                        boundBox.set( newXMin, newYMin, newXMax, newYMax )
                        self.spnX.setValue(dx)
                        self.spnY.setValue(dy)
                    else:
                        QMessageBox.information(self, self.tr("Vector grid"), self.tr("Unable to compute extents aligned on selected raster layer"))
        self.updateExtents( boundBox )

    def updateExtents( self, boundBox ):
        self.xMin.setText( unicode( boundBox.xMinimum() ) )
        self.yMin.setText( unicode( boundBox.yMinimum() ) )
        self.xMax.setText( unicode( boundBox.xMaximum() ) )
        self.yMax.setText( unicode( boundBox.yMaximum() ) )

    def accept(self):
        self.buttonOk.setEnabled( False )
        if self.xMin.text() == "" or self.xMax.text() == "" or self.yMin.text() == "" or self.yMax.text() == "":
            QMessageBox.information(self, self.tr("Vector grid"), self.tr("Please specify valid extent coordinates"))
        elif self.outShape.text() == "":
            QMessageBox.information(self, self.tr("Vector grid"), self.tr("Please specify output shapefile"))
        else:
            try:
                boundBox = QgsRectangle(
                float( self.xMin.text() ),
                float( self.yMin.text() ),
                float( self.xMax.text() ),
                float( self.yMax.text() ) )
            except:
                QMessageBox.information(self, self.tr("Vector grid"), self.tr("Invalid extent coordinates entered"))
            xSpace = self.spnX.value()
            ySpace = self.spnY.value()
            if self.rdoPolygons.isChecked():
              polygon = True
            else:
              polygon = False
            self.outShape.clear()
            QApplication.setOverrideCursor(Qt.WaitCursor)
            self.compute( boundBox, xSpace, ySpace, polygon )
            QApplication.restoreOverrideCursor()
            addToTOC = QMessageBox.question(self, self.tr("Generate Vector Grid"), self.tr("Created output shapefile:\n%1\n\nWould you like to add the new layer to the TOC?").arg(unicode(self.shapefileName)), QMessageBox.Yes, QMessageBox.No, QMessageBox.NoButton)
            if addToTOC == QMessageBox.Yes:
                ftools_utils.addShapeToCanvas( self.shapefileName )
                self.populateLayers()
        self.progressBar.setValue( 0 )
        self.buttonOk.setEnabled( True )

    def compute( self, bound, xOffset, yOffset, polygon ):
        crs = None
        layer = ftools_utils.getMapLayerByName(unicode(self.inShape.currentText()))

        if layer is None:
          crs = self.iface.mapCanvas().mapRenderer().destinationCrs()
        else:
          crs = layer.crs()
        if not crs.isValid(): crs = None
        if polygon:
            fields = {0:QgsField("ID", QVariant.Int), 1:QgsField("XMIN", QVariant.Double), 2:QgsField("XMAX", QVariant.Double),
            3:QgsField("YMIN", QVariant.Double), 4:QgsField("YMAX", QVariant.Double)}
            check = QFile(self.shapefileName)
            if check.exists():
                if not QgsVectorFileWriter.deleteShapeFile(self.shapefileName):
                    return
            writer = QgsVectorFileWriter(self.shapefileName, self.encoding, fields, QGis.WKBPolygon, crs)
        else:
            fields = {0:QgsField("ID", QVariant.Int), 1:QgsField("COORD", QVariant.Double)}
            check = QFile(self.shapefileName)
            if check.exists():
                if not QgsVectorFileWriter.deleteShapeFile(self.shapefileName):
                    return
            writer = QgsVectorFileWriter(self.shapefileName, self.encoding, fields, QGis.WKBLineString, crs)
        outFeat = QgsFeature()
        outGeom = QgsGeometry()
        idVar = 0
        self.progressBar.setValue( 0 )
        if not polygon:
            # counters for progressbar - update every 5%
            count = 0
            count_max = (bound.yMaximum() - bound.yMinimum()) / yOffset
            count_update = count_max * 0.10
            y = bound.yMaximum()
            while y >= bound.yMinimum():
                pt1 = QgsPoint(bound.xMinimum(), y)
                pt2 = QgsPoint(bound.xMaximum(), y)
                line = [pt1, pt2]
                outFeat.setGeometry(outGeom.fromPolyline(line))
                outFeat.addAttribute(0, QVariant(idVar))
                outFeat.addAttribute(1, QVariant(y))
                writer.addFeature(outFeat)
                y = y - yOffset
                idVar = idVar + 1
                count += 1
                if int( math.fmod( count, count_update ) ) == 0:
                    prog = int( count / count_max * 50 )
                    self.progressBar.setValue( prog )
            self.progressBar.setValue( 50 )
            # counters for progressbar - update every 5%
            count = 0
            count_max = (bound.xMaximum() - bound.xMinimum()) / xOffset
            count_update = count_max * 0.10
            x = bound.xMinimum()
            while x <= bound.xMaximum():
                pt1 = QgsPoint(x, bound.yMaximum())
                pt2 = QgsPoint(x, bound.yMinimum())
                line = [pt1, pt2]
                outFeat.setGeometry(outGeom.fromPolyline(line))
                outFeat.addAttribute(0, QVariant(idVar))
                outFeat.addAttribute(1, QVariant(x))
                writer.addFeature(outFeat)
                x = x + xOffset
                idVar = idVar + 1
                count += 1
                if int( math.fmod( count, count_update ) ) == 0:
                    prog = 50 + int( count / count_max * 50 )
                    self.progressBar.setValue( prog )
        else:
            # counters for progressbar - update every 5%
            count = 0
            count_max = (bound.yMaximum() - bound.yMinimum()) / yOffset
            count_update = count_max * 0.05
            y = bound.yMaximum()
            while y >= bound.yMinimum():
                x = bound.xMinimum()
                while x <= bound.xMaximum():
                    pt1 = QgsPoint(x, y)
                    pt2 = QgsPoint(x + xOffset, y)
                    pt3 = QgsPoint(x + xOffset, y - yOffset)
                    pt4 = QgsPoint(x, y - yOffset)
                    pt5 = QgsPoint(x, y)
                    polygon = [[pt1, pt2, pt3, pt4, pt5]]
                    outFeat.setGeometry(outGeom.fromPolygon(polygon))
                    outFeat.addAttribute(0, QVariant(idVar))
                    outFeat.addAttribute(1, QVariant(x))
                    outFeat.addAttribute(2, QVariant(x + xOffset))
                    outFeat.addAttribute(3, QVariant(y - yOffset))
                    outFeat.addAttribute(4, QVariant(y))
                    writer.addFeature(outFeat)
                    idVar = idVar + 1
                    x = x + xOffset
                y = y - yOffset
                count += 1
                if int( math.fmod( count, count_update ) ) == 0:
                    prog = int( count / count_max * 100 )

        self.progressBar.setValue( 100 )
        #self.progressBar.setRange( 0, 100 )
        del writer

    def outFile(self):
        self.outShape.clear()
        ( self.shapefileName, self.encoding ) = ftools_utils.saveDialog( self )
        if self.shapefileName is None or self.encoding is None:
            return
        self.outShape.setText( QString( self.shapefileName ) )

    def chkAlignToggled(self):
        if self.chkAlign.isChecked():
            self.spnX.setEnabled( False )
            self.lblX.setEnabled( False )
            self.spnY.setEnabled( False )
            self.lblY.setEnabled( False )
        else:
            self.spnX.setEnabled( True )
            self.lblX.setEnabled( True )
            self.spnY.setEnabled( not self.chkLock.isChecked() )
            self.lblY.setEnabled( not self.chkLock.isChecked() )

    def getClosestPixel(self, startVal, targetVal, step, isMin ):
        foundVal = None
        tmpVal = startVal
        # find pixels covering the extent - slighlyt inneficient b/c loop on all elements before xMin
        if targetVal < startVal:
            backOneStep = not isMin
            step = - step
            while foundVal is None:
                if tmpVal <= targetVal:
                    if backOneStep:
                        tmpVal -= step
                    foundVal = tmpVal
                tmpVal += step
        else:
            backOneStep = isMin
            while foundVal is None:
                if tmpVal >= targetVal:
                    if backOneStep:
                        tmpVal -= step
                    foundVal = tmpVal
                tmpVal += step
        return foundVal

