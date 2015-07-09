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

from PyQt4 import uic
from PyQt4.QtGui import QDialog, QPushButton, QAbstractItemView, QDialogButtonBox, QStandardItemModel, QStandardItem

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'DlgFixedTable.ui'))


class FixedTableDialog(BASE, WIDGET):

    def __init__(self, param, table):
        super(FixedTableDialog, self).__init__(None)
        self.setupUi(self)

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

        if not self.param.fixedNumOfRows:
            self.btnAdd.setEnabled(False)
            self.btnRemove.setEnabled(False)
            self.btnRemoveAll.setEnabled(False)

        self.populateTable(table)

    def populateTable(self, table):
        cols = len(self.param.cols)
        rows = len(table)
        model = QStandardItemModel(rows, cols)

        # Set headers
        model.setHorizontalHeaderLabels(self.param.cols)

        # Populate table
        for i in xrange(rows):
            for j in xrange(cols):
                item = QStandardItem(table[i][j])
                model.setItem(i, j, item)
        self.tblView.setModel(model)

    def accept(self):
        cols = self.tblView.model().columnCount()
        rows = self.tblView.model().rowCount()
        self.rettable = []
        for i in xrange(rows):
            self.rettable.append(list())
            for j in xrange(cols):
                self.rettable[i].append(unicode(self.tblView.model().item(i, j).text()))
        QDialog.accept(self)

    def reject(self):
        QDialog.reject(self)

    def removeRows(self, removeAll=False):
        if removeAll:
            self.tblView.model().clear()
            self.tblView.model().setHorizontalHeaderLabels(self.param.cols)
        else:
            indexes = self.tblView.selectionModel().selectedRows()
            indexes.sort()
            self.tblView.setUpdatesEnabled(False)
            for i in reversed(indexes):
                self.tblView.model().removeRows(i.row(), 1)
            self.tblView.setUpdatesEnabled(True)

    def addRow(self):
        items = [QStandardItem('0') for i in xrange(self.tblView.model().columnCount())]
        self.tblView.model().appendRow(items)
