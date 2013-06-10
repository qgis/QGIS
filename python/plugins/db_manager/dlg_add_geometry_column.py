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

from .db_plugins.plugin import DbError
from .dlg_db_error import DlgDbError
from .db_plugins.plugin import TableField

from .ui.ui_DlgAddGeometryColumn import Ui_DbManagerDlgAddGeometryColumn as Ui_Dialog

class DlgAddGeometryColumn(QDialog, Ui_Dialog):

	GEOM_TYPES = ["POINT", "LINESTRING", "POLYGON", "MULTIPOINT", "MULTILINESTRING", "MULTIPOLYGON", "GEOMETRYCOLLECTION"]

	def __init__(self, parent=None, table=None, db=None):
		QDialog.__init__(self, parent)
		self.table = table
		self.db = self.table.database() if self.table and self.table.database() else db
		self.setupUi(self)

		self.connect(self.buttonBox, SIGNAL("accepted()"), self.createGeomColumn)

	def createGeomColumn(self):
		""" first check whether everything's fine """
		if self.editName.text() == "":
			QMessageBox.critical(self, self.tr("Sorry"), self.tr("field name must not be empty"))
			return

		name = self.editName.text()
		geom_type = self.GEOM_TYPES[ self.cboType.currentIndex() ]
		dim = self.spinDim.value()
		try:
			srid = int(self.editSrid.text())
		except ValueError:
			srid = -1
		createSpatialIndex = False

		# now create the geometry column
		QApplication.setOverrideCursor(Qt.WaitCursor)
		try:
			self.table.addGeometryColumn(name, geom_type, srid, dim, createSpatialIndex)
		except DbError, e:
			DlgDbError.showError(e, self)
			return
		finally:
			QApplication.restoreOverrideCursor()

		self.accept()
