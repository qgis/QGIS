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

__author__ = "Giuseppe Sucameli"
__date__ = "April 2012"
__copyright__ = "(C) 2012, Giuseppe Sucameli"

from qgis.PyQt import uic
from qgis.PyQt.QtWidgets import QDialog, QMessageBox

from .db_plugins.plugin import TableField
from .gui_utils import GuiUtils

Ui_Dialog, _ = uic.loadUiType(GuiUtils.get_ui_file_path("DlgFieldProperties.ui"))


class DlgFieldProperties(QDialog, Ui_Dialog):

    def __init__(self, parent=None, fld=None, table=None, db=None):
        QDialog.__init__(self, parent)
        self.fld = fld
        self.table = self.fld.table() if self.fld and self.fld.table() else table
        self.db = self.table.database() if self.table and self.table.database() else db
        self.setupUi(self)

        for item in self.db.connector.fieldTypes():
            self.cboType.addItem(item)

        supportCom = self.db.supportsComment()
        if not supportCom:
            self.label_6.setVisible(False)
            self.editCom.setVisible(False)

        self.setField(fld)

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
        tab = self.table.name
        field = fld.name
        res = self.db.connector.getComment(tab, field)
        self.editCom.setText(res)  # Set comment value

    def getField(self, newCopy=False):
        fld = TableField(self.table) if not self.fld or newCopy else self.fld
        fld.name = self.editName.text()
        fld.dataType = self.cboType.currentText()
        fld.notNull = not self.chkNull.isChecked()
        fld.default = self.editDefault.text()
        fld.hasDefault = fld.default != ""
        fld.comment = self.editCom.text()
        # length field also used for geometry definition, so we should
        # not cast its value to int
        if self.editLength.text() != "":
            fld.modifier = self.editLength.text()
        else:
            fld.modifier = None
        return fld

    def onOK(self):
        """first check whether everything's fine"""
        fld = self.getField(True)  # don't change the original copy
        if fld.name == "":
            QMessageBox.critical(
                self, self.tr("DB Manager"), self.tr("Field name must not be empty.")
            )
            return
        if fld.dataType == "":
            QMessageBox.critical(
                self, self.tr("DB Manager"), self.tr("Field type must not be empty.")
            )
            return

        self.accept()
