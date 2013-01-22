# -*- coding:utf-8 -*-
"""
/***************************************************************************
Python Conosle for QGIS
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
        #self.iface = iface
        self.parent = parent
        self.setupUi(self)
        #self.show()

        self.listPath = []

        self.restoreSettings()
        self.initialCheck()
        self.fontConfig()

        self.lineEdit.setReadOnly(True)

        self.addAPIpath.setIcon(QIcon(":/images/themes/default/symbologyAdd.png"))
        self.addAPIpath.setToolTip(QCoreApplication.translate("PythonConsole", "Add API path"))
        self.removeAPIpath.setIcon(QIcon(":/images/themes/default/symbologyRemove.png"))
        self.removeAPIpath.setToolTip(QCoreApplication.translate("PythonConsole", "Remove API path"))

        self.connect( self.preloadAPI,
                      SIGNAL("stateChanged(int)"), self.initialCheck)
        self.connect(self.browseButton,
                     SIGNAL("clicked()"), self.loadAPIFile)
        self.connect(self.addAPIpath,
                     SIGNAL("clicked()"), self.addAPI)
        self.connect(self.removeAPIpath,
                     SIGNAL("clicked()"), self.removeAPI)

    def initialCheck(self):
        if self.preloadAPI.isChecked():
            self.enableDisable(False)
        else:
            self.enableDisable(True)

    def enableDisable(self, value):
        self.tableWidget.setEnabled(value)
        self.lineEdit.setEnabled(value)
        self.browseButton.setEnabled(value)
        self.addAPIpath.setEnabled(value)
        self.removeAPIpath.setEnabled(value)

    def loadAPIFile(self):
        settings = QSettings()
        lastDirPath = settings.value("pythonConsole/lastDirAPIPath").toString()
        fileAPI = QFileDialog.getOpenFileName(
                        self, "Open API File", lastDirPath, "API file (*.api)")
        self.lineEdit.setText(fileAPI)

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

    def addAPI(self):
        if self.lineEdit.text() == "":
            return
        path = self.lineEdit.text()
        count = self.tableWidget.rowCount()
        self.tableWidget.setColumnCount(2)
        self.tableWidget.insertRow(count)
        pathItem = QTableWidgetItem(path)
        pathSplit = path.split("/")
        apiName = pathSplit[-1][0:-4]
        apiNameItem = QTableWidgetItem(apiName)
        self.tableWidget.setItem(count, 0, apiNameItem)
        self.tableWidget.setItem(count, 1, pathItem)
        self.tableWidget.setHorizontalHeaderLabels([self.tr("API"), self.tr("PATH")])
        self.tableWidget.horizontalHeader().setResizeMode(0, QHeaderView.ResizeToContents)
        self.tableWidget.horizontalHeader().show()
        self.tableWidget.horizontalHeader().setResizeMode(1, QHeaderView.Stretch)
        #self.tableWidget.resizeRowsToContents()
        self.lineEdit.clear()

    def removeAPI(self):
        listItemSel = self.tableWidget.selectedIndexes()
        #row = self.tableWidget.currentRow()
        for indx in listItemSel:
            self.tableWidget.removeRow(indx.row())

    def fontConfig(self):
        #fontFamily = ['Courier','Monospace','Aurulent Sans','Bitstream Vera Serif']
        #for i in range(0, len(fontFamily)):
            #self.comboBox.addItem(fontFamily[i])
        settings = QSettings()
        self.fontComboBox.setCurrentIndex(settings.value("pythonConsole/fontfamilyindex").toInt()[0])

    def saveSettings(self):
        settings = QSettings()
        settings.setValue("pythonConsole/preloadAPI", QVariant(self.preloadAPI.isChecked()))
        fontFamilyIndex = self.fontComboBox.currentIndex()
        settings.setValue("pythonConsole/fontfamilyindex", QVariant(fontFamilyIndex))
        fontFamilyText = self.fontComboBox.currentText()
        settings.setValue("pythonConsole/fontfamilytext", QVariant(fontFamilyText))
        fontSize = self.spinBox.value()
        for i in range(0, self.tableWidget.rowCount()):
            text = self.tableWidget.item(i, 1).text()
            self.listPath.append(text)
        settings.setValue("pythonConsole/fontsize", QVariant(fontSize))
        settings.setValue("pythonConsole/userAPI", QVariant(self.listPath))

    def restoreSettings(self):
        settings = QSettings()
        self.spinBox.setValue(settings.value("pythonConsole/fontsize").toInt()[0])
        self.preloadAPI.setChecked(settings.value("pythonConsole/preloadAPI", True).toBool())
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
        #self.comboBox.setCurrentIndex(settings.value("pythonConsole/fontfamilyindex").toInt()[0])

    def reject( self ):
        QDialog.reject( self )
