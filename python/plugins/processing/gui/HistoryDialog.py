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
import re
from datetime import datetime

from qgis.core import QgsApplication
from qgis.gui import QgsGui, QgsHelp
from qgis.PyQt import uic
from qgis.PyQt.QtCore import Qt, QCoreApplication, QDate
from qgis.PyQt.QtWidgets import QAction, QPushButton, QDialogButtonBox, QStyle, QMessageBox, QFileDialog, QMenu, QTreeWidgetItem, QShortcut
from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.Qsci import QsciScintilla

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

        self.text.setReadOnly(True)
        self.text.setCaretLineVisible(False)
        self.text.setLineNumbersVisible(False)  # NO linenumbers for the input line
        self.text.setFoldingVisible(False)
        self.text.setEdgeMode(QsciScintilla.EdgeNone)
        self.text.setWrapMode(QsciScintilla.SC_WRAP_WORD)

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
        shorcut = QShortcut(Qt.Key_Return, self.tree, context=Qt.WidgetShortcut, activated=self.executeAlgorithm)

        self.clearButton.clicked.connect(self.clearLog)
        self.saveButton.clicked.connect(self.saveLog)
        self.helpButton.clicked.connect(self.openHelp)

        self.tree.setContextMenuPolicy(Qt.CustomContextMenu)
        self.tree.customContextMenuRequested.connect(self.showPopupMenu)

        self.contextDateStrings = {}

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

    def contextDateString(self, date):
        if date in self.contextDateStrings:
            return self.contextDateStrings[date]

        if date == datetime.today().strftime('%Y-%m-%d'):
            self.contextDateStrings[date] = self.tr('Today')
        else:
            interval_days = (datetime.today() - datetime.strptime(date, '%Y-%m-%d')).days
            if interval_days == 1:
                self.contextDateStrings[date] = self.tr('Yesterday')
            elif interval_days < 8:
                self.contextDateStrings[date] = self.tr('Last 7 days')
            else:
                self.contextDateStrings[date] = QDate.fromString(date, 'yyyy-MM-dd').toString('MMMM yyyy')
        return self.contextDateString(date)

    def fillTree(self):
        self.tree.clear()
        entries = ProcessingLog.getLogEntries()

        if not entries:
            return

        names = {}
        icons = {}
        group_items = []
        current_group_item = -1
        current_date = ''
        for entry in entries:
            date = self.contextDateString(entry.date[0:10])
            if date != current_date:
                current_date = date
                current_group_item += 1
                group_items.append(QTreeWidgetItem())
                group_items[current_group_item].setText(0, date)
                group_items[current_group_item].setIcon(0, self.groupIcon)
            icon = self.keyIcon
            name = ''
            match = re.search('processing.run\\("(.*?)"', entry.text)
            if match.group:
                algorithm_id = match.group(1)
                if algorithm_id not in names:
                    algorithm = QgsApplication.processingRegistry().algorithmById(algorithm_id)
                    if algorithm:
                        names[algorithm_id] = algorithm.displayName()
                        icons[algorithm_id] = QgsApplication.processingRegistry().algorithmById(algorithm_id).icon()
                    else:
                        names[algorithm_id] = ''
                        icons[algorithm_id] = self.keyIcon
                name = names[algorithm_id]
                icon = icons[algorithm_id]
            item = TreeLogEntryItem(entry, True, name)
            item.setIcon(0, icon)
            group_items[current_group_item].insertChild(0, item)

        self.tree.addTopLevelItems(reversed(group_items))
        self.tree.topLevelItem(0).setExpanded(True)

    def executeAlgorithm(self):
        item = self.tree.currentItem()
        if isinstance(item, TreeLogEntryItem):
            if item.isAlg:
                script = 'import processing\n'
                # adding to this list? Also update the BatchPanel.py imports!!
                script += 'from qgis.core import QgsProcessingOutputLayerDefinition, QgsProcessingFeatureSourceDefinition, QgsProperty, QgsCoordinateReferenceSystem, QgsFeatureRequest\n'
                script += 'from qgis.PyQt.QtCore import QDate, QTime, QDateTime\n'
                script += 'from qgis.PyQt.QtGui import QColor\n'
                script += item.entry.text.replace('processing.run(', 'processing.execAlgorithmDialog(')
                self.close()
                exec(script)

    def changeText(self):
        item = self.tree.currentItem()
        if isinstance(item, TreeLogEntryItem):
            self.text.setText('"""\n' + self.tr('Double-click on the history item or paste the command below to re-run the algorithm') + '\n"""\n\n' +
                              item.entry.text.replace(LOG_SEPARATOR, '\n'))
        else:
            self.text.setText('')

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

    def __init__(self, entry, isAlg, algName):
        QTreeWidgetItem.__init__(self)
        self.entry = entry
        self.isAlg = isAlg
        self.setText(0, '[' + entry.date[:-3] + '] ' + algName + ' - ' + entry.text.split(LOG_SEPARATOR)[0])
