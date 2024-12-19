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
from qgis.PyQt.QtWidgets import QDialog, QApplication
from qgis.utils import OverrideCursor

from .db_plugins.plugin import DbError
from .dlg_db_error import DlgDbError
from .db_plugins.plugin import TableConstraint
from .gui_utils import GuiUtils

Ui_Dialog, _ = uic.loadUiType(GuiUtils.get_ui_file_path('DlgCreateConstraint.ui'))


class DlgCreateConstraint(QDialog, Ui_Dialog):

    def __init__(self, parent=None, table=None, db=None):
        QDialog.__init__(self, parent)
        self.table = table
        self.db = self.table.database() if self.table and self.table.database() else db
        self.setupUi(self)

        self.buttonBox.accepted.connect(self.createConstraint)
        self.populateColumns()

    def populateColumns(self):
        self.cboColumn.clear()
        for fld in self.table.fields():
            self.cboColumn.addItem(fld.name)

    def createConstraint(self):
        constr = self.getConstraint()

        # now create the constraint
        with OverrideCursor(Qt.CursorShape.WaitCursor):
            try:
                self.table.addConstraint(constr)
            except DbError as e:
                DlgDbError.showError(e, self)
                return

        self.accept()

    def getConstraint(self):
        constr = TableConstraint(self.table)
        constr.name = ""
        constr.type = TableConstraint.TypePrimaryKey if self.radPrimaryKey.isChecked() else TableConstraint.TypeUnique
        constr.columns = []
        column = self.cboColumn.currentText()
        for fld in self.table.fields():
            if fld.name == column:
                constr.columns.append(fld.num)
                break
        return constr
