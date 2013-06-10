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

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from .db_plugins.data_model import TableFieldsModel, TableConstraintsModel, TableIndexesModel
from .db_plugins.plugin import BaseError
from .dlg_db_error import DlgDbError

from .dlg_field_properties import DlgFieldProperties
from .dlg_add_geometry_column import DlgAddGeometryColumn
from .dlg_create_constraint import DlgCreateConstraint
from .dlg_create_index import DlgCreateIndex

from .ui.ui_DlgTableProperties import Ui_DbManagerDlgTableProperties as Ui_Dialog

class DlgTableProperties(QDialog, Ui_Dialog):
	def __init__(self, table, parent=None):
		QDialog.__init__(self, parent)
		self.table = table
		self.setupUi(self)

		self.db = self.table.database()

		m = TableFieldsModel(self)
		self.viewFields.setModel(m)

		m = TableConstraintsModel(self)
		self.viewConstraints.setModel(m)

		m = TableIndexesModel(self)
		self.viewIndexes.setModel(m)

		self.connect(self.btnAddColumn, SIGNAL("clicked()"), self.addColumn)
		self.connect(self.btnAddGeometryColumn, SIGNAL("clicked()"), self.addGeometryColumn)
		self.connect(self.btnEditColumn, SIGNAL("clicked()"), self.editColumn)
		self.connect(self.btnDeleteColumn, SIGNAL("clicked()"), self.deleteColumn)

		self.connect(self.btnAddConstraint, SIGNAL("clicked()"), self.addConstraint)
		self.connect(self.btnDeleteConstraint, SIGNAL("clicked()"), self.deleteConstraint)

		self.connect(self.btnAddIndex, SIGNAL("clicked()"), self.createIndex)
		self.connect(self.btnAddSpatialIndex, SIGNAL("clicked()"), self.createSpatialIndex)
		self.connect(self.btnDeleteIndex, SIGNAL("clicked()"), self.deleteIndex)

		self.populateViews()
		self.checkSupports()


	def checkSupports(self):
		allowEditColumns = self.db.connector.hasTableColumnEditingSupport()
		self.btnEditColumn.setEnabled(allowEditColumns)
		self.btnDeleteColumn.setEnabled(allowEditColumns)

		allowSpatial = self.db.connector.hasSpatialSupport()
		self.btnAddGeometryColumn.setEnabled(allowSpatial)
		self.btnAddSpatialIndex.setEnabled(allowSpatial)


	def populateViews(self):
		self.populateFields()
		self.populateConstraints()
		self.populateIndexes()


	def populateFields(self):
		""" load field information from database """

		m = self.viewFields.model()
		m.clear()

		for fld in self.table.fields():
			m.append( fld )

		for col in range(4):
			self.viewFields.resizeColumnToContents(col)

	def currentColumn(self):
		""" returns row index of selected column """
		sel = self.viewFields.selectionModel()
		indexes = sel.selectedRows()
		if len(indexes) == 0:
			QMessageBox.information(self, self.tr("Sorry"), self.tr("nothing selected"))
			return -1
		return indexes[0].row()

	def addColumn(self):
		""" open dialog to set column info and add column to table """
		dlg = DlgFieldProperties(self, None, self.table)
		if not dlg.exec_():
			return
		fld = dlg.getField()

		QApplication.setOverrideCursor(Qt.WaitCursor)
		self.emit(SIGNAL("aboutToChangeTable()"))
		try:
			# add column to table
			self.table.addField(fld)
			self.populateViews()
		except BaseError, e:
			DlgDbError.showError(e, self)
			return
		finally:
			QApplication.restoreOverrideCursor()

	def addGeometryColumn(self):
		""" open dialog to add geometry column """
		dlg = DlgAddGeometryColumn(self, self.table)
		if not dlg.exec_():
			return
		self.populateViews()

	def editColumn(self):
		""" open dialog to change column info and alter table appropriately """
		index = self.currentColumn()
		if index == -1:
			return

		m = self.viewFields.model()
		# get column in table
		# (there can be missing number if someone deleted a column)
		fld = m.getObject(index)

		dlg = DlgFieldProperties(self, fld, self.table)
		if not dlg.exec_():
			return
		new_fld = dlg.getField(True)

		QApplication.setOverrideCursor(Qt.WaitCursor)
		self.emit(SIGNAL("aboutToChangeTable()"))
		try:
			fld.update(new_fld.name, new_fld.type2String(), new_fld.notNull, new_fld.default2String())
			self.populateViews()
		except BaseError, e:
			DlgDbError.showError(e, self)
			return
		finally:
			QApplication.restoreOverrideCursor()

	def deleteColumn(self):
		""" delete currently selected column """
		index = self.currentColumn()
		if index == -1:
			return

		m = self.viewFields.model()
		fld = m.getObject(index)

		res = QMessageBox.question(self, self.tr("Are you sure"), self.tr("really delete column '%s'?") % fld.name, QMessageBox.Yes | QMessageBox.No)
		if res != QMessageBox.Yes:
			return

		QApplication.setOverrideCursor(Qt.WaitCursor)
		self.emit(SIGNAL("aboutToChangeTable()"))
		try:
			fld.delete()
			self.populateViews()
		except BaseError, e:
			DlgDbError.showError(e, self)
			return
		finally:
			QApplication.restoreOverrideCursor()


	def populateConstraints(self):
		constraints = self.table.constraints()
		if constraints == None:
			self.hideConstraints()	# not supported
			return

		m = self.viewConstraints.model()
		m.clear()

		for constr in constraints:
			m.append( constr )

		for col in range(3):
			self.viewConstraints.resizeColumnToContents(col)

	def hideConstraints(self):
		index = self.tabs.indexOf(self.tabConstraints)
		if index >= 0:
			self.tabs.setTabEnabled(index, False)

	def addConstraint(self):
		""" add primary key or unique constraint """

		dlg = DlgCreateConstraint(self, self.table)
		if not dlg.exec_():
			return
		self.populateViews()

	def deleteConstraint(self):
		""" delete a constraint """

		index = self.currentConstraint()
		if index == -1:
			return

		m = self.viewConstraints.model()
		constr = m.getObject(index)

		res = QMessageBox.question(self, self.tr("Are you sure"), self.tr("really delete constraint '%s'?") % constr.name, QMessageBox.Yes | QMessageBox.No)
		if res != QMessageBox.Yes:
			return

		QApplication.setOverrideCursor(Qt.WaitCursor)
		self.emit(SIGNAL("aboutToChangeTable()"))
		try:
			constr.delete()
			self.populateViews()
		except BaseError, e:
			DlgDbError.showError(e, self)
			return
		finally:
			QApplication.restoreOverrideCursor()

	def currentConstraint(self):
		""" returns row index of selected index """
		sel = self.viewConstraints.selectionModel()
		indexes = sel.selectedRows()
		if len(indexes) == 0:
			QMessageBox.information(self, self.tr("Sorry"), self.tr("nothing selected"))
			return -1
		return indexes[0].row()


	def populateIndexes(self):
		indexes = self.table.indexes()
		if indexes == None:
			self.hideIndexes()
			return

		m = self.viewIndexes.model()
		m.clear()

		for idx in indexes:
			m.append( idx )

		for col in range(2):
			self.viewIndexes.resizeColumnToContents(col)

	def hideIndexes(self):
		index = self.tabs.indexOf(self.tabIndexes)
		if index >= 0:
			self.tabs.setTabEnabled(index, False)

	def createIndex(self):
		""" create an index """
		dlg = DlgCreateIndex(self, self.table)
		if not dlg.exec_():
			return
		self.populateViews()

	def createSpatialIndex(self):
		""" create spatial index for the geometry column """
		if self.table.type != self.table.VectorType:
			QMessageBox.information(self, self.tr("Sorry"), self.tr("The selected table has no geometry"))
			return

		res = QMessageBox.question(self, self.tr("Create?"), self.tr("Create spatial index for field %s?") % self.table.geomColumn, QMessageBox.Yes | QMessageBox.No)
		if res != QMessageBox.Yes:
			return

		# TODO: first check whether the index doesn't exist already
		QApplication.setOverrideCursor(Qt.WaitCursor)
		self.emit(SIGNAL("aboutToChangeTable()"))

		try:
			self.table.createSpatialIndex()
			self.populateViews()
		except BaseError, e:
			DlgDbError.showError(e, self)
			return
		finally:
			QApplication.restoreOverrideCursor()

	def currentIndex(self):
		""" returns row index of selected index """
		sel = self.viewIndexes.selectionModel()
		indexes = sel.selectedRows()
		if len(indexes) == 0:
			QMessageBox.information(self, self.tr("Sorry"), self.tr("Nothing selected"))
			return -1
		return indexes[0].row()

	def deleteIndex(self):
		""" delete currently selected index """
		index = self.currentIndex()
		if index == -1:
			return

		m = self.viewIndexes.model()
		idx = m.getObject(index)

		res = QMessageBox.question(self, self.tr("Are you sure"), self.tr("really delete index '%s'?") % idx.name, QMessageBox.Yes | QMessageBox.No)
		if res != QMessageBox.Yes:
			return

		QApplication.setOverrideCursor(Qt.WaitCursor)
		self.emit(SIGNAL("aboutToChangeTable()"))
		try:
			idx.delete()
			self.populateViews()
		except BaseError, e:
			DlgDbError.showError(e, self)
			return
		finally:
			QApplication.restoreOverrideCursor()


