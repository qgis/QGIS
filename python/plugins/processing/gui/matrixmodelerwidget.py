# -*- coding: utf-8 -*-

"""
***************************************************************************
    MatrixModelerWidget.py
    ---------------------
    Date                 : May 2018
    Copyright            : (C) 2018 by Alexander Bruy
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
__date__ = 'May 2018'
__copyright__ = '(C) 2018, Alexander Bruy'

import os
import warnings

from qgis.PyQt import uic
from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtGui import QStandardItemModel, QStandardItem
from qgis.PyQt.QtWidgets import QInputDialog, QMessageBox

from qgis.core import QgsApplication

pluginPath = os.path.split(os.path.dirname(__file__))[0]

with warnings.catch_warnings():
    warnings.filterwarnings("ignore", category=DeprecationWarning)
    WIDGET, BASE = uic.loadUiType(
        os.path.join(pluginPath, 'ui', 'matrixmodelerwidgetbase.ui'))


class MatrixModelerWidget(BASE, WIDGET):

    def __init__(self, parent=None):
        super(MatrixModelerWidget, self).__init__(parent)
        self.setupUi(self)

        self.btnAddColumn.setIcon(QgsApplication.getThemeIcon('/mActionNewAttribute.svg'))
        self.btnRemoveColumn.setIcon(QgsApplication.getThemeIcon('/mActionDeleteAttribute.svg'))
        self.btnAddRow.setIcon(QgsApplication.getThemeIcon('/symbologyAdd.svg'))
        self.btnRemoveRow.setIcon(QgsApplication.getThemeIcon('/symbologyRemove.svg'))
        self.btnClear.setIcon(QgsApplication.getThemeIcon('console/iconClearConsole.svg'))

        self.btnAddColumn.clicked.connect(self.addColumn)
        self.btnRemoveColumn.clicked.connect(self.removeColumns)
        self.btnAddRow.clicked.connect(self.addRow)
        self.btnRemoveRow.clicked.connect(self.removeRows)
        self.btnClear.clicked.connect(self.clearTable)

        items = [QStandardItem('0')]
        model = QStandardItemModel()
        model.appendColumn(items)
        self.tblView.setModel(model)

        self.tblView.horizontalHeader().sectionDoubleClicked.connect(self.changeHeader)

    def addColumn(self):
        model = self.tblView.model()
        items = [QStandardItem('0') for i in range(model.rowCount())]
        model.appendColumn(items)

    def removeColumns(self):
        indexes = sorted(self.tblView.selectionModel().selectedColumns())
        self.tblView.setUpdatesEnabled(False)
        for i in reversed(indexes):
            self.tblView.model().removeColumns(i.column(), 1)
        self.tblView.setUpdatesEnabled(True)

    def addRow(self):
        model = self.tblView.model()
        items = [QStandardItem('0') for i in range(model.columnCount())]
        model.appendRow(items)

    def removeRows(self):
        indexes = sorted(self.tblView.selectionModel().selectedRows())
        self.tblView.setUpdatesEnabled(False)
        for i in reversed(indexes):
            self.tblView.model().removeRows(i.row(), 1)
        self.tblView.setUpdatesEnabled(True)

    def clearTable(self, removeAll=False):
        res = QMessageBox.question(self, self.tr('Clear?'), self.tr('Are you sure you want to clear table?'))
        if res == QMessageBox.Yes:
            self.tblView.model().clear()

    def changeHeader(self, index):
        txt, ok = QInputDialog.getText(self, self.tr("Enter column name"), self.tr("Column name"))
        if ok:
            self.tblView.model().setHeaderData(index, Qt.Horizontal, txt)

    def value(self):
        cols = self.tblView.model().columnCount()
        rows = self.tblView.model().rowCount()

        items = []
        for row in range(rows):
            for col in range(cols):
                items.append(str(self.tblView.model().item(row, col).text()))

        return items

    def setValue(self, headers, table):
        model = self.tblView.model()
        model.setHorizontalHeaderLabels(headers)

        cols = len(headers)
        rows = len(table) // cols
        model = QStandardItemModel(rows, cols)

        for row in range(rows):
            for col in range(cols):
                item = QStandardItem(str(table[row * cols + col]))
                model.setItem(row, col, item)
        self.tblView.setModel(model)

    def headers(self):
        headers = []
        model = self.tblView.model()
        for i in range(model.columnCount()):
            headers.append(str(model.headerData(i, Qt.Horizontal)))

        return headers

    def fixedRows(self):
        return self.chkFixedRows.isChecked()

    def setFixedRows(self, fixedRows):
        self.chkFixedRows.setChecked(fixedRows)
