# -*- coding: utf-8 -*-

"""
/***************************************************************************
Name                 : DB Manager
Description          : Database manager plugin for QuantumGIS
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

from PyQt4.QtCore import *
from PyQt4.QtGui import *

try:
	from . import resources_rc
except ImportError:
	pass

class DBManagerPlugin:
	def __init__(self, iface):
		self.iface = iface
		self.dlg = None

	def initGui(self):
		self.action = QAction( QIcon(":/db_manager/icon"), u"DB Manager", self.iface.mainWindow() )
		QObject.connect( self.action, SIGNAL( "triggered()" ), self.run )
		# Add toolbar button and menu item
		if hasattr( self.iface, 'addDatabaseToolBarIcon' ):
			self.iface.addDatabaseToolBarIcon(self.action)
		else:
			self.iface.addToolBarIcon(self.action)
		if hasattr( self.iface, 'addPluginToDatabaseMenu' ):
			self.iface.addPluginToDatabaseMenu( u"DB Manager", self.action )
		else:
			self.iface.addPluginToMenu( u"DB Manager", self.action )

	def unload(self):
		# Remove the plugin menu item and icon
		if hasattr( self.iface, 'removePluginDatabaseMenu' ):
			self.iface.removePluginDatabaseMenu( u"DB Manager", self.action )
		else:
			self.iface.removePluginMenu( u"DB Manager", self.action )
		if hasattr( self.iface, 'removeDatabaseToolBarIcon' ):
			self.iface.removeDatabaseToolBarIcon(self.action)
		else:
			self.iface.removeToolBarIcon(self.action)
		
		if self.dlg != None:
			self.dlg.close()

	def run(self):
		# keep opened only one instance
		if self.dlg == None:
			from db_manager import DBManager
			self.dlg = DBManager(self.iface, self.iface.mainWindow())
			QObject.connect(self.dlg, SIGNAL("destroyed(QObject *)"), self.onDestroyed)
		self.dlg.show()
		self.dlg.raise_()
		self.dlg.activateWindow()

	def onDestroyed(self, obj):
		self.dlg = None

