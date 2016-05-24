# -*- coding: utf-8 -*-

"""
***************************************************************************
    DirectorySelectorDialog.py
    ---------------------
    Date                 : May 2016
    Copyright            : (C) 2016 by Alexander Bruy
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

__author__ = 'Alexander Bruy'
__date__ = 'May 2016'
__copyright__ = '(C) 2016, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt import uic
from qgis.PyQt.QtCore import QSettings
from qgis.PyQt.QtWidgets import QDialog, QAbstractItemView, QPushButton, QDialogButtonBox, QFileDialog
from qgis.PyQt.QtGui import QStandardItemModel, QStandardItem

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'DlgMultipleSelection.ui'))


class DirectorySelectorDialog(BASE, WIDGET):

    def __init__(self, parent, options):
        super(DirectorySelectorDialog, self).__init__(None)
        self.setupUi(self)

        self.lstLayers.setSelectionMode(QAbstractItemView.ExtendedSelection)

        self.options = options

        # Additional buttons
        self.btnAdd = QPushButton(self.tr('Add'))
        self.buttonBox.addButton(self.btnAdd,
                                 QDialogButtonBox.ActionRole)
        self.btnRemove = QPushButton(self.tr('Remove'))
        self.buttonBox.addButton(self.btnRemove,
                                 QDialogButtonBox.ActionRole)
        self.btnRemoveAll = QPushButton(self.tr('Remove all'))
        self.buttonBox.addButton(self.btnRemoveAll,
                                 QDialogButtonBox.ActionRole)

        self.btnAdd.clicked.connect(self.addDirectory)
        self.btnRemove.clicked.connect(lambda: self.removeRows())
        self.btnRemoveAll.clicked.connect(lambda: self.removeRows(True))

        self.populateList()

    def populateList(self):
        model = QStandardItemModel()
        for option in self.options:
            item = QStandardItem(option)
            model.appendRow(item)

        self.lstLayers.setModel(model)

    def accept(self):
        self.selectedoptions = []
        model = self.lstLayers.model()
        for i in xrange(model.rowCount()):
            item = model.item(i)
            self.selectedoptions.append(item.text())
        QDialog.accept(self)

    def reject(self):
        QDialog.reject(self)

    def addDirectory(self):
        settings = QSettings()
        if settings.contains('/Processing/lastDirectory'):
            path = settings.value('/Processing/lastDirectory')
        else:
            path = ''

        folder = QFileDialog.getExistingDirectory(self,
                                                  self.tr('Select directory'),
                                                  path,
                                                  QFileDialog.ShowDirsOnly)

        if folder == '':
            return

        model = self.lstLayers.model()
        item = QStandardItem(folder)
        model.appendRow(item)

        settings.setValue('/Processing/lastDirectory',
                          os.path.dirname(folder))

    def removeRows(self, removeAll=False):
        if removeAll:
            self.lstLayers.model().clear()
        else:
            self.lstLayers.setUpdatesEnabled(False)
            indexes = sorted(self.lstLayers.selectionModel().selectedIndexes())
            for i in reversed(indexes):
                self.lstLayers.model().removeRow(i.row())
            self.lstLayers.setUpdatesEnabled(True)

    def value(self):
        folders = []
        model = self.lstLayers.model()
        for i in xrange(model.rowCount()):
            folders.append(model.item(i).text())

        return ';'.join(folders)
