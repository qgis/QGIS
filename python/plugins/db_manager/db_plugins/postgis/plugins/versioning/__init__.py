"""
/***************************************************************************
Name                 : Versioning plugin for DB Manager
Description          : Set up versioning support for a table
Date                 : Mar 12, 2012
copyright            : (C) 2012 by Giuseppe Sucameli
email                : brush.tyler@gmail.com
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

from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtWidgets import QAction, QApplication
from qgis.PyQt.QtGui import QIcon


# The load function is called when the "db" database or either one of its
# children db objects (table o schema) is selected by the user.
# @param db is the selected database
# @param mainwindow is the DBManager mainwindow


def load(db, mainwindow):
    # add the action to the DBManager menu
    action = QAction(
        QIcon(), QApplication.translate("DBManagerPlugin", "&Change Logging…"), db
    )
    mainwindow.registerAction(
        action, QApplication.translate("DBManagerPlugin", "&Table"), run
    )


# The run function is called once the user clicks on the action TopoViewer
# (look above at the load function) from the DBManager menu/toolbar.
# @param item is the selected db item (either db, schema or table)
# @param action is the clicked action on the DBManager menu/toolbar
# @param mainwindow is the DBManager mainwindow
def run(item, action, mainwindow):
    from .dlg_versioning import DlgVersioning

    dlg = DlgVersioning(item, mainwindow)

    QApplication.restoreOverrideCursor()
    try:
        dlg.exec()
    finally:
        QApplication.setOverrideCursor(Qt.CursorShape.WaitCursor)
