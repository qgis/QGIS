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

from PyQt4.QtCore import QCoreApplication, SIGNAL, QSize, QSettings, QFileInfo, Qt
from PyQt4.QtGui import QDialog, QIcon, QFileDialog, QMessageBox, QTableWidgetItem, QFont, QColor
from console_compile_apis import PrepareAPIDialog

from ui_console_settings import Ui_SettingsDialogPythonConsole

class optionsDialog(QDialog, Ui_SettingsDialogPythonConsole):
    def __init__(self, parent):
        QDialog.__init__(self, parent)
        self.setWindowTitle(QCoreApplication.translate("SettingsDialogPythonConsole", "Settings Python Console"))
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

        self.connect( self.preloadAPI,
                      SIGNAL("stateChanged(int)"), self.initialCheck)
        self.connect(self.addAPIpath,
                     SIGNAL("clicked()"), self.loadAPIFile)
        self.connect(self.removeAPIpath,
                     SIGNAL("clicked()"), self.removeAPI)
        self.compileAPIs.clicked.connect(self._prepareAPI)

        self.resetFontColor.setIcon(QIcon(":/images/themes/default/console/iconResetColorConsole.png"))
        self.resetFontColor.setIconSize(QSize(18, 18))
        self.resetFontColorEditor.setIcon(QIcon(":/images/themes/default/console/iconResetColorConsole.png"))
        self.resetFontColorEditor.setIconSize(QSize(18, 18))
        self.resetFontColor.clicked.connect(self._resetFontColor)
        self.resetFontColorEditor.clicked.connect(self._resetFontColorEditor)

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
        settings = QSettings()
        lastDirPath = settings.value("pythonConsole/lastDirAPIPath", "", type=str)
        fileAPI = QFileDialog.getOpenFileName(
            self, "Open API File", lastDirPath, "API file (*.api)")
        if fileAPI:
            self.addAPI(fileAPI)

            lastDirPath = QFileInfo(fileAPI).path()
            settings.setValue("pythonConsole/lastDirAPIPath", fileAPI)

    def _prepareAPI(self):
        if self.tableWidget.rowCount() != 0:
            pap_file = QFileDialog().getSaveFileName(self,
                                                    "",
                                                    '*.pap',
                                                    "Prepared APIs file (*.pap)")
        else:
            QMessageBox.information(self, self.tr("Warning!"),
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
                QMessageBox.information(self, self.tr("Warning!"),
                                        self.tr('Please specify API file or check "Use preloaded API files"'))
                return
        if self.groupBoxPreparedAPI.isChecked() and \
           not self.lineEdit.text():
            QMessageBox.information(self, self.tr("Warning!"),
                                    self.tr('The APIs file was not compiled, click on "Compile APIs..."'))
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

    def removeAPI(self):
        listItemSel = self.tableWidget.selectionModel().selectedRows()
        for index in reversed(listItemSel):
            self.tableWidget.removeRow(index.row())

    def saveSettings(self):
        settings = QSettings()
        settings.setValue("pythonConsole/preloadAPI", self.preloadAPI.isChecked())
        settings.setValue("pythonConsole/autoSaveScript", self.autoSaveScript.isChecked())

        fontFamilyText = self.fontComboBox.currentText()
        settings.setValue("pythonConsole/fontfamilytext", fontFamilyText)
        fontFamilyTextEditor = self.fontComboBoxEditor.currentText()
        settings.setValue("pythonConsole/fontfamilytextEditor", fontFamilyTextEditor)

        fontSize = self.spinBox.value()
        fontSizeEditor = self.spinBoxEditor.value()

        for i in range(0, self.tableWidget.rowCount()):
            text = self.tableWidget.item(i, 1).text()
            self.listPath.append(text)
        settings.setValue("pythonConsole/fontsize", fontSize)
        settings.setValue("pythonConsole/fontsizeEditor", fontSizeEditor)
        settings.setValue("pythonConsole/userAPI", self.listPath)

        settings.setValue("pythonConsole/autoCompThreshold", self.autoCompThreshold.value())
        settings.setValue("pythonConsole/autoCompThresholdEditor", self.autoCompThresholdEditor.value())

        settings.setValue("pythonConsole/autoCompleteEnabledEditor", self.groupBoxAutoCompletionEditor.isChecked())
        settings.setValue("pythonConsole/autoCompleteEnabled", self.groupBoxAutoCompletion.isChecked())

        settings.setValue("pythonConsole/usePreparedAPIFile", self.groupBoxPreparedAPI.isChecked())
        settings.setValue("pythonConsole/preparedAPIFile", self.lineEdit.text())

        if self.autoCompFromAPIEditor.isChecked():
            settings.setValue("pythonConsole/autoCompleteSourceEditor", 'fromAPI')
        elif self.autoCompFromDocEditor.isChecked():
            settings.setValue("pythonConsole/autoCompleteSourceEditor", 'fromDoc')
        elif self.autoCompFromDocAPIEditor.isChecked():
            settings.setValue("pythonConsole/autoCompleteSourceEditor", 'fromDocAPI')

        if self.autoCompFromAPI.isChecked():
            settings.setValue("pythonConsole/autoCompleteSource", 'fromAPI')
        elif self.autoCompFromDoc.isChecked():
            settings.setValue("pythonConsole/autoCompleteSource", 'fromDoc')
        elif self.autoCompFromDocAPI.isChecked():
            settings.setValue("pythonConsole/autoCompleteSource", 'fromDocAPI')

        settings.setValue("pythonConsole/enableObjectInsp", self.enableObjectInspector.isChecked())
        settings.setValue("pythonConsole/autoCloseBracket", self.autoCloseBracket.isChecked())
        settings.setValue("pythonConsole/autoCloseBracketEditor", self.autoCloseBracketEditor.isChecked())
        settings.setValue("pythonConsole/autoInsertionImport", self.autoInsertionImport.isChecked())
        settings.setValue("pythonConsole/autoInsertionImportEditor", self.autoInsertionImportEditor.isChecked())

        settings.setValue("pythonConsole/defaultFontColor", self.defaultFontColor.color())
        settings.setValue("pythonConsole/defaultFontColorEditor", self.defaultFontColorEditor.color())
        settings.setValue("pythonConsole/classFontColor", self.classFontColor.color())
        settings.setValue("pythonConsole/classFontColorEditor", self.classFontColorEditor.color())
        settings.setValue("pythonConsole/keywordFontColor", self.keywordFontColor.color())
        settings.setValue("pythonConsole/keywordFontColorEditor", self.keywordFontColorEditor.color())
        settings.setValue("pythonConsole/decorFontColor", self.decorFontColor.color())
        settings.setValue("pythonConsole/decorFontColorEditor", self.decorFontColorEditor.color())
        settings.setValue("pythonConsole/methodFontColor", self.methodFontColor.color())
        settings.setValue("pythonConsole/methodFontColorEditor", self.methodFontColorEditor.color())
        settings.setValue("pythonConsole/commentFontColor", self.commentFontColor.color())
        settings.setValue("pythonConsole/commentFontColorEditor", self.commentFontColorEditor.color())
        settings.setValue("pythonConsole/commentBlockFontColor", self.commentBlockFontColor.color())
        settings.setValue("pythonConsole/commentBlockFontColorEditor", self.commentBlockFontColorEditor.color())
        settings.setValue("pythonConsole/paperBackgroundColor", self.paperBackgroundColor.color())
        settings.setValue("pythonConsole/paperBackgroundColorEditor", self.paperBackgroundColorEditor.color())
        settings.setValue("pythonConsole/cursorColor", self.cursorColor.color())
        settings.setValue("pythonConsole/cursorColorEditor", self.cursorColorEditor.color())
        settings.setValue("pythonConsole/caretLineColor", self.caretLineColor.color())
        settings.setValue("pythonConsole/caretLineColorEditor", self.caretLineColorEditor.color())
        settings.setValue("pythonConsole/stderrFontColor", self.stderrFontColor.color())

        settings.setValue("pythonConsole/singleQuoteFontColor", self.singleQuoteFontColor.color())
        settings.setValue("pythonConsole/singleQuoteFontColorEditor", self.singleQuoteFontColorEditor.color())
        settings.setValue("pythonConsole/doubleQuoteFontColor", self.doubleQuoteFontColor.color())
        settings.setValue("pythonConsole/doubleQuoteFontColorEditor", self.doubleQuoteFontColorEditor.color())
        settings.setValue("pythonConsole/tripleSingleQuoteFontColor", self.tripleSingleQuoteFontColor.color())
        settings.setValue("pythonConsole/tripleSingleQuoteFontColorEditor", self.tripleSingleQuoteFontColorEditor.color())
        settings.setValue("pythonConsole/tripleDoubleQuoteFontColor", self.tripleDoubleQuoteFontColor.color())
        settings.setValue("pythonConsole/tripleDoubleQuoteFontColorEditor", self.tripleDoubleQuoteFontColorEditor.color())

    def restoreSettings(self):
        settings = QSettings()
        self.spinBox.setValue(settings.value("pythonConsole/fontsize", 10, type=int))
        self.spinBoxEditor.setValue(settings.value("pythonConsole/fontsizeEditor", 10, type=int))
        self.fontComboBox.setCurrentFont(QFont(settings.value("pythonConsole/fontfamilytext",
                                                              "Monospace")))
        self.fontComboBoxEditor.setCurrentFont(QFont(settings.value("pythonConsole/fontfamilytextEditor",
                                                                    "Monospace")))
        self.preloadAPI.setChecked(settings.value("pythonConsole/preloadAPI", True, type=bool))
        self.lineEdit.setText(settings.value("pythonConsole/preparedAPIFile", "", type=str))
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
        self.autoCompThresholdEditor.setValue(settings.value("pythonConsole/autoCompThresholdEditor", 2, type=int))
        self.groupBoxAutoCompletionEditor.setChecked(settings.value("pythonConsole/autoCompleteEnabledEditor", True, type=bool))
        self.groupBoxAutoCompletion.setChecked(settings.value("pythonConsole/autoCompleteEnabled", True, type=bool))

        self.enableObjectInspector.setChecked(settings.value("pythonConsole/enableObjectInsp", False, type=bool))
        self.autoCloseBracketEditor.setChecked(settings.value("pythonConsole/autoCloseBracketEditor", False, type=bool))
        self.autoCloseBracket.setChecked(settings.value("pythonConsole/autoCloseBracket", False, type=bool))
        self.autoInsertionImportEditor.setChecked(settings.value("pythonConsole/autoInsertionImportEditor", True, type=bool))
        self.autoInsertionImport.setChecked(settings.value("pythonConsole/autoInsertionImport", True, type=bool))

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

        ## Setting font lexer color
        self.defaultFontColor.setColor(QColor(settings.value("pythonConsole/defaultFontColor", QColor(Qt.black))))
        self.defaultFontColorEditor.setColor(QColor(settings.value("pythonConsole/defaultFontColorEditor", QColor(Qt.black))))
        self.keywordFontColor.setColor(QColor(settings.value("pythonConsole/keywordFontColor", QColor(Qt.darkGreen))))
        self.keywordFontColorEditor.setColor(QColor(settings.value("pythonConsole/keywordFontColorEditor", QColor(Qt.darkGreen))))
        self.classFontColor.setColor(QColor(settings.value("pythonConsole/classFontColor", QColor(Qt.blue))))
        self.classFontColorEditor.setColor(QColor(settings.value("pythonConsole/classFontColorEditor", QColor(Qt.blue))))
        self.methodFontColor.setColor(QColor(settings.value("pythonConsole/methodFontColor", QColor(Qt.darkGray))))
        self.methodFontColorEditor.setColor(QColor(settings.value("pythonConsole/methodFontColorEditor", QColor(Qt.darkGray))))
        self.decorFontColor.setColor(QColor(settings.value("pythonConsole/decorFontColor", QColor(Qt.darkBlue))))
        self.decorFontColorEditor.setColor(QColor(settings.value("pythonConsole/decorFontColorEditor", QColor(Qt.darkBlue))))
        self.commentFontColor.setColor(QColor(settings.value("pythonConsole/commentFontColor", QColor(Qt.gray))))
        self.commentFontColorEditor.setColor(QColor(settings.value("pythonConsole/commentFontColorEditor", QColor(Qt.gray))))
        self.commentBlockFontColor.setColor(QColor(settings.value("pythonConsole/commentBlockFontColor", QColor(Qt.gray))))
        self.commentBlockFontColorEditor.setColor(QColor(settings.value("pythonConsole/commentBlockFontColorEditor", QColor(Qt.gray))))
        self.paperBackgroundColor.setColor(QColor(settings.value("pythonConsole/paperBackgroundColor", QColor(Qt.white))))
        self.paperBackgroundColorEditor.setColor(QColor(settings.value("pythonConsole/paperBackgroundColorEditor", QColor(Qt.white))))
        self.caretLineColor.setColor(QColor(settings.value("pythonConsole/caretLineColor", QColor("#fcf3ed"))))
        self.caretLineColorEditor.setColor(QColor(settings.value("pythonConsole/caretLineColorEditor", QColor("#fcf3ed"))))
        self.cursorColor.setColor(QColor(settings.value("pythonConsole/cursorColor", QColor(Qt.black))))
        self.cursorColorEditor.setColor(QColor(settings.value("pythonConsole/cursorColorEditor", QColor(Qt.black))))
        self.stderrFontColor.setColor(QColor(settings.value("pythonConsole/stderrFontColor", QColor(Qt.red))))

        self.singleQuoteFontColor.setColor(settings.value("pythonConsole/singleQuoteFontColor", QColor(Qt.blue)))
        self.singleQuoteFontColorEditor.setColor(settings.value("pythonConsole/singleQuoteFontColorEditor", QColor(Qt.blue)))
        self.doubleQuoteFontColor.setColor(settings.value("pythonConsole/doubleQuoteFontColor", QColor(Qt.blue)))
        self.doubleQuoteFontColorEditor.setColor(settings.value("pythonConsole/doubleQuoteFontColorEditor", QColor(Qt.blue)))
        self.tripleSingleQuoteFontColor.setColor(settings.value("pythonConsole/tripleSingleQuoteFontColor", QColor(Qt.blue)))
        self.tripleSingleQuoteFontColorEditor.setColor(settings.value("pythonConsole/tripleSingleQuoteFontColorEditor", QColor(Qt.blue)))
        self.tripleDoubleQuoteFontColor.setColor(settings.value("pythonConsole/tripleDoubleQuoteFontColor", QColor(Qt.blue)))
        self.tripleDoubleQuoteFontColorEditor.setColor(settings.value("pythonConsole/tripleDoubleQuoteFontColorEditor", QColor(Qt.blue)))

    def _resetFontColor(self):
        self.defaultFontColor.setColor(QColor(Qt.black))
        self.keywordFontColor.setColor(QColor(Qt.darkGreen))
        self.classFontColor.setColor(QColor(Qt.blue))
        self.methodFontColor.setColor(QColor(Qt.darkGray))
        self.decorFontColor.setColor(QColor(Qt.darkBlue))
        self.commentFontColor.setColor(QColor(Qt.gray))
        self.commentBlockFontColor.setColor(QColor(Qt.gray))
        self.stderrFontColor.setColor(QColor(Qt.red))
        self.paperBackgroundColor.setColor(QColor(Qt.white))
        self.cursorColor.setColor(QColor(Qt.black))
        self.caretLineColor.setColor(QColor("#fcf3ed"))
        self.singleQuoteFontColor.setColor(QColor(Qt.blue))
        self.doubleQuoteFontColor.setColor(QColor(Qt.blue))
        self.tripleSingleQuoteFontColor.setColor(QColor(Qt.blue))
        self.tripleDoubleQuoteFontColor.setColor(QColor(Qt.blue))


    def _resetFontColorEditor(self):
        self.defaultFontColorEditor.setColor(QColor(Qt.black))
        self.keywordFontColorEditor.setColor(QColor(Qt.darkGreen))
        self.classFontColorEditor.setColor(QColor(Qt.blue))
        self.methodFontColorEditor.setColor(QColor(Qt.darkGray))
        self.decorFontColorEditor.setColor(QColor(Qt.darkBlue))
        self.commentFontColorEditor.setColor(QColor(Qt.gray))
        self.commentBlockFontColorEditor.setColor(QColor(Qt.gray))
        self.paperBackgroundColorEditor.setColor(QColor(Qt.white))
        self.cursorColorEditor.setColor(QColor(Qt.black))
        self.caretLineColorEditor.setColor(QColor("#fcf3ed"))
        self.singleQuoteFontColorEditor.setColor(QColor(Qt.blue))
        self.doubleQuoteFontColorEditor.setColor(QColor(Qt.blue))
        self.tripleSingleQuoteFontColorEditor.setColor(QColor(Qt.blue))
        self.tripleDoubleQuoteFontColorEditor.setColor(QColor(Qt.blue))

    def reject(self):
        self.restoreSettings()
        QDialog.reject(self)
