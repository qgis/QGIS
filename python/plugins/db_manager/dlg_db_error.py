"""
/***************************************************************************
Name                 : DB Manager
Description          : Database manager plugin for QGIS
Date                 : May 23, 2011
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
from qgis.PyQt.QtWidgets import QDialog

from .gui_utils import GuiUtils
from .db_plugins.plugin import DbError

Ui_Dialog, _ = uic.loadUiType(GuiUtils.get_ui_file_path('DlgDbError.ui'))


class DlgDbError(QDialog, Ui_Dialog):

    def __init__(self, e, parent=None):
        QDialog.__init__(self, parent)
        self.setupUi(self)

        def sanitize(txt):
            return "" if txt is None else "<pre>" + txt.replace('<', '&lt;') + "</pre>"

        if isinstance(e, DbError):
            self.setQueryMessage(sanitize(e.msg), sanitize(e.query))
        else:
            self.setMessage(sanitize(e.msg))

    def setMessage(self, msg):
        self.txtErrorMsg.setHtml(msg)
        self.stackedWidget.setCurrentIndex(0)

    def setQueryMessage(self, msg, query):
        self.txtQueryErrorMsg.setHtml(msg)
        self.txtQuery.setHtml(query)
        self.stackedWidget.setCurrentIndex(1)

    @staticmethod
    def showError(e, parent=None):
        dlg = DlgDbError(e, parent)
        dlg.exec()
