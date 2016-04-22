# -*- coding: utf-8 -*-

"""
***************************************************************************
    optionsTable.py
    ---------------------
    Date                 : June 2010
    Copyright            : (C) 2010 by Giuseppe Sucameli
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
__date__ = 'June 2010'
__copyright__ = '(C) 2010, Giuseppe Sucameli'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.PyQt.QtCore import pyqtSignal
from qgis.PyQt.QtWidgets import QWidget, QTableWidgetItem

from .ui_optionsTable import Ui_GdalToolsOptionsTable as Ui_OptionsTable


class GdalToolsOptionsTable(QWidget, Ui_OptionsTable):
    rowAdded = pyqtSignal(int)
    rowRemoved = pyqtSignal()

    def __init__(self, parent=None):
        QWidget.__init__(self, parent)

        self.setupUi(self)

        self.table.cellChanged.connect(self.cellValueChanged)

        self.table.itemSelectionChanged.connect(self.enableDeleteButton)
        self.btnAdd.clicked.connect(self.addNewRow)
        self.btnDel.clicked.connect(self.deleteRow)

        self.btnDel.setEnabled(False)

    def enableDeleteButton(self):
        self.btnDel.setEnabled(self.table.currentRow() >= 0)

    def addNewRow(self):
        self.table.insertRow(self.table.rowCount())
        # select the added row
        newRow = self.table.rowCount() - 1
        item = QTableWidgetItem()
        self.table.setItem(newRow, 0, item)
        self.table.setCurrentItem(item)
        self.rowAdded.emit(newRow)

    def deleteRow(self):
        if self.table.currentRow() >= 0:
            self.table.removeRow(self.table.currentRow())
            # select the previous row or the next one if there is no previous row
            item = self.table.item(self.table.currentRow(), 0)
            self.table.setCurrentItem(item)
            self.rowRemoved.emit()

    def options(self):
        options = []
        for row in range(0, self.table.rowCount()):
            name = self.table.item(row, 0)
            if not name:
                continue

            value = self.table.item(row, 1)
            if not value:
                continue

            options.append(name.text() + "=" + value.text())
        return options
