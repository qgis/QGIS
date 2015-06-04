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

from PyQt4.QtCore import Qt, QObject, QSettings, QByteArray, SIGNAL
from PyQt4.QtGui import QDialog, QAction, QKeySequence, QDialogButtonBox, QApplication, QCursor, QMessageBox, QClipboard, QInputDialog, QIcon
from PyQt4.Qsci import QsciAPIs

from qgis.core import QgsProject

from .db_plugins.plugin import BaseError
from .dlg_db_error import DlgDbError
from .dlg_query_builder import QueryBuilderDlg

try:
    from qgis.gui import QgsCodeEditorSQL
except:
    from .sqledit import SqlEdit
    from qgis import gui

    gui.QgsCodeEditorSQL = SqlEdit

from .ui.ui_DlgSqlWindow import Ui_DbManagerDlgSqlWindow as Ui_Dialog

import re


class DlgSqlWindow(QDialog, Ui_Dialog):
    def __init__(self, iface, db, parent=None):
        QDialog.__init__(self, parent)
        self.iface = iface
        self.db = db
        self.setupUi(self)
        self.setWindowTitle(
            u"%s - %s [%s]" % (self.windowTitle(), db.connection().connectionName(), db.connection().typeNameString()))

        self.defaultLayerName = 'QueryLayer'

        settings = QSettings()
        self.restoreGeometry(settings.value("/DB_Manager/sqlWindow/geometry", QByteArray(), type=QByteArray))

        self.editSql.setFocus()
        self.editSql.setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)
        self.initCompleter()

        # allow to copy results
        copyAction = QAction("copy", self)
        self.viewResult.addAction(copyAction)
        copyAction.setShortcuts(QKeySequence.Copy)
        QObject.connect(copyAction, SIGNAL("triggered()"), self.copySelectedResults)

        self.connect(self.btnExecute, SIGNAL("clicked()"), self.executeSql)
        self.connect(self.btnClear, SIGNAL("clicked()"), self.clearSql)
        self.connect(self.buttonBox.button(QDialogButtonBox.Close), SIGNAL("clicked()"), self.close)

        self.connect(self.presetStore, SIGNAL("clicked()"), self.storePreset)
        self.connect(self.presetDelete, SIGNAL("clicked()"), self.deletePreset)
        self.connect(self.presetCombo, SIGNAL("activated(QString)"), self.loadPreset)
        self.connect(self.presetCombo, SIGNAL("activated(QString)"), self.presetName.setText)
        self.updatePresetsCombobox()

        # hide the load query as layer if feature is not supported
        self._loadAsLayerAvailable = self.db.connector.hasCustomQuerySupport()
        self.loadAsLayerGroup.setVisible(self._loadAsLayerAvailable)
        if self._loadAsLayerAvailable:
            self.layerTypeWidget.hide()  # show if load as raster is supported
            self.connect(self.loadLayerBtn, SIGNAL("clicked()"), self.loadSqlLayer)
            self.connect(self.getColumnsBtn, SIGNAL("clicked()"), self.fillColumnCombos)
            self.connect(self.loadAsLayerGroup, SIGNAL("toggled(bool)"), self.loadAsLayerToggled)
            self.loadAsLayerToggled(False)

        self._createViewAvailable = self.db.connector.hasCreateSpatialViewSupport()
        self.btnCreateView.setVisible( self._createViewAvailable )
        if self._createViewAvailable:
            self.connect( self.btnCreateView, SIGNAL("clicked()"), self.createView )

        self.queryBuilderFirst = True
        self.queryBuilderBtn.setIcon(QIcon(":/db_manager/icons/sql.gif"))
        self.connect( self.queryBuilderBtn, SIGNAL("clicked()"), self.displayQueryBuilder )

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
        if query == "": return
        name = self.presetName.text()
        QgsProject.instance().writeEntry('DBManager', 'savedQueries/q' + str(name.__hash__()) + '/name', name)
        QgsProject.instance().writeEntry('DBManager', 'savedQueries/q' + str(name.__hash__()) + '/query', query)
        index = self.presetCombo.findText(name)
        if index == -1:
            self.presetCombo.addItem(name)
            self.presetCombo.setCurrentIndex(self.presetCombo.count() - 1)
        else:
            self.presetCombo.setCurrentIndex(index)

    def deletePreset(self):
        name = self.presetCombo.currentText()
        QgsProject.instance().removeEntry('DBManager', 'savedQueries/q' + str(name.__hash__()))
        self.presetCombo.removeItem(self.presetCombo.findText(name))
        self.presetCombo.setCurrentIndex(-1)

    def loadPreset(self, name):
        query = QgsProject.instance().readEntry('DBManager', 'savedQueries/q' + str(name.__hash__()) + '/query')[0]
        name = QgsProject.instance().readEntry('DBManager', 'savedQueries/q' + str(name.__hash__()) + '/name')[0]
        self.editSql.setText(query)

    def closeEvent(self, e):
        """ save window state """
        settings = QSettings()
        settings.setValue("/DB_Manager/sqlWindow/geometry", self.saveGeometry())

        QDialog.closeEvent(self, e)

    def loadAsLayerToggled(self, checked):
        self.loadAsLayerGroup.setChecked(checked)
        self.loadAsLayerWidget.setVisible(checked)

    def clearSql(self):
        self.editSql.clear()
        self.editSql.setFocus()

    def executeSql(self):

        sql = self._getSqlQuery()
        if sql == "": return

        QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))

        # delete the old model
        old_model = self.viewResult.model()
        self.viewResult.setModel(None)
        if old_model: old_model.deleteLater()

        self.uniqueCombo.clear()
        self.geomCombo.clear()

        try:
            # set the new model
            model = self.db.sqlResultModel(sql, self)
            self.viewResult.setModel(model)
            self.lblResult.setText(self.tr("%d rows, %.1f seconds") % (model.affectedRows(), model.secs()))

        except BaseError, e:
            QApplication.restoreOverrideCursor()
            DlgDbError.showError(e, self)
            return

        cols = self.viewResult.model().columnNames()
        cols.sort()
        self.uniqueCombo.addItems(cols)
        self.geomCombo.addItems(cols)

        self.update()
        QApplication.restoreOverrideCursor()

    def loadSqlLayer(self):
        hasUniqueField = self.uniqueColumnCheck.checkState() == Qt.Checked
        if hasUniqueField:
            uniqueFieldName = self.uniqueCombo.currentText()
        else:
            uniqueFieldName = None
        hasGeomCol = self.hasGeometryCol.checkState() == Qt.Checked
        if hasGeomCol:
            geomFieldName = self.geomCombo.currentText()
        else:
            geomFieldName = None

        query = self._getSqlQuery()
        if query == "": return

        # remove a trailing ';' from query if present
        if query.strip().endswith(';'):
            query = query.strip()[:-1]

        QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))

        from qgis.core import QgsMapLayer, QgsMapLayerRegistry

        layerType = QgsMapLayer.VectorLayer if self.vectorRadio.isChecked() else QgsMapLayer.RasterLayer

        # get a new layer name
        names = []
        for layer in QgsMapLayerRegistry.instance().mapLayers().values():
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
                                   self.avoidSelectById.isChecked())
        if layer.isValid():
            QgsMapLayerRegistry.instance().addMapLayers([layer], True)

        QApplication.restoreOverrideCursor()

    def fillColumnCombos(self):
        query = self._getSqlQuery()
        if query == "": return

        QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))
        self.uniqueCombo.clear()
        self.geomCombo.clear()

        # get a new alias
        aliasIndex = 0
        while True:
            alias = "_%s__%d" % ("subQuery", aliasIndex)
            escaped = re.compile('\\b("?)' + re.escape(alias) + '\\1\\b')
            if not escaped.search(query):
                break
            aliasIndex += 1

        # remove a trailing ';' from query if present
        if query.strip().endswith(';'):
            query = query.strip()[:-1]

        # get all the columns
        cols = []
        connector = self.db.connector
        sql = u"SELECT * FROM (%s\n) AS %s LIMIT 0" % ( unicode(query), connector.quoteId(alias) )

        c = None
        try:
            c = connector._execute(None, sql)
            cols = connector._get_cursor_columns(c)

        except BaseError as e:
            QApplication.restoreOverrideCursor()
            DlgDbError.showError(e, self)
            return

        finally:
            if c:
                c.close()
                del c

        # get sensible default columns. do this before sorting in case there's hints in the column order (eg, id is more likely to be first)
        try:
            defaultGeomCol = next(col for col in cols if col in ['geom', 'geometry', 'the_geom', 'way'])
        except:
            defaultGeomCol = None
        try:
            defaultUniqueCol = [col for col in cols if 'id' in col][0]
        except:
            defaultUniqueCol = None

        cols.sort()
        self.uniqueCombo.addItems(cols)
        self.geomCombo.addItems(cols)

        # set sensible default columns
        try:
            self.geomCombo.setCurrentIndex(cols.index(defaultGeomCol))
        except:
            pass
        try:
            self.uniqueCombo.setCurrentIndex(cols.index(defaultUniqueCol))
        except:
            pass

        QApplication.restoreOverrideCursor()

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
        for name, value in dictionary.iteritems():
            wordlist += value  # concat lists
        wordlist = list(set(wordlist))  # remove duplicates

        api = QsciAPIs(self.editSql.lexer())
        for word in wordlist:
            api.add(word)

        api.prepare()
        self.editSql.lexer().setAPIs(api)

    def displayQueryBuilder( self ):
        dlg = QueryBuilderDlg( self.iface, self.db, self, reset = self.queryBuilderFirst )
        self.queryBuilderFirst = False
        r = dlg.exec_()
        if r == QDialog.Accepted:
            self.editSql.setText( dlg.query )

    def createView( self ):
        name, ok = QInputDialog.getText(None, "View name", "View name")
        if ok:
            try:
                self.db.connector.createSpatialView( name, self._getSqlQuery() )
            except BaseError as e:
                DlgDbError.showError(e, self)

    def _getSqlQuery(self):
        sql = self.editSql.selectedText()
        if len(sql) == 0:
            sql = self.editSql.text()
        return sql
      
