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

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from ui_console_settings import Ui_SettingsDialogPythonConsole

class optionsDialog(QDialog, Ui_SettingsDialogPythonConsole):
    def __init__(self, parent):
        QDialog.__init__(self, parent)
        self.setWindowTitle(QCoreApplication.translate("SettingsDialogPythonConsole", "Settings Python Console"))
        self.parent = parent
        self.setupUi(self)

        self.listPath = []

        self.restoreSettings()
        self.initialCheck()
        self.autoCompletionOptions()
        self.fontConfig()

        self.addAPIpath.setIcon(QIcon(":/images/themes/default/symbologyAdd.png"))
        self.addAPIpath.setToolTip(QCoreApplication.translate("PythonConsole", "Add API path"))
        self.removeAPIpath.setIcon(QIcon(":/images/themes/default/symbologyRemove.png"))
        self.removeAPIpath.setToolTip(QCoreApplication.translate("PythonConsole", "Remove API path"))

        self.connect( self.preloadAPI,
                      SIGNAL("stateChanged(int)"), self.initialCheck)
        self.connect( self.autoCompleteEnabled,
                      SIGNAL("stateChanged(int)"), self.autoCompletionOptions)
        self.connect( self.autoCompleteEnabledEditor,
                      SIGNAL("stateChanged(int)"), self.autoCompletionOptions)
        self.connect(self.addAPIpath,
                     SIGNAL("clicked()"), self.loadAPIFile)
        self.connect(self.removeAPIpath,
                     SIGNAL("clicked()"), self.removeAPI)

    def initialCheck(self):
        if self.preloadAPI.isChecked():
            self.enableDisable(False)
        else:
            self.enableDisable(True)

    def enableDisable(self, value):
        self.tableWidget.setEnabled(value)
        self.addAPIpath.setEnabled(value)
        self.removeAPIpath.setEnabled(value)

    def autoCompletionOptions(self):
        if self.autoCompleteEnabled.isChecked():
            self.enableDisableAutoCompleteOptions(True)
        else:
            self.enableDisableAutoCompleteOptions(False)
        if self.autoCompleteEnabledEditor.isChecked():
            self.enableDisableAutoCompleteOptions(True, editor='editor')
        else:
            self.enableDisableAutoCompleteOptions(False, editor='editor')

    def enableDisableAutoCompleteOptions(self, value, editor=None):
        if editor:
            self.autoCompFromAPIEditor.setEnabled(value)
            self.autoCompFromDocAPIEditor.setEnabled(value)
            self.autoCompFromDocEditor.setEnabled(value)
            self.autoCompThresholdEditor.setEnabled(value)
        else:
            self.autoCompFromAPI.setEnabled(value)
            self.autoCompFromDocAPI.setEnabled(value)
            self.autoCompFromDoc.setEnabled(value)
            self.autoCompThreshold.setEnabled(value)

    def loadAPIFile(self):
        settings = QSettings()
        lastDirPath = settings.value("pythonConsole/lastDirAPIPath").toString()
        fileAPI = QFileDialog.getOpenFileName(
                        self, "Open API File", lastDirPath, "API file (*.api)")
        if fileAPI:
            self.addAPI(fileAPI)

            lastDirPath = QFileInfo(fileAPI).path()
            settings.setValue("pythonConsole/lastDirAPIPath", QVariant(fileAPI))

    def accept(self):
        if not self.preloadAPI.isChecked():
            if self.tableWidget.rowCount() == 0:
                QMessageBox.information(self, self.tr("Warning!"),
                                              self.tr('Please specify API file or check "Use preloaded API files"'))
                return
        self.saveSettings()
        self.listPath = []
        QDialog.accept( self )

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
        self.tableWidget.setHorizontalHeaderLabels([self.tr("API"), self.tr("PATH")])
        self.tableWidget.horizontalHeader().setResizeMode(0, QHeaderView.ResizeToContents)
        self.tableWidget.horizontalHeader().show()
        self.tableWidget.horizontalHeader().setResizeMode(1, QHeaderView.Stretch)

    def removeAPI(self):
        listItemSel = self.tableWidget.selectionModel().selectedRows()
        for index in reversed(listItemSel):
            self.tableWidget.removeRow(index.row())

    def fontConfig(self):
        #fontFamily = ['Courier','Monospace','Aurulent Sans','Bitstream Vera Serif']
        #for i in range(0, len(fontFamily)):
            #self.comboBox.addItem(fontFamily[i])
        settings = QSettings()
        self.fontComboBox.setCurrentIndex(settings.value("pythonConsole/fontfamilyindex").toInt()[0])
        self.fontComboBoxEditor.setCurrentIndex(settings.value("pythonConsole/fontfamilyindexEditor").toInt()[0])

    def saveSettings(self):
        settings = QSettings()
        settings.setValue("pythonConsole/preloadAPI", QVariant(self.preloadAPI.isChecked()))
        settings.setValue("pythonConsole/autoSaveScript", QVariant(self.autoSaveScript.isChecked()))
        fontFamilyIndex = self.fontComboBox.currentIndex()
        settings.setValue("pythonConsole/fontfamilyindex", QVariant(fontFamilyIndex))
        fontFamilyText = self.fontComboBox.currentText()
        settings.setValue("pythonConsole/fontfamilytext", QVariant(fontFamilyText))

        fontFamilyIndexEditor = self.fontComboBoxEditor.currentIndex()
        settings.setValue("pythonConsole/fontfamilyindexEditor", QVariant(fontFamilyIndexEditor))
        fontFamilyTextEditor = self.fontComboBoxEditor.currentText()
        settings.setValue("pythonConsole/fontfamilytextEditor", QVariant(fontFamilyTextEditor))

        fontSize = self.spinBox.value()
        fontSizeEditor = self.spinBoxEditor.value()

        for i in range(0, self.tableWidget.rowCount()):
            text = self.tableWidget.item(i, 1).text()
            self.listPath.append(text)
        settings.setValue("pythonConsole/fontsize", QVariant(fontSize))
        settings.setValue("pythonConsole/fontsizeEditor", QVariant(fontSizeEditor))
        settings.setValue("pythonConsole/userAPI", QVariant(self.listPath))

        settings.setValue("pythonConsole/autoCompThreshold", QVariant(self.autoCompThreshold.value()))
        settings.setValue("pythonConsole/autoCompThresholdEditor", QVariant(self.autoCompThresholdEditor.value()))

        if self.autoCompFromAPIEditor.isChecked():
            settings.setValue("pythonConsole/autoCompleteSourceEditor", QVariant('fromAPI'))
        elif self.autoCompFromDocEditor.isChecked():
            settings.setValue("pythonConsole/autoCompleteSourceEditor", QVariant('fromDoc'))
        elif self.autoCompFromDocAPIEditor.isChecked():
            settings.setValue("pythonConsole/autoCompleteSourceEditor", QVariant('fromDocAPI'))

        if self.autoCompFromAPI.isChecked():
            settings.setValue("pythonConsole/autoCompleteSource", QVariant('fromAPI'))
        elif self.autoCompFromDoc.isChecked():
            settings.setValue("pythonConsole/autoCompleteSource", QVariant('fromDoc'))
        elif self.autoCompFromDocAPI.isChecked():
            settings.setValue("pythonConsole/autoCompleteSource", QVariant('fromDocAPI'))

        settings.setValue("pythonConsole/autoCompleteEnabledEditor", QVariant(self.autoCompleteEnabledEditor.isChecked()))
        settings.setValue("pythonConsole/autoCompleteEnabled", QVariant(self.autoCompleteEnabled.isChecked()))

    def restoreSettings(self):
        settings = QSettings()
        self.spinBox.setValue(settings.value("pythonConsole/fontsize", 10).toInt()[0])
        self.spinBoxEditor.setValue(settings.value("pythonConsole/fontsizeEditor", 10).toInt()[0])
        self.preloadAPI.setChecked(settings.value("pythonConsole/preloadAPI").toBool())
        itemTable = settings.value("pythonConsole/userAPI").toStringList()
        for i in range(len(itemTable)):
            self.tableWidget.insertRow(i)
            self.tableWidget.setColumnCount(2)
            pathSplit = itemTable[i].split("/")
            apiName = pathSplit[-1][0:-4]
            self.tableWidget.setItem(i, 0, QTableWidgetItem(apiName))
            self.tableWidget.setItem(i, 1, QTableWidgetItem(itemTable[i]))
            self.tableWidget.setHorizontalHeaderLabels([self.tr("API"), self.tr("PATH")])
            self.tableWidget.horizontalHeader().setResizeMode(0, QHeaderView.ResizeToContents)
            self.tableWidget.horizontalHeader().show()
            self.tableWidget.horizontalHeader().setResizeMode(1, QHeaderView.Stretch)
        self.autoSaveScript.setChecked(settings.value("pythonConsole/autoSaveScript", False).toBool())

        self.autoCompThreshold.setValue(settings.value("pythonConsole/autoCompThreshold", 2).toInt()[0])
        self.autoCompThresholdEditor.setValue(settings.value("pythonConsole/autoCompThresholdEditor", 2).toInt()[0])

        self.autoCompleteEnabledEditor.setChecked(settings.value("pythonConsole/autoCompleteEnabledEditor", True).toBool())
        self.autoCompleteEnabled.setChecked(settings.value("pythonConsole/autoCompleteEnabled", True).toBool())

        if settings.value("pythonConsole/autoCompleteSource") == 'fromDoc':
            self.autoCompFromDoc.setChecked(True)
        elif settings.value("pythonConsole/autoCompleteSource") == 'fromAPI':
            self.autoCompFromAPI.setChecked(True)
        elif settings.value("pythonConsole/autoCompleteSource") == 'fromDocAPI':
            self.autoCompFromDocAPI.setChecked(True)

        if settings.value("pythonConsole/autoCompleteSourceEditor") == 'fromDoc':
            self.autoCompFromDocEditor.setChecked(True)
        elif settings.value("pythonConsole/autoCompleteSourceEditor") == 'fromAPI':
            self.autoCompFromAPIEditor.setChecked(True)
        elif settings.value("pythonConsole/autoCompleteSourceEditor") == 'fromDocAPI':
            self.autoCompFromDocAPIEditor.setChecked(True)

    def reject(self):
        QDialog.reject(self)
