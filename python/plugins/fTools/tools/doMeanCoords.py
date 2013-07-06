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
from math import *
from ui_frmMeanCoords import Ui_Dialog

class Dialog(QDialog, Ui_Dialog):
    def __init__(self, iface, function):
        QDialog.__init__(self, iface.mainWindow())
        self.iface = iface
        self.function = function
        self.setupUi(self)
        self.updateUi()
        QObject.connect(self.toolOut, SIGNAL("clicked()"), self.outFile)
        QObject.connect(self.inShape, SIGNAL("currentIndexChanged(QString)"), self.update)
        self.buttonOk = self.buttonBox_2.button( QDialogButtonBox.Ok )
        self.progressBar.setValue(0)
        self.populateLayers()

    def populateLayers( self ):
        layers = ftools_utils.getLayerNames([QGis.Point, QGis.Line, QGis.Polygon])
        self.inShape.blockSignals(True)
        self.inShape.clear()
        self.inShape.blockSignals(False)
        self.inShape.addItems(layers)

    def updateUi(self):
        if self.function == 1:
            self.setWindowTitle( self.tr("Mean coordinates") )
            self.sizeValue.setVisible(False)
            self.label_size.setVisible(False)
        elif self.function == 2:
            self.setWindowTitle( self.tr("Standard distance") )
        self.resize(381, 100)

    def update(self, inputLayer):
        self.weightField.clear()
        self.uniqueField.clear()
        self.weightField.addItem( self.tr("(Optional) Weight field") )
        self.uniqueField.addItem( self.tr("(Optional) Unique ID field") )
        self.changedLayer = ftools_utils.getVectorLayerByName(inputLayer)
        changedField = ftools_utils.getFieldList(self.changedLayer)
        for f in changedField:
            if f.type() == QVariant.Int or f.type() == QVariant.Double:
                self.weightField.addItem(unicode(f.name()))
            self.uniqueField.addItem(unicode(f.name()))

    def accept(self):
        self.buttonOk.setEnabled( False )
        if self.inShape.currentText() == "":
            QMessageBox.information(self, self.tr("Coordinate statistics"), self.tr("No input vector layer specified"))
        elif self.outShape.text() == "":
            QMessageBox.information(self, self.tr("Coordinate statistics"), self.tr("Please specify output shapefile"))
        else:
            inName = self.inShape.currentText()
            outPath = self.outShape.text()
            outName = ftools_utils.getShapefileName(outPath)

            self.compute(inName, outPath, self.weightField.currentText(), self.sizeValue.value(), self.uniqueField.currentText())
            self.progressBar.setValue(100)
            self.outShape.clear()
            addToTOC = QMessageBox.question(self, self.tr("Coordinate statistics"), self.tr("Created output point shapefile:\n%s\n\nWould you like to add the new layer to the TOC?") % ( outPath ), QMessageBox.Yes, QMessageBox.No, QMessageBox.NoButton)
            if addToTOC == QMessageBox.Yes:
                vlayer = QgsVectorLayer(outPath, unicode(outName), "ogr")
                QgsMapLayerRegistry.instance().addMapLayers([vlayer])
                self.populateLayers()
        self.progressBar.setValue(0)
        self.buttonOk.setEnabled( True )

    def outFile(self):
        self.outShape.clear()
        ( self.shapefileName, self.encoding ) = ftools_utils.saveDialog( self )
        if self.shapefileName is None or self.encoding is None:
            return
        self.outShape.setText( self.shapefileName )

    def compute(self, inName, outName, weightField="", times=1, uniqueField=""):
        vlayer = ftools_utils.getVectorLayerByName(inName)
        provider = vlayer.dataProvider()
        weightIndex = provider.fieldNameIndex(weightField)
        uniqueIndex = provider.fieldNameIndex(uniqueField)
        feat = QgsFeature()
        sRs = provider.crs()
        check = QFile(self.shapefileName)
        if check.exists():
            if not QgsVectorFileWriter.deleteShapeFile(self.shapefileName):
                return
        if uniqueIndex <> -1:
            uniqueValues = ftools_utils.getUniqueValues(provider, int( uniqueIndex ) )
            single = False
        else:
            uniqueValues = [1]
            single = True
        if self.function == 2:
            fieldList = QgsFields()
            fieldList.append( QgsField("STD_DIST", QVariant.Double) )
            fieldList.append( QgsField("UID", QVariant.String) )
            writer = QgsVectorFileWriter(self.shapefileName, self.encoding, fieldList, QGis.WKBPolygon, sRs)
        else:
            fieldList = QgsFields()
            fieldList.append( QgsField("MEAN_X", QVariant.Double) )
            fieldList.append( QgsField("MEAN_Y", QVariant.Double) )
            fieldList.append( QgsField("UID", QVariant.String) )
            writer = QgsVectorFileWriter(self.shapefileName, self.encoding, fieldList, QGis.WKBPoint, sRs)
        outfeat = QgsFeature()
        outfeat.setFields( fieldList )
        points = []
        weights = []
        nFeat = provider.featureCount() * len(uniqueValues)
        nElement = 0
        self.progressBar.setValue(0)
        self.progressBar.setRange(0, nFeat)
        for j in uniqueValues:
            cx = 0.00
            cy = 0.00
            points = []
            weights = []
            fit = provider.getFeatures()
            while fit.nextFeature(feat):
                nElement += 1
                self.progressBar.setValue(nElement)
                if single:
                    check = unicode(j).strip()
                else:
                    check = unicode(feat[uniqueIndex]).strip()
                if check == unicode(j).strip():
                    cx = 0.00
                    cy = 0.00
                    if weightIndex == -1:
                        weight = 1.00
                    else:
                        weight = float(feat[weightIndex])
                    geom = QgsGeometry(feat.geometry())
                    geom = ftools_utils.extractPoints(geom)
                    for i in geom:
                        cx += i.x()
                        cy += i.y()
                    points.append(QgsPoint((cx / len(geom)), (cy / len(geom))))
                    weights.append(weight)
            sumWeight = sum(weights)
            cx = 0.00
            cy = 0.00
            item = 0
            for item, i in enumerate(points):
                cx += i.x() * weights[item]
                cy += i.y() * weights[item]
            cx = cx / sumWeight
            cy = cy / sumWeight
            meanPoint = QgsPoint(cx, cy)
            if self.function == 2:
                values = []
                md = 0.00
                sd = 0.00
                dist = QgsDistanceArea()
                item = 0
                for i in points:
                    tempDist = dist.measureLine(i, meanPoint)
                    values.append(tempDist)
                    item += 1
                    md += tempDist
                md = md / item
                for i in values:
                    sd += (i-md)*(i-md)
                sd = sqrt(sd/item)
                outfeat.setGeometry(QgsGeometry.fromPoint(meanPoint).buffer(sd * times, 10))
                outfeat.setAttribute(0, sd)
                outfeat.setAttribute(1, j)
            else:
                outfeat.setGeometry(QgsGeometry.fromPoint(meanPoint))
                outfeat.setAttribute(0, cx)
                outfeat.setAttribute(1, cy)
                outfeat.setAttribute(2, j)
            writer.addFeature(outfeat)
            if single:
                break
        del writer
