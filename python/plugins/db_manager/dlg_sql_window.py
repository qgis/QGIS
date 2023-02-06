# -*- coding: utf-8 -*-

"""
/***************************************************************************
Name                 : DB Manager
Description          : Database manager plugin for QGIS
Date                 : May 23, 2011
copyright            : (C) 2011 by Giuseppe Sucameli
email                : brush.tyler@gmail.com

The content of this file is based on
- PG_Manager by Martin Dobias (GPLv2 license)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
"""
from builtins import zip
from builtins import next
from builtins import str
from hashlib import md5

import os

from qgis.PyQt.QtCore import Qt, pyqtSignal, QDir, QCoreApplication
from qgis.PyQt.QtWidgets import (QDialog,
                                 QWidget,
                                 QAction,
                                 QApplication,
                                 QInputDialog,
                                 QStyledItemDelegate,
                                 QTableWidgetItem,
                                 QFileDialog,
                                 QMessageBox
                                 )
from qgis.PyQt.QtGui import (QKeySequence,
                             QCursor,
                             QClipboard,
                             QIcon,
                             QStandardItemModel,
                             QStandardItem
                             )
from qgis.PyQt.Qsci import QsciAPIs, QsciScintilla

from qgis.core import (
    QgsProject,
    QgsApplication,
    QgsTask,
    QgsSettings,
    QgsMapLayerType
)
from qgis.utils import OverrideCursor

from .db_plugins.plugin import BaseError
from .db_plugins.postgis.plugin import PGDatabase
from .dlg_db_error import DlgDbError
from .dlg_query_builder import QueryBuilderDlg

try:
    from qgis.gui import QgsCodeEditorSQL  # NOQA
except:
    from .sqledit import SqlEdit
    from qgis import gui

    gui.QgsCodeEditorSQL = SqlEdit

from .ui.ui_DlgSqlWindow import Ui_DbManagerDlgSqlWindow as Ui_Dialog

import re


def check_comments_in_sql(raw_sql_input):
    lines = []
    for line in raw_sql_input.splitlines():
        if not line.strip().startswith('--'):
            if '--' in line:
                comments = re.finditer(r'--', line)
                comment_positions = [
                    match.start()
                    for match in comments
                ]
                identifiers = re.finditer(r'"(?:[^"]|"")*"', line)
                quotes = re.finditer(r"'(?:[^']|'')*'", line)
                quote_positions = []
                for match in identifiers:
                    quote_positions.append((match.start(), match.end()))
                for match in quotes:
                    quote_positions.append((match.start(), match.end()))
                unquoted_comments = comment_positions.copy()
                for comment in comment_positions:
                    for quote_position in quote_positions:
                        if comment >= quote_position[0] and comment < quote_position[1]:
                            unquoted_comments.remove(comment)
                if len(unquoted_comments) > 0:
                    lines.append(line[:unquoted_comments[0]])
                else:
                    lines.append(line)
            else:
                lines.append(line)
    sql = ' '.join(lines)
    return sql.strip()


class DlgSqlWindow(QWidget, Ui_Dialog):
    nameChanged = pyqtSignal(str)
    QUERY_HISTORY_LIMIT = 20
    hasChanged = False

    def __init__(self, iface, db, parent=None):
        QWidget.__init__(self, parent)
        self.mainWindow = parent
        self.iface = iface
        self.db = db
        self.dbType = db.connection().typeNameString()
        self.connectionName = db.connection().connectionName()
        self.filter = ""
        self.modelAsync = None
        self.allowMultiColumnPk = isinstance(db,
                                             PGDatabase)  # at the moment only PostgreSQL allows a primary key to span multiple columns, SpatiaLite doesn't
        self.aliasSubQuery = isinstance(db, PGDatabase)  # only PostgreSQL requires subqueries to be aliases
        self.setupUi(self)
        self.setWindowTitle(
            self.tr("{0} - {1} [{2}]").format(self.windowTitle(), self.connectionName, self.dbType))

        self.defaultLayerName = self.tr('QueryLayer')

        if self.allowMultiColumnPk:
            self.uniqueColumnCheck.setText(self.tr("Column(s) with unique values"))
        else:
            self.uniqueColumnCheck.setText(self.tr("Column with unique values"))

        self.editSql.setFocus()
        self.editSql.setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)
        self.editSql.setLineNumbersVisible(True)
        self.initCompleter()
        self.editSql.textChanged.connect(lambda: self.setHasChanged(True))

        settings = QgsSettings()
        self.history = settings.value('DB_Manager/queryHistory/' + self.dbType, {self.connectionName: []})
        if self.connectionName not in self.history:
            self.history[self.connectionName] = []

        self.queryHistoryWidget.setVisible(False)
        self.queryHistoryTableWidget.verticalHeader().hide()
        self.queryHistoryTableWidget.doubleClicked.connect(self.insertQueryInEditor)
        self.populateQueryHistory()
        self.btnQueryHistory.toggled.connect(self.showHideQueryHistory)

        self.btnCancel.setEnabled(False)
        self.btnCancel.clicked.connect(self.executeSqlCanceled)
        self.btnCancel.setShortcut(QKeySequence.Cancel)
        self.progressBar.setEnabled(False)
        self.progressBar.setRange(0, 100)
        self.progressBar.setValue(0)
        self.progressBar.setFormat("")
        self.progressBar.setAlignment(Qt.AlignCenter)

        # allow copying results
        copyAction = QAction("copy", self)
        self.viewResult.addAction(copyAction)
        copyAction.setShortcuts(QKeySequence.Copy)

        copyAction.triggered.connect(self.copySelectedResults)

        self.btnExecute.clicked.connect(self.executeSql)
        self.btnSetFilter.clicked.connect(self.setFilter)
        self.btnClear.clicked.connect(self.clearSql)

        self.presetStore.clicked.connect(self.storePreset)
        self.presetSaveAsFile.clicked.connect(self.saveAsFilePreset)
        self.presetLoadFile.clicked.connect(self.loadFilePreset)
        self.presetDelete.clicked.connect(self.deletePreset)
        self.presetCombo.activated[str].connect(self.loadPreset)
        self.presetCombo.activated[str].connect(self.presetName.setText)

        self.updatePresetsCombobox()

        self.geomCombo.setEditable(True)
        self.geomCombo.lineEdit().setReadOnly(True)

        self.uniqueCombo.setEditable(True)
        self.uniqueCombo.lineEdit().setReadOnly(True)
        self.uniqueModel = QStandardItemModel(self.uniqueCombo)
        self.uniqueCombo.setModel(self.uniqueModel)
        if self.allowMultiColumnPk:
            self.uniqueCombo.setItemDelegate(QStyledItemDelegate())
            self.uniqueModel.itemChanged.connect(self.uniqueChanged)  # react to the (un)checking of an item
            self.uniqueCombo.lineEdit().textChanged.connect(
                self.uniqueTextChanged)  # there are other events that change the displayed text and some of them can not be caught directly

        # hide the load query as layer if feature is not supported
        self._loadAsLayerAvailable = self.db.connector.hasCustomQuerySupport()
        self.loadAsLayerGroup.setVisible(self._loadAsLayerAvailable)
        if self._loadAsLayerAvailable:
            self.layerTypeWidget.hide()  # show if load as raster is supported
            self.loadLayerBtn.clicked.connect(self.loadSqlLayer)
            self.getColumnsBtn.clicked.connect(self.fillColumnCombos)
            self.loadAsLayerGroup.toggled.connect(self.loadAsLayerToggled)
            self.loadAsLayerToggled(False)

        self._createViewAvailable = self.db.connector.hasCreateSpatialViewSupport()
        self.btnCreateView.setVisible(self._createViewAvailable)
        if self._createViewAvailable:
            self.btnCreateView.clicked.connect(self.createView)

        self.queryBuilderFirst = True
        self.queryBuilderBtn.setIcon(QIcon(":/db_manager/icons/sql.gif"))
        self.queryBuilderBtn.clicked.connect(self.displayQueryBuilder)

        self.presetName.textChanged.connect(self.nameChanged)

    def insertQueryInEditor(self, item):
        sql = item.data(Qt.DisplayRole)
        self.editSql.insertText(sql)

    def showHideQueryHistory(self, visible):
        self.queryHistoryWidget.setVisible(visible)

    def populateQueryHistory(self):
        self.queryHistoryTableWidget.clearContents()
        self.queryHistoryTableWidget.setRowCount(0)
        dictlist = self.history[self.connectionName]

        if not dictlist:
            return

        for i in range(len(dictlist)):
            self.queryHistoryTableWidget.insertRow(0)
            queryItem = QTableWidgetItem(dictlist[i]['query'])
            rowsItem = QTableWidgetItem(str(dictlist[i]['rows']))
            durationItem = QTableWidgetItem(str(dictlist[i]['secs']))
            self.queryHistoryTableWidget.setItem(0, 0, queryItem)
            self.queryHistoryTableWidget.setItem(0, 1, rowsItem)
            self.queryHistoryTableWidget.setItem(0, 2, durationItem)

        self.queryHistoryTableWidget.resizeColumnsToContents()
        self.queryHistoryTableWidget.resizeRowsToContents()

    def writeQueryHistory(self, sql, affectedRows, secs):
        if len(self.history[self.connectionName]) >= self.QUERY_HISTORY_LIMIT:
            self.history[self.connectionName].pop(0)

        settings = QgsSettings()
        self.history[self.connectionName].append({'query': sql,
                                                  'rows': affectedRows,
                                                  'secs': secs})
        settings.setValue('DB_Manager/queryHistory/' + self.dbType, self.history)

        self.populateQueryHistory()

    def getQueryHash(self, name):
        return 'q%s' % md5(name.encode('utf8')).hexdigest()

    def updatePresetsCombobox(self):
        self.presetCombo.clear()

        names = []
        entries = QgsProject.instance().subkeyList('DBManager', 'savedQueries')
        for entry in entries:
            name = QgsProject.instance().readEntry('DBManager', 'savedQueries/' + entry + '/name')[0]
            names.append(name)

        for name in sorted(names):
            self.presetCombo.addItem(name)
        self.presetCombo.setCurrentIndex(-1)

    def storePreset(self):
        query = self._getSqlQuery()
        if query == "":
            return
        name = str(self.presetName.text())
        QgsProject.instance().writeEntry('DBManager', 'savedQueries/' + self.getQueryHash(name) + '/name', name)
        QgsProject.instance().writeEntry('DBManager', 'savedQueries/' + self.getQueryHash(name) + '/query', query)
        index = self.presetCombo.findText(name)
        if index == -1:
            self.presetCombo.addItem(name)
            self.presetCombo.setCurrentIndex(self.presetCombo.count() - 1)
        else:
            self.presetCombo.setCurrentIndex(index)

    def saveAsFilePreset(self):
        settings = QgsSettings()
        lastDir = settings.value('DB_Manager/lastDirSQLFIle', "")

        query = self.editSql.text()
        if query == "":
            return

        filename, _ = QFileDialog.getSaveFileName(
            self,
            self.tr('Save SQL Query'),
            lastDir,
            self.tr("SQL File (*.sql *.SQL)"))

        if filename:
            if not filename.lower().endswith('.sql'):
                filename += ".sql"

            with open(filename, 'w') as f:
                f.write(query)
                lastDir = os.path.dirname(filename)
                settings.setValue('DB_Manager/lastDirSQLFile', lastDir)

    def loadFilePreset(self):
        settings = QgsSettings()
        lastDir = settings.value('DB_Manager/lastDirSQLFIle', "")

        filename, _ = QFileDialog.getOpenFileName(
            self,
            self.tr("Load SQL Query"),
            lastDir,
            self.tr("SQL File (*.sql *.SQL);;All Files (*)"))

        if filename:
            with open(filename, 'r') as f:
                self.editSql.clear()
                for line in f:
                    self.editSql.insertText(line)
                lastDir = os.path.dirname(filename)
                settings.setValue('DB_Manager/lastDirSQLFile', lastDir)

    def deletePreset(self):
        name = self.presetCombo.currentText()
        QgsProject.instance().removeEntry('DBManager', 'savedQueries/' + self.getQueryHash(name))
        self.presetCombo.removeItem(self.presetCombo.findText(name))
        self.presetCombo.setCurrentIndex(-1)

    def loadPreset(self, name):
        query = QgsProject.instance().readEntry('DBManager', 'savedQueries/' + self.getQueryHash(name) + '/query')[0]
        self.editSql.setText(query)

    def loadAsLayerToggled(self, checked):
        self.loadAsLayerGroup.setChecked(checked)
        self.loadAsLayerWidget.setVisible(checked)
        if checked:
            self.fillColumnCombos()

    def clearSql(self):
        self.editSql.clear()
        self.editSql.setFocus()
        self.filter = ""
        self.setHasChanged(True)

    def updateUiWhileSqlExecution(self, status):
        if status:
            for i in range(0, self.mainWindow.tabs.count()):
                if i != self.mainWindow.tabs.currentIndex():
                    self.mainWindow.tabs.setTabEnabled(i, False)

            self.mainWindow.menuBar.setEnabled(False)
            self.mainWindow.toolBar.setEnabled(False)
            self.mainWindow.tree.setEnabled(False)

            for w in self.findChildren(QWidget):
                w.setEnabled(False)

            self.btnCancel.setEnabled(True)
            self.progressBar.setEnabled(True)
            self.progressBar.setRange(0, 0)
        else:
            for i in range(0, self.mainWindow.tabs.count()):
                if i != self.mainWindow.tabs.currentIndex():
                    self.mainWindow.tabs.setTabEnabled(i, True)

            self.mainWindow.refreshTabs()
            self.mainWindow.menuBar.setEnabled(True)
            self.mainWindow.toolBar.setEnabled(True)
            self.mainWindow.tree.setEnabled(True)

            for w in self.findChildren(QWidget):
                w.setEnabled(True)

            self.btnCancel.setEnabled(False)
            self.progressBar.setRange(0, 100)
            self.progressBar.setEnabled(False)

    def executeSqlCanceled(self):
        self.btnCancel.setEnabled(False)
        self.btnCancel.setText(QCoreApplication.translate("DlgSqlWindow", "Canceling…"))
        self.modelAsync.cancel()

    def executeSqlCompleted(self):
        self.updateUiWhileSqlExecution(False)

        with OverrideCursor(Qt.WaitCursor):
            if self.modelAsync.task.status() == QgsTask.Complete:
                model = self.modelAsync.model
                self.showError(None)
                self.viewResult.setModel(model)
                self.lblResult.setText(self.tr("{0} rows, {1:.3f} seconds").format(model.affectedRows(), model.secs()))
                cols = self.viewResult.model().columnNames()
                quotedCols = [
                    self.db.connector.quoteId(col)
                    for col in cols
                ]

                self.setColumnCombos(cols, quotedCols)

                self.writeQueryHistory(self.modelAsync.task.sql, model.affectedRows(), model.secs())
                self.update()
            elif not self.modelAsync.canceled:
                self.showError(self.modelAsync.error)

                self.uniqueModel.clear()
                self.geomCombo.clear()

            self.btnCancel.setText(self.tr("Cancel"))

    def executeSql(self):
        sql = self._getExecutableSqlQuery()
        if sql == "":
            return

        # delete the old model
        old_model = self.viewResult.model()
        self.viewResult.setModel(None)
        if old_model:
            old_model.deleteLater()

        try:
            self.modelAsync = self.db.sqlResultModelAsync(sql, self)
            self.modelAsync.done.connect(self.executeSqlCompleted)
            self.updateUiWhileSqlExecution(True)
            QgsApplication.taskManager().addTask(self.modelAsync.task)
        except Exception as e:
            self.showError(e)
            self.uniqueModel.clear()
            self.geomCombo.clear()
            return

    def showError(self, error):
        '''Shows the error or hides it if error is None'''
        if error:
            self.viewResult.setVisible(False)
            self.errorText.setVisible(True)
            self.errorText.setText(error.msg)
            self.errorText.setWrapMode(QsciScintilla.WrapWord)
        else:
            self.viewResult.setVisible(True)
            self.errorText.setVisible(False)

    def _getSqlLayer(self, _filter):
        hasUniqueField = self.uniqueColumnCheck.checkState() == Qt.Checked
        if hasUniqueField and self.allowMultiColumnPk:
            uniqueFieldName = ",".join(
                item.data()
                for item in self.uniqueModel.findItems("*", Qt.MatchWildcard)
                if item.checkState() == Qt.Checked
            )
        elif (
            hasUniqueField
            and not self.allowMultiColumnPk
            and self.uniqueCombo.currentIndex() >= 0
        ):
            uniqueFieldName = self.uniqueModel.item(self.uniqueCombo.currentIndex()).data()
        else:
            uniqueFieldName = None
        hasGeomCol = self.hasGeometryCol.checkState() == Qt.Checked
        if hasGeomCol:
            geomFieldName = self.geomCombo.currentText()
        else:
            geomFieldName = None

        query = self._getExecutableSqlQuery()
        if query == "":
            return None

        # remove a trailing ';' from query if present
        if query.strip().endswith(';'):
            query = query.strip()[:-1]

        layerType = QgsMapLayerType.VectorLayer if self.vectorRadio.isChecked() else QgsMapLayerType.RasterLayer

        # get a new layer name
        names = []
        for layer in list(QgsProject.instance().mapLayers().values()):
            names.append(layer.name())

        layerName = self.layerNameEdit.text()
        if layerName == "":
            layerName = self.defaultLayerName
        newLayerName = layerName
        index = 1
        while newLayerName in names:
            index += 1
            newLayerName = "%s_%d" % (layerName, index)

        # create the layer
        layer = self.db.toSqlLayer(query, geomFieldName, uniqueFieldName, newLayerName, layerType,
                                   self.avoidSelectById.isChecked(), _filter)
        if layer.isValid():
            return layer
        else:
            e = BaseError(self.tr("There was an error creating the SQL layer, please check the logs for further information."))
            DlgDbError.showError(e, self)
            return None

    def loadSqlLayer(self):
        with OverrideCursor(Qt.WaitCursor):
            layer = self._getSqlLayer(self.filter)
            if layer is None:
                return

            QgsProject.instance().addMapLayers([layer], True)

    def fillColumnCombos(self):
        query = self._getExecutableSqlQuery()
        if query == "":
            return

        with OverrideCursor(Qt.WaitCursor):
            # remove a trailing ';' from query if present
            if query.strip().endswith(';'):
                query = query.strip()[:-1]

            # get all the columns
            quotedCols = []
            connector = self.db.connector
            if self.aliasSubQuery:
                # get a new alias
                aliasIndex = 0
                while True:
                    alias = "_subQuery__%d" % aliasIndex
                    escaped = re.compile('\\b("?)' + re.escape(alias) + '\\1\\b')
                    if not escaped.search(query):
                        break
                    aliasIndex += 1

                sql = "SELECT * FROM (%s\n) AS %s LIMIT 0" % (str(query), connector.quoteId(alias))
            else:
                sql = "SELECT * FROM (%s\n) WHERE 1=0" % str(query)

            c = None
            try:
                c = connector._execute(None, sql)
                cols = connector._get_cursor_columns(c)
                for col in cols:
                    quotedCols.append(connector.quoteId(col))

            except BaseError as e:
                DlgDbError.showError(e, self)
                self.uniqueModel.clear()
                self.geomCombo.clear()
                return

            finally:
                if c:
                    c.close()
                    del c

            self.setColumnCombos(cols, quotedCols)

    def setColumnCombos(self, cols, quotedCols):
        # get sensible default columns. do this before sorting in case there's hints in the column order (e.g., id is more likely to be first)
        try:
            defaultGeomCol = next(col for col in cols if col in ['geom', 'geometry', 'the_geom', 'way'])
        except:
            defaultGeomCol = None
        try:
            defaultUniqueCol = [col for col in cols if 'id' in col][0]
        except:
            defaultUniqueCol = None

        colNames = sorted(zip(cols, quotedCols))
        newItems = []
        uniqueIsFilled = False
        for (col, quotedCol) in colNames:
            item = QStandardItem(col)
            item.setData(quotedCol)
            item.setEnabled(True)
            item.setCheckable(self.allowMultiColumnPk)
            item.setSelectable(not self.allowMultiColumnPk)
            if self.allowMultiColumnPk:
                matchingItems = self.uniqueModel.findItems(col)
                if matchingItems:
                    item.setCheckState(matchingItems[0].checkState())
                    uniqueIsFilled = uniqueIsFilled or matchingItems[0].checkState() == Qt.Checked
                else:
                    item.setCheckState(Qt.Unchecked)
            newItems.append(item)
        if self.allowMultiColumnPk:
            self.uniqueModel.clear()
            self.uniqueModel.appendColumn(newItems)
            self.uniqueChanged()
        else:
            previousUniqueColumn = self.uniqueCombo.currentText()
            self.uniqueModel.clear()
            self.uniqueModel.appendColumn(newItems)
            if self.uniqueModel.findItems(previousUniqueColumn):
                self.uniqueCombo.setEditText(previousUniqueColumn)
                uniqueIsFilled = True

        oldGeometryColumn = self.geomCombo.currentText()
        self.geomCombo.clear()
        self.geomCombo.addItems(cols)
        self.geomCombo.setCurrentIndex(self.geomCombo.findText(oldGeometryColumn, Qt.MatchExactly))

        # set sensible default columns if the columns are not already set
        try:
            if self.geomCombo.currentIndex() == -1:
                self.geomCombo.setCurrentIndex(cols.index(defaultGeomCol))
        except:
            pass
        items = self.uniqueModel.findItems(defaultUniqueCol)
        if items and not uniqueIsFilled:
            if self.allowMultiColumnPk:
                items[0].setCheckState(Qt.Checked)
            else:
                self.uniqueCombo.setEditText(defaultUniqueCol)

    def copySelectedResults(self):
        if len(self.viewResult.selectedIndexes()) <= 0:
            return
        model = self.viewResult.model()

        # convert to string using tab as separator
        text = model.headerToString("\t")
        for idx in self.viewResult.selectionModel().selectedRows():
            text += "\n" + model.rowToString(idx.row(), "\t")

        QApplication.clipboard().setText(text, QClipboard.Selection)
        QApplication.clipboard().setText(text, QClipboard.Clipboard)

    def initCompleter(self):
        dictionary = None
        if self.db:
            dictionary = self.db.connector.getSqlDictionary()
        if not dictionary:
            # use the generic sql dictionary
            from .sql_dictionary import getSqlDictionary

            dictionary = getSqlDictionary()

        wordlist = []
        for value in dictionary.values():
            wordlist += value  # concat lists
        wordlist = list(set(wordlist))  # remove duplicates

        api = QsciAPIs(self.editSql.lexer())
        for word in wordlist:
            api.add(word)

        api.prepare()
        self.editSql.lexer().setAPIs(api)

    def displayQueryBuilder(self):
        dlg = QueryBuilderDlg(self.iface, self.db, self, reset=self.queryBuilderFirst)
        self.queryBuilderFirst = False
        r = dlg.exec_()
        if r == QDialog.Accepted:
            self.editSql.setText(dlg.query)

    def createView(self):
        name, ok = QInputDialog.getText(None, self.tr("View Name"), self.tr("View name"))
        if ok:
            try:
                self.db.connector.createSpatialView(name, self._getExecutableSqlQuery())
            except BaseError as e:
                DlgDbError.showError(e, self)

    def _getSqlQuery(self):
        sql = self.editSql.selectedText()
        if len(sql) == 0:
            sql = self.editSql.text()
        return sql

    def _getExecutableSqlQuery(self):
        sql = self._getSqlQuery().strip()

        uncommented_sql = check_comments_in_sql(sql)
        uncommented_sql = uncommented_sql.rstrip(';')
        return uncommented_sql

    def uniqueChanged(self):
        # when an item is (un)checked, simply trigger an update of the combobox text
        self.uniqueTextChanged(None)

    def uniqueTextChanged(self, text):
        # Whenever there is new text displayed in the combobox, check if it is the correct one and if not, display the correct one.
        label = ", ".join(
            item.text()
            for item in self.uniqueModel.findItems("*", Qt.MatchWildcard)
            if item.checkState() == Qt.Checked
        )
        if text != label:
            self.uniqueCombo.setEditText(label)

    def setFilter(self):
        from qgis.gui import QgsQueryBuilder
        layer = self._getSqlLayer("")
        if not layer:
            return

        dlg = QgsQueryBuilder(layer)
        dlg.setSql(self.filter)
        if dlg.exec_():
            self.filter = dlg.sql()
        layer.deleteLater()

    def setHasChanged(self, hasChanged):
        self.hasChanged = hasChanged

    def close(self):
        if self.hasChanged:
            ret = QMessageBox.question(
                self, self.tr('Unsaved Changes?'),
                self.tr('There are unsaved changes. Do you want to keep them?'),
                QMessageBox.Save | QMessageBox.Cancel | QMessageBox.Discard, QMessageBox.Cancel)

            if ret == QMessageBox.Save:
                self.saveAsFilePreset()
                return True
            elif ret == QMessageBox.Discard:
                return True
            else:
                return False
        else:
            return True
