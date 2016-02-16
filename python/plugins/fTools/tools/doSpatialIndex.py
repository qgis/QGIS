# -*- coding: utf-8 -*-

"""
***************************************************************************
    doSpatialIndex.py - build spatial index for vector layers or files
     --------------------------------------
    Date                 : 11-Nov-2011
    Copyright            : (C) 2011 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

from PyQt4.QtCore import Qt, QObject, SIGNAL, QThread, QMutex
from PyQt4.QtGui import QDialog, QDialogButtonBox, QListWidgetItem, QAbstractItemView, QErrorMessage, QMessageBox
from qgis.core import QGis, QgsProviderRegistry, QgsVectorLayer, QgsVectorDataProvider

import ftools_utils

from ui_frmSpatialIndex import Ui_Dialog


class Dialog(QDialog, Ui_Dialog):

    def __init__(self, iface):
        QDialog.__init__(self, iface.mainWindow())
        self.setupUi(self)
        self.iface = iface

        self.workThread = None

        self.btnOk = self.buttonBox.button(QDialogButtonBox.Ok)
        self.btnClose = self.buttonBox.button(QDialogButtonBox.Close)

        QObject.connect(self.chkExternalFiles, SIGNAL("stateChanged( int )"), self.toggleExternalFiles)
        QObject.connect(self.btnSelectFiles, SIGNAL("clicked()"), self.selectFiles)
        QObject.connect(self.lstLayers, SIGNAL("itemSelectionChanged()"), self.updateLayerList)
        QObject.connect(self.btnSelectAll, SIGNAL("clicked()"), self.selectAll)
        QObject.connect(self.btnSelectNone, SIGNAL("clicked()"), self.selectNone)
        QObject.connect(self.btnClearList, SIGNAL("clicked()"), self.clearList)

        self.manageGui()

    def manageGui(self):
        self.btnSelectFiles.setEnabled(False)
        self.btnClearList.setEnabled(False)

        self.fillLayersList()

    def fillLayersList(self):
        self.lstLayers.clear()
        layers = ftools_utils.getLayerNames([QGis.Line, QGis.Point, QGis.Polygon])
        for lay in layers:
            source = ftools_utils.getVectorLayerByName(lay).source()
            item = QListWidgetItem(lay, self.lstLayers)
            item.setData(Qt.UserRole, source)
            item.setData(Qt.ToolTipRole, source)

    def toggleExternalFiles(self):
        if self.chkExternalFiles.isChecked():
            self.btnSelectFiles.setEnabled(True)
            self.btnClearList.setEnabled(True)
            self.btnSelectAll.setEnabled(False)
            self.btnSelectNone.setEnabled(False)

            self.lstLayers.clear()
            self.lstLayers.setSelectionMode(QAbstractItemView.NoSelection)
            self.layers = []
        else:
            self.btnSelectFiles.setEnabled(False)
            self.btnClearList.setEnabled(False)
            self.btnSelectAll.setEnabled(True)
            self.btnSelectNone.setEnabled(True)

            self.fillLayersList()
            self.lstLayers.setSelectionMode(QAbstractItemView.ExtendedSelection)
            self.updateLayerList()

    def updateLayerList(self):
        self.layers = []
        selection = self.lstLayers.selectedItems()
        for item in selection:
            self.layers.append(item.text())

    def selectFiles(self):
        filters = QgsProviderRegistry.instance().fileVectorFilters()
        files, self.encoding = ftools_utils.openDialog(self, filtering=filters, dialogMode="MultipleFiles")
        if files is None:
            return

        self.layers.extend([unicode(f) for f in files])
        self.lstLayers.addItems(files)

    def selectAll(self):
        self.lstLayers.selectAll()

    def selectNone(self):
        self.lstLayers.clearSelection()

    def clearList(self):
        self.layers = []
        self.lstLayers.clear()

    def accept(self):
        self.btnOk.setEnabled(False)

        self.workThread = SpatialIdxThread(self.layers, self.chkExternalFiles.isChecked())
        self.progressBar.setRange(0, len(self.layers))

        QObject.connect(self.workThread, SIGNAL("layerProcessed()"), self.layerProcessed)
        QObject.connect(self.workThread, SIGNAL("processFinished( PyQt_PyObject )"), self.processFinished)
        QObject.connect(self.workThread, SIGNAL("processInterrupted()"), self.processInterrupted)

        self.btnClose.setText(self.tr("Cancel"))
        QObject.disconnect(self.buttonBox, SIGNAL("rejected()"), self.reject)
        QObject.connect(self.btnClose, SIGNAL("clicked()"), self.stopProcessing)

        self.workThread.start()

    def layerProcessed(self):
        self.progressBar.setValue(self.progressBar.value() + 1)

    def processInterrupted(self):
        self.restoreGui()

    def processFinished(self, errors):
        self.stopProcessing()
        self.restoreGui()

        if errors:
            msg = self.tr("Processing of the following layers/files ended with error:<br><br>") + "<br>".join(errors)
            QErrorMessage(self).showMessage(msg)

        QMessageBox.information(self, self.tr("Finished"), self.tr("Processing completed."))

    def stopProcessing(self):
        if self.workThread is not None:
            self.workThread.stop()
            self.workThread = None

    def restoreGui(self):
        self.progressBar.setValue(0)
        QObject.connect(self.buttonBox, SIGNAL("rejected()"), self.reject)
        self.btnClose.setText(self.tr("Close"))
        self.btnOk.setEnabled(True)

        if self.chkExternalFiles.isChecked():
            self.clearList()


class SpatialIdxThread(QThread):

    def __init__(self, layers, isFiles):
        QThread.__init__(self, QThread.currentThread())
        self.layers = layers
        self.isFiles = isFiles

        self.mutex = QMutex()
        self.stopMe = 0

        self.errors = []

    def run(self):
        self.mutex.lock()
        self.stopMe = 0
        self.mutex.unlock()

        interrupted = False

        if self.isFiles:
            for layer in self.layers:
                vl = QgsVectorLayer(layer, "tmp", "ogr")
                provider = vl.dataProvider()
                if provider.capabilities() & QgsVectorDataProvider.CreateSpatialIndex:
                    if not provider.createSpatialIndex():
                        self.errors.append(layer)
                else:
                    self.errors.append(layer)

                self.emit(SIGNAL("layerProcessed()"))

                self.mutex.lock()
                s = self.stopMe
                self.mutex.unlock()
                if s == 1:
                    interrupted = True
                    break
        else:
            for layer in self.layers:
                vl = ftools_utils.getVectorLayerByName(layer)
                provider = vl.dataProvider()
                if provider.capabilities() & QgsVectorDataProvider.CreateSpatialIndex:
                    if not provider.createSpatialIndex():
                        self.errors.append(layer)
                else:
                    self.errors.append(layer)

                self.emit(SIGNAL("layerProcessed()"))

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
