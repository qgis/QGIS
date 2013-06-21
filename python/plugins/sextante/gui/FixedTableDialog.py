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

from PyQt4 import QtCore, QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *


class FixedTableDialog(QtGui.QDialog):
    def __init__(self, param, table):
        QtGui.QDialog.__init__(self)
        self.setModal(True)
        self.param = param
        self.rettable = table
        self.setupUi()
        self.rettable = None

    def setupUi(self):
        self.resize(600, 350)
        self.setWindowTitle("Fixed Table")
        self.horizontalLayout = QtGui.QHBoxLayout()
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.buttonBox = QtGui.QDialogButtonBox()
        self.buttonBox.setOrientation(QtCore.Qt.Vertical)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.table = QtGui.QTableWidget()
        self.table.setColumnCount(len(self.param.cols))
        for i in range(len(self.param.cols)):
            self.table.setColumnWidth(i,380 / len(self.param.cols))
            self.table.setHorizontalHeaderItem(i, QtGui.QTableWidgetItem(self.param.cols[i]))
        self.table.horizontalHeader().setResizeMode(QtGui.QHeaderView.Stretch)
        self.table.setRowCount(len(self.rettable))
        for i in range(len(self.rettable)):
            self.table.setRowHeight(i,22)
        self.table.verticalHeader().setVisible(False)
        self.addRowButton = QtGui.QPushButton()
        self.addRowButton.setText("Add row")
        self.addRowButton.setEnabled(not self.param.fixedNumOfRows)
        self.removeRowButton = QtGui.QPushButton()
        self.removeRowButton.setText("Remove row")
        self.removeRowButton.setEnabled(not self.param.fixedNumOfRows)
        self.buttonBox.addButton(self.addRowButton, QtGui.QDialogButtonBox.ActionRole)
        self.buttonBox.addButton(self.removeRowButton, QtGui.QDialogButtonBox.ActionRole)
        self.setTableContent()
        self.horizontalLayout.addWidget(self.table)
        self.horizontalLayout.addWidget(self.buttonBox)
        self.setLayout(self.horizontalLayout)
        QObject.connect(self.buttonBox, QtCore.SIGNAL("accepted()"), self.okPressed)
        QObject.connect(self.buttonBox, QtCore.SIGNAL("rejected()"), self.cancelPressed)
        QObject.connect(self.addRowButton, QtCore.SIGNAL("clicked()"), self.addRow)
        QObject.connect(self.removeRowButton, QtCore.SIGNAL("clicked()"), self.removeRow)
        QtCore.QMetaObject.connectSlotsByName(self)

    def setTableContent(self):
        for i in range(len(self.rettable)):
            for j in range(len(self.rettable[0])):
                self.table.setItem(i,j,QtGui.QTableWidgetItem(self.rettable[i][j]))

    def okPressed(self):
        self.rettable = []
        for i in range(self.table.rowCount()):
            self.rettable.append(list())
            for j in range(self.table.columnCount()):
                self.rettable[i].append(unicode(self.table.item(i,j).text()))
        self.close()

    def cancelPressed(self):
        self.rettable = None
        self.close()

    def removeRow(self):
        if self.table.rowCount() > 1:
            self.table.setRowCount(self.table.rowCount()-1)

    def addRow(self):
        self.table.setRowCount(self.table.rowCount()+1)
        self.table.setRowHeight(self.table.rowCount()-1, 22)
        for i in range(self.table.columnCount()):
            self.table.setItem(self.table.rowCount()-1,i,QtGui.QTableWidgetItem("0"))