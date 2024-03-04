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
from qgis.PyQt.QtCore import Qt, QModelIndex
from qgis.PyQt.QtWidgets import QItemDelegate, QComboBox, QDialog, QPushButton, QDialogButtonBox, QMessageBox, QApplication
from qgis.PyQt.QtCore import QItemSelectionModel, pyqtSignal

from qgis.utils import OverrideCursor

from .db_plugins.data_model import TableFieldsModel
from .db_plugins.plugin import DbError, ConnectionError
from .dlg_db_error import DlgDbError
from .gui_utils import GuiUtils

Ui_Dialog, _ = uic.loadUiType(GuiUtils.get_ui_file_path('DlgCreateTable.ui'))


class TableFieldsDelegate(QItemDelegate):
    """ delegate with some special item editors """

    columnNameChanged = pyqtSignal()

    def __init__(self, field_types, parent=None):
        QItemDelegate.__init__(self, parent)
        self.fieldTypes = field_types

    def createEditor(self, parent, option, index):
        # special combobox for field type
        if index.column() == 1:
            cbo = QComboBox(parent)
            cbo.setEditable(True)
            cbo.setFrame(False)
            for item in self.fieldTypes:
                cbo.addItem(item)
            return cbo
        return QItemDelegate.createEditor(self, parent, option, index)

    def setEditorData(self, editor, index):
        """ load data from model to editor """
        m = index.model()
        if index.column() == 1:
            txt = m.data(index, Qt.ItemDataRole.DisplayRole)
            editor.setEditText(txt)
        else:
            # use default
            QItemDelegate.setEditorData(self, editor, index)

    def setModelData(self, editor, model, index):
        """ save data from editor back to model """
        if index.column() == 1:
            model.setData(index, editor.currentText())
        else:
            # use default
            QItemDelegate.setModelData(self, editor, model, index)
            if index.column() == 0:
                self.columnNameChanged.emit()


class DlgCreateTable(QDialog, Ui_Dialog):
    GEOM_TYPES = ["POINT", "LINESTRING", "POLYGON", "MULTIPOINT", "MULTILINESTRING", "MULTIPOLYGON",
                  "GEOMETRYCOLLECTION"]

    def __init__(self, item, parent=None):
        QDialog.__init__(self, parent)
        self.item = item
        self.setupUi(self)

        self.db = self.item.database()
        self.schemas = self.db.schemas()
        self.hasSchemas = self.schemas is not None
        self.fieldTypes = self.db.connector.fieldTypes()

        m = TableFieldsModel(self, True)  # it's editable
        self.fields.setModel(m)
        self.fields.setColumnHidden(3, True)  # hide Default column

        d = TableFieldsDelegate(self.fieldTypes, self)
        self.fields.setItemDelegate(d)

        self.fields.setColumnWidth(0, 140)
        self.fields.setColumnWidth(1, 140)
        self.fields.setColumnWidth(2, 50)

        b = QPushButton(self.tr("&Create"))
        self.buttonBox.addButton(b, QDialogButtonBox.ButtonRole.ActionRole)

        self.btnAddField.clicked.connect(self.addField)
        self.btnDeleteField.clicked.connect(self.deleteField)
        self.btnFieldUp.clicked.connect(self.fieldUp)
        self.btnFieldDown.clicked.connect(self.fieldDown)
        b.clicked.connect(self.createTable)

        self.chkGeomColumn.clicked.connect(self.updateUi)

        self.fields.selectionModel().selectionChanged.connect(self.updateUiFields)
        d.columnNameChanged.connect(self.updatePkeyCombo)

        self.populateSchemas()
        self.updateUi()
        self.updateUiFields()

    def populateSchemas(self):
        self.cboSchema.clear()
        if not self.hasSchemas:
            self.hideSchemas()
            return

        index = -1
        for schema in self.schemas:
            self.cboSchema.addItem(schema.name)
            if hasattr(self.item, 'schema') and schema.name == self.item.schema().name:
                index = self.cboSchema.count() - 1
        self.cboSchema.setCurrentIndex(index)

    def hideSchemas(self):
        self.cboSchema.setEnabled(False)

    def updateUi(self):
        useGeom = self.chkGeomColumn.isChecked()
        self.cboGeomType.setEnabled(useGeom)
        self.editGeomColumn.setEnabled(useGeom)
        self.spinGeomDim.setEnabled(useGeom)
        self.editGeomSrid.setEnabled(useGeom)
        self.chkSpatialIndex.setEnabled(useGeom)

    def updateUiFields(self):
        fld = self.selectedField()
        if fld is not None:
            up_enabled = (fld != 0)
            down_enabled = (fld != self.fields.model().rowCount() - 1)
            del_enabled = True
        else:
            up_enabled, down_enabled, del_enabled = False, False, False
        self.btnFieldUp.setEnabled(up_enabled)
        self.btnFieldDown.setEnabled(down_enabled)
        self.btnDeleteField.setEnabled(del_enabled)

    def updatePkeyCombo(self, selRow=None):
        """ called when list of columns changes. if 'sel' is None, it keeps current index """

        if selRow is None:
            selRow = self.cboPrimaryKey.currentIndex()

        self.cboPrimaryKey.clear()

        m = self.fields.model()
        for row in range(m.rowCount()):
            name = m.data(m.index(row, 0))
            self.cboPrimaryKey.addItem(name)

        self.cboPrimaryKey.setCurrentIndex(selRow)

    def addField(self):
        """Adds new field to the end of field table """
        m = self.fields.model()
        newRow = m.rowCount()
        m.insertRows(newRow, 1)

        indexName = m.index(newRow, 0, QModelIndex())
        indexType = m.index(newRow, 1, QModelIndex())
        indexNull = m.index(newRow, 2, QModelIndex())

        m.setData(indexName, "new_field")
        colType = self.fieldTypes[0]
        if newRow == 0:
            # adding the first row, use auto-incrementing column type if any
            if "serial" in self.fieldTypes:  # PostgreSQL
                colType = "serial"
        m.setData(indexType, colType)
        m.setData(indexNull, None, Qt.ItemDataRole.DisplayRole)
        m.setData(indexNull, Qt.CheckState.Unchecked, Qt.ItemDataRole.CheckStateRole)

        # selects the new row
        sel = self.fields.selectionModel()
        sel.select(indexName, QItemSelectionModel.SelectionFlag.Rows | QItemSelectionModel.SelectionFlag.ClearAndSelect)

        # starts editing
        self.fields.edit(indexName)

        self.updatePkeyCombo(0 if newRow == 0 else None)

    def selectedField(self):
        sel = self.fields.selectedIndexes()
        if len(sel) < 1:
            return None
        return sel[0].row()

    def deleteField(self):
        """Deletes selected field """
        row = self.selectedField()
        if row is None:
            QMessageBox.information(self, self.tr("DB Manager"), self.tr("No field selected."))
        else:
            self.fields.model().removeRows(row, 1)

        self.updatePkeyCombo()

    def fieldUp(self):
        """ move selected field up """
        row = self.selectedField()
        if row is None:
            QMessageBox.information(self, self.tr("DB Manager"), self.tr("No field selected."))
            return
        if row == 0:
            QMessageBox.information(self, self.tr("DB Manager"), self.tr("Field is already at the top."))
            return

        # take row and reinsert it
        rowdata = self.fields.model().takeRow(row)
        self.fields.model().insertRow(row - 1, rowdata)

        # set selection again
        index = self.fields.model().index(row - 1, 0, QModelIndex())
        self.fields.selectionModel().select(index, QItemSelectionModel.SelectionFlag.Rows | QItemSelectionModel.SelectionFlag.ClearAndSelect)

        self.updatePkeyCombo()

    def fieldDown(self):
        """ move selected field down """
        row = self.selectedField()
        if row is None:
            QMessageBox.information(self, self.tr("DB Manager"), self.tr("No field selected."))
            return
        if row == self.fields.model().rowCount() - 1:
            QMessageBox.information(self, self.tr("DB Manager"), self.tr("Field is already at the bottom."))
            return

        # take row and reinsert it
        rowdata = self.fields.model().takeRow(row)
        self.fields.model().insertRow(row + 1, rowdata)

        # set selection again
        index = self.fields.model().index(row + 1, 0, QModelIndex())
        self.fields.selectionModel().select(index, QItemSelectionModel.SelectionFlag.Rows | QItemSelectionModel.SelectionFlag.ClearAndSelect)

        self.updatePkeyCombo()

    def createTable(self):
        """Creates table with chosen fields, optionally add a geometry column """
        if not self.hasSchemas:
            schema = None
        else:
            schema = str(self.cboSchema.currentText())
            if len(schema) == 0:
                QMessageBox.information(self, self.tr("DB Manager"), self.tr("A valid schema must be selected first."))
                return

        table = str(self.editName.text())
        if len(table) == 0:
            QMessageBox.information(self, self.tr("DB Manager"), self.tr("A valid table name is required."))
            return

        m = self.fields.model()
        if m.rowCount() == 0:
            QMessageBox.information(self, self.tr("DB Manager"), self.tr("At least one field is required."))
            return

        useGeomColumn = self.chkGeomColumn.isChecked()
        if useGeomColumn:
            geomColumn = str(self.editGeomColumn.text())
            if len(geomColumn) == 0:
                QMessageBox.information(self, self.tr("DB Manager"), self.tr("A name is required for the geometry column."))
                return

            geomType = self.GEOM_TYPES[self.cboGeomType.currentIndex()]
            geomDim = self.spinGeomDim.value()
            try:
                geomSrid = int(self.editGeomSrid.text())
            except ValueError:
                geomSrid = 0
            useSpatialIndex = self.chkSpatialIndex.isChecked()

        flds = m.getFields()
        pk_index = self.cboPrimaryKey.currentIndex()
        if pk_index >= 0:
            flds[pk_index].primaryKey = True

        # commit to DB
        with OverrideCursor(Qt.CursorShape.WaitCursor):
            try:
                if not useGeomColumn:
                    self.db.createTable(table, flds, schema)
                else:
                    geom = geomColumn, geomType, geomSrid, geomDim, useSpatialIndex
                    self.db.createVectorTable(table, flds, geom, schema)

            except (ConnectionError, DbError) as e:
                DlgDbError.showError(e, self)

        # clear UI
        self.editName.clear()
        self.fields.model().removeRows(0, self.fields.model().rowCount())
        self.cboPrimaryKey.clear()
        self.chkGeomColumn.setChecked(False)
        self.chkSpatialIndex.setChecked(False)
        self.editGeomSrid.clear()

        self.cboGeomType.setEnabled(False)
        self.editGeomColumn.setEnabled(False)
        self.spinGeomDim.setEnabled(False)
        self.editGeomSrid.setEnabled(False)
        self.chkSpatialIndex.setEnabled(False)

        QMessageBox.information(self, self.tr("DB Manager"), self.tr("Table created successfully."))
