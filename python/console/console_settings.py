# -*- coding:utf-8 -*-
"""
/***************************************************************************
Python Console for QGIS
                             -------------------
begin                : 2012-09-10
copyright            : (C) 2012 by Salvatore Larosa
email                : lrssvtml (at) gmail (dot) com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
Some portions of code were taken from https://code.google.com/p/pydee/
"""

from qgis.PyQt.QtCore import QCoreApplication, QUrl
from qgis.PyQt.QtWidgets import QWidget, QFileDialog, QMessageBox, QTableWidgetItem, QHBoxLayout
from qgis.PyQt.QtGui import QIcon, QDesktopServices

from qgis.core import QgsSettings, QgsApplication
from qgis.gui import QgsOptionsPageWidget, QgsOptionsWidgetFactory

from .console_compile_apis import PrepareAPIDialog
from .ui_console_settings import Ui_SettingsDialogPythonConsole


class ConsoleOptionsFactory(QgsOptionsWidgetFactory):

    def __init__(self):
        super(QgsOptionsWidgetFactory, self).__init__()

    def icon(self):
        return QgsApplication.getThemeIcon('/console/mIconRunConsole.svg')

    def path(self):
        return ['ide']

    def createWidget(self, parent):
        return ConsoleOptionsPage(parent)


class ConsoleOptionsPage(QgsOptionsPageWidget):

    def __init__(self, parent):
        super(ConsoleOptionsPage, self).__init__(parent)
        self.options_widget = ConsoleOptionsWidget(parent)
        layout = QHBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setMargin(0)
        self.setLayout(layout)
        layout.addWidget(self.options_widget)
        self.setObjectName('consoleOptions')

    def apply(self):
        self.options_widget.accept()

    def helpKey(self):
        return 'plugins/python_console.html'


class ConsoleOptionsWidget(QWidget, Ui_SettingsDialogPythonConsole):

    def __init__(self, parent):
        super().__init__(parent)
        self.setWindowTitle(QCoreApplication.translate(
            "SettingsDialogPythonConsole", "Python Console Settings"))
        self.parent = parent
        self.setupUi(self)

        self.listPath = []
        self.lineEdit.setReadOnly(True)

        self.restoreSettings()
        self.initialCheck()

        self.addAPIpath.setIcon(QIcon(":/images/themes/default/symbologyAdd.svg"))
        self.addAPIpath.setToolTip(QCoreApplication.translate("PythonConsole", "Add API path"))
        self.removeAPIpath.setIcon(QIcon(":/images/themes/default/symbologyRemove.svg"))
        self.removeAPIpath.setToolTip(QCoreApplication.translate("PythonConsole", "Remove API path"))

        self.preloadAPI.stateChanged.connect(self.initialCheck)
        self.addAPIpath.clicked.connect(self.loadAPIFile)
        self.removeAPIpath.clicked.connect(self.removeAPI)
        self.compileAPIs.clicked.connect(self._prepareAPI)

        self.generateToken.clicked.connect(self.generateGHToken)

    def generateGHToken(self):
        description = self.tr("PyQGIS Console")
        url = 'https://github.com/settings/tokens/new?description={}&scopes=gist'.format(description)
        QDesktopServices.openUrl(QUrl(url))

    def initialCheck(self):
        if self.preloadAPI.isChecked():
            self.enableDisable(False)
        else:
            self.enableDisable(True)

    def enableDisable(self, value):
        self.tableWidget.setEnabled(value)
        self.addAPIpath.setEnabled(value)
        self.removeAPIpath.setEnabled(value)
        self.groupBoxPreparedAPI.setEnabled(value)

    def loadAPIFile(self):
        settings = QgsSettings()
        lastDirPath = settings.value("pythonConsole/lastDirAPIPath", "", type=str)
        fileAPI, selected_filter = QFileDialog.getOpenFileName(
            self, "Open API File", lastDirPath, "API file (*.api)")
        if fileAPI:
            self.addAPI(fileAPI)
            settings.setValue("pythonConsole/lastDirAPIPath", fileAPI)

    def _prepareAPI(self):
        if self.tableWidget.rowCount() != 0:
            pap_file, filter = QFileDialog().getSaveFileName(
                self,
                "",
                '*.pap',
                "Prepared APIs file (*.pap)")
        else:
            QMessageBox.information(
                self, self.tr("Warning!"),
                self.tr('You need to add some APIs file in order to compile'))
            return
        if pap_file:
            api_lexer = 'QsciLexerPython'
            api_files = []
            count = self.tableWidget.rowCount()
            for i in range(0, count):
                api_files.append(self.tableWidget.item(i, 1).text())
            api_dlg = PrepareAPIDialog(api_lexer, api_files, pap_file, self)
            api_dlg.show()
            api_dlg.activateWindow()
            api_dlg.raise_()
            api_dlg.prepareAPI()
            self.lineEdit.setText(pap_file)

    def accept(self):
        if not self.preloadAPI.isChecked() and \
                not self.groupBoxPreparedAPI.isChecked():
            if self.tableWidget.rowCount() == 0:
                QMessageBox.information(
                    self, self.tr("Warning!"),
                    self.tr('Please specify API file or check "Use preloaded API files"'))
                return
        if self.groupBoxPreparedAPI.isChecked() and \
                not self.lineEdit.text():
            QMessageBox.information(
                self, self.tr("Warning!"),
                QCoreApplication.translate('optionsDialog', 'The APIs file was not compiled, click on "Compile APIs…"')
            )
            return
        self.saveSettings()
        self.listPath = []

    def addAPI(self, pathAPI):
        count = self.tableWidget.rowCount()
        self.tableWidget.setColumnCount(2)
        self.tableWidget.insertRow(count)
        pathItem = QTableWidgetItem(pathAPI)
        pathSplit = pathAPI.split("/")
        apiName = pathSplit[-1][0:-4]
        apiNameItem = QTableWidgetItem(apiName)
        self.tableWidget.setItem(count, 0, apiNameItem)
        self.tableWidget.setItem(count, 1, pathItem)

    def removeAPI(self):
        listItemSel = self.tableWidget.selectionModel().selectedRows()
        for index in reversed(listItemSel):
            self.tableWidget.removeRow(index.row())

    def saveSettings(self):
        settings = QgsSettings()
        settings.setValue("pythonConsole/preloadAPI", self.preloadAPI.isChecked())
        settings.setValue("pythonConsole/autoSaveScript", self.autoSaveScript.isChecked())

        settings.setValue("pythonConsole/accessTokenGithub", self.tokenGhLineEdit.text())

        for i in range(0, self.tableWidget.rowCount()):
            text = self.tableWidget.item(i, 1).text()
            self.listPath.append(text)
        settings.setValue("pythonConsole/userAPI", self.listPath)

        settings.setValue("pythonConsole/autoCompThreshold", self.autoCompThreshold.value())
        settings.setValue("pythonConsole/autoCompleteEnabled", self.groupBoxAutoCompletion.isChecked())

        settings.setValue("pythonConsole/usePreparedAPIFile", self.groupBoxPreparedAPI.isChecked())
        settings.setValue("pythonConsole/preparedAPIFile", self.lineEdit.text())

        if self.autoCompFromAPI.isChecked():
            settings.setValue("pythonConsole/autoCompleteSource", 'fromAPI')
        elif self.autoCompFromDoc.isChecked():
            settings.setValue("pythonConsole/autoCompleteSource", 'fromDoc')
        elif self.autoCompFromDocAPI.isChecked():
            settings.setValue("pythonConsole/autoCompleteSource", 'fromDocAPI')

        settings.setValue("pythonConsole/enableObjectInsp", self.enableObjectInspector.isChecked())
        settings.setValue("pythonConsole/autoCloseBracket", self.autoCloseBracket.isChecked())
        settings.setValue("pythonConsole/autoSurround", self.autoSurround.isChecked())
        settings.setValue("pythonConsole/autoInsertImport", self.autoInsertImport.isChecked())

    def restoreSettings(self):
        settings = QgsSettings()
        self.preloadAPI.setChecked(settings.value("pythonConsole/preloadAPI", True, type=bool))
        self.lineEdit.setText(settings.value("pythonConsole/preparedAPIFile", "", type=str))
        self.tokenGhLineEdit.setText(settings.value("pythonConsole/accessTokenGithub", "", type=str))
        itemTable = settings.value("pythonConsole/userAPI", [])
        if itemTable:
            self.tableWidget.setRowCount(0)
            for i in range(len(itemTable)):
                self.tableWidget.insertRow(i)
                self.tableWidget.setColumnCount(2)
                pathSplit = itemTable[i].split("/")
                apiName = pathSplit[-1][0:-4]
                self.tableWidget.setItem(i, 0, QTableWidgetItem(apiName))
                self.tableWidget.setItem(i, 1, QTableWidgetItem(itemTable[i]))
        self.autoSaveScript.setChecked(settings.value("pythonConsole/autoSaveScript", False, type=bool))

        self.autoCompThreshold.setValue(settings.value("pythonConsole/autoCompThreshold", 2, type=int))
        self.groupBoxAutoCompletion.setChecked(settings.value("pythonConsole/autoCompleteEnabled", True, type=bool))

        self.enableObjectInspector.setChecked(settings.value("pythonConsole/enableObjectInsp", False, type=bool))
        self.autoCloseBracket.setChecked(settings.value("pythonConsole/autoCloseBracket", True, type=bool))
        self.autoSurround.setChecked(settings.value("pythonConsole/autoSurround", True, type=bool))
        self.autoInsertImport.setChecked(settings.value("pythonConsole/autoInsertImport", False, type=bool))

        if settings.value("pythonConsole/autoCompleteSource") == 'fromDoc':
            self.autoCompFromDoc.setChecked(True)
        elif settings.value("pythonConsole/autoCompleteSource") == 'fromAPI':
            self.autoCompFromAPI.setChecked(True)
        elif settings.value("pythonConsole/autoCompleteSource") == 'fromDocAPI':
            self.autoCompFromDocAPI.setChecked(True)
