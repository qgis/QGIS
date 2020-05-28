# -*- coding: utf-8 -*-

"""
***************************************************************************
    MultipleInputDialog.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

import os
import warnings
from pathlib import Path

from qgis.core import (QgsSettings,
                       QgsProcessing,
                       QgsVectorFileWriter,
                       QgsProviderRegistry,
                       QgsProcessingModelChildParameterSource)
from qgis.PyQt import uic
from qgis.PyQt.QtCore import Qt, QByteArray, QCoreApplication
from qgis.PyQt.QtWidgets import QDialog, QAbstractItemView, QPushButton, QDialogButtonBox, QFileDialog
from qgis.PyQt.QtGui import QStandardItemModel, QStandardItem

pluginPath = os.path.split(os.path.dirname(__file__))[0]
with warnings.catch_warnings():
    warnings.filterwarnings("ignore", category=DeprecationWarning)
    WIDGET, BASE = uic.loadUiType(
        os.path.join(pluginPath, 'ui', 'DlgMultipleSelection.ui'))


class MultipleInputDialog(BASE, WIDGET):

    def __init__(self, options, selectedoptions=None, datatype=None):
        super(MultipleInputDialog, self).__init__(None)
        self.setupUi(self)
        self.datatype = datatype
        self.model = None

        self.options = []
        for i, option in enumerate(options):
            if option is None or isinstance(option, str):
                self.options.append((i, option))
            else:
                self.options.append((option[0], option[1]))

        self.selectedoptions = selectedoptions or []

        # Additional buttons
        self.btnSelectAll = QPushButton(self.tr('Select All'))
        self.buttonBox.addButton(self.btnSelectAll,
                                 QDialogButtonBox.ActionRole)
        self.btnClearSelection = QPushButton(self.tr('Clear Selection'))
        self.buttonBox.addButton(self.btnClearSelection,
                                 QDialogButtonBox.ActionRole)
        self.btnToggleSelection = QPushButton(self.tr('Toggle Selection'))
        self.buttonBox.addButton(self.btnToggleSelection,
                                 QDialogButtonBox.ActionRole)
        if self.datatype is not None:
            btnAddFile = QPushButton(QCoreApplication.translate("MultipleInputDialog", 'Add File(s)…'))
            btnAddFile.clicked.connect(self.addFiles)
            self.buttonBox.addButton(btnAddFile,
                                     QDialogButtonBox.ActionRole)

            btnAddDir = QPushButton(QCoreApplication.translate("MultipleInputDialog", 'Add Directory…'))
            btnAddDir.clicked.connect(self.addDirectory)
            self.buttonBox.addButton(btnAddDir,
                                     QDialogButtonBox.ActionRole)

        self.btnSelectAll.clicked.connect(lambda: self.selectAll(True))
        self.btnClearSelection.clicked.connect(lambda: self.selectAll(False))
        self.btnToggleSelection.clicked.connect(self.toggleSelection)

        self.settings = QgsSettings()
        self.restoreGeometry(self.settings.value("/Processing/multipleInputDialogGeometry", QByteArray()))

        self.lstLayers.setSelectionMode(QAbstractItemView.ExtendedSelection)
        self.lstLayers.setDragDropMode(QAbstractItemView.InternalMove)

        self.populateList()
        self.finished.connect(self.saveWindowGeometry)

    def saveWindowGeometry(self):
        self.settings.setValue("/Processing/multipleInputDialogGeometry", self.saveGeometry())

    def populateList(self):
        self.model = QStandardItemModel()
        for value, text in self.options:
            item = QStandardItem(text)
            item.setData(value, Qt.UserRole)
            item.setCheckState(Qt.Checked if value in self.selectedoptions else Qt.Unchecked)
            item.setCheckable(True)
            item.setDropEnabled(False)
            self.model.appendRow(item)

        # add extra options (e.g. manually added layers)
        for t in [o for o in self.selectedoptions if not isinstance(o, int)]:
            if isinstance(t, QgsProcessingModelChildParameterSource):
                item = QStandardItem(t.staticValue())
            else:
                item = QStandardItem(t)
            item.setData(item.text(), Qt.UserRole)
            item.setCheckState(Qt.Checked)
            item.setCheckable(True)
            item.setDropEnabled(False)
            self.model.appendRow(item)

        self.lstLayers.setModel(self.model)

    def accept(self):
        self.selectedoptions = []
        model = self.lstLayers.model()
        for i in range(model.rowCount()):
            item = model.item(i)
            if item.checkState() == Qt.Checked:
                self.selectedoptions.append(item.data(Qt.UserRole))
        QDialog.accept(self)

    def reject(self):
        self.selectedoptions = None
        QDialog.reject(self)

    def getItemsToModify(self):
        items = []
        if len(self.lstLayers.selectedIndexes()) > 1:
            for i in self.lstLayers.selectedIndexes():
                items.append(self.model.itemFromIndex(i))
        else:
            for i in range(self.model.rowCount()):
                items.append(self.model.item(i))
        return items

    def selectAll(self, value):
        for item in self.getItemsToModify():
            item.setCheckState(Qt.Checked if value else Qt.Unchecked)

    def toggleSelection(self):
        for item in self.getItemsToModify():
            checked = item.checkState() == Qt.Checked
            item.setCheckState(Qt.Unchecked if checked else Qt.Checked)

    def getFileFilter(self, datatype):
        """
        Returns a suitable file filter pattern for the specified parameter definition
        :param param:
        :return:
        """
        if datatype == QgsProcessing.TypeRaster:
            return QgsProviderRegistry.instance().fileRasterFilters()
        elif datatype == QgsProcessing.TypeFile:
            return self.tr('All files (*.*)')
        else:
            exts = QgsVectorFileWriter.supportedFormatExtensions()
            for i in range(len(exts)):
                exts[i] = self.tr('{0} files (*.{1})').format(exts[i].upper(), exts[i].lower())
            return self.tr('All files (*.*)') + ';;' + ';;'.join(exts)

    def addFiles(self):
        filter = self.getFileFilter(self.datatype)

        settings = QgsSettings()
        path = str(settings.value('/Processing/LastInputPath'))

        ret, selected_filter = QFileDialog.getOpenFileNames(self, self.tr('Select File(s)'),
                                                            path, filter)
        if ret:
            files = list(ret)
            settings.setValue('/Processing/LastInputPath',
                              os.path.dirname(str(files[0])))
            for filename in files:
                item = QStandardItem(filename)
                item.setData(filename, Qt.UserRole)
                item.setCheckState(Qt.Checked)
                item.setCheckable(True)
                item.setDropEnabled(False)
                self.model.appendRow(item)

    def addDirectory(self):
        settings = QgsSettings()
        path = str(settings.value('/Processing/LastInputPath'))

        ret = QFileDialog.getExistingDirectory(self, self.tr('Select File(s)'), path)
        if ret:
            exts = []

            if self.datatype == QgsProcessing.TypeVector:
                exts = QgsVectorFileWriter.supportedFormatExtensions()
            elif self.datatype == QgsProcessing.TypeRaster:
                for t in QgsProviderRegistry.instance().fileRasterFilters().split(';;')[1:]:
                    for e in t[t.index('(') + 1:-1].split(' '):
                        if e != "*.*" and e.startswith("*."):
                            exts.append(e[2:])

            files = []
            for pp in Path(ret).rglob("*"):
                if not pp.is_file():
                    continue

                if exts and pp.suffix[1:] not in exts:
                    continue

                p = pp.as_posix()

                files.append(p)

            settings.setValue('/Processing/LastInputPath', ret)

            for filename in files:
                item = QStandardItem(filename)
                item.setData(filename, Qt.UserRole)
                item.setCheckState(Qt.Checked)
                item.setCheckable(True)
                item.setDropEnabled(False)
                self.model.appendRow(item)
