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

from qgis.PyQt.QtCore import QCoreApplication, QSize, Qt
from qgis.PyQt.QtWidgets import QDialog, QFileDialog, QMessageBox, QTableWidgetItem
from qgis.PyQt.QtGui import QIcon, QFont, QColor, QFontDatabase

from qgis.core import QgsSettings

from .console_base import QgsQsciScintillaBase
from .console_compile_apis import PrepareAPIDialog
from .ui_console_settings import Ui_SettingsDialogPythonConsole


class optionsDialog(QDialog, Ui_SettingsDialogPythonConsole):

    def __init__(self, parent):
        QDialog.__init__(self, parent)
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

        self.resetFontColor.setIcon(QIcon(":/images/themes/default/mActionUndo.svg"))
        self.resetFontColor.setIconSize(QSize(18, 18))
        self.resetFontColor.clicked.connect(self._resetFontColor)

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
        QDialog.accept(self)

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

        fontFamilyText = self.fontComboBox.currentText()
        settings.setValue("pythonConsole/fontfamilytext", fontFamilyText)

        fontSize = self.spinBox.value()

        for i in range(0, self.tableWidget.rowCount()):
            text = self.tableWidget.item(i, 1).text()
            self.listPath.append(text)
        settings.setValue("pythonConsole/fontsize", fontSize)
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
        settings.setValue("pythonConsole/autoInsertionImport", self.autoInsertionImport.isChecked())

        settings.setValue("pythonConsole/defaultFontColor", self.defaultFontColor.color())
        settings.setValue("pythonConsole/classFontColor", self.classFontColor.color())
        settings.setValue("pythonConsole/keywordFontColor", self.keywordFontColor.color())
        settings.setValue("pythonConsole/decorFontColor", self.decorFontColor.color())
        settings.setValue("pythonConsole/numberFontColor", self.numberFontColor.color())
        settings.setValue("pythonConsole/methodFontColor", self.methodFontColor.color())
        settings.setValue("pythonConsole/commentFontColor", self.commentFontColor.color())
        settings.setValue("pythonConsole/commentBlockFontColor", self.commentBlockFontColor.color())
        settings.setValue("pythonConsole/paperBackgroundColor", self.paperBackgroundColor.color())
        settings.setValue("pythonConsole/cursorColor", self.cursorColor.color())
        settings.setValue("pythonConsole/caretLineColor", self.caretLineColor.color())
        settings.setValue("pythonConsole/stderrFontColor", self.stderrFontColor.color())

        settings.setValue("pythonConsole/singleQuoteFontColor", self.singleQuoteFontColor.color())
        settings.setValue("pythonConsole/doubleQuoteFontColor", self.doubleQuoteFontColor.color())
        settings.setValue("pythonConsole/tripleSingleQuoteFontColor", self.tripleSingleQuoteFontColor.color())
        settings.setValue("pythonConsole/tripleDoubleQuoteFontColor", self.tripleDoubleQuoteFontColor.color())
        settings.setValue("pythonConsole/edgeColor", self.edgeColor.color())
        settings.setValue("pythonConsole/marginBackgroundColor", self.marginBackgroundColor.color())
        settings.setValue("pythonConsole/marginForegroundColor", self.marginForegroundColor.color())
        settings.setValue("pythonConsole/foldColor", self.foldColor.color())
        settings.setValue("pythonConsole/selectionBackgroundColor", self.selectionBackgroundColor.color())
        settings.setValue("pythonConsole/selectionForegroundColor", self.selectionForegroundColor.color())
        settings.setValue("pythonConsole/matchedBraceBackgroundColor", self.matchedBraceBackgroundColor.color())
        settings.setValue("pythonConsole/matchedBraceForegroundColor", self.matchedBraceForegroundColor.color())

    def restoreSettings(self):
        settings = QgsSettings()
        font = QFontDatabase.systemFont(QFontDatabase.FixedFont)
        self.spinBox.setValue(settings.value("pythonConsole/fontsize", font.pointSize(), type=int))
        self.fontComboBox.setCurrentFont(QFont(settings.value("pythonConsole/fontfamilytext",
                                                              font.family())))
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
        self.groupBoxAutoCompletion.setChecked(settings.value("pythonConsole/autoCompleteEnabled", True, type=bool))

        self.enableObjectInspector.setChecked(settings.value("pythonConsole/enableObjectInsp", False, type=bool))
        self.autoCloseBracket.setChecked(settings.value("pythonConsole/autoCloseBracket", False, type=bool))
        self.autoInsertionImport.setChecked(settings.value("pythonConsole/autoInsertionImport", True, type=bool))

        if settings.value("pythonConsole/autoCompleteSource") == 'fromDoc':
            self.autoCompFromDoc.setChecked(True)
        elif settings.value("pythonConsole/autoCompleteSource") == 'fromAPI':
            self.autoCompFromAPI.setChecked(True)
        elif settings.value("pythonConsole/autoCompleteSource") == 'fromDocAPI':
            self.autoCompFromDocAPI.setChecked(True)

        # Setting font lexer color
        self.defaultFontColor.setColor(QColor(settings.value("pythonConsole/defaultFontColor", QColor(QgsQsciScintillaBase.DEFAULT_COLOR))))
        self.keywordFontColor.setColor(QColor(settings.value("pythonConsole/keywordFontColor", QColor(QgsQsciScintillaBase.KEYWORD_COLOR))))
        self.classFontColor.setColor(QColor(settings.value("pythonConsole/classFontColor", QColor(QgsQsciScintillaBase.CLASS_COLOR))))
        self.methodFontColor.setColor(QColor(settings.value("pythonConsole/methodFontColor", QColor(QgsQsciScintillaBase.METHOD_COLOR))))
        self.decorFontColor.setColor(QColor(settings.value("pythonConsole/decorFontColor", QColor(QgsQsciScintillaBase.DECORATION_COLOR))))
        self.numberFontColor.setColor(QColor(settings.value("pythonConsole/numberFontColor", QColor(QgsQsciScintillaBase.NUMBER_COLOR))))
        self.commentFontColor.setColor(QColor(settings.value("pythonConsole/commentFontColor", QColor(QgsQsciScintillaBase.COMMENT_COLOR))))
        self.commentBlockFontColor.setColor(QColor(settings.value("pythonConsole/commentBlockFontColor", QColor(QgsQsciScintillaBase.COMMENT_BLOCK_COLOR))))
        self.paperBackgroundColor.setColor(QColor(settings.value("pythonConsole/paperBackgroundColor", QColor(QgsQsciScintillaBase.BACKGROUND_COLOR))))
        self.caretLineColor.setColor(QColor(settings.value("pythonConsole/caretLineColor", QColor(QgsQsciScintillaBase.CARET_LINE_COLOR))))
        self.cursorColor.setColor(QColor(settings.value("pythonConsole/cursorColor", QColor(QgsQsciScintillaBase.CURSOR_COLOR))))
        self.singleQuoteFontColor.setColor(settings.value("pythonConsole/singleQuoteFontColor", QColor(QgsQsciScintillaBase.SINGLE_QUOTE_COLOR)))
        self.doubleQuoteFontColor.setColor(settings.value("pythonConsole/doubleQuoteFontColor", QColor(QgsQsciScintillaBase.DOUBLE_QUOTE_COLOR)))
        self.tripleSingleQuoteFontColor.setColor(settings.value("pythonConsole/tripleSingleQuoteFontColor", QColor(QgsQsciScintillaBase.TRIPLE_SINGLE_QUOTE_COLOR)))
        self.tripleDoubleQuoteFontColor.setColor(settings.value("pythonConsole/tripleDoubleQuoteFontColor", QColor(QgsQsciScintillaBase.TRIPLE_DOUBLE_QUOTE_COLOR)))
        self.marginBackgroundColor.setColor(settings.value("pythonConsole/marginBackgroundColor", QColor(QgsQsciScintillaBase.MARGIN_BACKGROUND_COLOR)))
        self.marginForegroundColor.setColor(settings.value("pythonConsole/marginForegroundColor", QColor(QgsQsciScintillaBase.MARGIN_FOREGROUND_COLOR)))
        self.selectionForegroundColor.setColor(settings.value("pythonConsole/selectionForegroundColor", QColor(QgsQsciScintillaBase.SELECTION_FOREGROUND_COLOR)))
        self.selectionBackgroundColor.setColor(settings.value("pythonConsole/selectionBackgroundColor", QColor(QgsQsciScintillaBase.SELECTION_BACKGROUND_COLOR)))
        self.matchedBraceForegroundColor.setColor(settings.value("pythonConsole/matchedBraceForegroundColor", QColor(QgsQsciScintillaBase.MATCHED_BRACE_FOREGROUND_COLOR)))
        self.matchedBraceBackgroundColor.setColor(settings.value("pythonConsole/matchedBraceBackgroundColor", QColor(QgsQsciScintillaBase.MATCHED_BRACE_BACKGROUND_COLOR)))
        self.stderrFontColor.setColor(QColor(settings.value("pythonConsole/stderrFontColor", QColor(QgsQsciScintillaBase.ERROR_COLOR))))
        self.edgeColor.setColor(settings.value("pythonConsole/edgeColor", QColor(QgsQsciScintillaBase.EDGE_COLOR)))
        self.foldColor.setColor(settings.value("pythonConsole/foldColor", QColor(QgsQsciScintillaBase.FOLD_COLOR)))

    def _resetFontColor(self):
        self.defaultFontColor.setColor(QColor(QgsQsciScintillaBase.DEFAULT_COLOR))
        self.keywordFontColor.setColor(QColor(QgsQsciScintillaBase.KEYWORD_COLOR))
        self.classFontColor.setColor(QColor(QgsQsciScintillaBase.CLASS_COLOR))
        self.methodFontColor.setColor(QColor(QgsQsciScintillaBase.METHOD_COLOR))
        self.decorFontColor.setColor(QColor(QgsQsciScintillaBase.DECORATION_COLOR))
        self.numberFontColor.setColor(QColor(QgsQsciScintillaBase.NUMBER_COLOR))
        self.commentFontColor.setColor(QColor(QgsQsciScintillaBase.COMMENT_COLOR))
        self.commentBlockFontColor.setColor(QColor(QgsQsciScintillaBase.COMMENT_BLOCK_COLOR))
        self.paperBackgroundColor.setColor(QColor(QgsQsciScintillaBase.BACKGROUND_COLOR))
        self.cursorColor.setColor(QColor(QgsQsciScintillaBase.CURSOR_COLOR))
        self.caretLineColor.setColor(QColor(QgsQsciScintillaBase.CARET_LINE_COLOR))
        self.singleQuoteFontColor.setColor(QColor(QgsQsciScintillaBase.SINGLE_QUOTE_COLOR))
        self.doubleQuoteFontColor.setColor(QColor(QgsQsciScintillaBase.DOUBLE_QUOTE_COLOR))
        self.tripleSingleQuoteFontColor.setColor(QColor(QgsQsciScintillaBase.TRIPLE_SINGLE_QUOTE_COLOR))
        self.tripleDoubleQuoteFontColor.setColor(QColor(QgsQsciScintillaBase.TRIPLE_DOUBLE_QUOTE_COLOR))
        self.marginBackgroundColor.setColor(QColor(QgsQsciScintillaBase.MARGIN_BACKGROUND_COLOR))
        self.marginForegroundColor.setColor(QColor(QgsQsciScintillaBase.MARGIN_FOREGROUND_COLOR))
        self.selectionBackgroundColor.setColor(QColor(QgsQsciScintillaBase.SELECTION_BACKGROUND_COLOR))
        self.selectionForegroundColor.setColor(QColor(QgsQsciScintillaBase.SELECTION_FOREGROUND_COLOR))
        self.matchedBraceBackgroundColor.setColor(QColor(QgsQsciScintillaBase.MATCHED_BRACE_BACKGROUND_COLOR))
        self.matchedBraceForegroundColor.setColor(QColor(QgsQsciScintillaBase.MATCHED_BRACE_FOREGROUND_COLOR))
        self.stderrFontColor.setColor(QColor(QgsQsciScintillaBase.ERROR_COLOR))
        self.edgeColor.setColor(QColor(QgsQsciScintillaBase.EDGE_COLOR))
        self.foldColor.setColor(QColor(QgsQsciScintillaBase.FOLD_COLOR))

    def reject(self):
        self.restoreSettings()
        QDialog.reject(self)
