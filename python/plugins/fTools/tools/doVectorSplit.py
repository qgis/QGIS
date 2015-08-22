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

from PyQt4.QtCore import QObject, SIGNAL, QThread, QMutex, QFile
from PyQt4.QtGui import QDialog, QDialogButtonBox, QMessageBox, QErrorMessage
from qgis.core import QGis, QgsFeature, QgsVectorFileWriter

import ftools_utils
from ui_frmVectorSplit import Ui_Dialog


class Dialog(QDialog, Ui_Dialog):

    def __init__(self, iface):
        QDialog.__init__(self, iface.mainWindow())
        self.iface = iface

        self.setupUi(self)
        self.setWindowTitle(self.tr("Split vector layer"))

        QObject.connect(self.toolOut, SIGNAL("clicked()"), self.outFile)
        QObject.connect(self.inShape, SIGNAL("currentIndexChanged(QString)"), self.update)

        self.workThread = None

        self.btnOk = self.buttonBox_2.button(QDialogButtonBox.Ok)
        self.btnClose = self.buttonBox_2.button(QDialogButtonBox.Close)

        # populate layer list
        layers = ftools_utils.getLayerNames([QGis.Point, QGis.Line, QGis.Polygon])
        self.inShape.addItems(layers)

    def update(self, inputLayer):
        self.inField.clear()
        changedLayer = ftools_utils.getVectorLayerByName(inputLayer)
        changedField = ftools_utils.getFieldList(changedLayer)
        for f in changedField:
            self.inField.addItem(unicode(f.name()))

    def outFile(self):
        self.outShape.clear()
        (self.folderName, self.encoding) = ftools_utils.dirDialog(self)
        if self.folderName is None or self.encoding is None:
            return
        self.outShape.setText(self.folderName)

    def accept(self):
        inShape = self.inShape.currentText()
        outDir = self.outShape.text()
        if inShape == "":
            QMessageBox.information(self, self.tr("Vector Split"),
                                    self.tr("No input shapefile specified"))
            return
        elif outDir == "":
            QMessageBox.information(self, self.tr("Vector Split"),
                                    self.tr("Please specify output shapefile"))
            return

        self.btnOk.setEnabled(False)

        vLayer = ftools_utils.getVectorLayerByName(unicode(self.inShape.currentText()))
        self.workThread = SplitThread(vLayer, self.inField.currentText(), self.encoding, outDir)

        QObject.connect(self.workThread, SIGNAL("rangeCalculated(PyQt_PyObject)"), self.setProgressRange)
        QObject.connect(self.workThread, SIGNAL("valueProcessed()"), self.valueProcessed)
        QObject.connect(self.workThread, SIGNAL("processFinished(PyQt_PyObject)"), self.processFinished)
        QObject.connect(self.workThread, SIGNAL("processInterrupted()"), self.processInterrupted)

        self.btnClose.setText(self.tr("Cancel"))
        QObject.disconnect(self.buttonBox_2, SIGNAL("rejected()"), self.reject)
        QObject.connect(self.btnClose, SIGNAL("clicked()"), self.stopProcessing)

        self.workThread.start()

    def setProgressRange(self, maximum):
        self.progressBar.setRange(0, maximum)

    def valueProcessed(self):
        self.progressBar.setValue(self.progressBar.value() + 1)

    def restoreGui(self):
        self.progressBar.setValue(0)
        self.outShape.clear()
        QObject.connect(self.buttonBox_2, SIGNAL("rejected()"), self.reject)
        self.btnClose.setText(self.tr("Close"))
        self.btnOk.setEnabled(True)

    def stopProcessing(self):
        if self.workThread is not None:
            self.workThread.stop()
            self.workThread = None

    def processInterrupted(self):
        self.restoreGui()

    def processFinished(self, errors):
        self.stopProcessing()
        outPath = self.outShape.text()
        self.restoreGui()

        if errors:
            msg = self.tr("Processing of the following layers/files ended with error:<br><br>") + "<br>".join(errors)
            QErrorMessage(self).showMessage(msg)

        QMessageBox.information(self, self.tr("Vector Split"),
                                self.tr("Created output shapefiles in folder:\n%s") % (outPath))


class SplitThread(QThread):

    def __init__(self, layer, splitField, encoding, outDir):
        QThread.__init__(self, QThread.currentThread())
        self.layer = layer
        self.field = splitField
        self.encoding = encoding
        self.outDir = outDir

        self.mutex = QMutex()
        self.stopMe = 0

        self.errors = []

    def run(self):
        self.mutex.lock()
        self.stopMe = 0
        self.mutex.unlock()

        interrupted = False

        outPath = self.outDir

        if outPath.find("\\") != -1:
            outPath.replace("\\", "/")

        if not outPath.endswith("/"):
            outPath = outPath + "/"

        provider = self.layer.dataProvider()
        index = provider.fieldNameIndex(self.field)
        unique = ftools_utils.getUniqueValues(provider, int(index))
        baseName = unicode(outPath + self.layer.name() + "_" + self.field + "_")

        fieldList = ftools_utils.getFieldList(self.layer)
        sRs = provider.crs()
        geom = self.layer.wkbType()
        inFeat = QgsFeature()

        self.emit(SIGNAL("rangeCalculated(PyQt_PyObject)"), len(unique))

        for i in unique:
            check = QFile(baseName + "_" + unicode(i).strip() + ".shp")
            fName = check.fileName()
            if check.exists():
                if not QgsVectorFileWriter.deleteShapeFile(fName):
                    self.errors.append(fName)
                    continue

            writer = QgsVectorFileWriter(fName, self.encoding, fieldList, geom, sRs)

            fit = provider.getFeatures()
            while fit.nextFeature(inFeat):
                atMap = inFeat.attributes()
                if atMap[index] == i:
                    writer.addFeature(inFeat)
            del writer

            self.emit(SIGNAL("valueProcessed()"))

            self.mutex.lock()
            s = self.stopMe
            self.mutex.unlock()
            if s == 1:
                interrupted = True
                break

        if not interrupted:
            self.emit(SIGNAL("processFinished( PyQt_PyObject )"), self.errors)
        else:
            self.emit(SIGNAL("processInterrupted()"))

    def stop(self):
        self.mutex.lock()
        self.stopMe = 1
        self.mutex.unlock()

        QThread.wait(self)
