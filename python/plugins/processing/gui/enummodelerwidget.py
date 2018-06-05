# -*- coding: utf-8 -*-

"""
***************************************************************************
    EnumModelerWidget.py
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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import warnings

from qgis.PyQt import uic
from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtGui import QStandardItemModel, QStandardItem
from qgis.PyQt.QtWidgets import QMessageBox

from qgis.core import QgsApplication

pluginPath = os.path.split(os.path.dirname(__file__))[0]

with warnings.catch_warnings():
    warnings.filterwarnings("ignore", category=DeprecationWarning)
    WIDGET, BASE = uic.loadUiType(
        os.path.join(pluginPath, 'ui', 'enummodelerwidgetbase.ui'))


class EnumModelerWidget(BASE, WIDGET):

    def __init__(self, parent=None):
        super(EnumModelerWidget, self).__init__(parent)
        self.setupUi(self)

        self.btnAdd.setIcon(QgsApplication.getThemeIcon('/symbologyAdd.svg'))
        self.btnRemove.setIcon(QgsApplication.getThemeIcon('/symbologyRemove.svg'))
        self.btnClear.setIcon(QgsApplication.getThemeIcon('console/iconClearConsole.svg'))

        self.btnAdd.clicked.connect(self.addItem)
        self.btnRemove.clicked.connect(lambda: self.removeItems())
        self.btnClear.clicked.connect(lambda: self.removeItems(True))

        self.lstItems.setModel(QStandardItemModel())

        self.lstItems.model().itemChanged.connect(self.onItemChanged)

    def onItemChanged(self, item):
        model = self.lstItems.model()
        checkedItem = None
        for i in range(model.rowCount()):
            itm = model.item(i)
            if itm.checkState() == Qt.Checked and itm.data() == Qt.Checked:
                checkedItem = i
                break

        model.blockSignals(True)
        if checkedItem is None:
            item.setData(item.checkState())
        else:
            if self.chkAllowMultiple.isChecked():
                item.setData(item.checkState())
            else:
                model.item(checkedItem).setCheckState(Qt.Unchecked)
                model.item(checkedItem).setData(Qt.Unchecked)

                item.setData(item.checkState())
        model.blockSignals(False)

    def addItem(self):
        model = self.lstItems.model()

        item = QStandardItem('new item')
        item.setCheckable(True)
        item.setDropEnabled(False)
        item.setData(Qt.Unchecked)

        model.appendRow(item)

    def removeItems(self, removeAll=False):
        if removeAll:
            res = QMessageBox.question(self, self.tr('Clear?'), self.tr('Are you sure you want to delete all items?'))
            if res == QMessageBox.Yes:
                self.lstItems.model().clear()
        else:
            self.lstItems.setUpdatesEnabled(False)
            indexes = sorted(self.lstItems.selectionModel().selectedIndexes())
            for i in reversed(indexes):
                self.lstItems.model().removeRow(i.row())
            self.lstItems.setUpdatesEnabled(True)

    def options(self):
        items = []
        model = self.lstItems.model()
        for i in range(model.rowCount()):
            item = model.item(i)
            items.append(item.text())

        return items

    def defaultOptions(self):
        options = []
        model = self.lstItems.model()
        for i in range(model.rowCount()):
            item = model.item(i)
            if item.checkState() == Qt.Checked:
                if not self.allowMultiple():
                    return i
                options.append(i)
        return options if len(options) > 0 else None

    def allowMultiple(self):
        return self.chkAllowMultiple.isChecked()

    def setOptions(self, options):
        model = self.lstItems.model()
        for i in options:
            item = QStandardItem(i)
            item.setCheckable(True)
            item.setDropEnabled(False)
            item.setData(Qt.Unchecked)

            model.appendRow(item)

    def setDefault(self, indexes):
        if indexes is None:
            return
        model = self.lstItems.model()
        if not isinstance(indexes, (list, tuple)):
            indexes = [indexes]
        for i in indexes:
            item = model.item(i)
            if item:
                item.setCheckState(Qt.Checked)
                item.setData(Qt.Checked)

    def setAllowMultiple(self, allowMultiple):
        self.chkAllowMultiple.setChecked(allowMultiple)
