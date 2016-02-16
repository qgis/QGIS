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

from PyQt4.QtCore import QObject, SIGNAL, QFile
from PyQt4.QtGui import QDialog, QDialogButtonBox, QMessageBox
import ftools_utils
from qgis.core import QGis, QgsFields, QgsVectorFileWriter, QgsFeatureRequest, QgsFeature, QgsGeometry
from ui_frmIntersectLines import Ui_Dialog


class Dialog(QDialog, Ui_Dialog):

    def __init__(self, iface):
        QDialog.__init__(self, iface.mainWindow())
        self.iface = iface
        # Set up the user interface from Designer.
        self.setupUi(self)
        QObject.connect(self.toolOut, SIGNAL("clicked()"), self.outFile)
        QObject.connect(self.inLine1, SIGNAL("currentIndexChanged(QString)"), self.update1)
        QObject.connect(self.inLine2, SIGNAL("currentIndexChanged(QString)"), self.update2)
        self.setWindowTitle(self.tr("Line intersections"))
        self.buttonOk = self.buttonBox_2.button(QDialogButtonBox.Ok)
        self.progressBar.setValue(0)
        self.populateLayers()

    def populateLayers(self):
        layers = ftools_utils.getLayerNames([QGis.Line])
        self.inLine1.blockSignals(True)
        self.inLine2.blockSignals(True)
        self.inLine1.clear()
        self.inLine2.clear()
        self.inLine1.blockSignals(False)
        self.inLine2.blockSignals(False)
        self.inLine1.addItems(layers)
        self.inLine2.addItems(layers)

    def update1(self, inputLayer):
        self.inField1.clear()
        changedLayer = ftools_utils.getVectorLayerByName(unicode(inputLayer))
        changedField = ftools_utils.getFieldList(changedLayer)
        for f in changedField:
            self.inField1.addItem(unicode(f.name()))

    def update2(self, inputLayer):
        self.inField2.clear()
        changedLayer = ftools_utils.getVectorLayerByName(unicode(inputLayer))
        changedField = ftools_utils.getFieldList(changedLayer)
        for f in changedField:
            self.inField2.addItem(unicode(f.name()))

    def accept(self):
        self.buttonOk.setEnabled(False)
        if self.inLine1.currentText() == "":
            QMessageBox.information(self, self.tr("Locate Line Intersections"), self.tr("Please specify input line layer"))
        elif self.outShape.text() == "":
            QMessageBox.information(self, self.tr("Locate Line Intersections"), self.tr("Please specify output shapefile"))
        elif self.inLine2.currentText() == "":
            QMessageBox.information(self, self.tr("Locate Line Intersections"), self.tr("Please specify line intersect layer"))
        elif self.inField1.currentText() == "":
            QMessageBox.information(self, self.tr("Locate Line Intersections"), self.tr("Please specify input unique ID field"))
        elif self.inField2.currentText() == "":
            QMessageBox.information(self, self.tr("Locate Line Intersections"), self.tr("Please specify intersect unique ID field"))
        else:
            line1 = self.inLine1.currentText()
            line2 = self.inLine2.currentText()
            field1 = self.inField1.currentText()
            field2 = self.inField2.currentText()
            outPath = self.outShape.text()
            self.outShape.clear()
            self.compute(line1, line2, field1, field2, outPath, self.progressBar)
            self.progressBar.setValue(100)
            if self.addToCanvasCheck.isChecked():
                addCanvasCheck = ftools_utils.addShapeToCanvas(unicode(outPath))
                if not addCanvasCheck:
                    QMessageBox.warning(self, self.tr("Geoprocessing"), self.tr("Error loading output shapefile:\n%s") % (unicode(outPath)))
                self.populateLayers()
            else:
                QMessageBox.information(self, self.tr("Generate Centroids"), self.tr("Created output shapefile:\n%s") % (unicode(outPath)))
        self.progressBar.setValue(0)
        self.buttonOk.setEnabled(True)

    def outFile(self):
        self.outShape.clear()
        (self.shapefileName, self.encoding) = ftools_utils.saveDialog(self)
        if self.shapefileName is None or self.encoding is None:
            return
        self.outShape.setText(self.shapefileName)

    def compute(self, line1, line2, field1, field2, outPath, progressBar):

        layer1 = ftools_utils.getVectorLayerByName(line1)
        provider1 = layer1.dataProvider()
        fieldList = ftools_utils.getFieldList(layer1)
        index1 = provider1.fieldNameIndex(field1)
        field1 = fieldList[index1]
        field1.setName(unicode(field1.name()) + "_1")

        layer2 = ftools_utils.getVectorLayerByName(line2)
        provider2 = layer2.dataProvider()
        fieldList = ftools_utils.getFieldList(layer2)
        index2 = provider2.fieldNameIndex(field2)
        field2 = fieldList[index2]
        field2.setName(unicode(field2.name()) + "_2")

        fieldList = QgsFields()
        fieldList.append(field1)
        fieldList.append(field2)
        sRs = provider1.crs()
        check = QFile(self.shapefileName)
        if check.exists():
            if not QgsVectorFileWriter.deleteShapeFile(self.shapefileName):
                return

        writer = QgsVectorFileWriter(self.shapefileName, self.encoding, fieldList, QGis.WKBPoint, sRs)
        inFeat = QgsFeature()
        inFeatB = QgsFeature()
        outFeat = QgsFeature()
        outFields = QgsFields()
        outFields.append(field1)
        outFields.append(field2)
        outFeat.setFields(outFields)
        start = 15.00
        add = 85.00 / layer1.featureCount()

        index = ftools_utils.createIndex(provider2)

        singlelayer_tempList = []
        fit1 = provider1.getFeatures(QgsFeatureRequest().setSubsetOfAttributes([index1]))
        while fit1.nextFeature(inFeat):
            inGeom = inFeat.geometry()
            v1 = inFeat.attributes()[index1]

            lineList = index.intersects(inGeom.boundingBox())
            for i in lineList:
                provider2.getFeatures(QgsFeatureRequest().setFilterFid(int(i)).setSubsetOfAttributes([index2])).nextFeature(inFeatB)
                tmpGeom = QgsGeometry(inFeatB.geometry())
                v2 = inFeatB.attributes()[index2]

                if inGeom.intersects(tmpGeom):
                    tempGeom = inGeom.intersection(tmpGeom)
                    if tempGeom.type() == QGis.Point:
                        tempList = []
                        if tempGeom.isMultipart():
                            tempList = tempGeom.asMultiPoint()
                        else:
                            tempList.append(tempGeom.asPoint())

                        for j in tempList:
                            # if same layer, avoid insert duplicated points
                            if line1 == line2:
                                if j not in singlelayer_tempList:
                                    singlelayer_tempList.append(j)
                                    outFeat.setGeometry(tempGeom.fromPoint(j))
                                    outFeat.setAttribute(0, v1)
                                    outFeat.setAttribute(1, v2)
                                    writer.addFeature(outFeat)
                            else:
                                outFeat.setGeometry(tempGeom.fromPoint(j))
                                outFeat.setAttribute(0, v1)
                                outFeat.setAttribute(1, v2)
                                writer.addFeature(outFeat)

            start = start + add
            progressBar.setValue(start)

        del writer
