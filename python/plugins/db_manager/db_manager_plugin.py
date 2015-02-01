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

from PyQt4.QtCore import Qt, QObject, SIGNAL
from PyQt4.QtGui import QAction, QIcon, QApplication

try:
        from . import resources_rc
except ImportError:
        pass

class DBManagerPlugin:
        def __init__(self, iface):
                self.iface = iface
                self.dlg = None

        def initGui(self):
                self.action = QAction( QIcon(":/db_manager/icon"), QApplication.translate("DBManagerPlugin","DB Manager"), self.iface.mainWindow() )
                self.action.setObjectName("dbManager")
                QObject.connect( self.action, SIGNAL( "triggered()" ), self.run )
                # Add toolbar button and menu item
                if hasattr( self.iface, 'addDatabaseToolBarIcon' ):
                        self.iface.addDatabaseToolBarIcon(self.action)
                else:
                        self.iface.addToolBarIcon(self.action)
                if hasattr( self.iface, 'addPluginToDatabaseMenu' ):
                        self.iface.addPluginToDatabaseMenu( QApplication.translate("DBManagerPlugin","DB Manager"), self.action )
                else:
                        self.iface.addPluginToMenu( QApplication.translate("DBManagerPlugin","DB Manager"), self.action )

        def unload(self):
                # Remove the plugin menu item and icon
                if hasattr( self.iface, 'removePluginDatabaseMenu' ):
                        self.iface.removePluginDatabaseMenu( QApplication.translate("DBManagerPlugin","DB Manager"), self.action )
                else:
                        self.iface.removePluginMenu( QApplication.translate("DBManagerPlugin","DB Manager"), self.action )
                if hasattr( self.iface, 'removeDatabaseToolBarIcon' ):
                        self.iface.removeDatabaseToolBarIcon(self.action)
                else:
                        self.iface.removeToolBarIcon(self.action)

                if self.dlg is not None:
                        self.dlg.close()

        def run(self):
                # keep opened only one instance
                if self.dlg is None:
                        from db_manager import DBManager
                        self.dlg = DBManager(self.iface)
                        QObject.connect(self.dlg, SIGNAL("destroyed(QObject *)"), self.onDestroyed)
                self.dlg.show()
                self.dlg.raise_()
                self.dlg.setWindowState( self.dlg.windowState() & ~Qt.WindowMinimized )
                self.dlg.activateWindow()

        def onDestroyed(self, obj):
                self.dlg = None
