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

from qgis.PyQt.QtCore import QCoreApplication, QSize, QUrl, QObject, Qt, pyqtSignal
from qgis.PyQt.QtWidgets import QDialog, QFileDialog, QMessageBox, QTableWidgetItem, QHBoxLayout
from qgis.PyQt.QtGui import QIcon, QFont, QColor, QFontDatabase, QDesktopServices

from qgis.core import QgsSettings, QgsApplication
from qgis.gui import QgsOptionsPageWidget, QgsOptionsWidgetFactory, QgsCodeEditor

from .console_base import QgsPythonConsoleBase
from .console_compile_apis import PrepareAPIDialog
from .ui_console_settings import Ui_SettingsDialogPythonConsole


class SettingsWatcher(QObject):
    settingsChanged = pyqtSignal()


settingsWatcher = SettingsWatcher()


class ConsoleOptionsFactory(QgsOptionsWidgetFactory):

    def __init__(self):
        super(QgsOptionsWidgetFactory, self).__init__()

    def icon(self):
        return QgsApplication.getThemeIcon('/console/mIconRunConsole.svg')

    def createWidget(self, parent):
        return ConsoleOptionsPage(parent)


class ConsoleOptionsPage(QgsOptionsPageWidget):

    def __init__(self, parent):
        super(ConsoleOptionsPage, self).__init__(parent)
        self.options_widget = optionsDialog(parent)
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
        settingsWatcher.settingsChanged.emit()
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

        settings.setValue("pythonConsole/accessTokenGithub", self.tokenGhLineEdit.text())

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

        QgsCodeEditor.setColor(QgsCodeEditor.ColorRole.Default, self.defaultFontColor.color())
        QgsCodeEditor.setColor(QgsCodeEditor.ColorRole.Class, self.classFontColor.color())
        QgsCodeEditor.setColor(QgsCodeEditor.ColorRole.Keyword, self.keywordFontColor.color())
        QgsCodeEditor.setColor(QgsCodeEditor.ColorRole.Decoration, self.decorFontColor.color())
        QgsCodeEditor.setColor(QgsCodeEditor.ColorRole.Number, self.numberFontColor.color())
        QgsCodeEditor.setColor(QgsCodeEditor.ColorRole.Method, self.methodFontColor.color())
        QgsCodeEditor.setColor(QgsCodeEditor.ColorRole.Comment, self.commentFontColor.color())
        QgsCodeEditor.setColor(QgsCodeEditor.ColorRole.CommentBlock, self.commentBlockFontColor.color())
        QgsCodeEditor.setColor(QgsCodeEditor.ColorRole.Background, self.paperBackgroundColor.color())
        QgsCodeEditor.setColor(QgsCodeEditor.ColorRole.Cursor, self.cursorColor.color())
        QgsCodeEditor.setColor(QgsCodeEditor.ColorRole.CaretLine, self.caretLineColor.color())
        QgsCodeEditor.setColor(QgsCodeEditor.ColorRole.Error, self.stderrFontColor.color())

        QgsCodeEditor.setColor(QgsCodeEditor.ColorRole.SingleQuote, self.singleQuoteFontColor.color())
        QgsCodeEditor.setColor(QgsCodeEditor.ColorRole.DoubleQuote, self.doubleQuoteFontColor.color())
        QgsCodeEditor.setColor(QgsCodeEditor.ColorRole.TripleSingleQuote, self.tripleSingleQuoteFontColor.color())
        QgsCodeEditor.setColor(QgsCodeEditor.ColorRole.TripleDoubleQuote, self.tripleDoubleQuoteFontColor.color())
        QgsCodeEditor.setColor(QgsCodeEditor.ColorRole.Edge, self.edgeColor.color())
        QgsCodeEditor.setColor(QgsCodeEditor.ColorRole.MarginBackground, self.marginBackgroundColor.color())
        QgsCodeEditor.setColor(QgsCodeEditor.ColorRole.MarginForeground, self.marginForegroundColor.color())
        QgsCodeEditor.setColor(QgsCodeEditor.ColorRole.Fold, self.foldColor.color())
        QgsCodeEditor.setColor(QgsCodeEditor.ColorRole.SelectionBackground, self.selectionBackgroundColor.color())
        QgsCodeEditor.setColor(QgsCodeEditor.ColorRole.SelectionForeground, self.selectionForegroundColor.color())
        QgsCodeEditor.setColor(QgsCodeEditor.ColorRole.MatchedBraceBackground, self.matchedBraceBackgroundColor.color())
        QgsCodeEditor.setColor(QgsCodeEditor.ColorRole.MatchedBraceForeground, self.matchedBraceForegroundColor.color())

    def restoreSettings(self):
        settings = QgsSettings()
        font = QFontDatabase.systemFont(QFontDatabase.FixedFont)
        self.spinBox.setValue(settings.value("pythonConsole/fontsize", font.pointSize(), type=int))
        self.fontComboBox.setCurrentFont(QFont(settings.value("pythonConsole/fontfamilytext",
                                                              font.family())))
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
        self.autoCloseBracket.setChecked(settings.value("pythonConsole/autoCloseBracket", False, type=bool))
        self.autoInsertionImport.setChecked(settings.value("pythonConsole/autoInsertionImport", True, type=bool))

        if settings.value("pythonConsole/autoCompleteSource") == 'fromDoc':
            self.autoCompFromDoc.setChecked(True)
        elif settings.value("pythonConsole/autoCompleteSource") == 'fromAPI':
            self.autoCompFromAPI.setChecked(True)
        elif settings.value("pythonConsole/autoCompleteSource") == 'fromDocAPI':
            self.autoCompFromDocAPI.setChecked(True)

        # Setting font lexer color
        self.defaultFontColor.setColor(QgsCodeEditor.color(QgsCodeEditor.ColorRole.Default))
        self.keywordFontColor.setColor(QgsCodeEditor.color(QgsCodeEditor.ColorRole.Keyword))
        self.classFontColor.setColor(QgsCodeEditor.color(QgsCodeEditor.ColorRole.Class))
        self.methodFontColor.setColor(QgsCodeEditor.color(QgsCodeEditor.ColorRole.Method))
        self.decorFontColor.setColor(QgsCodeEditor.color(QgsCodeEditor.ColorRole.Decoration))
        self.numberFontColor.setColor(QgsCodeEditor.color(QgsCodeEditor.ColorRole.Number))
        self.commentFontColor.setColor(QgsCodeEditor.color(QgsCodeEditor.ColorRole.Comment))
        self.commentBlockFontColor.setColor(QgsCodeEditor.color(QgsCodeEditor.ColorRole.CommentBlock))
        self.paperBackgroundColor.setColor(QgsCodeEditor.color(QgsCodeEditor.ColorRole.Background))
        self.cursorColor.setColor(QgsCodeEditor.color(QgsCodeEditor.ColorRole.Cursor))
        self.caretLineColor.setColor(QgsCodeEditor.color(QgsCodeEditor.ColorRole.CaretLine))
        self.singleQuoteFontColor.setColor(QgsCodeEditor.color(QgsCodeEditor.ColorRole.SingleQuote))
        self.doubleQuoteFontColor.setColor(QgsCodeEditor.color(QgsCodeEditor.ColorRole.DoubleQuote))
        self.tripleSingleQuoteFontColor.setColor(QgsCodeEditor.color(QgsCodeEditor.ColorRole.TripleSingleQuote))
        self.tripleDoubleQuoteFontColor.setColor(QgsCodeEditor.color(QgsCodeEditor.ColorRole.TripleDoubleQuote))
        self.marginBackgroundColor.setColor(QgsCodeEditor.color(QgsCodeEditor.ColorRole.MarginBackground))
        self.marginForegroundColor.setColor(QgsCodeEditor.color(QgsCodeEditor.ColorRole.MarginForeground))
        self.selectionBackgroundColor.setColor(QgsCodeEditor.color(QgsCodeEditor.ColorRole.SelectionBackground))
        self.selectionForegroundColor.setColor(QgsCodeEditor.color(QgsCodeEditor.ColorRole.SelectionForeground))
        self.matchedBraceBackgroundColor.setColor(QgsCodeEditor.color(QgsCodeEditor.ColorRole.MatchedBraceBackground))
        self.matchedBraceForegroundColor.setColor(QgsCodeEditor.color(QgsCodeEditor.ColorRole.MatchedBraceForeground))
        self.stderrFontColor.setColor(QgsCodeEditor.color(QgsCodeEditor.ColorRole.Error))
        self.edgeColor.setColor(QgsCodeEditor.color(QgsCodeEditor.ColorRole.Edge))
        self.foldColor.setColor(QgsCodeEditor.color(QgsCodeEditor.ColorRole.Fold))

    def _resetFontColor(self):
        self.defaultFontColor.setColor(QgsCodeEditor.defaultColor(QgsCodeEditor.ColorRole.Default))
        self.keywordFontColor.setColor(QgsCodeEditor.defaultColor(QgsCodeEditor.ColorRole.Keyword))
        self.classFontColor.setColor(QgsCodeEditor.defaultColor(QgsCodeEditor.ColorRole.Class))
        self.methodFontColor.setColor(QgsCodeEditor.defaultColor(QgsCodeEditor.ColorRole.Method))
        self.decorFontColor.setColor(QgsCodeEditor.defaultColor(QgsCodeEditor.ColorRole.Decoration))
        self.numberFontColor.setColor(QgsCodeEditor.defaultColor(QgsCodeEditor.ColorRole.Number))
        self.commentFontColor.setColor(QgsCodeEditor.defaultColor(QgsCodeEditor.ColorRole.Comment))
        self.commentBlockFontColor.setColor(QgsCodeEditor.defaultColor(QgsCodeEditor.ColorRole.CommentBlock))
        self.paperBackgroundColor.setColor(QgsCodeEditor.defaultColor(QgsCodeEditor.ColorRole.Background))
        self.cursorColor.setColor(QgsCodeEditor.defaultColor(QgsCodeEditor.ColorRole.Cursor))
        self.caretLineColor.setColor(QgsCodeEditor.defaultColor(QgsCodeEditor.ColorRole.CaretLine))
        self.singleQuoteFontColor.setColor(QgsCodeEditor.defaultColor(QgsCodeEditor.ColorRole.SingleQuote))
        self.doubleQuoteFontColor.setColor(QgsCodeEditor.defaultColor(QgsCodeEditor.ColorRole.DoubleQuote))
        self.tripleSingleQuoteFontColor.setColor(QgsCodeEditor.defaultColor(QgsCodeEditor.ColorRole.TripleSingleQuote))
        self.tripleDoubleQuoteFontColor.setColor(QgsCodeEditor.defaultColor(QgsCodeEditor.ColorRole.TripleDoubleQuote))
        self.marginBackgroundColor.setColor(QgsCodeEditor.defaultColor(QgsCodeEditor.ColorRole.MarginBackground))
        self.marginForegroundColor.setColor(QgsCodeEditor.defaultColor(QgsCodeEditor.ColorRole.MarginForeground))
        self.selectionBackgroundColor.setColor(QgsCodeEditor.defaultColor(QgsCodeEditor.ColorRole.SelectionBackground))
        self.selectionForegroundColor.setColor(QgsCodeEditor.defaultColor(QgsCodeEditor.ColorRole.SelectionForeground))
        self.matchedBraceBackgroundColor.setColor(QgsCodeEditor.defaultColor(QgsCodeEditor.ColorRole.MatchedBraceBackground))
        self.matchedBraceForegroundColor.setColor(QgsCodeEditor.defaultColor(QgsCodeEditor.ColorRole.MatchedBraceForeground))
        self.stderrFontColor.setColor(QgsCodeEditor.defaultColor(QgsCodeEditor.ColorRole.Error))
        self.edgeColor.setColor(QgsCodeEditor.defaultColor(QgsCodeEditor.ColorRole.Edge))
        self.foldColor.setColor(QgsCodeEditor.defaultColor(QgsCodeEditor.ColorRole.Fold))

    def reject(self):
        self.restoreSettings()
        QDialog.reject(self)
