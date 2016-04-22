# -*- coding: utf-8 -*-

"""
***************************************************************************
    inOutSelector.py
    ---------------------
    Date                 : April 2011
    Copyright            : (C) 2011 by Giuseppe Sucameli
    Email                : brush dot tyler at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Giuseppe Sucameli'
__date__ = 'April 2011'
__copyright__ = '(C) 2011, Giuseppe Sucameli'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.PyQt.QtCore import Qt, pyqtSignal, pyqtProperty
from qgis.PyQt.QtWidgets import QWidget, QComboBox

from qgis.core import QgsMapLayerRegistry, QgsMapLayer

from .ui_inOutSelector import Ui_GdalToolsInOutSelector


class GdalToolsInOutSelector(QWidget, Ui_GdalToolsInOutSelector):
    FILE = 0x1
    LAYER = 0x2
    MULTIFILE = 0x4    # NOT IMPLEMENTED YET

    FILE_LAYER = 0x1 | 0x2
    FILES = 0x1 | 0x4    # NOT IMPLEMENTED YET
    FILES_LAYER = 0x3 | 0x4    # NOT IMPLEMENTED YET

    selectClicked = pyqtSignal()
    filenameChanged = pyqtSignal()
    layerChanged = pyqtSignal()

    def __init__(self, parent=None, type=None):
        QWidget.__init__(self, parent)

        self.setupUi(self)
        self.setFocusPolicy(Qt.StrongFocus)
        self.combo.setInsertPolicy(QComboBox.NoInsert)

        self.clear()

        self.typ = None
        if type is None:
            self.resetType()
        else:
            self.setType(type)

        self.selectBtn.clicked.connect(self.selectClicked)
        self.fileEdit.textChanged.connect(self.textChanged)
        self.combo.editTextChanged.connect(self.textChanged)
        self.combo.currentIndexChanged.connect(self.indexChanged)

    def clear(self):
        self.filenames = []
        self.fileEdit.clear()
        self.clearComboState()
        self.combo.clear()

    def textChanged(self):
        if self.getType() & self.MULTIFILE:
            self.filenames = self.fileEdit.text().split(",")
        if self.getType() & self.LAYER:
            index = self.combo.currentIndex()
            if index >= 0:
                text = self.combo.currentText()
                if text != self.combo.itemText(index):
                    return self.setFilename(text)
        self.filenameChanged.emit()

    def indexChanged(self):
        self.layerChanged.emit()
        self.filenameChanged.emit()

    def setType(self, type):
        if type == self.typ:
            return

        if type & self.MULTIFILE:    # MULTITYPE IS NOT IMPLEMENTED YET
            type = type & ~self.MULTIFILE

        self.typ = type

        self.selectBtn.setVisible(self.getType() & self.FILE)
        self.combo.setVisible(self.getType() & self.LAYER)
        self.fileEdit.setVisible(not (self.getType() & self.LAYER))
        self.combo.setEditable(self.getType() & self.FILE)

        if self.getType() & self.FILE:
            self.setFocusProxy(self.selectBtn)
        else:
            self.setFocusProxy(self.combo)

        # send signals to refresh connected widgets
        self.filenameChanged.emit()
        self.layerChanged.emit()

    def getType(self):
        return self.typ

    def resetType(self):
        self.setType(self.FILE_LAYER)

    selectorType = pyqtProperty("int", getType, setType, resetType)

    def setFilename(self, fn=None):
        self.blockSignals(True)
        prevFn, prevLayer = self.filename(), self.layer()

        if isinstance(fn, QgsMapLayer):
            fn = fn.source()

        elif isinstance(fn, str) or isinstance(fn, unicode):
            fn = unicode(fn)

        # TODO test
        elif isinstance(fn, list):
            if len(fn) > 0:
                if self.getType() & self.MULTIFILE:
                    self.filenames = fn
                #fn = "".join( fn, "," )
                fn = ",".join(fn)
            else:
                fn = ''

        else:
            fn = ''

        if not (self.getType() & self.LAYER):
            self.fileEdit.setText(fn)
        else:
            self.combo.setCurrentIndex(-1)
            self.combo.setEditText(fn)

        self.blockSignals(False)
        if self.filename() != prevFn:
            self.filenameChanged.emit()
        if self.layer() != prevLayer:
            self.layerChanged.emit()

    def setLayer(self, layer=None):
        if not (self.getType() & self.LAYER):
            return self.setFilename(layer)

        self.blockSignals(True)
        prevFn, prevLayer = self.filename(), self.layer()

        if isinstance(layer, QgsMapLayer):
            if self.combo.findData(layer.id()) >= 0:
                index = self.combo.findData(layer.id())
                self.combo.setCurrentIndex(index)
            else:
                self.combo.setCurrentIndex(-1)
                self.combo.setEditText(layer.source())

        elif isinstance(layer, int) and layer >= 0 and layer < self.combo.count():
            self.combo.setCurrentIndex(layer)

        else:
            self.combo.clearEditText()
            self.combo.setCurrentIndex(-1)

        self.blockSignals(False)
        if self.filename() != prevFn:
            self.filenameChanged.emit()
        if self.layer() != prevLayer:
            self.layerChanged.emit()

    def setLayers(self, layers=None):
        if layers is None or not hasattr(layers, '__iter__') or len(layers) <= 0:
            self.combo.clear()
            return

        self.blockSignals(True)
        prevFn, prevLayer = self.filename(), self.layer()
        self.saveComboState()

        self.combo.clear()
        for l in layers:
            self.combo.addItem(l.name(), l.id())

        self.restoreComboState()
        self.blockSignals(False)
        if self.filename() != prevFn:
            self.filenameChanged.emit()
        if self.layer() != prevLayer:
            self.layerChanged.emit()

    def clearComboState(self):
        self.prevState = None

    def saveComboState(self):
        index = self.combo.currentIndex()
        text = self.combo.currentText()
        layerID = self.combo.itemData(index) if index >= 0 else ""
        self.prevState = (index, text, layerID)

    def restoreComboState(self):
        if self.prevState is None:
            return
        index, text, layerID = self.prevState

        if index < 0:
            if text == '' and self.combo.count() > 0:
                index = 0

        elif self.combo.findData(layerID) < 0:
            index = -1
            text = ""

        else:
            index = self.combo.findData(layerID)

        self.combo.setCurrentIndex(index)
        if index >= 0:
            text = self.combo.itemText(index)
        self.combo.setEditText(text)

    def layer(self):
        if self.getType() != self.FILE and self.combo.currentIndex() >= 0:
            layerID = self.combo.itemData(self.combo.currentIndex())
            return QgsMapLayerRegistry.instance().mapLayer(layerID)
        return None

    def filename(self):
        if not (self.getType() & self.LAYER):
            if self.getType() & self.MULTIFILE:
                return self.filenames
            return self.fileEdit.text()

        if self.combo.currentIndex() < 0:
            if self.getType() & self.MULTIFILE:
                return self.filenames
            return self.combo.currentText()
        layer = self.layer()
        if layer is not None:
            return layer.source()

        return ''
