# -*- coding: utf-8 -*-

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

from PyQt4.QtCore import Qt, SIGNAL
from PyQt4.QtGui import QDialog, QApplication

from .db_plugins.plugin import DbError
from .dlg_db_error import DlgDbError
from .db_plugins.plugin import TableConstraint

from .ui.ui_DlgCreateConstraint import Ui_DbManagerDlgCreateConstraint as Ui_Dialog

class DlgCreateConstraint(QDialog, Ui_Dialog):
        def __init__(self, parent=None, table=None, db=None):
                QDialog.__init__(self, parent)
                self.table = table
                self.db = self.table.database() if self.table and self.table.database() else db
                self.setupUi(self)

                self.connect(self.buttonBox, SIGNAL("accepted()"), self.createConstraint)
                self.populateColumns()

        def populateColumns(self):
                self.cboColumn.clear()
                for fld in self.table.fields():
                        self.cboColumn.addItem(fld.name)


        def createConstraint(self):
                constr = self.getConstraint()

                # now create the constraint
                QApplication.setOverrideCursor(Qt.WaitCursor)
                try:
                        self.table.addConstraint(constr)
                except DbError, e:
                        DlgDbError.showError(e, self)
                        return
                finally:
                        QApplication.restoreOverrideCursor()

                self.accept()

        def getConstraint(self):
                constr = TableConstraint(self.table)
                constr.name = u""
                constr.type = TableConstraint.TypePrimaryKey if self.radPrimaryKey.isChecked() else TableConstraint.TypeUnique
                constr.columns = []
                column = self.cboColumn.currentText()
                for fld in self.table.fields():
                        if fld.name == column:
                                constr.columns.append( fld.num )
                                break
                return constr
