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
from qgis.PyQt.QtCore import Qt, pyqtSignal
from qgis.PyQt.QtWidgets import QDialog, QMessageBox, QApplication

from qgis.utils import OverrideCursor

from .db_plugins.data_model import (
    TableFieldsModel,
    TableConstraintsModel,
    TableIndexesModel,
)
from .db_plugins.plugin import BaseError, DbError
from .dlg_db_error import DlgDbError

from .dlg_field_properties import DlgFieldProperties
from .dlg_add_geometry_column import DlgAddGeometryColumn
from .dlg_create_constraint import DlgCreateConstraint
from .dlg_create_index import DlgCreateIndex
from .gui_utils import GuiUtils


Ui_Dialog, _ = uic.loadUiType(GuiUtils.get_ui_file_path("DlgTableProperties.ui"))


class DlgTableProperties(QDialog, Ui_Dialog):
    aboutToChangeTable = pyqtSignal()

    def __init__(self, table, parent=None):
        QDialog.__init__(self, parent)
        self.table = table
        self.setupUi(self)

        self.db = self.table.database()

        supportCom = self.db.supportsComment()
        if not supportCom:
            self.tabs.removeTab(3)

        m = TableFieldsModel(self)
        self.viewFields.setModel(m)

        m = TableConstraintsModel(self)
        self.viewConstraints.setModel(m)

        m = TableIndexesModel(self)
        self.viewIndexes.setModel(m)

        # Display comment in line edit
        m = self.table.comment
        self.viewComment.setPlainText(m)

        self.btnAddColumn.clicked.connect(self.addColumn)
        self.btnAddGeometryColumn.clicked.connect(self.addGeometryColumn)
        self.btnEditColumn.clicked.connect(self.editColumn)
        self.btnDeleteColumn.clicked.connect(self.deleteColumn)

        self.btnAddConstraint.clicked.connect(self.addConstraint)
        self.btnDeleteConstraint.clicked.connect(self.deleteConstraint)

        self.btnAddIndex.clicked.connect(self.createIndex)
        self.btnAddSpatialIndex.clicked.connect(self.createSpatialIndex)
        self.btnDeleteIndex.clicked.connect(self.deleteIndex)

        # Connect button add Comment to function
        self.btnAddComment.clicked.connect(self.createComment)
        # Connect button delete Comment to function
        self.btnDeleteComment.clicked.connect(self.deleteComment)

        self.refresh()

    def refresh(self):
        self.populateViews()
        self.checkSupports()

    def checkSupports(self):
        allowEditColumns = self.db.connector.hasTableColumnEditingSupport()
        self.btnEditColumn.setEnabled(allowEditColumns)
        self.btnDeleteColumn.setEnabled(allowEditColumns)

        self.btnAddGeometryColumn.setEnabled(
            self.db.connector.canAddGeometryColumn(
                (self.table.schemaName(), self.table.name)
            )
        )
        self.btnAddSpatialIndex.setEnabled(
            self.db.connector.canAddSpatialIndex(
                (self.table.schemaName(), self.table.name)
            )
        )

    def populateViews(self):
        self.populateFields()
        self.populateConstraints()
        self.populateIndexes()

    def populateFields(self):
        """load field information from database"""
        m = self.viewFields.model()
        m.clear()

        for fld in self.table.fields():
            m.append(fld)

        for col in range(4):
            self.viewFields.resizeColumnToContents(col)

    def currentColumn(self):
        """returns row index of selected column"""
        sel = self.viewFields.selectionModel()
        indexes = sel.selectedRows()
        if len(indexes) == 0:
            QMessageBox.information(
                self, self.tr("DB Manager"), self.tr("No columns were selected.")
            )
            return -1
        return indexes[0].row()

    def addColumn(self):
        """open dialog to set column info and add column to table"""
        dlg = DlgFieldProperties(self, None, self.table)
        if not dlg.exec():
            return
        fld = dlg.getField()

        with OverrideCursor(Qt.CursorShape.WaitCursor):
            self.aboutToChangeTable.emit()
            try:
                # add column to table
                self.table.addField(fld)
                self.refresh()
            except BaseError as e:
                DlgDbError.showError(e, self)

    def addGeometryColumn(self):
        """open dialog to add geometry column"""
        dlg = DlgAddGeometryColumn(self, self.table)
        if not dlg.exec():
            return
        self.refresh()

    def editColumn(self):
        """open dialog to change column info and alter table appropriately"""
        index = self.currentColumn()
        if index == -1:
            return

        m = self.viewFields.model()
        # get column in table
        # (there can be missing number if someone deleted a column)
        fld = m.getObject(index)

        dlg = DlgFieldProperties(self, fld, self.table)
        if not dlg.exec():
            return
        new_fld = dlg.getField(True)

        with OverrideCursor(Qt.CursorShape.WaitCursor):
            self.aboutToChangeTable.emit()
            try:
                fld.update(
                    new_fld.name,
                    new_fld.type2String(),
                    new_fld.notNull,
                    new_fld.default2String(),
                    new_fld.comment,
                )
                self.refresh()
            except BaseError as e:
                DlgDbError.showError(e, self)

    def deleteColumn(self):
        """Deletes currently selected column"""
        index = self.currentColumn()
        if index == -1:
            return

        m = self.viewFields.model()
        fld = m.getObject(index)

        res = QMessageBox.question(
            self,
            self.tr("Delete Column"),
            self.tr("Are you sure you want to delete column '{0}'?").format(fld.name),
            QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
        )
        if res != QMessageBox.StandardButton.Yes:
            return

        with OverrideCursor(Qt.CursorShape.WaitCursor):
            self.aboutToChangeTable.emit()
            try:
                fld.delete()
                self.refresh()
            except BaseError as e:
                DlgDbError.showError(e, self)

    def populateConstraints(self):
        constraints = self.table.constraints()
        if constraints is None:
            self.hideConstraints()  # not supported
            return

        m = self.viewConstraints.model()
        m.clear()

        for constr in constraints:
            m.append(constr)

        for col in range(3):
            self.viewConstraints.resizeColumnToContents(col)

    def hideConstraints(self):
        index = self.tabs.indexOf(self.tabConstraints)
        if index >= 0:
            self.tabs.setTabEnabled(index, False)

    def addConstraint(self):
        """Adds primary key or unique constraint"""

        dlg = DlgCreateConstraint(self, self.table)
        if not dlg.exec():
            return
        self.refresh()

    def deleteConstraint(self):
        """Deletes a constraint"""

        index = self.currentConstraint()
        if index == -1:
            return

        m = self.viewConstraints.model()
        constr = m.getObject(index)

        res = QMessageBox.question(
            self,
            self.tr("Delete Constraint"),
            self.tr("Are you sure you want to delete constraint '{0}'?").format(
                constr.name
            ),
            QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
        )
        if res != QMessageBox.StandardButton.Yes:
            return

        with OverrideCursor(Qt.CursorShape.WaitCursor):
            self.aboutToChangeTable.emit()
            try:
                constr.delete()
                self.refresh()
            except BaseError as e:
                DlgDbError.showError(e, self)

    def currentConstraint(self):
        """returns row index of selected index"""
        sel = self.viewConstraints.selectionModel()
        indexes = sel.selectedRows()
        if len(indexes) == 0:
            QMessageBox.information(
                self, self.tr("DB Manager"), self.tr("No constraints were selected.")
            )
            return -1
        return indexes[0].row()

    def populateIndexes(self):
        indexes = self.table.indexes()
        if indexes is None:
            self.hideIndexes()
            return

        m = self.viewIndexes.model()
        m.clear()

        for idx in indexes:
            m.append(idx)

        for col in range(2):
            self.viewIndexes.resizeColumnToContents(col)

    def hideIndexes(self):
        index = self.tabs.indexOf(self.tabIndexes)
        if index >= 0:
            self.tabs.setTabEnabled(index, False)

    def createIndex(self):
        """Creates an index"""
        dlg = DlgCreateIndex(self, self.table)
        if not dlg.exec():
            return
        self.refresh()

    def createSpatialIndex(self):
        """Creates spatial index for the geometry column"""
        if self.table.type != self.table.VectorType:
            QMessageBox.information(
                self,
                self.tr("DB Manager"),
                self.tr("The selected table has no geometry."),
            )
            return

        res = QMessageBox.question(
            self,
            self.tr("Create Spatial Index"),
            self.tr("Create spatial index for field {0}?").format(
                self.table.geomColumn
            ),
            QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
        )
        if res != QMessageBox.StandardButton.Yes:
            return

        # TODO: first check whether the index doesn't exist already
        with OverrideCursor(Qt.CursorShape.WaitCursor):
            self.aboutToChangeTable.emit()

            try:
                self.table.createSpatialIndex()
                self.refresh()
            except BaseError as e:
                DlgDbError.showError(e, self)

    def currentIndex(self):
        """returns row index of selected index"""
        sel = self.viewIndexes.selectionModel()
        indexes = sel.selectedRows()
        if len(indexes) == 0:
            QMessageBox.information(
                self, self.tr("DB Manager"), self.tr("No indices were selected.")
            )
            return -1
        return indexes[0].row()

    def deleteIndex(self):
        """Deletes currently selected index"""
        index = self.currentIndex()
        if index == -1:
            return

        m = self.viewIndexes.model()
        idx = m.getObject(index)

        res = QMessageBox.question(
            self,
            self.tr("Delete Index"),
            self.tr("Are you sure you want to delete index '{0}'?").format(idx.name),
            QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
        )
        if res != QMessageBox.StandardButton.Yes:
            return

        with OverrideCursor(Qt.CursorShape.WaitCursor):
            self.aboutToChangeTable.emit()
            try:
                idx.delete()
                self.refresh()
            except BaseError as e:
                DlgDbError.showError(e, self)

    def createComment(self):
        """Adds a comment to the selected table"""
        try:
            schem = self.table.schema().name
            tab = self.table.name
            com = self.viewComment.toPlainText()
            self.db.connector.commentTable(schem, tab, com)
            self.table.comment = com
        except DbError as e:
            DlgDbError.showError(e, self)
            return
        self.refresh()
        # Display successful message
        QMessageBox.information(
            self, self.tr("Add comment"), self.tr("Table successfully commented")
        )

    def deleteComment(self):
        """Drops the comment on the selected table"""
        try:
            schem = self.table.schema().name
            tab = self.table.name
            self.db.connector.commentTable(schem, tab)
        except DbError as e:
            DlgDbError.showError(e, self)
            return
        self.refresh()
        # Refresh line edit, put a void comment
        self.viewComment.setPlainText("")
        self.table.comment = ""
        # Display successful message
        QMessageBox.information(
            self, self.tr("Delete comment"), self.tr("Comment deleted")
        )
