"""
/***************************************************************************
Name                 : DB Manager
Description          : Database manager plugin for QGIS
Date                 : Oct 13, 2011
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

from qgis.PyQt import uic
from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtWidgets import QDialog, QMessageBox, QApplication
from qgis.utils import OverrideCursor

from .db_plugins.plugin import DbError
from .dlg_db_error import DlgDbError
from .db_plugins.plugin import TableIndex
from .gui_utils import GuiUtils

Ui_Dialog, _ = uic.loadUiType(GuiUtils.get_ui_file_path("DlgCreateIndex.ui"))


class DlgCreateIndex(QDialog, Ui_Dialog):

    def __init__(self, parent=None, table=None, db=None):
        QDialog.__init__(self, parent)
        self.table = table
        self.db = self.table.database() if self.table and self.table.database() else db
        self.setupUi(self)

        self.buttonBox.accepted.connect(self.createIndex)

        self.cboColumn.currentIndexChanged.connect(self.columnChanged)
        self.populateColumns()

    def populateColumns(self):
        self.cboColumn.clear()
        for fld in self.table.fields():
            self.cboColumn.addItem(fld.name)

    def columnChanged(self):
        self.editName.setText(f"idx_{self.table.name}_{self.cboColumn.currentText()}")

    def createIndex(self):
        idx = self.getIndex()
        if idx.name == "":
            QMessageBox.critical(
                self, self.tr("Error"), self.tr("Please enter a name for the index.")
            )
            return

        # now create the index
        with OverrideCursor(Qt.CursorShape.WaitCursor):
            try:
                self.table.addIndex(idx)
            except DbError as e:
                DlgDbError.showError(e, self)
                return

        self.accept()

    def getIndex(self):
        idx = TableIndex(self.table)
        idx.name = self.editName.text()
        idx.columns = []
        colname = self.cboColumn.currentText()
        for fld in self.table.fields():
            if fld.name == colname:
                idx.columns.append(fld.num)
                break
        return idx
