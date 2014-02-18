# -*- coding: utf-8 -*-

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

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from .info_viewer import InfoViewer
from .table_viewer import TableViewer
from .layer_preview import LayerPreview

from .db_tree import DBTree

from .db_plugins.plugin import BaseError
from .dlg_db_error import DlgDbError


class DBManager(QMainWindow):

	def __init__(self, iface, parent=None):
		QMainWindow.__init__(self, parent)
		self.setAttribute(Qt.WA_DeleteOnClose)
		self.setupUi()
		self.iface = iface

		# restore the window state
		settings = QSettings()
		self.restoreGeometry( settings.value("/DB_Manager/mainWindow/geometry", QByteArray(), type=QByteArray ) )
		self.restoreState( settings.value("/DB_Manager/mainWindow/windowState", QByteArray(), type=QByteArray ) )

		self.connect(self.tabs, SIGNAL("currentChanged(int)"), self.tabChanged)
		self.connect(self.tree, SIGNAL("selectedItemChanged"), self.itemChanged)
		self.itemChanged(None)


	def closeEvent(self, e):
		self.unregisterAllActions()

		# save the window state
		settings = QSettings()
		settings.setValue( "/DB_Manager/mainWindow/windowState", self.saveState() )
		settings.setValue( "/DB_Manager/mainWindow/geometry", self.saveGeometry() )

		QMainWindow.closeEvent(self, e)


	def refreshItem(self, item=None):
		QApplication.setOverrideCursor(Qt.WaitCursor)
		try:
			if item == None:
				item = self.tree.currentItem()
			self.tree.refreshItem(item)	# refresh item children in the db tree
		except BaseError, e:
			DlgDbError.showError(e, self)
			return
		finally:
			QApplication.restoreOverrideCursor()

	def itemChanged(self, item):
		QApplication.setOverrideCursor(Qt.WaitCursor)
		try:
			self.reloadButtons()
			self.refreshTabs()
		except BaseError, e:
			DlgDbError.showError(e, self)
			return
		finally:
			QApplication.restoreOverrideCursor()


	def reloadButtons(self):
		db = self.tree.currentDatabase()
		if not hasattr(self, '_lastDb'):
			self._lastDb = db

		elif db == self._lastDb:
			return

		# remove old actions
		if self._lastDb != None:
			self.unregisterAllActions()

		# add actions of the selected database
		self._lastDb = db
		if self._lastDb != None:
			self._lastDb.registerAllActions(self)


	def tabChanged(self, index):
		QApplication.setOverrideCursor(Qt.WaitCursor)
		try:
			self.refreshTabs()
		except BaseError, e:
			DlgDbError.showError(e, self)
			return
		finally:
			QApplication.restoreOverrideCursor()


	def refreshTabs(self):
		index = self.tabs.currentIndex()
		item  = self.tree.currentItem()
		table  = self.tree.currentTable()

		# enable/disable tabs
		self.tabs.setTabEnabled( self.tabs.indexOf(self.table), table != None )
		self.tabs.setTabEnabled( self.tabs.indexOf(self.preview), table != None and table.type in [table.VectorType, table.RasterType] and table.geomColumn != None )
		# show the info tab if the current tab is disabled
		if not self.tabs.isTabEnabled( index ):
			self.tabs.setCurrentWidget( self.info )

		current_tab = self.tabs.currentWidget()
		if current_tab == self.info:
			self.info.showInfo( item )
		elif current_tab == self.table:
			self.table.loadData( item )
		elif current_tab == self.preview:
			self.preview.loadPreview( item )


	def refreshActionSlot(self):
		self.info.setDirty()
		self.table.setDirty()
		self.preview.setDirty()
		self.refreshItem()

	def importActionSlot(self):
		db = self.tree.currentDatabase()
		if db is None:
			QMessageBox.information(self, self.tr("Sorry"), self.tr("No database selected or you are not connected to it."))
			return

		outUri = db.uri()
		schema = self.tree.currentSchema()
		if schema:
			outUri.setDataSource( schema.name, "", "", "" )

		from .dlg_import_vector import DlgImportVector
		dlg = DlgImportVector(None, db, outUri, self)
		dlg.exec_()

	def exportActionSlot(self):
		table = self.tree.currentTable()
		if table is None:
			QMessageBox.information(self, self.tr("Sorry"), self.tr("Select the table you want export to file."))
			return

		inLayer = table.toMapLayer()

		from .dlg_export_vector import DlgExportVector
		dlg = DlgExportVector(inLayer, table.database(), self)
		dlg.exec_()

		inLayer.deleteLater()

	def runSqlWindow(self):
		db = self.tree.currentDatabase()
		if db == None:
			QMessageBox.information(self, self.tr("Sorry"), self.tr("No database selected or you are not connected to it."))
			return

		from dlg_sql_window import DlgSqlWindow
		dlg = DlgSqlWindow(self.iface, db, self)
		#refreshDb = lambda x: self.refreshItem( db.connection() ) # refresh the database tree
		#self.connect( dlg, SIGNAL( "queryExecuted(const QString &)" ), refreshDb )
		dlg.show()
		dlg.exec_()


	def showSystemTables(self):
		self.tree.showSystemTables( self.actionShowSystemTables.isChecked() )


	def registerAction(self, action, menuName, callback=None):
		""" register an action to the manager's main menu """
		if not hasattr(self, '_registeredDbActions'):
			self._registeredDbActions = {}

		if callback != None:
			invoke_callback = lambda x: self.invokeCallback( callback )

		if menuName == None or menuName == "":
			self.addAction( action )

			if not self._registeredDbActions.has_key(menuName):
				self._registeredDbActions[menuName] = list()
			self._registeredDbActions[menuName].append(action)

			if callback != None:
				QObject.connect( action, SIGNAL("triggered(bool)"), invoke_callback )
			return True

		# search for the menu
		actionMenu = None
		helpMenuAction = None
		for a in self.menuBar.actions():
			if not a.menu() or a.menu().title() != menuName:
				continue
			if a.menu() != self.menuHelp:
				helpMenuAction = a

			actionMenu = a
			break

		# not found, add a new menu before the help menu
		if actionMenu == None:
			menu = QMenu(menuName, self)
			if helpMenuAction != None:
				actionMenu = self.menuBar.insertMenu(helpMenuAction, menu)
			else:
				actionMenu = self.menuBar.addMenu(menu)

		menu = actionMenu.menu()
		menuActions = menu.actions()

		# get the placeholder's position to insert before it
		pos = 0
		for pos in range(len(menuActions)):
			if menuActions[pos].isSeparator() and menuActions[pos].objectName().endswith("_placeholder"):
				menuActions[pos].setVisible(True)
				break

		if pos < len(menuActions):
			before = menuActions[pos]
			menu.insertAction( before, action )
		else:
			menu.addAction( action )

		actionMenu.setVisible(True)	# show the menu

		if not self._registeredDbActions.has_key(menuName):
			self._registeredDbActions[menuName] = list()
		self._registeredDbActions[menuName].append(action)

		if callback != None:
			QObject.connect( action, SIGNAL("triggered(bool)"), invoke_callback )

		return True


	def invokeCallback(self, callback, params=None):
		""" Call a method passing the selected item in the database tree,
			the sender (usually a QAction), the plugin mainWindow and
			optionally additional parameters.

			This method takes care to override and restore the cursor,
			but also catches exceptions and displays the error dialog.
		"""
		QApplication.setOverrideCursor(Qt.WaitCursor)
		try:
			if params is None:
				callback( self.tree.currentItem(), self.sender(), self )
			else:
				callback( self.tree.currentItem(), self.sender(), self, *params )

		except BaseError, e:
			# catch database errors and display the error dialog
			DlgDbError.showError(e, self)
			return

		finally:
			QApplication.restoreOverrideCursor()


	def unregisterAction(self, action, menuName):
		if not hasattr(self, '_registeredDbActions'):
			return

		if menuName == None or menuName == "":
			self.removeAction( action )

			if self._registeredDbActions.has_key(menuName):
				if self._registeredDbActions[menuName].count( action ) > 0:
					self._registeredDbActions[menuName].remove( action )

			action.deleteLater()
			return True

		for a in self.menuBar.actions():
			if not a.menu() or a.menu().title() != menuName:
				continue

			menu = a.menu()
			menuActions = menu.actions()

			menu.removeAction( action )
			if menu.isEmpty():	# hide the menu
				a.setVisible(False)

			if self._registeredDbActions.has_key(menuName):
				if self._registeredDbActions[menuName].count( action ) > 0:
					self._registeredDbActions[menuName].remove( action )

				# hide the placeholder if there're no other registered actions
				if len(self._registeredDbActions[menuName]) <= 0:
					for i in range(len(menuActions)):
						if menuActions[i].isSeparator() and menuActions[i].objectName().endswith("_placeholder"):
							menuActions[i].setVisible(False)
							break

			action.deleteLater()
			return True

		return False

	def unregisterAllActions(self):
		if not hasattr(self, '_registeredDbActions'):
			return

		for menuName in self._registeredDbActions:
			for action in list(self._registeredDbActions[menuName]):
				self.unregisterAction( action, menuName )
		del self._registeredDbActions

	def setupUi(self):
		self.setWindowTitle(self.tr("DB Manager"))
		self.setWindowIcon(QIcon(":/db_manager/icon"))
		self.resize(QSize(700,500).expandedTo(self.minimumSizeHint()))

		# create central tab widget
		self.tabs = QTabWidget()
		self.info = InfoViewer(self)
		self.tabs.addTab(self.info, self.tr("Info"))
		self.table = TableViewer(self)
		self.tabs.addTab(self.table, self.tr("Table"))
		self.preview = LayerPreview(self)
		self.tabs.addTab(self.preview, self.tr("Preview"))
		self.setCentralWidget(self.tabs)

		# create database tree
		self.dock = QDockWidget("Tree", self)
		self.dock.setObjectName("DB_Manager_DBView")
		self.dock.setFeatures(QDockWidget.DockWidgetMovable)
		self.tree = DBTree(self)
		self.dock.setWidget(self.tree)
		self.addDockWidget(Qt.LeftDockWidgetArea, self.dock)

		# create status bar
		self.statusBar = QStatusBar(self)
		self.setStatusBar(self.statusBar)

		# create menus
		self.menuBar = QMenuBar(self)
		self.menuDb = QMenu(self.tr("&Database"), self)
		actionMenuDb = self.menuBar.addMenu(self.menuDb)
		self.menuSchema = QMenu(self.tr("&Schema"), self)
		actionMenuSchema = self.menuBar.addMenu(self.menuSchema)
		self.menuTable = QMenu(self.tr("&Table"), self)
		actionMenuTable = self.menuBar.addMenu(self.menuTable)
		self.menuHelp = None # QMenu(self.tr("&Help"), self)
		#actionMenuHelp = self.menuBar.addMenu(self.menuHelp)

		self.setMenuBar(self.menuBar)

		# create toolbar
		self.toolBar = QToolBar("Default", self)
		self.toolBar.setObjectName("DB_Manager_ToolBar")
		self.addToolBar(self.toolBar)

		# create menus' actions

		# menu DATABASE
		sep = self.menuDb.addSeparator(); sep.setObjectName("DB_Manager_DbMenu_placeholder"); sep.setVisible(False)
		self.actionRefresh = self.menuDb.addAction( QIcon(":/db_manager/actions/refresh"), self.tr("&Refresh"), self.refreshActionSlot, QKeySequence("F5") )
		self.actionSqlWindow = self.menuDb.addAction( QIcon(":/db_manager/actions/sql_window"), self.tr("&SQL window"), self.runSqlWindow, QKeySequence("F2") )
		self.menuDb.addSeparator()
		self.actionClose = self.menuDb.addAction( QIcon(), self.tr("&Exit"), self.close, QKeySequence("CTRL+Q") )

		# menu SCHEMA
		sep = self.menuSchema.addSeparator(); sep.setObjectName("DB_Manager_SchemaMenu_placeholder"); sep.setVisible(False)
		actionMenuSchema.setVisible(False)

		# menu TABLE
		sep = self.menuTable.addSeparator(); sep.setObjectName("DB_Manager_TableMenu_placeholder"); sep.setVisible(False)
		self.actionImport = self.menuTable.addAction( QIcon(":/db_manager/actions/import"), self.tr("&Import layer/file"), self.importActionSlot )
		self.actionExport = self.menuTable.addAction( QIcon(":/db_manager/actions/export"), self.tr("&Export to file"), self.exportActionSlot )
		self.menuTable.addSeparator()
		#self.actionShowSystemTables = self.menuTable.addAction(self.tr("Show system tables/views"), self.showSystemTables)
		#self.actionShowSystemTables.setCheckable(True)
		#self.actionShowSystemTables.setChecked(True)
		actionMenuTable.setVisible(False)

		# add actions to the toolbar
		self.toolBar.addAction( self.actionRefresh )
		self.toolBar.addAction( self.actionSqlWindow )
		self.toolBar.addAction( self.actionImport )
		self.toolBar.addAction( self.actionExport )
