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

from qgis.PyQt.QtCore import Qt, pyqtSignal
from qgis.PyQt.QtWidgets import QDialog, QWidget, QAction, QApplication, QStyledItemDelegate
from qgis.PyQt.QtGui import QKeySequence, QCursor, QClipboard, QIcon, QStandardItemModel, QStandardItem
from qgis.PyQt.Qsci import QsciAPIs
from qgis.PyQt.QtXml import QDomDocument

from qgis.core import QgsProject, QgsDataSourceUri, QgsReadWriteContext
from qgis.utils import OverrideCursor

from .db_plugins import createDbPlugin
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

from .ui.ui_DlgSqlLayerWindow import Ui_DbManagerDlgSqlLayerWindow as Ui_Dialog

import re


class DlgSqlLayerWindow(QWidget, Ui_Dialog):
    nameChanged = pyqtSignal(str)

    def __init__(self, iface, layer, parent=None):
        QWidget.__init__(self, parent)
        self.iface = iface
        self.layer = layer

        uri = QgsDataSourceUri(layer.source())
        dbplugin = None
        db = None
        if layer.dataProvider().name() == 'postgres':
            dbplugin = createDbPlugin('postgis', 'postgres')
        elif layer.dataProvider().name() == 'spatialite':
            dbplugin = createDbPlugin('spatialite', 'spatialite')
        elif layer.dataProvider().name() == 'oracle':
            dbplugin = createDbPlugin('oracle', 'oracle')
        elif layer.dataProvider().name() == 'virtual':
            dbplugin = createDbPlugin('vlayers', 'virtual')
        elif layer.dataProvider().name() == 'ogr':
            dbplugin = createDbPlugin('gpkg', 'gpkg')
        if dbplugin:
            dbplugin.connectToUri(uri)
            db = dbplugin.db

        self.dbplugin = dbplugin
        self.db = db
        self.filter = ""
        self.allowMultiColumnPk = isinstance(db, PGDatabase)  # at the moment only PostgreSQL allows a primary key to span multiple columns, SpatiaLite doesn't
        self.aliasSubQuery = isinstance(db, PGDatabase)  # only PostgreSQL requires subqueries to be aliases
        self.setupUi(self)
        self.setWindowTitle(
            u"%s - %s [%s]" % (self.windowTitle(), db.connection().connectionName(), db.connection().typeNameString()))

        self.defaultLayerName = 'QueryLayer'

        if self.allowMultiColumnPk:
            self.uniqueColumnCheck.setText(self.tr("Column(s) with unique values"))
        else:
            self.uniqueColumnCheck.setText(self.tr("Column with unique values"))

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

        self.editSql.textChanged.connect(self.updatePresetButtonsState)
        self.presetName.textChanged.connect(self.updatePresetButtonsState)
        self.presetCombo.currentIndexChanged.connect(self.updatePresetButtonsState)

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

        self.layerTypeWidget.hide()  # show if load as raster is supported
        # self.loadLayerBtn.clicked.connect(self.loadSqlLayer)
        self.updateLayerBtn.clicked.connect(self.updateSqlLayer)
        self.getColumnsBtn.clicked.connect(self.fillColumnCombos)

        self.queryBuilderFirst = True
        self.queryBuilderBtn.setIcon(QIcon(":/db_manager/icons/sql.gif"))
        self.queryBuilderBtn.clicked.connect(self.displayQueryBuilder)

        self.presetName.textChanged.connect(self.nameChanged)

        # Update from layer
        # First the SQL from QgsDataSourceUri table
        sql = uri.table().replace('\n', ' ').strip()
        if uri.keyColumn() == '_uid_':
            match = re.search(r'^\(SELECT .+ AS _uid_,\* FROM \((.*)\) AS _subq_.+_\s*\)$', sql, re.S | re.X | re.IGNORECASE)
            if match:
                sql = match.group(1)
        else:
            match = re.search(r'^\((SELECT .+ FROM .+)\)$', sql, re.S | re.X | re.IGNORECASE)
            if match:
                sql = match.group(1)
        # Need to check on table() since the parentheses were removed by the regexp
        if not uri.table().startswith('(') and not uri.table().endswith(')'):
            schema = uri.schema()
            if schema and schema.upper() != 'PUBLIC':
                sql = 'SELECT * FROM {0}.{1}'.format(self.db.connector.quoteId(schema), self.db.connector.quoteId(sql))
            else:
                sql = 'SELECT * FROM {0}'.format(self.db.connector.quoteId(sql))
        self.editSql.setText(sql)
        self.executeSql()

        # Then the columns
        self.geomCombo.setCurrentIndex(self.geomCombo.findText(uri.geometryColumn(), Qt.MatchExactly))
        if uri.keyColumn() != '_uid_':
            self.uniqueColumnCheck.setCheckState(Qt.Checked)
            if self.allowMultiColumnPk:
                itemsData = uri.keyColumn().split(',')
                for item in self.uniqueModel.findItems("*", Qt.MatchWildcard):
                    if item.data() in itemsData:
                        item.setCheckState(Qt.Checked)
            else:
                keyColumn = uri.keyColumn()
                if self.uniqueModel.findItems(keyColumn):
                    self.uniqueCombo.setEditText(keyColumn)

        # Finally layer name, filter and selectAtId
        self.layerNameEdit.setText(layer.name())
        self.filter = uri.sql()
        if uri.selectAtIdDisabled():
            self.avoidSelectById.setCheckState(Qt.Checked)

    def getQueryHash(self, name):
        return 'q%s' % md5(name.encode('utf8')).hexdigest()

    def updatePresetButtonsState(self, *args):
        """Slot called when the combo box or the sql or the query name have changed:
           sets store button state"""
        self.presetStore.setEnabled(bool(self._getSqlQuery() and self.presetName.text()))
        self.presetDelete.setEnabled(bool(self.presetCombo.currentIndex() != -1))

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
        QgsProject.instance().writeEntry('DBManager', 'savedQueries/' + self.getQueryHash(name) + '/name', name)
        QgsProject.instance().writeEntry('DBManager', 'savedQueries/' + self.getQueryHash(name) + '/query', query)
        index = self.presetCombo.findText(name)
        if index == -1:
            self.presetCombo.addItem(name)
            self.presetCombo.setCurrentIndex(self.presetCombo.count() - 1)
        else:
            self.presetCombo.setCurrentIndex(index)

    def deletePreset(self):
        name = self.presetCombo.currentText()
        QgsProject.instance().removeEntry('DBManager', 'savedQueries/q' + self.getQueryHash(name))
        self.presetCombo.removeItem(self.presetCombo.findText(name))
        self.presetCombo.setCurrentIndex(-1)

    def loadPreset(self, name):
        query = QgsProject.instance().readEntry('DBManager', 'savedQueries/' + self.getQueryHash(name) + '/query')[0]
        name = QgsProject.instance().readEntry('DBManager', 'savedQueries/' + self.getQueryHash(name) + '/name')[0]
        self.editSql.setText(query)

    def clearSql(self):
        self.editSql.clear()
        self.editSql.setFocus()
        self.filter = ""

    def executeSql(self):

        sql = self._getSqlQuery()
        if sql == "":
            return

        with OverrideCursor(Qt.WaitCursor):

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
                self.lblResult.setText(self.tr("{0} rows, {1:.3f} seconds").format(model.affectedRows(), model.secs()))
                cols = self.viewResult.model().columnNames()
                for col in cols:
                    quotedCols.append(self.db.connector.quoteId(col))

            except BaseError as e:
                DlgDbError.showError(e, self)
                self.uniqueModel.clear()
                self.geomCombo.clear()
                return

            self.setColumnCombos(cols, quotedCols)

            self.update()

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

        from qgis.core import QgsMapLayer

        layerType = QgsMapLayer.VectorLayer if self.vectorRadio.isChecked() else QgsMapLayer.RasterLayer

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
            newLayerName = u"%s_%d" % (layerName, index)

        # create the layer
        layer = self.db.toSqlLayer(query, geomFieldName, uniqueFieldName, newLayerName, layerType,
                                   self.avoidSelectById.isChecked(), _filter)
        if layer.isValid():
            return layer
        else:
            return None

    def loadSqlLayer(self):
        with OverrideCursor(Qt.WaitCursor):
            layer = self._getSqlLayer(self.filter)
            if layer is None:
                return

            QgsProject.instance().addMapLayers([layer], True)

    def updateSqlLayer(self):
        with OverrideCursor(Qt.WaitCursor):
            layer = self._getSqlLayer(self.filter)
            if layer is None:
                return

            # self.layer.dataProvider().setDataSourceUri(layer.dataProvider().dataSourceUri())
            # self.layer.dataProvider().reloadData()
            XMLDocument = QDomDocument("style")
            XMLMapLayers = XMLDocument.createElement("maplayers")
            XMLMapLayer = XMLDocument.createElement("maplayer")
            self.layer.writeLayerXml(XMLMapLayer, XMLDocument, QgsReadWriteContext())
            XMLMapLayer.firstChildElement("datasource").firstChild().setNodeValue(layer.source())
            XMLMapLayers.appendChild(XMLMapLayer)
            XMLDocument.appendChild(XMLMapLayers)
            self.layer.readLayerXml(XMLMapLayer, QgsReadWriteContext())
            self.layer.reload()
            self.iface.actionDraw().trigger()
            self.iface.mapCanvas().refresh()

    def fillColumnCombos(self):
        query = self._getSqlQuery()
        if query == "":
            return

        with OverrideCursor(Qt.WaitCursor):
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

                sql = u"SELECT * FROM (%s\n) AS %s LIMIT 0" % (str(query), connector.quoteId(alias))
            else:
                sql = u"SELECT * FROM (%s\n) WHERE 1=0" % str(query)

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
