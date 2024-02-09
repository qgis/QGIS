"""
/***************************************************************************
Name                 : DB Manager
Description          : Database manager plugin for QGIS
Date                 : May 23, 2011
copyright            : (C) 2011 by Giuseppe Sucameli
email                : brush.tyler@gmail.com

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

from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtWidgets import QTableView, QAbstractItemView, QApplication, QAction
from qgis.PyQt.QtGui import QKeySequence, QCursor, QClipboard

from qgis.utils import OverrideCursor

from .db_plugins.plugin import DbError, Table
from .dlg_db_error import DlgDbError


class TableViewer(QTableView):

    def __init__(self, parent=None):
        QTableView.__init__(self, parent)
        self.setSelectionBehavior(QAbstractItemView.SelectionBehavior.SelectRows)
        self.setSelectionMode(QAbstractItemView.SelectionMode.ExtendedSelection)

        self.item = None
        self.dirty = False

        # allow copying results
        copyAction = QAction(QApplication.translate("DBManagerPlugin", "Copy"), self)
        self.addAction(copyAction)
        copyAction.setShortcuts(QKeySequence.StandardKey.Copy)
        copyAction.triggered.connect(self.copySelectedResults)

        self._clear()

    def refresh(self):
        self.dirty = True
        self.loadData(self.item)

    def loadData(self, item):
        if item == self.item and not self.dirty:
            return
        self._clear()
        if item is None:
            return

        if isinstance(item, Table):
            self._loadTableData(item)
        else:
            return

        self.item = item
        self.item.aboutToChange.connect(self.setDirty)

    def setDirty(self, val=True):
        self.dirty = val

    def _clear(self):
        if self.item is not None:
            try:
                self.item.aboutToChange.disconnect(self.setDirty)
            except:
                # do not raise any error if self.item was deleted
                pass

        self.item = None
        self.dirty = False

        # delete the old model
        model = self.model()
        self.setModel(None)
        if model:
            model.deleteLater()

    def _loadTableData(self, table):
        with OverrideCursor(Qt.CursorShape.WaitCursor):
            try:
                # set the new model
                self.setModel(table.tableDataModel(self))
            except DbError as e:
                DlgDbError.showError(e, self)
            else:
                self.update()

    def copySelectedResults(self):
        if len(self.selectedIndexes()) <= 0:
            return
        model = self.model()

        # convert to string using tab as separator
        text = model.headerToString("\t")
        for idx in self.selectionModel().selectedRows():
            text += "\n" + model.rowToString(idx.row(), "\t")

        QApplication.clipboard().setText(text, QClipboard.Mode.Selection)
        QApplication.clipboard().setText(text, QClipboard.Mode.Clipboard)
