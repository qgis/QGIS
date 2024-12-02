"""
***************************************************************************
    MultipleExternalInputDialog.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
                           (C) 2013 by CS Systemes d'information (CS SI)
    Email                : volayaf at gmail dot com
                           otb at c-s dot fr (CS SI)
    Contributors         : Victor Olaya  - basis from MultipleInputDialog
                           Alexia Mondot (CS SI) - new parameter
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Victor Olaya"
__date__ = "August 2012"
__copyright__ = "(C) 2012, Victor Olaya"

import os
import warnings

from qgis.core import QgsSettings
from qgis.PyQt import uic
from qgis.PyQt.QtCore import QByteArray
from qgis.PyQt.QtWidgets import (
    QDialog,
    QAbstractItemView,
    QPushButton,
    QDialogButtonBox,
    QFileDialog,
)
from qgis.PyQt.QtGui import QStandardItemModel, QStandardItem

pluginPath = os.path.split(os.path.dirname(__file__))[0]

with warnings.catch_warnings():
    warnings.filterwarnings("ignore", category=DeprecationWarning)
    WIDGET, BASE = uic.loadUiType(
        os.path.join(pluginPath, "ui", "DlgMultipleSelection.ui")
    )


class MultipleFileInputDialog(BASE, WIDGET):

    def __init__(self, options):
        super().__init__(None)
        self.setupUi(self)

        self.lstLayers.setSelectionMode(
            QAbstractItemView.SelectionMode.ExtendedSelection
        )

        self.selectedoptions = options

        # Additional buttons
        self.btnAdd = QPushButton(self.tr("Add file"))
        self.buttonBox.addButton(self.btnAdd, QDialogButtonBox.ButtonRole.ActionRole)
        self.btnRemove = QPushButton(self.tr("Remove file(s)"))
        self.buttonBox.addButton(self.btnRemove, QDialogButtonBox.ButtonRole.ActionRole)
        self.btnRemoveAll = QPushButton(self.tr("Remove all"))
        self.buttonBox.addButton(
            self.btnRemoveAll, QDialogButtonBox.ButtonRole.ActionRole
        )

        self.btnAdd.clicked.connect(self.addFile)
        self.btnRemove.clicked.connect(lambda: self.removeRows())
        self.btnRemoveAll.clicked.connect(lambda: self.removeRows(True))

        self.settings = QgsSettings()
        self.restoreGeometry(
            self.settings.value(
                "/Processing/multipleFileInputDialogGeometry", QByteArray()
            )
        )

        self.populateList()
        self.finished.connect(self.saveWindowGeometry)

    def saveWindowGeometry(self):
        self.settings.setValue(
            "/Processing/multipleInputDialogGeometry", self.saveGeometry()
        )

    def populateList(self):
        model = QStandardItemModel()
        for option in self.selectedoptions:
            item = QStandardItem(option)
            model.appendRow(item)

        self.lstLayers.setModel(model)

    def accept(self):
        self.selectedoptions = []
        model = self.lstLayers.model()
        for i in range(model.rowCount()):
            item = model.item(i)
            self.selectedoptions.append(item.text())
        QDialog.accept(self)

    def reject(self):
        QDialog.reject(self)

    def addFile(self):
        settings = QgsSettings()
        if settings.contains("/Processing/LastInputPath"):
            path = settings.value("/Processing/LastInputPath")
        else:
            path = ""

        files, selected_filter = QFileDialog.getOpenFileNames(
            self, self.tr("Select File(s)"), path, self.tr("All files (*.*)")
        )

        if len(files) == 0:
            return

        model = self.lstLayers.model()
        for filePath in files:
            item = QStandardItem(filePath)
            model.appendRow(item)

        settings.setValue("/Processing/LastInputPath", os.path.dirname(files[0]))

    def removeRows(self, removeAll=False):
        if removeAll:
            self.lstLayers.model().clear()
        else:
            self.lstLayers.setUpdatesEnabled(False)
            indexes = sorted(self.lstLayers.selectionModel().selectedIndexes())
            for i in reversed(indexes):
                self.lstLayers.model().removeRow(i.row())
            self.lstLayers.setUpdatesEnabled(True)
