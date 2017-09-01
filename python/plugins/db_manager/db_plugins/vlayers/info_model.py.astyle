# -*- coding: utf-8 -*-

"""
/***************************************************************************
Name                 : Virtual layers plugin for DB Manager
Date                 : December 2015
copyright            : (C) 2015 by Hugo Mercier
email                : hugo dot mercier at oslandia dot com

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

from qgis.PyQt.QtWidgets import QApplication

from ..info_model import DatabaseInfo
from ..html_elems import HtmlTable


class LDatabaseInfo(DatabaseInfo):

    def __init__(self, db):
        self.db = db

    def connectionDetails(self):
        tbl = [
        ]
        return HtmlTable(tbl)

    def generalInfo(self):
        self.db.connector.getInfo()
        tbl = [
            (QApplication.translate("DBManagerPlugin", "SQLite version:"), "3")
        ]
        return HtmlTable(tbl)

    def privilegesDetails(self):
        return None
