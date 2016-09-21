# -*- coding: utf-8 -*-

"""
/***************************************************************************
Name                 : DB Manager
Description          : Database manager plugin for QGIS
Date                 : May 23, 2011
copyright            : (C) 2011 by Giuseppe Sucameli
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
from builtins import object

from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtWidgets import QAction, QApplication
from qgis.PyQt.QtGui import QIcon

from qgis.core import QgsMapLayerRegistry, QgsMapLayer, QgsDataSourceUri
import re

from . import resources_rc  # NOQA


class DBManagerPlugin(object):

    def __init__(self, iface):
        self.iface = iface
        self.dlg = None

    def initGui(self):
        self.action = QAction(QIcon(":/db_manager/icon"), QApplication.translate("DBManagerPlugin", "DB Manager"),
                              self.iface.mainWindow())
        self.action.setObjectName("dbManager")
        self.action.triggered.connect(self.run)
        # Add toolbar button and menu item
        if hasattr(self.iface, 'addDatabaseToolBarIcon'):
            self.iface.addDatabaseToolBarIcon(self.action)
        else:
            self.iface.addToolBarIcon(self.action)
        if hasattr(self.iface, 'addPluginToDatabaseMenu'):
            self.iface.addPluginToDatabaseMenu(QApplication.translate("DBManagerPlugin", "DB Manager"), self.action)
        else:
            self.iface.addPluginToMenu(QApplication.translate("DBManagerPlugin", "DB Manager"), self.action)

        self.layerAction = QAction(QIcon(":/db_manager/icon"), QApplication.translate("DBManagerPlugin", "Update Sql Layer"),
                                   self.iface.mainWindow())
        self.layerAction.setObjectName("dbManagerUpdateSqlLayer")
        self.layerAction.triggered.connect(self.onUpdateSqlLayer)
        self.iface.legendInterface().addLegendLayerAction(self.layerAction, "", "dbManagerUpdateSqlLayer", QgsMapLayer.VectorLayer, False)
        for l in list(QgsMapLayerRegistry.instance().mapLayers().values()):
            self.onLayerWasAdded(l)
        QgsMapLayerRegistry.instance().layerWasAdded.connect(self.onLayerWasAdded)

    def unload(self):
        # Remove the plugin menu item and icon
        if hasattr(self.iface, 'removePluginDatabaseMenu'):
            self.iface.removePluginDatabaseMenu(QApplication.translate("DBManagerPlugin", "DB Manager"), self.action)
        else:
            self.iface.removePluginMenu(QApplication.translate("DBManagerPlugin", "DB Manager"), self.action)
        if hasattr(self.iface, 'removeDatabaseToolBarIcon'):
            self.iface.removeDatabaseToolBarIcon(self.action)
        else:
            self.iface.removeToolBarIcon(self.action)

        self.iface.legendInterface().removeLegendLayerAction(self.layerAction)
        QgsMapLayerRegistry.instance().layerWasAdded.disconnect(self.onLayerWasAdded)

        if self.dlg is not None:
            self.dlg.close()

    def onLayerWasAdded(self, aMapLayer):
        if hasattr(aMapLayer, 'dataProvider') and aMapLayer.dataProvider().name() in ['postgres', 'spatialite', 'oracle']:
            uri = QgsDataSourceUri(aMapLayer.source())
            if re.search('^\(SELECT .+ FROM .+\)$', uri.table(), re.S):
                self.iface.legendInterface().addLegendLayerActionForLayer(self.layerAction, aMapLayer)
        # virtual has QUrl source
        # url = QUrl(QUrl.fromPercentEncoding(l.source()))
        # url.queryItemValue('query')
        # url.queryItemValue('uid')
        # url.queryItemValue('geometry')

    def onUpdateSqlLayer(self):
        l = self.iface.legendInterface().currentLayer()
        if l.dataProvider().name() in ['postgres', 'spatialite', 'oracle']:
            uri = QgsDataSourceUri(l.source())
            if re.search('^\(SELECT .+ FROM .+\)$', uri.table(), re.S):
                self.run()
                self.dlg.runSqlLayerWindow(l)
        # virtual has QUrl source
        # url = QUrl(QUrl.fromPercentEncoding(l.source()))
        # url.queryItemValue('query')
        # url.queryItemValue('uid')
        # url.queryItemValue('geometry')

    def run(self):
        # keep opened only one instance
        if self.dlg is None:
            from .db_manager import DBManager

            self.dlg = DBManager(self.iface)
            self.dlg.destroyed.connect(self.onDestroyed)
        self.dlg.show()
        self.dlg.raise_()
        self.dlg.setWindowState(self.dlg.windowState() & ~Qt.WindowMinimized)
        self.dlg.activateWindow()

    def onDestroyed(self, obj):
        self.dlg = None
