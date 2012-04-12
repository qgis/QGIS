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
from random import *
from math import *
from ui_frmRegPoints import Ui_Dialog

class Dialog(QDialog, Ui_Dialog):
    def __init__(self, iface):
        QDialog.__init__(self, iface.mainWindow())
        self.iface = iface
        self.setupUi(self)
        self.xMin.setValidator(QDoubleValidator(self.xMin))
        self.xMax.setValidator(QDoubleValidator(self.xMax))
        self.yMin.setValidator(QDoubleValidator(self.yMin))
        self.yMax.setValidator(QDoubleValidator(self.yMax))
        QObject.connect(self.toolOut, SIGNAL("clicked()"), self.outFile)
        self.setWindowTitle( self.tr("Regular points") )
        self.buttonOk = self.buttonBox_2.button( QDialogButtonBox.Ok )
        self.progressBar.setValue(0)
        self.mapCanvas = self.iface.mapCanvas()
        self.populateLayers()

    def populateLayers( self ):
        layers = ftools_utils.getLayerNames("all")
        self.inShape.clear()
        self.inShape.addItems(layers)

    def accept(self):
        self.buttonOk.setEnabled( False )
        if not self.rdoCoordinates.isChecked() and self.inShape.currentText() == "":
            QMessageBox.information(self, self.tr("Generate Regular Points"), self.tr("Please specify input layer"))
        elif self.rdoCoordinates.isChecked() and (self.xMin.text() == "" or self.xMax.text() == "" or self.yMin.text() == "" or self.yMax.text() == ""):
            QMessageBox.information(self, self.tr("Generate Regular Points"), self.tr("Please properly specify extent coordinates"))
        elif self.outShape.text() == "":
            QMessageBox.information(self, self.tr("Generate Regular Points"), self.tr("Please specify output shapefile"))
        else:
            inName = self.inShape.currentText()
            outPath = self.outShape.text()
            self.outShape.clear()
            if outPath.contains("\\"):
                outName = outPath.right((outPath.length() - outPath.lastIndexOf("\\")) - 1)
            else:
                outName = outPath.right((outPath.length() - outPath.lastIndexOf("/")) - 1)
            if outName.endsWith(".shp"):
                outName = outName.left(outName.length() - 4)
            if self.rdoSpacing.isChecked(): value = self.spnSpacing.value()
            else: value = self.spnNumber.value()
            if self.chkRandom.isChecked(): offset = True
            else: offset = False
            if self.rdoBoundary.isChecked():
                mLayer = ftools_utils.getMapLayerByName(unicode(inName))
                boundBox = mLayer.extent()
                crs = mLayer.crs()
            else:
                boundBox = QgsRectangle(float(self.xMin.text()), float(self.yMin.text()), float(self.xMax.text()), float(self.yMax.text()))
                crs = self.mapCanvas.mapRenderer().destinationSrs()
                print crs.isValid()
                if not crs.isValid(): crs = None
            self.regularize(boundBox, outPath, offset, value, self.rdoSpacing.isChecked(), self.spnInset.value(), crs)
            addToTOC = QMessageBox.question(self, self.tr("Generate Regular Points"), self.tr("Created output point shapefile:\n%1\n\nWould you like to add the new layer to the TOC?").arg( outPath ), QMessageBox.Yes, QMessageBox.No, QMessageBox.NoButton)
            if addToTOC == QMessageBox.Yes:
                self.vlayer = QgsVectorLayer(outPath, unicode(outName), "ogr")
                QgsMapLayerRegistry.instance().addMapLayer(self.vlayer)
                self.populateLayers()
        self.progressBar.setValue(0)
        self.buttonOk.setEnabled( True )

    def outFile(self):
        self.outShape.clear()
        ( self.shapefileName, self.encoding ) = ftools_utils.saveDialog( self )
        if self.shapefileName is None or self.encoding is None:
            return
        self.outShape.setText( QString( self.shapefileName ) )

# Generate list of random points
    def simpleRandom(self, n, bound, xmin, xmax, ymin, ymax):
        seed()
        points = []
        i = 1
        while i <= n:
            pGeom = QgsGeometry().fromPoint(QgsPoint(xmin + (xmax-xmin) * random(), ymin + (ymax-ymin) * random()))
            if pGeom.intersects(bound):
                points.append(pGeom)
                i = i + 1
        return points

    def regularize(self, bound, outPath, offset, value, gridType, inset, crs):
        area = bound.width() * bound.height()
        if offset:
            seed()
        if gridType:
            pointSpacing = value
        else:
            # Calculate grid spacing
            pointSpacing = sqrt(area / value)
        outFeat = QgsFeature()
        fields = { 0 : QgsField("ID", QVariant.Int) }
        check = QFile(self.shapefileName)
        if check.exists():
            if not QgsVectorFileWriter.deleteShapeFile(self.shapefileName):
                return
        writer = QgsVectorFileWriter(self.shapefileName, self.encoding, fields, QGis.WKBPoint, crs)
        #writer = QgsVectorFileWriter(unicode(outPath), "CP1250", fields, QGis.WKBPoint, None)
        idVar = 0
        count = 10.00
        add = 90.00 / (area / pointSpacing)
        y = bound.yMaximum() - inset
        while y >= bound.yMinimum():
            x = bound.xMinimum() + inset
            while x <= bound.xMaximum():
                if offset:
                    pGeom = QgsGeometry().fromPoint(QgsPoint(uniform(x - (pointSpacing / 2.0), x + (pointSpacing / 2.0)),
                    uniform(y - (pointSpacing / 2.0), y + (pointSpacing / 2.0))))
                else:
                    pGeom = QgsGeometry().fromPoint(QgsPoint(x, y))
                if pGeom.intersects(bound):
                    outFeat.setGeometry(pGeom)
                    outFeat.addAttribute(0, QVariant(idVar))
                    writer.addFeature(outFeat)
                    idVar = idVar + 1
                    x = x + pointSpacing
                    count = count + add
                    self.progressBar.setValue(count)
            y = y - pointSpacing
        del writer

