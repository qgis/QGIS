# -*- coding: utf-8 -*-

"""
***************************************************************************
    FixedTableDialog.py
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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import warnings

from qgis.gui import QgsGui

from qgis.PyQt import uic
from qgis.PyQt.QtWidgets import QDialog, QPushButton, QAbstractItemView, QDialogButtonBox
from qgis.PyQt.QtGui import QStandardItemModel, QStandardItem

pluginPath = os.path.split(os.path.dirname(__file__))[0]

with warnings.catch_warnings():
    warnings.filterwarnings("ignore", category=DeprecationWarning)
    WIDGET, BASE = uic.loadUiType(
        os.path.join(pluginPath, 'ui', 'DlgFixedTable.ui'))


class FixedTableDialog(BASE, WIDGET):

    def __init__(self, param, table):
        """
        Constructor for FixedTableDialog
        :param param: linked processing parameter
        :param table: initial table contents - squashed to 1-dimensional!
        """
        super().__init__(None)

        self.setupUi(self)

        QgsGui.instance().enableAutoGeometryRestore(self)

        self.tblView.setSelectionBehavior(QAbstractItemView.SelectRows)
        self.tblView.setSelectionMode(QAbstractItemView.ExtendedSelection)

        self.param = param
        self.rettable = None

        # Additional buttons
        self.btnAdd = QPushButton(self.tr('Add row'))
        self.buttonBox.addButton(self.btnAdd,
                                 QDialogButtonBox.ActionRole)
        self.btnRemove = QPushButton(self.tr('Remove row(s)'))
        self.buttonBox.addButton(self.btnRemove,
                                 QDialogButtonBox.ActionRole)
        self.btnRemoveAll = QPushButton(self.tr('Remove all'))
        self.buttonBox.addButton(self.btnRemoveAll,
                                 QDialogButtonBox.ActionRole)

        self.btnAdd.clicked.connect(self.addRow)
        self.btnRemove.clicked.connect(lambda: self.removeRows())
        self.btnRemoveAll.clicked.connect(lambda: self.removeRows(True))

        if self.param.hasFixedNumberRows():
            self.btnAdd.setEnabled(False)
            self.btnRemove.setEnabled(False)
            self.btnRemoveAll.setEnabled(False)

        self.populateTable(table)

    def populateTable(self, table):
        cols = len(self.param.headers())
        rows = len(table) // cols
        model = QStandardItemModel(rows, cols)

        # Set headers
        model.setHorizontalHeaderLabels(self.param.headers())

        # Populate table
        for row in range(rows):
            for col in range(cols):
                item = QStandardItem(str(table[row * cols + col]))
                model.setItem(row, col, item)
        self.tblView.setModel(model)

    def accept(self):
        cols = self.tblView.model().columnCount()
        rows = self.tblView.model().rowCount()
        # Table MUST BE 1-dimensional to match core QgsProcessingParameterMatrix expectations
        self.rettable = []
        for row in range(rows):
            for col in range(cols):
                self.rettable.append(str(self.tblView.model().item(row, col).text()))
        QDialog.accept(self)

    def reject(self):
        QDialog.reject(self)

    def removeRows(self, removeAll=False):
        if removeAll:
            self.tblView.model().clear()
            self.tblView.model().setHorizontalHeaderLabels(self.param.headers())
        else:
            indexes = sorted(self.tblView.selectionModel().selectedRows())
            self.tblView.setUpdatesEnabled(False)
            for i in reversed(indexes):
                self.tblView.model().removeRows(i.row(), 1)
            self.tblView.setUpdatesEnabled(True)

    def addRow(self):
        items = [QStandardItem('0') for i in range(self.tblView.model().columnCount())]
        self.tblView.model().appendRow(items)
