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
import sys
import warnings
import json
from functools import partial
from typing import Optional

from qgis.PyQt import uic
from qgis.PyQt.Qsci import QsciScintilla
from qgis.PyQt.QtCore import Qt, QCoreApplication, QDateTime, QMimeData
from qgis.PyQt.QtGui import QClipboard
from qgis.PyQt.QtWidgets import QAction, QPushButton, QDialogButtonBox, QStyle, QMessageBox, QFileDialog, QMenu, \
    QTreeWidgetItem, QShortcut, QApplication
from qgis.core import QgsApplication, Qgis
from qgis.gui import QgsGui, QgsHelp, QgsHistoryEntry

from processing.gui import TestTools

pluginPath = os.path.split(os.path.dirname(__file__))[0]

with warnings.catch_warnings():
    warnings.filterwarnings("ignore", category=DeprecationWarning)
    WIDGET, BASE = uic.loadUiType(
        os.path.join(pluginPath, 'ui', 'DlgHistory.ui'))


class HistoryDialog(BASE, WIDGET):
    LOG_SEPARATOR = '|~|'

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
        shorcut = QShortcut(Qt.Key_Enter, self.tree, context=Qt.WidgetShortcut, activated=self.executeAlgorithm)

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
            QgsGui.historyProviderRegistry().clearHistory(Qgis.HistoryProviderBackend.LocalProfile)
            self.fillTree()

    def saveLog(self):
        fileName, filter = QFileDialog.getSaveFileName(self,
                                                       self.tr('Save File'), '.', self.tr('Log files (*.log *.LOG)'))

        if fileName == '':
            return

        if not fileName.lower().endswith('.log'):
            fileName += '.log'

        entries = QgsGui.historyProviderRegistry().queryEntries(providerId='processing')
        with open(fileName, 'w', encoding='utf-8') as f:
            for entry in entries:
                f.write('ALGORITHM{}{}{}{}\n'.format(HistoryDialog.LOG_SEPARATOR,
                                                     entry.timestamp.toString("YYYY-mm-dd HH:MM:SS"),
                                                     HistoryDialog.LOG_SEPARATOR,
                                                     entry.entry.get('python_command')))

    def openHelp(self):
        QgsHelp.openHelp("processing/history.html")

    def contextDateString(self, date: QDateTime):
        if date in self.contextDateStrings:
            return self.contextDateStrings[date]

        if date.date() == QDateTime.currentDateTime().date():
            self.contextDateStrings[date] = self.tr('Today')
        else:
            interval_days = date.date().daysTo(QDateTime.currentDateTime().date())
            if interval_days == 1:
                self.contextDateStrings[date] = self.tr('Yesterday')
            elif interval_days < 8:
                self.contextDateStrings[date] = self.tr('Last 7 days')
            else:
                self.contextDateStrings[date] = date.toString('MMMM yyyy')
        return self.contextDateString(date)

    def fillTree(self):
        self.tree.clear()
        entries = QgsGui.historyProviderRegistry().queryEntries(providerId='processing')

        if not entries:
            return

        names = {}
        icons = {}
        group_items = []
        current_group_item = -1
        current_date = ''
        for entry in entries:
            date = self.contextDateString(entry.timestamp)
            if date != current_date:
                current_date = date
                current_group_item += 1
                group_items.append(QTreeWidgetItem())
                group_items[current_group_item].setText(0, date)
                group_items[current_group_item].setIcon(0, self.groupIcon)
            icon = self.keyIcon
            name = ''
            algorithm_id = entry.entry.get('algorithm_id')
            if algorithm_id:
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
            item = TreeLogEntryItem(entry, name)
            item.setIcon(0, icon)
            group_items[current_group_item].insertChild(0, item)

        self.tree.addTopLevelItems(reversed(group_items))
        self.tree.topLevelItem(0).setExpanded(True)

    def executeAlgorithm(self):
        item = self.tree.currentItem()
        if isinstance(item, TreeLogEntryItem):
            if item.as_python_command():
                script = 'import processing\n'
                # adding to this list? Also update the BatchPanel.py imports!!
                script += 'from qgis.core import QgsProcessingOutputLayerDefinition, QgsProcessingFeatureSourceDefinition, QgsProperty, QgsCoordinateReferenceSystem, QgsFeatureRequest\n'
                script += 'from qgis.PyQt.QtCore import QDate, QTime, QDateTime\n'
                script += 'from qgis.PyQt.QtGui import QColor\n'
                script += item.as_python_command().replace('processing.run(', 'processing.execAlgorithmDialog(')
                self.close()
                exec(script)

    def changeText(self):
        item = self.tree.currentItem()
        if isinstance(item, TreeLogEntryItem):
            self.text.setText('"""\n' + self.tr(
                'Double-click on the history item or paste the command below to re-run the algorithm') + '\n"""\n\n' +
                item.as_python_command())
        else:
            self.text.setText('')

    def createTest(self):
        item = self.tree.currentItem()
        if isinstance(item, TreeLogEntryItem):
            if item.as_python_command():
                TestTools.createTest(item.as_python_command())

    def copy_text(self, text: str):
        """
        Copies a text string to clipboard
        """
        m = QMimeData()
        m.setText(text)
        cb = QApplication.clipboard()

        if sys.platform in ("linux", "linux2"):
            cb.setMimeData(m, QClipboard.Selection)
        cb.setMimeData(m, QClipboard.Clipboard)

    def showPopupMenu(self, point):
        item = self.tree.currentItem()
        if isinstance(item, TreeLogEntryItem):
            popupmenu = QMenu()

            python_command = item.as_python_command()
            if python_command:
                python_action = QAction(
                    QCoreApplication.translate('HistoryDialog', 'Copy as Python Command'), self.tree)
                python_action.setIcon(QgsApplication.getThemeIcon("mIconPythonFile.svg"))
                python_action.triggered.connect(partial(self.copy_text, python_command))
                popupmenu.addAction(python_action)

            qgis_process_command = item.as_qgis_process_command()
            if qgis_process_command:
                qgis_process_action = QAction(
                    QCoreApplication.translate('HistoryDialog', 'Copy as qgis_process Command'), self.tree)
                qgis_process_action.setIcon(QgsApplication.getThemeIcon("mActionTerminal.svg"))
                qgis_process_action.triggered.connect(partial(self.copy_text, qgis_process_command))
                popupmenu.addAction(qgis_process_action)

            inputs_json = item.inputs_map()
            if inputs_json:
                as_json_action = QAction(
                    QCoreApplication.translate('HistoryDialog', 'Copy as JSON'), self.tree)
                as_json_action.setIcon(QgsApplication.getThemeIcon("mActionEditCopy.svg"))
                as_json_action.triggered.connect(partial(self.copy_text, json.dumps(inputs_json, indent=2)))
                popupmenu.addAction(as_json_action)

            if not popupmenu.isEmpty():
                popupmenu.addSeparator()

            if python_command:
                create_test_action = QAction(QCoreApplication.translate('HistoryDialog', 'Create Test…'), self.tree)
                create_test_action.triggered.connect(self.createTest)
                popupmenu.addAction(create_test_action)

            popupmenu.exec_(self.tree.mapToGlobal(point))


class TreeLogEntryItem(QTreeWidgetItem):

    def __init__(self, entry: QgsHistoryEntry, algName):
        QTreeWidgetItem.__init__(self)
        self.entry = entry

        parameters = entry.entry.get('parameters')
        if isinstance(parameters, dict) and parameters.get('inputs'):
            entry_description = str(parameters['inputs'])
        else:
            entry_description = entry.entry.get('python_command', '')

        if len(entry_description) > 300:
            entry_description = entry_description[:299] + '…'

        self.setText(0, f'[{entry.timestamp.toString("yyyy-MM-dd hh:mm")}] {algName} - {entry_description}')

    def as_python_command(self) -> Optional[str]:
        """
        Returns the entry as a python command, if possible
        """
        return self.entry.entry.get('python_command')

    def as_qgis_process_command(self) -> Optional[str]:
        """
        Returns the entry as a qgis_process command, if possible
        """
        return self.entry.entry.get('process_command')

    def inputs_map(self) -> Optional[dict]:
        """
        Returns the entry inputs as a dict
        """
        return self.entry.entry.get('parameters')
