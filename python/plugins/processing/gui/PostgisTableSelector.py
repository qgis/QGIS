# -*- coding: utf-8 -*-

"""
***************************************************************************
    PostgisTableSelector.py
    ---------------------
    Date                 : November 2015
    Copyright            : (C) 2015 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'November 2015'
__copyright__ = '(C) 2015, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


import os
from qgis.PyQt.QtCore import QSettings
from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtWidgets import QTreeWidgetItem, QMessageBox
from qgis.PyQt import uic
from qgis.core import QgsDataSourceURI, QgsCredentials
from processing.tools.postgis import GeoDB

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'DlgPostgisTableSelector.ui'))


class PostgisTableSelector(BASE, WIDGET):

    def __init__(self, parent, tablename):
        super(PostgisTableSelector, self).__init__(parent)
        self.connection = None
        self.table = None
        self.schema = None
        self.setupUi(self)
        settings = QSettings()
        settings.beginGroup('/PostgreSQL/connections/')
        names = settings.childGroups()
        settings.endGroup()
        for n in names:
            item = ConnectionItem(n)
            self.treeConnections.addTopLevelItem(item)

        def itemExpanded(item):
            try:
                item.populateSchemas()
            except:
                pass

        self.treeConnections.itemExpanded.connect(itemExpanded)

        self.textTableName.setText(tablename)

        self.buttonBox.accepted.connect(self.okPressed)
        self.buttonBox.rejected.connect(self.cancelPressed)

    def cancelPressed(self):
        self.close()

    def okPressed(self):
        if self.textTableName.text().strip() == "":
            self.textTableName.setStyleSheet("QLineEdit{background: yellow}")
            return
        item = self.treeConnections.currentItem()
        if isinstance(item, ConnectionItem):
            QMessageBox.warning(self, "Wrong selection", "Select a schema item in the tree")
            return
        self.schema = item.text(0)
        self.table = self.textTableName.text().strip()
        self.connection = item.parent().text(0)
        self.close()


class ConnectionItem(QTreeWidgetItem):

    def __init__(self, connection):
        self.connIcon = QIcon(os.path.dirname(__file__) + '/../images/postgis.png')
        self.schemaIcon = QIcon(os.path.dirname(__file__) + '/../images/namespace.png')

        QTreeWidgetItem.__init__(self)
        self.setChildIndicatorPolicy(QTreeWidgetItem.ShowIndicator)
        self.connection = connection
        self.setText(0, connection)
        self.setIcon(0, self.connIcon)

    def populateSchemas(self):
        if self.childCount() != 0:
            return
        geodb = GeoDB.from_name(self.connection)
        schemas = geodb.list_schemas()
        for oid, name, owner, perms in schemas:
            item = QTreeWidgetItem()
            item.setText(0, name)
            item.setIcon(0, self.schemaIcon)
            self.addChild(item)
