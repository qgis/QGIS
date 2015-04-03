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

from PyQt4.QtCore import Qt, QObject, SIGNAL, QVariant, QFile
from PyQt4.QtGui import QDialog, QDialogButtonBox, QDoubleValidator, QMessageBox, QApplication
from qgis.core import QGis, QgsMapLayerRegistry, QgsMapLayer, QgsRectangle, QgsFields, QgsField, QgsVectorFileWriter, QgsPoint, QgsFeature, QgsGeometry

import ftools_utils
from ui_frmVectorGrid import Ui_Dialog
import math

class Dialog(QDialog, Ui_Dialog):
    def __init__(self, iface):
        QDialog.__init__(self, iface.mainWindow())
        self.iface = iface
        self.setupUi(self)
        QObject.connect(self.toolOut, SIGNAL("clicked()"), self.outFile)
        QObject.connect(self.spnX, SIGNAL("valueChanged(double)"), self.offset)
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
            if self.addToCanvasCheck.isChecked():
                addCanvasCheck = ftools_utils.addShapeToCanvas(unicode(self.shapefileName))
                if not addCanvasCheck:
                    QMessageBox.warning( self, self.tr("Generate Vector Grid"), self.tr( "Error loading output shapefile:\n%s" ) % ( unicode( self.shapefileName ) ))
                self.populateLayers()
            else:
                QMessageBox.information(self, self.tr("Generate Vector Grid"),self.tr("Created output shapefile:\n%s" ) % ( unicode( self.shapefileName )))
        self.progressBar.setValue( 0 )
        self.buttonOk.setEnabled( True )

    def compute( self, bound, xOffset, yOffset, polygon ):
        crs = None
        layer = ftools_utils.getMapLayerByName(unicode(self.inShape.currentText()))
        
        if self.angle.value() != 0.0:
            bound = self.initRotation(bound)

        if layer is None:
          crs = self.iface.mapCanvas().mapRenderer().destinationCrs()
        else:
          crs = layer.crs()
        if not crs.isValid(): crs = None

        fields = QgsFields()
        fields.append( QgsField("ID", QVariant.Int) )
        fieldCount = 1

        if polygon:
            fields.append( QgsField("X_MIN", QVariant.Double) )
            fields.append( QgsField("X_MAX", QVariant.Double) )
            fields.append( QgsField("Y_MIN", QVariant.Double) )
            fields.append( QgsField("Y_MAX", QVariant.Double) )
            fieldCount = 5
            check = QFile(self.shapefileName)
            if check.exists():
                if not QgsVectorFileWriter.deleteShapeFile(self.shapefileName):
                    return
            writer = QgsVectorFileWriter(self.shapefileName, self.encoding, fields, QGis.WKBPolygon, crs)
        else:
            fields.append( QgsField("COORD", QVariant.Double) )
            fieldCount = 2
            check = QFile(self.shapefileName)
            if check.exists():
                if not QgsVectorFileWriter.deleteShapeFile(self.shapefileName):
                    return
            writer = QgsVectorFileWriter(self.shapefileName, self.encoding, fields, QGis.WKBLineString, crs)
        outFeat = QgsFeature()
        outFeat.initAttributes(fieldCount)
        outFeat.setFields(fields)
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
                
                if self.angle.value() != 0.0:
                    self.rotatePoint(pt1)
                    self.rotatePoint(pt2)

                line = [pt1, pt2]
                outFeat.setGeometry(outGeom.fromPolyline(line))
                outFeat.setAttribute(0, idVar)
                outFeat.setAttribute(1, y)
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

                if self.angle.value() != 0.0:
                    self.rotatePoint(pt1)
                    self.rotatePoint(pt2)

                line = [pt1, pt2]
                outFeat.setGeometry(outGeom.fromPolyline(line))
                outFeat.setAttribute(0, idVar)
                outFeat.setAttribute(1, x)
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
                    
                    if self.angle.value() != 0.0:
                        self.rotatePoint(pt1)
                        self.rotatePoint(pt2)
                        self.rotatePoint(pt3)
                        self.rotatePoint(pt4)
                        self.rotatePoint(pt5)
                        
                    polygon = [[pt1, pt2, pt3, pt4, pt5]]
                    outFeat.setGeometry(outGeom.fromPolygon(polygon))
                    outFeat.setAttribute(0, idVar)
                    outFeat.setAttribute(1, x)
                    outFeat.setAttribute(2, x + xOffset)
                    outFeat.setAttribute(3, y - yOffset)
                    outFeat.setAttribute(4, y)
                    writer.addFeature(outFeat)
                    idVar = idVar + 1
                    x = x + xOffset
                y = y - yOffset
                count += 1
                if int( math.fmod( count, count_update ) ) == 0:
                    prog = int( count / count_max * 100 )

        self.progressBar.setValue( 100 )
        del writer

    def initRotation(self,  boundBox):
        # calculate rotation parameters..interpreted from affine transformation plugin

        anchorPoint = boundBox.center()
        # We convert the angle from degree to radiant
        rad = self.angle.value()  * math.pi / 180.0

        a = math.cos( rad );
        b = -1 * math.sin( rad );
        c = anchorPoint.x() - math.cos( rad ) * anchorPoint.x() + math.sin( rad ) * anchorPoint.y();
        d = math.sin( rad );
        e = math.cos( rad );
        f = anchorPoint.y() - math.sin( rad ) * anchorPoint.x() - math.cos( rad ) * anchorPoint.y();

        self.rotationParams = (a,b,c,d,e,f)
        
        # Rotate the bounding box to set a new extent
        ptMin = QgsPoint(boundBox.xMinimum(),  boundBox.yMinimum())
        ptMax = QgsPoint(boundBox.xMaximum(),  boundBox.yMaximum())
        
        self.rotatePoint(ptMin)
        self.rotatePoint(ptMax)
        
        newBoundBox = QgsRectangle(ptMin,  ptMax)
        newBoundBox.combineExtentWith(boundBox)
        
        return newBoundBox

    def rotatePoint(self,  point):
        x = self.rotationParams[0] * point.x() + self.rotationParams[1] * point.y() + self.rotationParams[2];
        y = self.rotationParams[3] * point.x() + self.rotationParams[4] * point.y() + self.rotationParams[5];
        point.setX(x)
        point.setY(y)

    def outFile(self):
        self.outShape.clear()
        ( self.shapefileName, self.encoding ) = ftools_utils.saveDialog( self )
        if self.shapefileName is None or self.encoding is None:
            return
        self.outShape.setText( self.shapefileName )

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
