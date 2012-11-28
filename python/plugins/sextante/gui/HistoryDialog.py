# -*- coding: utf-8 -*-

"""
***************************************************************************
    HistoryDialog.py
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

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import QtCore, QtGui
from sextante.core.SextanteLog import SextanteLog

class HistoryDialog(QtGui.QDialog):
    def __init__(self):
        QtGui.QDialog.__init__(self)
        self.setModal(True)
        self.setupUi()

    def setupUi(self):
        self.resize(650, 505)
        self.verticalLayout = QtGui.QVBoxLayout()
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setMargin(0)
        self.tree = QtGui.QTreeWidget()
        self.tree.setHeaderHidden(True)
        self.tree.doubleClicked.connect(self.executeAlgorithm)
        self.verticalLayout.addWidget(self.tree)
        self.tree.currentItemChanged.connect(self.changeText)
        self.groupIcon = QtGui.QIcon()
        self.groupIcon.addPixmap(self.style().standardPixmap(QtGui.QStyle.SP_DirClosedIcon),
                QtGui.QIcon.Normal, QtGui.QIcon.Off)
        self.groupIcon.addPixmap(self.style().standardPixmap(QtGui.QStyle.SP_DirOpenIcon),
                QtGui.QIcon.Normal, QtGui.QIcon.On)
        self.keyIcon = QtGui.QIcon()
        self.keyIcon.addPixmap(self.style().standardPixmap(QtGui.QStyle.SP_FileIcon))
        self.fillTree()
        self.text = QtGui.QTextEdit()
        self.verticalLayout.addWidget(self.text)
        self.text.setReadOnly(True)
        self.closeButton = QtGui.QPushButton()
        self.closeButton.setText("Close")
        self.clearButton = QtGui.QPushButton()
        self.clearButton.setText("Clear history")
        self.horizontalLayout= QtGui.QHBoxLayout()
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.horizontalLayout.addStretch(1000)
        self.horizontalLayout.addWidget(self.clearButton)
        self.horizontalLayout.addWidget(self.closeButton)
        QObject.connect(self.closeButton, QtCore.SIGNAL("clicked()"), self.closeWindow)
        QObject.connect(self.clearButton, QtCore.SIGNAL("clicked()"), self.clearLog)
        self.verticalLayout.addLayout(self.horizontalLayout)
        self.setWindowTitle("History")
        self.setLayout(self.verticalLayout)
        QtCore.QMetaObject.connectSlotsByName(self)


    def closeWindow(self):
        self.close()

    def clearLog(self):
        SextanteLog.clearLog()
        self.fillTree()

    def fillTree(self):
        self.tree.clear()
        elements = SextanteLog.getLogEntries()
        for category in elements.keys():
            groupItem = QtGui.QTreeWidgetItem()
            groupItem.setText(0,category)
            groupItem.setIcon(0, self.groupIcon)
            for entry in elements[category]:
                item = TreeLogEntryItem(entry, category==SextanteLog.LOG_ALGORITHM)
                item.setIcon(0, self.keyIcon)
                groupItem.insertChild(0,item)
            self.tree.addTopLevelItem(groupItem)


    def executeAlgorithm(self):
        item = self.tree.currentItem()
        if isinstance(item, TreeLogEntryItem):
            if item.isAlg:
                script = "import sextante\n"
                script+=item.entry.text.replace("runalg(","runandload(")
                exec(script)


    def changeText(self):
        item = self.tree.currentItem()
        if isinstance(item, TreeLogEntryItem):
                self.text.setText(item.entry.text.replace("|","\n"))


class TreeLogEntryItem(QtGui.QTreeWidgetItem):
    def __init__(self, entry, isAlg):
        QTreeWidgetItem.__init__(self)
        self.entry = entry
        self.isAlg = isAlg
        self.setText(0, "[" + entry.date + "] " + entry.text.split("|")[0])
