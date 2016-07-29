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

from qgis.PyQt.QtCore import Qt, pyqtSignal
from qgis.PyQt.QtWidgets import QDialog, QWidget, QAction, QApplication, QInputDialog, QStyledItemDelegate
from qgis.PyQt.QtGui import QKeySequence, QCursor, QClipboard, QIcon, QStandardItemModel, QStandardItem
from qgis.PyQt.Qsci import QsciAPIs

from qgis.core import QgsProject

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


class DlgSqlWindow(QWidget, Ui_Dialog):
    nameChanged = pyqtSignal(str)

    def __init__(self, iface, db, parent=None):
        QWidget.__init__(self, parent)
        self.iface = iface
        self.db = db
        self.filter = ""
        self.allowMultiColumnPk = isinstance(db, PGDatabase)  # at the moment only PostgreSQL allows a primary key to span multiple columns, spatialite doesn't
        self.aliasSubQuery = isinstance(db, PGDatabase)       # only PostgreSQL requires subqueries to be aliases
        self.setupUi(self)
        self.setWindowTitle(
            u"%s - %s [%s]" % (self.windowTitle(), db.connection().connectionName(), db.connection().typeNameString()))

        self.defaultLayerName = 'QueryLayer'

        if self.allowMultiColumnPk:
            self.uniqueColumnCheck.setText(self.trUtf8("Column(s) with unique values"))
        else:
            self.uniqueColumnCheck.setText(self.trUtf8("Column with unique values"))

        self.editSql.setFocus()
        self.editSql.setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)
        self.editSql.setMarginVisible(True)
        self.initCompleter()

        # allow copying results
        copyAction = QAction("copy", self)
        self.viewResult.addAction(copyAction)
        copyAction.setShortcuts(QKeySequence.Copy)

        copyAction.triggered.connect(self.copySelectedResults)

        self.btnExecute.clicked.connect(self.executeSql)
        self.btnSetFilter.clicked.connect(self.setFilter)
        self.btnClear.clicked.connect(self.clearSql)

        self.presetStore.clicked.connect(self.storePreset)
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
            self.uniqueModel.itemChanged.connect(self.uniqueChanged)                 # react to the (un)checking of an item
            self.uniqueCombo.lineEdit().textChanged.connect(self.uniqueTextChanged)  # there are other events that change the displayed text and some of them can not be caught directly

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
        name = self.presetName.text()
        QgsProject.instance().writeEntry('DBManager', 'savedQueries/q' + unicode(name.__hash__()) + '/name', name)
        QgsProject.instance().writeEntry('DBManager', 'savedQueries/q' + unicode(name.__hash__()) + '/query', query)
        index = self.presetCombo.findText(name)
        if index == -1:
            self.presetCombo.addItem(name)
            self.presetCombo.setCurrentIndex(self.presetCombo.count() - 1)
        else:
            self.presetCombo.setCurrentIndex(index)

    def deletePreset(self):
        name = self.presetCombo.currentText()
        QgsProject.instance().removeEntry('DBManager', 'savedQueries/q' + unicode(name.__hash__()))
        self.presetCombo.removeItem(self.presetCombo.findText(name))
        self.presetCombo.setCurrentIndex(-1)

    def loadPreset(self, name):
        query = QgsProject.instance().readEntry('DBManager', 'savedQueries/q' + unicode(name.__hash__()) + '/query')[0]
        name = QgsProject.instance().readEntry('DBManager', 'savedQueries/q' + unicode(name.__hash__()) + '/name')[0]
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

    def executeSql(self):

        sql = self._getSqlQuery()
        if sql == "":
            return

        QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))

        # delete the old model
        old_model = self.viewResult.model()
        self.viewResult.setModel(None)
        if old_model:
            old_model.deleteLater()

        cols = []
        quotedCols = []

        try:
            # set the new model
            model = self.db.sqlResultModel(sql, self)
            self.viewResult.setModel(model)
            self.lblResult.setText(self.tr("%d rows, %.1f seconds") % (model.affectedRows(), model.secs()))
            cols = self.viewResult.model().columnNames()
            for col in cols:
                quotedCols.append(self.db.connector.quoteId(col))

        except BaseError as e:
            QApplication.restoreOverrideCursor()
            DlgDbError.showError(e, self)
            self.uniqueModel.clear()
            self.geomCombo.clear()
            return

        self.setColumnCombos(cols, quotedCols)

        self.update()
        QApplication.restoreOverrideCursor()

    def _getSqlLayer(self, _filter):
        hasUniqueField = self.uniqueColumnCheck.checkState() == Qt.Checked
        if hasUniqueField:
            if self.allowMultiColumnPk:
                checkedCols = []
                for item in self.uniqueModel.findItems("*", Qt.MatchWildcard):
                    if item.checkState() == Qt.Checked:
                        checkedCols.append(item.data())
                uniqueFieldName = ",".join(checkedCols)
            elif self.uniqueCombo.currentIndex() >= 0:
                uniqueFieldName = self.uniqueModel.item(self.uniqueCombo.currentIndex()).data()
            else:
                uniqueFieldName = None
        else:
            uniqueFieldName = None
        hasGeomCol = self.hasGeometryCol.checkState() == Qt.Checked
        if hasGeomCol:
            geomFieldName = self.geomCombo.currentText()
        else:
            geomFieldName = None

        query = self._getSqlQuery()
        if query == "":
            return None

        # remove a trailing ';' from query if present
        if query.strip().endswith(';'):
            query = query.strip()[:-1]

        from qgis.core import QgsMapLayer, QgsMapLayerRegistry

        layerType = QgsMapLayer.VectorLayer if self.vectorRadio.isChecked() else QgsMapLayer.RasterLayer

        # get a new layer name
        names = []
        for layer in list(QgsMapLayerRegistry.instance().mapLayers().values()):
            names.append(layer.name())

        layerName = self.layerNameEdit.text()
        if layerName == "":
            layerName = self.defaultLayerName
        newLayerName = layerName
        index = 1
        while newLayerName in names:
            index += 1
            newLayerName = u"%s_%d" % (layerName, index)

        # create the layer
        layer = self.db.toSqlLayer(query, geomFieldName, uniqueFieldName, newLayerName, layerType,
                                   self.avoidSelectById.isChecked(), _filter)
        if layer.isValid():
            return layer
        else:
            return None

    def loadSqlLayer(self):
        QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))
        try:
            layer = self._getSqlLayer(self.filter)
            if layer is None:
                return

            from qgis.core import QgsMapLayerRegistry
            QgsMapLayerRegistry.instance().addMapLayers([layer], True)
        finally:
            QApplication.restoreOverrideCursor()

    def fillColumnCombos(self):
        query = self._getSqlQuery()
        if query == "":
            return

        QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))

        # remove a trailing ';' from query if present
        if query.strip().endswith(';'):
            query = query.strip()[:-1]

        # get all the columns
        cols = []
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

            sql = u"SELECT * FROM (%s\n) AS %s LIMIT 0" % (unicode(query), connector.quoteId(alias))
        else:
            sql = u"SELECT * FROM (%s\n) WHERE 1=0" % unicode(query)

        c = None
        try:
            c = connector._execute(None, sql)
            cols = connector._get_cursor_columns(c)
            for col in cols:
                quotedCols.append(connector.quoteId(col))

        except BaseError as e:
            QApplication.restoreOverrideCursor()
            DlgDbError.showError(e, self)
            self.uniqueModel.clear()
            self.geomCombo.clear()
            return

        finally:
            if c:
                c.close()
                del c

        self.setColumnCombos(cols, quotedCols)

        QApplication.restoreOverrideCursor()

    def setColumnCombos(self, cols, quotedCols):
        # get sensible default columns. do this before sorting in case there's hints in the column order (eg, id is more likely to be first)
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
        try:
            pass
        except:
            pass

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
        for name, value in dictionary.items():
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
        name, ok = QInputDialog.getText(None, "View name", "View name")
        if ok:
            try:
                self.db.connector.createSpatialView(name, self._getSqlQuery())
            except BaseError as e:
                DlgDbError.showError(e, self)

    def _getSqlQuery(self):
        sql = self.editSql.selectedText()
        if len(sql) == 0:
            sql = self.editSql.text()
        return sql

    def uniqueChanged(self):
        # when an item is (un)checked, simply trigger an update of the combobox text
        self.uniqueTextChanged(None)

    def uniqueTextChanged(self, text):
        # Whenever there is new text displayed in the combobox, check if it is the correct one and if not, display the correct one.
        checkedItems = []
        for item in self.uniqueModel.findItems("*", Qt.MatchWildcard):
            if item.checkState() == Qt.Checked:
                checkedItems.append(item.text())
        label = ", ".join(checkedItems)
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
