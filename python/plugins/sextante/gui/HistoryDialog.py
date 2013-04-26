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
from sextante.gui import TestTools
from sextante.core.SextanteLog import SextanteLog
from sextante.ui.ui_DlgHistory import Ui_DlgHistory


class HistoryDialog(QDialog, Ui_DlgHistory):
    def __init__(self):
        QDialog.__init__(self)
        self.setupUi(self)

        self.groupIcon = QIcon()
        self.groupIcon.addPixmap(self.style().standardPixmap(QStyle.SP_DirClosedIcon),
                QIcon.Normal, QIcon.Off)
        self.groupIcon.addPixmap(self.style().standardPixmap(QStyle.SP_DirOpenIcon),
                QIcon.Normal, QIcon.On)

        self.keyIcon = QIcon()
        self.keyIcon.addPixmap(self.style().standardPixmap(QStyle.SP_FileIcon))

        self.clearButton = QPushButton(self.tr("Clear"))
        self.clearButton.setToolTip(self.tr("Clear history and log"))
        self.buttonBox.addButton(self.clearButton, QDialogButtonBox.ActionRole)

        self.tree.doubleClicked.connect(self.executeAlgorithm)
        self.tree.currentItemChanged.connect(self.changeText)
        self.clearButton.clicked.connect(self.clearLog)

        self.tree.customContextMenuRequested.connect(self.showPopupMenu)

        self.fillTree()

    def clearLog(self):
        SextanteLog.clearLog()
        self.fillTree()

    def fillTree(self):
        self.tree.clear()
        elements = SextanteLog.getLogEntries()
        for category in elements.keys():
            groupItem = QTreeWidgetItem()
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

    def createTest(self):
        item = self.tree.currentItem()
        if isinstance(item, TreeLogEntryItem):
            if item.isAlg:
                TestTools.createTest(item.entry.text)

    def showPopupMenu(self,point):
        item = self.tree.currentItem()
        if isinstance(item, TreeLogEntryItem):
           if item.isAlg:
                popupmenu = QMenu()
                createTestAction = QAction(self.tr("Create test"), self.tree)
                createTestAction.triggered.connect(self.createTest)
                popupmenu.addAction(createTestAction)
                popupmenu.exec_(self.tree.mapToGlobal(point))

class TreeLogEntryItem(QTreeWidgetItem):
    def __init__(self, entry, isAlg):
        QTreeWidgetItem.__init__(self)
        self.entry = entry
        self.isAlg = isAlg
        self.setText(0, "[" + entry.date + "] " + entry.text.split("|")[0])
