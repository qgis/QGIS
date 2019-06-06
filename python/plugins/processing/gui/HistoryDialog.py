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

import os
import warnings

from qgis.core import QgsApplication
from qgis.gui import QgsGui, QgsHelp
from qgis.PyQt import uic
from qgis.PyQt.QtCore import Qt, QCoreApplication
from qgis.PyQt.QtWidgets import QAction, QPushButton, QDialogButtonBox, QStyle, QMessageBox, QFileDialog, QMenu, QTreeWidgetItem
from qgis.PyQt.QtGui import QIcon
from processing.gui import TestTools
from processing.core.ProcessingLog import ProcessingLog, LOG_SEPARATOR

pluginPath = os.path.split(os.path.dirname(__file__))[0]

with warnings.catch_warnings():
    warnings.filterwarnings("ignore", category=DeprecationWarning)
    WIDGET, BASE = uic.loadUiType(
        os.path.join(pluginPath, 'ui', 'DlgHistory.ui'))


class HistoryDialog(BASE, WIDGET):

    def __init__(self):
        super(HistoryDialog, self).__init__(None)
        self.setupUi(self)

        QgsGui.instance().enableAutoGeometryRestore(self)

        self.groupIcon = QgsApplication.getThemeIcon('mIconFolder.svg')

        self.keyIcon = self.style().standardIcon(QStyle.SP_FileIcon)

        self.clearButton = QPushButton(self.tr('Clear'))
        self.clearButton.setToolTip(self.tr('Clear history'))
        self.buttonBox.addButton(self.clearButton, QDialogButtonBox.ActionRole)

        self.helpButton = QPushButton(self.tr('Help'))
        self.helpButton.setToolTip(self.tr('Show help'))
        self.buttonBox.addButton(self.helpButton, QDialogButtonBox.HelpRole)

        self.saveButton = QPushButton(QCoreApplication.translate('HistoryDialog', 'Save As…'))
        self.saveButton.setToolTip(self.tr('Save history'))
        self.buttonBox.addButton(self.saveButton, QDialogButtonBox.ActionRole)

        self.tree.doubleClicked.connect(self.executeAlgorithm)
        self.tree.currentItemChanged.connect(self.changeText)
        self.clearButton.clicked.connect(self.clearLog)
        self.saveButton.clicked.connect(self.saveLog)
        self.helpButton.clicked.connect(self.openHelp)

        self.tree.setContextMenuPolicy(Qt.CustomContextMenu)
        self.tree.customContextMenuRequested.connect(self.showPopupMenu)

        self.fillTree()

    def clearLog(self):
        reply = QMessageBox.question(self,
                                     self.tr('Confirmation'),
                                     self.tr('Are you sure you want to clear the history?'),
                                     QMessageBox.Yes | QMessageBox.No,
                                     QMessageBox.No
                                     )
        if reply == QMessageBox.Yes:
            ProcessingLog.clearLog()
            self.fillTree()

    def saveLog(self):
        fileName, filter = QFileDialog.getSaveFileName(self,
                                                       self.tr('Save File'), '.', self.tr('Log files (*.log *.LOG)'))

        if fileName == '':
            return

        if not fileName.lower().endswith('.log'):
            fileName += '.log'

        ProcessingLog.saveLog(fileName)

    def openHelp(self):
        QgsHelp.openHelp("processing/history.html")

    def fillTree(self):
        self.tree.clear()
        entries = ProcessingLog.getLogEntries()
        groupItem = QTreeWidgetItem()
        groupItem.setText(0, 'ALGORITHM')
        groupItem.setIcon(0, self.groupIcon)
        for entry in entries:
            item = TreeLogEntryItem(entry, True)
            item.setIcon(0, self.keyIcon)
            groupItem.insertChild(0, item)
        self.tree.addTopLevelItem(groupItem)
        groupItem.setExpanded(True)

    def executeAlgorithm(self):
        item = self.tree.currentItem()
        if isinstance(item, TreeLogEntryItem):
            if item.isAlg:
                script = 'import processing\n'
                script += 'from qgis.core import QgsProcessingOutputLayerDefinition, QgsProcessingFeatureSourceDefinition, QgsProperty, QgsCoordinateReferenceSystem\n'
                script += item.entry.text.replace('processing.run(', 'processing.execAlgorithmDialog(')
                self.close()
                exec(script)

    def changeText(self):
        item = self.tree.currentItem()
        if isinstance(item, TreeLogEntryItem):
            self.text.setText(item.entry.text.replace(LOG_SEPARATOR, '\n'))

    def createTest(self):
        item = self.tree.currentItem()
        if isinstance(item, TreeLogEntryItem):
            if item.isAlg:
                TestTools.createTest(item.entry.text)

    def showPopupMenu(self, point):
        item = self.tree.currentItem()
        if isinstance(item, TreeLogEntryItem):
            if item.isAlg:
                popupmenu = QMenu()
                createTestAction = QAction(QCoreApplication.translate('HistoryDialog', 'Create Test…'), self.tree)
                createTestAction.triggered.connect(self.createTest)
                popupmenu.addAction(createTestAction)
                popupmenu.exec_(self.tree.mapToGlobal(point))


class TreeLogEntryItem(QTreeWidgetItem):

    def __init__(self, entry, isAlg):
        QTreeWidgetItem.__init__(self)
        self.entry = entry
        self.isAlg = isAlg
        self.setText(0, '[' + entry.date + '] ' + entry.text.split(LOG_SEPARATOR)[0])
