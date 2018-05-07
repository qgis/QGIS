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

from qgis.PyQt import uic
from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtGui import QStandardItemModel, QStandardItem

from qgis.core import QgsApplication

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'enummodelerwidgetbase.ui'))


class EnumModelerWidget(BASE, WIDGET):

    def __init__(self, parent=None):
        super(EnumModelerWidget, self).__init__(parent)
        self.setupUi(self)

        self.btnAdd.setIcon(QgsApplication.getThemeIcon('/symbologyAdd.svg'))
        self.btnRemove.setIcon(QgsApplication.getThemeIcon('/symbologyRemove.svg'))
        self.btnClear.setIcon(QgsApplication.getThemeIcon('/mIconClearText.svg'))

        self.btnAdd.clicked.connect(self.addItem)
        self.btnRemove.clicked.connect(lambda: self.removeItems())
        self.btnClear.clicked.connect(lambda: self.removeItems(True))

        self.lstItems.setModel(QStandardItemModel())

        self.lstItems.clicked.connect(self.handleCheckbox)

    def handleCheckbox(self, index):
        model = self.lstItems.model()
        clickedItem = model.itemFromIndex(index)

        prevIndex = None
        for i in range(model.rowCount()):
            if model.item(i).checkState() == Qt.Checked:
                prevIndex = i
                break

        if prevIndex is None:
            clickedItem.setCheckState(Qt.Checked)
        else:
            if self.chkAllowMultiple.isChecked():
                clickedItem.setCheckState(Qt.Checked)
            else:
                model.item(prevIndex).setCheckState(Qt.Unchecked)
                clickedItem.setCheckState(Qt.Checked)

    def addItem(self):
        model = self.lstItems.model()

        item = QStandardItem('new item')
        item.setCheckable(True)
        item.setCheckState(Qt.Unchecked)
        item.setDropEnabled(False)

        model.appendRow(item)

    def removeItems(self, removeAll=False):
        if removeAll:
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
                options.append(i)
        return options if len(options) > 0 else None

    def allowMultiple(self):
        return self.chkAllowMultiple.isChecked()

    def setOptions(self, options):
        model = self.lstItems.model()
        for i in options:
            item = QStandardItem()
            item.setCheckable(True)
            item.setCheckState(Qt.Unchecked)
            item.setDropEnabled(False)

            model.appendRow(item)

    def setDefault(self, index):
        model = self.lstItems.model()
        item = model.item(index, 0)
        if item:
            item.setCheckState(Qt.Checked)

    def setAllowMultiple(self, allowMultiple):
        self.chkAllowMultiple.setChecked(allowMultiple)

        model = self.lstItems.model()
        for i in range(model.rowCount()):
            if model.item(i).checkState() == Qt.Checked:
                model.item(i).setCheckState(Qt.Unchecked)
                break
