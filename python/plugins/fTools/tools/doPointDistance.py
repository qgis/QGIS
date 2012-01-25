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
from ui_frmPointDistance import Ui_Dialog
import csv, codecs, cStringIO
import ftools_utils
from math import *

class UnicodeWriter:
    """
    A CSV writer which will write rows to CSV file "f",
    which is encoded in the given encoding.
    Taken from http://docs.python.org/library/csv.html
    to allow handling of non-ascii output
    """

    def __init__(self, f, dialect=csv.excel, encoding="utf-8", **kwds):
        # Redirect output to a queue
        self.queue = cStringIO.StringIO()
        self.writer = csv.writer(self.queue, dialect=dialect, **kwds)
        self.stream = f
        self.encoder = codecs.getincrementalencoder(encoding)()

    def writerow(self, row):
        try:
            self.writer.writerow([s.encode("utf-8") for s in row])
        except:
            self.writer.writerow(row)
        # Fetch UTF-8 output from the queue ...
        data = self.queue.getvalue()
        data = data.decode("utf-8")
        # ... and re-encode it into the target encoding
        data = self.encoder.encode(data)
        # write to the target stream
        self.stream.write(data)
        # empty queue
        self.queue.truncate(0)

    def writerows(self, rows):
        for row in rows:
            self.writerow(row)

class Dialog(QDialog, Ui_Dialog):
    def __init__(self, iface):
        QDialog.__init__(self, iface.mainWindow())
        self.iface = iface
        # Set up the user interface from Designer.
        self.setupUi(self)
        QObject.connect(self.btnFile, SIGNAL("clicked()"), self.saveFile)
        QObject.connect(self.inPoint1, SIGNAL("currentIndexChanged(QString)"), self.update1)
        QObject.connect(self.inPoint2, SIGNAL("currentIndexChanged(QString)"), self.update2)
        self.buttonOk = self.buttonBox_2.button( QDialogButtonBox.Ok )
        # populate layer list
        self.setWindowTitle(self.tr("Distance matrix"))
        self.progressBar.setValue(0)
        mapCanvas = self.iface.mapCanvas()
        layers = ftools_utils.getLayerNames( [ QGis.Point ] )
        self.inPoint1.addItems(layers)
        self.inPoint2.addItems(layers)

    def update1(self, inputLayer):
        self.inField1.clear()
        changedLayer = ftools_utils.getVectorLayerByName(unicode(inputLayer))
        changedField = ftools_utils.getFieldList(changedLayer)
        for i in changedField:
            if changedField[i].type() == QVariant.Int or \
            changedField[i].type() == QVariant.String:
                self.inField1.addItem(unicode(changedField[i].name()))

    def update2(self, inputLayer):
        self.inField2.clear()
        changedLayer = ftools_utils.getVectorLayerByName(unicode(inputLayer))
        changedField = ftools_utils.getFieldList(changedLayer)
        for i in changedField:
            if changedField[i].type() == QVariant.Int or \
            changedField[i].type() == QVariant.String:
                self.inField2.addItem(unicode(changedField[i].name()))

    def accept(self):
        self.buttonOk.setEnabled( False )
        if self.inPoint1.currentText() == "":
            QMessageBox.information(self, self.tr("Create Point Distance Matrix"), self.tr("Please specify input point layer"))
        elif self.outFile.text() == "":
            QMessageBox.information(self, self.tr("Create Point Distance Matrix"), self.tr("Please specify output file"))
        elif self.inPoint2.currentText() == "":
            QMessageBox.information(self, self.tr("Create Point Distance Matrix"), self.tr("Please specify target point layer"))
        elif self.inField1.currentText() == "":
            QMessageBox.information(self, self.tr("Create Point Distance Matrix"), self.tr("Please specify input unique ID field"))
        elif self.inField2.currentText() == "":
            QMessageBox.information(self, self.tr("Create Point Distance Matrix"), self.tr("Please specify target unique ID field"))
        else:
            point1 = self.inPoint1.currentText()
            point2 = self.inPoint2.currentText()
            field1 = self.inField1.currentText()
            field2 = self.inField2.currentText()
            outPath = self.outFile.text()
            if self.rdoLinear.isChecked(): matType = "Linear"
            elif self.rdoStandard.isChecked(): matType = "Standard"
            else: matType = "Summary"
            if self.chkNearest.isChecked(): nearest = self.spnNearest.value()
            else: nearest = 0
            if outPath.contains("\\"):
                outName = outPath.right((outPath.length() - outPath.lastIndexOf("\\")) - 1)
            else:
                outName = outPath.right((outPath.length() - outPath.lastIndexOf("/")) - 1)
            if outName.endsWith(".csv"):
                outName = outName.left(outName.length() - 4)
            self.outFile.clear()
            self.compute(point1, point2, field1, field2, outPath, matType, nearest, self.progressBar)
            self.progressBar.setValue(100)
            addToTOC = QMessageBox.information(self, "Create Point Distance Matrix", self.tr("Created output matrix:\n") + outPath)
        self.progressBar.setValue(0)
        self.buttonOk.setEnabled( True )

    def saveFile(self):
        self.outFile.clear()
        fileDialog = QFileDialog()
        outName = fileDialog.getSaveFileName(self, "Output Distance Matrix",".", "Delimited txt file (*.csv)")
        fileCheck = QFile(outName)
        filePath = QFileInfo(outName).absoluteFilePath()
        if filePath.right(4) != ".csv": filePath = filePath + ".csv"
        if not outName.isEmpty():
            self.outFile.insert(filePath)

    def compute(self, line1, line2, field1, field2, outPath, matType, nearest, progressBar):
        layer1 = ftools_utils.getVectorLayerByName(line1)
        layer2 = ftools_utils.getVectorLayerByName(line2)
        provider1 = layer1.dataProvider()
        provider2 = layer2.dataProvider()
        allAttrs = provider1.attributeIndexes()
        provider1.select(allAttrs)
        allAttrs = provider2.attributeIndexes()
        provider2.select(allAttrs)
        sindex = QgsSpatialIndex()
        inFeat = QgsFeature()
        while provider2.nextFeature(inFeat):
            sindex.insertFeature(inFeat)
        provider2.rewind()
        if nearest < 1: nearest = layer2.featureCount()
        else: nearest = nearest
        index1 = provider1.fieldNameIndex(field1)
        index2 = provider2.fieldNameIndex(field2)
        sRs = provider1.crs()
        distArea = QgsDistanceArea()
        #use srs of the first layer (users should ensure that they are both in the same projection)
        #distArea.setSourceSRS(sRs)

        f = open(unicode(outPath), "wb")
        writer = UnicodeWriter(f)
        if matType <> "Standard":
            if matType == "Linear":
                writer.writerow(["InputID", "TargetID", "Distance"])
            else:
                writer.writerow(["InputID", "MEAN", "STDDEV", "MIN", "MAX"])
            self.linearMatrix(writer, provider1, provider2, index1, index2, nearest, distArea, matType, sindex, progressBar)
        else:
            self.regularMatrix(writer, provider1, provider2, index1, index2, nearest, distArea, sindex, progressBar)
        f.close()

    def regularMatrix(self, writer, provider1, provider2, index1, index2, nearest, distArea, sindex, progressBar):
        inFeat = QgsFeature()
        outFeat = QgsFeature()
        inGeom = QgsGeometry()
        outGeom = QgsGeometry()
        first = True
        start = 15.00
        add = 85.00 / provider1.featureCount()
        while provider1.nextFeature(inFeat):
            inGeom = inFeat.geometry()
            inID = inFeat.attributeMap()[index1].toString()
            if first:
                featList = sindex.nearestNeighbor(inGeom.asPoint(), nearest)
                first = False
                data = ["ID"]
                for i in featList:
                    provider2.featureAtId(int(i), outFeat, True, [index2])
                    data.append(unicode(outFeat.attributeMap()[index2].toString()))
                writer.writerow(data)
            data = [unicode(inID)]
            for j in featList:
                provider2.featureAtId(int(j), outFeat, True)
                outGeom = outFeat.geometry()
                dist = distArea.measureLine(inGeom.asPoint(), outGeom.asPoint())
                data.append(str(float(dist)))
            writer.writerow(data)
            start = start + add
            progressBar.setValue(start)
        del writer

    def linearMatrix(self, writer, provider1, provider2, index1, index2, nearest, distArea, matType, sindex, progressBar):
        inFeat = QgsFeature()
        outFeat = QgsFeature()
        inGeom = QgsGeometry()
        outGeom = QgsGeometry()
        start = 15.00
        add = 85.00 / provider1.featureCount()
        while provider1.nextFeature(inFeat):
            inGeom = inFeat.geometry()
            inID = inFeat.attributeMap()[index1].toString()
            featList = sindex.nearestNeighbor(inGeom.asPoint(), nearest)
            distList = []
            vari = 0.00
            for i in featList:
                provider2.featureAtId(int(i), outFeat, True, [index2])
                outID = outFeat.attributeMap()[index2].toString()
                outGeom = outFeat.geometry()
                dist = distArea.measureLine(inGeom.asPoint(), outGeom.asPoint())
                if dist > 0:
                    if matType == "Linear": writer.writerow([unicode(inID), unicode(outID), str(dist)])
                    else: distList.append(float(dist))
            if matType == "Summary":
                mean = sum(distList) / len(distList)
                for i in distList:
                    vari = vari + ((i - mean)*(i - mean))
                vari = sqrt(vari / len(distList))
                writer.writerow([unicode(inID), str(mean), str(vari), str(min(distList)), str(max(distList))])
            start = start + add
            progressBar.setValue(start)
        del writer
