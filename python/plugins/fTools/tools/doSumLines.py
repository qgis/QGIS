# -*- coding: utf-8 -*-
#-----------------------------------------------------------
# 
# Sum Lines In Polygons
#
# A QGIS plugin for calculating the total sum of line 
# lengths in each polygon of an input vector polygon layer.
#
# Copyright (C) 2008  Carson Farmer
#
# EMAIL: carson.farmer (at) gmail.com
# WEB  : www.geog.uvic.ca/spar/carson
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
from ui_frmSumLines import Ui_Dialog

class Dialog(QDialog, Ui_Dialog):

    def __init__(self, iface):
        QDialog.__init__(self)
        self.iface = iface
        # Set up the user interface from Designer.
        self.setupUi(self)
        QObject.connect(self.toolOut, SIGNAL("clicked()"), self.outFile)
        self.setWindowTitle(self.tr("Sum line lengths"))
        # populate layer list
        self.progressBar.setValue(0)
        mapCanvas = self.iface.mapCanvas()
        layers = ftools_utils.getLayerNames([QGis.Line])
        self.inPoint.addItems(layers)
        layers = ftools_utils.getLayerNames([QGis.Polygon])
        self.inPolygon.addItems(layers)
    
    def accept(self):
        if self.inPolygon.currentText() == "":
            QMessageBox.information(self, self.tr("Sum Line Lengths In Polyons"), self.tr("Please specify input polygon vector layer"))
        elif self.outShape.text() == "":
            QMessageBox.information(self, self.tr("Sum Line Lengths In Polyons"), self.tr("Please specify output shapefile"))
        elif self.inPoint.currentText() == "":
            QMessageBox.information(self, self.tr("Sum Line Lengths In Polyons"), self.tr("Please specify input line vector layer"))
        elif self.lnField.text() == "":
            QMessageBox.information(self, self.tr("Sum Line Lengths In Polyons"), self.tr("Please specify output length field"))
        else:
            inPoly = self.inPolygon.currentText()
            inLns = self.inPoint.currentText()
            inField = self.lnField.text()
            outPath = self.outShape.text()
            if outPath.contains("\\"):
                outName = outPath.right((outPath.length() - outPath.lastIndexOf("\\")) - 1)
            else:
                outName = outPath.right((outPath.length() - outPath.lastIndexOf("/")) - 1)
            if outName.endsWith(".shp"):
                outName = outName.left(outName.length() - 4)
            self.compute(inPoly, inLns, inField, outPath, self.progressBar)
            self.outShape.clear()
            addToTOC = QMessageBox.question(self, self.tr("Sum line lengths"), self.tr("Created output shapefile:\n%1\n\nWould you like to add the new layer to the TOC?").arg(unicode(outPath)), QMessageBox.Yes, QMessageBox.No, QMessageBox.NoButton)
            if addToTOC == QMessageBox.Yes:
                self.vlayer = QgsVectorLayer(outPath, unicode(outName), "ogr")
                QgsMapLayerRegistry.instance().addMapLayer(self.vlayer)
        self.progressBar.setValue(0)

    def outFile(self):
        self.outShape.clear()
        ( self.shapefileName, self.encoding ) = ftools_utils.saveDialog( self )
        if self.shapefileName is None or self.encoding is None:
            return
        self.outShape.setText( QString( self.shapefileName ) )

    def compute(self, inPoly, inLns, inField, outPath, progressBar):
        polyLayer = ftools_utils.getVectorLayerByName(inPoly)
        lineLayer = ftools_utils.getVectorLayerByName(inLns)
        polyProvider = polyLayer.dataProvider()
        lineProvider = lineLayer.dataProvider()
        if polyProvider.crs() <> lineProvider.crs():
            QMessageBox.warning(self, self.tr("CRS warning!"), self.tr("Warning: Input layers have non-matching CRS.\nThis may cause unexpected results."))
        allAttrs = polyProvider.attributeIndexes()
        polyProvider.select(allAttrs)
        allAttrs = lineProvider.attributeIndexes()
        lineProvider.select(allAttrs)
        fieldList = ftools_utils.getFieldList(polyLayer)
        index = polyProvider.fieldNameIndex(unicode(inField))
        if index == -1:
            index = polyProvider.fieldCount()
            field = QgsField(unicode(inField), QVariant.Double, "real", 24, 15, self.tr("length field"))
            fieldList[index] = field
        sRs = polyProvider.crs()
        inFeat = QgsFeature()
        inFeatB = QgsFeature()
        outFeat = QgsFeature()
        inGeom = QgsGeometry()
        outGeom = QgsGeometry()
        distArea = QgsDistanceArea()
        lineProvider.rewind()
        start = 15.00
        add = 85.00 / polyProvider.featureCount()
        check = QFile(self.shapefileName)
        if check.exists():
            if not QgsVectorFileWriter.deleteShapeFile(self.shapefileName):
                return
        writer = QgsVectorFileWriter(self.shapefileName, self.encoding, fieldList, polyProvider.geometryType(), sRs)
        #writer = QgsVectorFileWriter(outPath, "UTF-8", fieldList, polyProvider.geometryType(), sRs)
        spatialIndex = ftools_utils.createIndex( lineProvider )
        while polyProvider.nextFeature(inFeat):
            inGeom = QgsGeometry(inFeat.geometry())
            atMap = inFeat.attributeMap()
            lineList = []
            length = 0
            #(check, lineList) = lineLayer.featuresInRectangle(inGeom.boundingBox(), True, False)
            #lineLayer.select(inGeom.boundingBox(), False)
            #lineList = lineLayer.selectedFeatures()
            lineList = spatialIndex.intersects(inGeom.boundingBox())
            if len(lineList) > 0: check = 0
            else: check = 1
            if check == 0:
                for i in lineList:
                    lineProvider.featureAtId( int( i ), inFeatB , True, allAttrs )
                    tmpGeom = QgsGeometry( inFeatB.geometry() )
                    if inGeom.intersects(tmpGeom):
                        outGeom = inGeom.intersection(tmpGeom)
                        length = length + distArea.measure(outGeom)
            outFeat.setGeometry(inGeom)
            outFeat.setAttributeMap(atMap)
            outFeat.addAttribute(index, QVariant(length))
            writer.addFeature(outFeat)
            start = start + 1
            progressBar.setValue(start)
        del writer
