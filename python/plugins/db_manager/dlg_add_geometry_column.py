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
from .gui_utils import GuiUtils

Ui_Dialog, _ = uic.loadUiType(GuiUtils.get_ui_file_path('DlgAddGeometryColumn.ui'))


class DlgAddGeometryColumn(QDialog, Ui_Dialog):
    GEOM_TYPES = ["POINT", "LINESTRING", "POLYGON", "MULTIPOINT", "MULTILINESTRING", "MULTIPOLYGON",
                  "GEOMETRYCOLLECTION"]

    def __init__(self, parent=None, table=None, db=None):
        QDialog.__init__(self, parent)
        self.table = table
        self.db = self.table.database() if self.table and self.table.database() else db
        self.setupUi(self)

        self.buttonBox.accepted.connect(self.createGeomColumn)

    def createGeomColumn(self):
        """ first check whether everything's fine """
        if self.editName.text() == "":
            QMessageBox.critical(self, self.tr("DB Manager"), self.tr("Field name must not be empty."))
            return

        name = self.editName.text()
        geom_type = self.GEOM_TYPES[self.cboType.currentIndex()]
        dim = self.spinDim.value()
        try:
            srid = int(self.editSrid.text())
        except ValueError:
            srid = -1
        createSpatialIndex = False

        # now create the geometry column
        with OverrideCursor(Qt.CursorShape.WaitCursor):
            try:
                self.table.addGeometryColumn(name, geom_type, srid, dim, createSpatialIndex)
            except DbError as e:
                DlgDbError.showError(e, self)
                return

        self.accept()
