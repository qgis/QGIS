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
from random import *
import math, ftools_utils
from ui_frmRandPoints import Ui_Dialog

class Dialog(QDialog, Ui_Dialog):
    def __init__(self, iface):
        QDialog.__init__(self, iface.mainWindow())
        self.iface = iface
        self.setupUi(self)
        QObject.connect(self.toolOut, SIGNAL("clicked()"), self.outFile)
        QObject.connect(self.inShape, SIGNAL("currentIndexChanged(QString)"), self.update)
        self.progressBar.setValue(0)
        self.setWindowTitle(self.tr("Random Points"))
        self.buttonOk = self.buttonBox_2.button( QDialogButtonBox.Ok )
        self.populateLayers()

    def populateLayers( self ):
        layers = ftools_utils.getLayerNames([QGis.Polygon, "Raster"])
        QObject.disconnect(self.inShape, SIGNAL("currentIndexChanged(QString)"), self.update)
        self.inShape.clear()
        self.inShape.addItems(layers)
        QObject.connect(self.inShape, SIGNAL("currentIndexChanged(QString)"), self.update)

# If input layer is changed, update field list
    def update(self, inputLayer):
        self.cmbField.clear()
        changedLayer = ftools_utils.getMapLayerByName(unicode(inputLayer))
        if changedLayer.type() == changedLayer.VectorLayer:
            self.rdoStratified.setEnabled(True)
            self.rdoDensity.setEnabled(True)
            self.rdoField.setEnabled(True)
            self.label_4.setEnabled(True)
            changedLayer = ftools_utils.getVectorLayerByName(inputLayer)
            changedFields = ftools_utils.getFieldList(changedLayer)
            for i in changedFields:
                self.cmbField.addItem(unicode(changedFields[i].name()))
        else:
            self.rdoUnstratified.setChecked(True)
            self.rdoStratified.setEnabled(False)
            self.rdoDensity.setEnabled(False)
            self.rdoField.setEnabled(False)
            self.spnStratified.setEnabled(False)
            self.spnDensity.setEnabled(False)
            self.cmbField.setEnabled(False)
            self.label_4.setEnabled(False)

# when 'OK' button is pressed, gather required inputs, and initiate random points generation
    def accept(self):
        self.buttonOk.setEnabled( False )
        if self.inShape.currentText() == "":
            QMessageBox.information(self, self.tr("Random Points"), self.tr("No input layer specified"))
        elif self.outShape.text() == "":
            QMessageBox.information(self, self.tr("Random Points"), self.tr("Please specify output shapefile"))
        else:
            inName = self.inShape.currentText()
            self.progressBar.setValue(1)
            outPath = self.outShape.text()
            self.progressBar.setValue(2.5)
            if outPath.contains("\\"):
                outName = outPath.right((outPath.length() - outPath.lastIndexOf("\\")) - 1)
            else:
                outName = outPath.right((outPath.length() - outPath.lastIndexOf("/")) - 1)
            if outName.endsWith(".shp"):
                outName = outName.left(outName.length() - 4)
            self.progressBar.setValue(5)
            mLayer = ftools_utils.getMapLayerByName(unicode(inName))
            if mLayer.type() == mLayer.VectorLayer:
                inLayer = QgsVectorLayer(unicode(mLayer.source()),  unicode(mLayer.name()),  unicode(mLayer.dataProvider().name()))
                if self.rdoUnstratified.isChecked():
                    design = self.tr("unstratified")
                    value = self.spnUnstratified.value()
                elif self.rdoStratified.isChecked():
                    design = self.tr("stratified")
                    value = self.spnStratified.value()
                elif self.rdoDensity.isChecked():
                    design = self.tr("density")
                    value = self.spnDensity.value()
                else:
                    design = self.tr("field")
                    value = unicode(self.cmbField.currentText())
            elif mLayer.type() == mLayer.RasterLayer:
                inLayer = QgsRasterLayer(unicode(mLayer.source()), unicode(mLayer.name()))
                design = self.tr("unstratified")
                value = self.spnUnstratified.value()
            else:
                QMessageBox.information(self, self.tr("Random Points"), self.tr("Unknown layer type..."))
            minimum = 0.00
            self.progressBar.setValue(10)
            self.randomize(inLayer, outPath, minimum, design, value)
            self.progressBar.setValue(100)
            self.outShape.clear()
            addToTOC = QMessageBox.question(self, self.tr("Random Points"),
            self.tr("Created output point shapefile:\n%1\n\nWould you like to add the new layer to the TOC?").arg(outPath), QMessageBox.Yes, QMessageBox.No, QMessageBox.NoButton)
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

# combine all polygons in layer to create single polygon (slow for complex polygons)
    def createSinglePolygon(self, vlayer):
        provider = vlayer.dataProvider()
        allAttrs = provider.attributeIndexes()
        provider.select(allAttrs)
        feat = QgsFeature()
        geom = QgsGeometry()
        #geom2 = QgsGeometry()
        provider.nextFeature(feat)
        geom = QgsGeometry(feat.geometry())
        count = 10.00
        add = ( 40.00 - 10.00 ) / provider.featureCount()
        #provider.rewind()
        #provider.nextFeature(feat)
        #geom = QgsGeometry(feat.geometry())
        while provider.nextFeature(feat):
            geom = geom.combine(QgsGeometry( feat.geometry() ))
            count = count + add
            self.progressBar.setValue(count)
        return geom

# Generate list of random points
    def simpleRandom(self, n, bound, xmin, xmax, ymin, ymax):
        seed()
        points = []
        i = 1
        count = 40.00
        if n == 0:
          return []
        add = ( 70.00 - 40.00 ) / n
        while i <= n:
            pGeom = QgsGeometry().fromPoint(QgsPoint(xmin + (xmax-xmin) * random(), ymin + (ymax-ymin) * random()))
            if pGeom.intersects(bound):
                points.append(pGeom)
                i = i + 1
                count = count + add
                self.progressBar.setValue(count)
        return points

    def vectorRandom(self, n, layer, xmin, xmax, ymin, ymax):
        provider = layer.dataProvider()
        provider.select([])
        index = ftools_utils.createIndex(provider)
        seed()
        points = []
        feat = QgsFeature()
        i = 1
        count = 40.00
        add = ( 70.00 - 40.00 ) / n
        while i <= n:
            point = QgsPoint(xmin + (xmax-xmin) * random(), ymin + (ymax-ymin) * random())
            pGeom = QgsGeometry().fromPoint(point)
            ids = index.intersects(pGeom.buffer(5,5).boundingBox())
            for id in ids:
                provider.featureAtId(int(id),feat,True)
                tGeom = QgsGeometry(feat.geometry())
                if pGeom.intersects(tGeom):
                    points.append(pGeom)
                    i = i + 1
                    count = count + add
                    self.progressBar.setValue(count)
                    break
        return points

    def randomize(self, inLayer, outPath, minimum, design, value):
        outFeat = QgsFeature()
        if design == self.tr("unstratified"):
            ext = inLayer.extent()
            if inLayer.type() == inLayer.RasterLayer:
                points = self.simpleRandom(int(value), ext, ext.xMinimum(),
                ext.xMaximum(), ext.yMinimum(), ext.yMaximum())
            else:
                points = self.vectorRandom(int(value), inLayer,
                ext.xMinimum(), ext.xMaximum(), ext.yMinimum(), ext.yMaximum())
        else: points = self.loopThruPolygons(inLayer, value, design)
        crs = self.iface.mapCanvas().mapRenderer().destinationSrs()
        if not crs.isValid(): crs = None
        fields = { 0 : QgsField("ID", QVariant.Int) }
        check = QFile(self.shapefileName)
        if check.exists():
            if not QgsVectorFileWriter.deleteShapeFile(self.shapefileName):
                return
        writer = QgsVectorFileWriter(self.shapefileName, self.encoding, fields, QGis.WKBPoint, crs)
        idVar = 0
        count = 70.00
        add = ( 100.00 - 70.00 ) / len(points)
        for i in points:
            outFeat.setGeometry(i)
            outFeat.addAttribute(0, QVariant(idVar))
            writer.addFeature(outFeat)
            idVar = idVar + 1
            count = count + add
            self.progressBar.setValue(count)
        del writer

#
    def loopThruPolygons(self, inLayer, numRand, design):
        sProvider = inLayer.dataProvider()
        sAllAttrs = sProvider.attributeIndexes()
        sProvider.select(sAllAttrs)
        sFeat = QgsFeature()
        sGeom = QgsGeometry()
        sPoints = []
        if design == self.tr("field"):
            for (i, attr) in sProvider.fields().iteritems():
                if (unicode(numRand) == attr.name()):
                    index = i #get input field index
                    break
        count = 10.00
        add = 60.00 / sProvider.featureCount()
        while sProvider.nextFeature(sFeat):
            sGeom = sFeat.geometry()
            if design == self.tr("density"):
                sDistArea = QgsDistanceArea()
                value = int(round(numRand * sDistArea.measure(sGeom)))
            elif design == self.tr("field"):
                sAtMap = sFeat.attributeMap()
                value = sAtMap[index].toInt()[0]
            else:
                value = numRand
            sExt = sGeom.boundingBox()
            sPoints.extend(self.simpleRandom(value, sGeom, sExt.xMinimum(), sExt.xMaximum(), sExt.yMinimum(), sExt.yMaximum()))
            count = count + add
            self.progressBar.setValue(count)
        return sPoints
