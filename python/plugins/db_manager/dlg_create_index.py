# -*- coding: utf-8 -*-

"""
/***************************************************************************
Name                 : DB Manager
Description          : Database manager plugin for QuantumGIS
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

from .db_plugins.plugin import DbError
from .dlg_db_error import DlgDbError
from .db_plugins.plugin import TableIndex

from .ui.ui_DlgCreateIndex import Ui_DbManagerDlgCreateIndex as Ui_Dialog

class DlgCreateIndex(QDialog, Ui_Dialog):
	def __init__(self, parent=None, table=None, db=None):
		QDialog.__init__(self, parent)
		self.table = table
		self.db = self.table.database() if self.table and self.table.database() else db
		self.setupUi(self)

		self.connect(self.buttonBox, SIGNAL("accepted()"), self.createIndex)

		self.connect(self.cboColumn, SIGNAL("currentIndexChanged(int)"), self.columnChanged)
		self.populateColumns()


	def populateColumns(self):
		self.cboColumn.clear()
		for fld in self.table.fields():
			self.cboColumn.addItem(fld.name)

	def columnChanged(self):
		self.editName.setText(u"idx_%s_%s" % (self.table.name, self.cboColumn.currentText()))


	def createIndex(self):
		idx = self.getIndex()
		if idx.name == "":
			QMessageBox.critical(self, "error", "Please enter some name for the index")
			return

		# now create the index
		QApplication.setOverrideCursor(Qt.WaitCursor)
		try:
			self.table.addIndex(idx)
		except DbError, e:
			DlgDbError.showError(e, self)
			return
		finally:
			QApplication.restoreOverrideCursor()

		self.accept()

	def getIndex(self):
		idx = TableIndex(self.table)
		idx.name = self.editName.text()
		idx.columns = []
		colname = self.cboColumn.currentText()
		for fld in self.table.fields():
			if fld.name == colname:
				idx.columns.append( fld.num )
				break
		return idx

