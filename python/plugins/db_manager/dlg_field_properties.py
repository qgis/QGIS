# -*- coding: utf-8 -*-

"""
***************************************************************************
    dlg_field_properties.py
    ---------------------
    Date                 : April 2012
    Copyright            : (C) 2012 by Giuseppe Sucameli
    Email                : brush dot tyler at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""
from builtins import str

__author__ = 'Giuseppe Sucameli'
__date__ = 'April 2012'
__copyright__ = '(C) 2012, Giuseppe Sucameli'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.PyQt.QtWidgets import QDialog, QMessageBox

from .db_plugins.plugin import TableField

from .ui.ui_DlgFieldProperties import Ui_DbManagerDlgFieldProperties as Ui_Dialog


class DlgFieldProperties(QDialog, Ui_Dialog):

    def __init__(self, parent=None, fld=None, table=None, db=None):
        QDialog.__init__(self, parent)
        self.fld = fld
        self.table = self.fld.table() if self.fld and self.fld.table() else table
        self.db = self.table.database() if self.table and self.table.database() else db
        self.setupUi(self)

        for item in self.db.connector.fieldTypes():
            self.cboType.addItem(item)
        self.setField(self.fld)

        self.buttonBox.accepted.connect(self.onOK)

    def setField(self, fld):
        if fld is None:
            return
        self.editName.setText(fld.name)
        self.cboType.setEditText(fld.dataType)
        if fld.modifier:
            self.editLength.setText(str(fld.modifier))
        self.chkNull.setChecked(not fld.notNull)
        if fld.hasDefault:
            self.editDefault.setText(fld.default)
        # This is an ugly patch, but the comments PR https://github.com/qgis/QGIS/pull/8831 added
        # support for postgres only and broke all the others :(
        try:
            # Check with SQL query if a comment exists for the field
            sql_cpt = "Select count(*) from pg_description pd, pg_class pc, pg_attribute pa where relname = '%s' and attname = '%s' and pa.attrelid = pc.oid and pd.objoid = pc.oid and pd.objsubid = pa.attnum" % (self.table.name, self.editName.text())
            # Get the comment for the field with SQL Query
            sql = "Select pd.description from pg_description pd, pg_class pc, pg_attribute pa where relname = '%s' and attname = '%s' and pa.attrelid = pc.oid and pd.objoid = pc.oid and pd.objsubid = pa.attnum" % (self.table.name, self.editName.text())
            c = self.db.connector._execute(None, sql_cpt) # Execute check query
            res = self.db.connector._fetchone(c)[0] # Fetch data
            # Check if result is 1 then it's ok, else we don't want to get a value
            if res == 1:
                c = self.db.connector._execute(None, sql) # Execute query returning the comment value
                res = self.db.connector._fetchone(c)[0] # Fetch the comment value
                self.db.connector._close_cursor(c) # Close cursor
                self.editCom.setText(res) # Set comment value
        except:
            self.editCom.setEnabled(False)

    def getField(self, newCopy=False):
        fld = TableField(self.table) if not self.fld or newCopy else self.fld
        fld.name = self.editName.text()
        fld.dataType = self.cboType.currentText()
        fld.notNull = not self.chkNull.isChecked()
        fld.default = self.editDefault.text()
        fld.hasDefault = fld.default != ""
        # Get the comment from the LineEdit
        fld.comment = self.editCom.text()
        try:
            modifier = int(self.editLength.text())
        except ValueError:
            ok = False
        else:
            ok = True
        fld.modifier = modifier if ok else None
        return fld

    def onOK(self):
        """ first check whether everything's fine """
        fld = self.getField(True)  # don't change the original copy
        if fld.name == "":
            QMessageBox.critical(self, self.tr("DB Manager"), self.tr("Field name must not be empty."))
            return
        if fld.dataType == "":
            QMessageBox.critical(self, self.tr("DB Manager"), self.tr("Field type must not be empty."))
            return

        self.accept()
