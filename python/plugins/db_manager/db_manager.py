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

import functools

from qgis.PyQt.QtCore import Qt, QByteArray, QSize
from qgis.PyQt.QtWidgets import QMainWindow, QApplication, QMenu, QTabWidget, QGridLayout, QSpacerItem, QSizePolicy, QDockWidget, QStatusBar, QMenuBar, QToolBar, QTabBar
from qgis.PyQt.QtGui import QIcon, QKeySequence

from qgis.gui import QgsMessageBar
from qgis.core import (
    Qgis,
    QgsApplication,
    QgsSettings,
    QgsMapLayerType
)
from qgis.utils import OverrideCursor

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
        settings = QgsSettings()
        self.restoreGeometry(settings.value("/DB_Manager/mainWindow/geometry", QByteArray(), type=QByteArray))
        self.restoreState(settings.value("/DB_Manager/mainWindow/windowState", QByteArray(), type=QByteArray))

        self.toolBar.setIconSize(self.iface.iconSize())
        self.toolBarOrientation()
        self.toolBar.orientationChanged.connect(self.toolBarOrientation)
        self.tabs.currentChanged.connect(self.tabChanged)
        self.tree.selectedItemChanged.connect(self.itemChanged)
        self.tree.model().dataChanged.connect(self.iface.reloadConnections)
        self.itemChanged(None)

    def closeEvent(self, e):
        self.unregisterAllActions()
        # clear preview, this will delete the layer in preview tab
        self.preview.loadPreview(None)

        # save the window state
        settings = QgsSettings()
        settings.setValue("/DB_Manager/mainWindow/windowState", self.saveState())
        settings.setValue("/DB_Manager/mainWindow/geometry", self.saveGeometry())

        QMainWindow.closeEvent(self, e)

    def refreshItem(self, item=None):
        with OverrideCursor(Qt.WaitCursor):
            try:
                if item is None:
                    item = self.tree.currentItem()
                self.tree.refreshItem(item)  # refresh item children in the db tree
            except BaseError as e:
                DlgDbError.showError(e, self)

    def itemChanged(self, item):
        with OverrideCursor(Qt.WaitCursor):
            try:
                self.reloadButtons()
                # Force-reload information on the layer
                self.info.setDirty()
                # clear preview, this will delete the layer in preview tab
                self.preview.loadPreview(None)
                self.refreshTabs()
            except BaseError as e:
                DlgDbError.showError(e, self)

    def reloadButtons(self):
        db = self.tree.currentDatabase()
        if not hasattr(self, '_lastDb'):
            self._lastDb = db

        elif db == self._lastDb:
            return

        # remove old actions
        if self._lastDb is not None:
            self.unregisterAllActions()

        # add actions of the selected database
        self._lastDb = db
        if self._lastDb is not None:
            self._lastDb.registerAllActions(self)

    def tabChanged(self, index):
        with OverrideCursor(Qt.WaitCursor):
            try:
                self.refreshTabs()
            except BaseError as e:
                DlgDbError.showError(e, self)

    def refreshTabs(self):
        index = self.tabs.currentIndex()
        item = self.tree.currentItem()
        table = self.tree.currentTable()

        # enable/disable tabs
        self.tabs.setTabEnabled(self.tabs.indexOf(self.table), table is not None)
        self.tabs.setTabEnabled(self.tabs.indexOf(self.preview), table is not None and table.type in [table.VectorType,
                                                                                                      table.RasterType] and table.geomColumn is not None)
        # show the info tab if the current tab is disabled
        if not self.tabs.isTabEnabled(index):
            self.tabs.setCurrentWidget(self.info)

        current_tab = self.tabs.currentWidget()
        if current_tab == self.info:
            self.info.showInfo(item)
        elif current_tab == self.table:
            self.table.loadData(item)
        elif current_tab == self.preview:
            self.preview.loadPreview(item)

    def refreshActionSlot(self):
        self.info.setDirty()
        self.table.setDirty()
        self.preview.setDirty()
        self.refreshItem()

    def importActionSlot(self):
        db = self.tree.currentDatabase()
        if db is None:
            self.infoBar.pushMessage(self.tr("No database selected or you are not connected to it."),
                                     Qgis.Info, self.iface.messageTimeout())
            return

        outUri = db.uri()
        schema = self.tree.currentSchema()
        if schema:
            outUri.setDataSource(schema.name, "", "", "")

        from .dlg_import_vector import DlgImportVector

        dlg = DlgImportVector(None, db, outUri, self)
        dlg.exec_()

    def exportActionSlot(self):
        table = self.tree.currentTable()
        if table is None:
            self.infoBar.pushMessage(self.tr("Select the table you want export to file."), Qgis.Info,
                                     self.iface.messageTimeout())
            return

        inLayer = table.toMapLayer()
        if inLayer.type() != QgsMapLayerType.VectorLayer:
            self.infoBar.pushMessage(
                self.tr("Select a vector or a tabular layer you want export."),
                Qgis.Warning, self.iface.messageTimeout())
            return

        from .dlg_export_vector import DlgExportVector

        dlg = DlgExportVector(inLayer, table.database(), self)
        dlg.exec_()

        inLayer.deleteLater()

    def runSqlWindow(self):
        db = self.tree.currentDatabase()
        if db is None:
            self.infoBar.pushMessage(self.tr("No database selected or you are not connected to it."),
                                     Qgis.Info, self.iface.messageTimeout())
            # force displaying of the message, it appears on the first tab (i.e. Info)
            self.tabs.setCurrentIndex(0)
            return

        from .dlg_sql_window import DlgSqlWindow

        query = DlgSqlWindow(self.iface, db, self)
        dbname = db.connection().connectionName()
        tabname = self.tr("Query ({0})").format(dbname)
        index = self.tabs.addTab(query, tabname)
        self.tabs.setTabIcon(index, db.connection().icon())
        self.tabs.setCurrentIndex(index)
        query.nameChanged.connect(functools.partial(self.update_query_tab_name, index, dbname))

    def runSqlLayerWindow(self, layer):
        from .dlg_sql_layer_window import DlgSqlLayerWindow
        query = DlgSqlLayerWindow(self.iface, layer, self)
        lname = layer.name()
        tabname = self.tr("Layer ({0})").format(lname)
        index = self.tabs.addTab(query, tabname)
        # self.tabs.setTabIcon(index, db.connection().icon())
        self.tabs.setCurrentIndex(index)

    def update_query_tab_name(self, index, dbname, queryname):
        if not queryname:
            queryname = self.tr("Query")
        tabname = "%s (%s)" % (queryname, dbname)
        self.tabs.setTabText(index, tabname)

    def showSystemTables(self):
        self.tree.showSystemTables(self.actionShowSystemTables.isChecked())

    def registerAction(self, action, menuName, callback=None):
        """ register an action to the manager's main menu """
        if not hasattr(self, '_registeredDbActions'):
            self._registeredDbActions = {}

        if callback is not None:
            def invoke_callback(x):
                return self.invokeCallback(callback)

        if menuName is None or menuName == "":
            self.addAction(action)

            if menuName not in self._registeredDbActions:
                self._registeredDbActions[menuName] = list()
            self._registeredDbActions[menuName].append(action)

            if callback is not None:
                action.triggered.connect(invoke_callback)
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
        if actionMenu is None:
            menu = QMenu(menuName, self)
            if helpMenuAction is not None:
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
            menu.insertAction(before, action)
        else:
            menu.addAction(action)

        actionMenu.setVisible(True)  # show the menu

        if menuName not in self._registeredDbActions:
            self._registeredDbActions[menuName] = list()
        self._registeredDbActions[menuName].append(action)

        if callback is not None:
            action.triggered.connect(invoke_callback)

        return True

    def invokeCallback(self, callback, *params):
        """ Call a method passing the selected item in the database tree,
                the sender (usually a QAction), the plugin mainWindow and
                optionally additional parameters.

                This method takes care to override and restore the cursor,
                but also catches exceptions and displays the error dialog.
        """
        with OverrideCursor(Qt.WaitCursor):
            try:
                callback(self.tree.currentItem(), self.sender(), self, *params)
            except BaseError as e:
                # catch database errors and display the error dialog
                DlgDbError.showError(e, self)

    def unregisterAction(self, action, menuName):
        if not hasattr(self, '_registeredDbActions'):
            return

        if menuName is None or menuName == "":
            self.removeAction(action)

            if menuName in self._registeredDbActions:
                if self._registeredDbActions[menuName].count(action) > 0:
                    self._registeredDbActions[menuName].remove(action)

            action.deleteLater()
            return True

        for a in self.menuBar.actions():
            if not a.menu() or a.menu().title() != menuName:
                continue

            menu = a.menu()
            menuActions = menu.actions()

            menu.removeAction(action)
            if menu.isEmpty():  # hide the menu
                a.setVisible(False)

            if menuName in self._registeredDbActions:
                if self._registeredDbActions[menuName].count(action) > 0:
                    self._registeredDbActions[menuName].remove(action)

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
                self.unregisterAction(action, menuName)
        del self._registeredDbActions

    def close_tab(self, index):
        widget = self.tabs.widget(index)
        if widget not in [self.info, self.table, self.preview]:
            if hasattr(widget, "close"):
                if widget.close():
                    self.tabs.removeTab(index)
                    widget.deleteLater()
            else:
                self.tabs.removeTab(index)
                widget.deleteLater()

    def toolBarOrientation(self):
        button_style = Qt.ToolButtonIconOnly
        if self.toolBar.orientation() == Qt.Horizontal:
            button_style = Qt.ToolButtonTextBesideIcon

        widget = self.toolBar.widgetForAction(self.actionImport)
        widget.setToolButtonStyle(button_style)
        widget = self.toolBar.widgetForAction(self.actionExport)
        widget.setToolButtonStyle(button_style)

    def setupUi(self):
        self.setWindowTitle(self.tr("DB Manager"))
        self.setWindowIcon(QIcon(":/db_manager/icon"))
        self.resize(QSize(700, 500).expandedTo(self.minimumSizeHint()))

        # create central tab widget and add the first 3 tabs: info, table and preview
        self.tabs = QTabWidget()
        self.info = InfoViewer(self)
        self.tabs.addTab(self.info, self.tr("Info"))
        self.table = TableViewer(self)
        self.tabs.addTab(self.table, self.tr("Table"))
        self.preview = LayerPreview(self)
        self.tabs.addTab(self.preview, self.tr("Preview"))
        self.setCentralWidget(self.tabs)

        # display close button for all tabs but the first 3 ones, i.e.
        # HACK: just hide the close button where not needed (GS)
        self.tabs.setTabsClosable(True)
        self.tabs.tabCloseRequested.connect(self.close_tab)
        tabbar = self.tabs.tabBar()
        for i in range(3):
            btn = tabbar.tabButton(i, QTabBar.RightSide) if tabbar.tabButton(i, QTabBar.RightSide) else tabbar.tabButton(i, QTabBar.LeftSide)
            btn.resize(0, 0)
            btn.hide()

        # Creates layout for message bar
        self.layout = QGridLayout(self.info)
        self.layout.setContentsMargins(0, 0, 0, 0)
        spacerItem = QSpacerItem(20, 40, QSizePolicy.Minimum, QSizePolicy.Expanding)
        self.layout.addItem(spacerItem, 1, 0, 1, 1)
        # init messageBar instance
        self.infoBar = QgsMessageBar(self.info)
        sizePolicy = QSizePolicy(QSizePolicy.Minimum, QSizePolicy.Fixed)
        self.infoBar.setSizePolicy(sizePolicy)
        self.layout.addWidget(self.infoBar, 0, 0, 1, 1)

        # create database tree
        self.dock = QDockWidget(self.tr("Providers"), self)
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
        self.menuBar.addMenu(self.menuDb)
        self.menuSchema = QMenu(self.tr("&Schema"), self)
        actionMenuSchema = self.menuBar.addMenu(self.menuSchema)
        self.menuTable = QMenu(self.tr("&Table"), self)
        actionMenuTable = self.menuBar.addMenu(self.menuTable)
        self.menuHelp = None  # QMenu(self.tr("&Help"), self)
        # actionMenuHelp = self.menuBar.addMenu(self.menuHelp)

        self.setMenuBar(self.menuBar)

        # create toolbar
        self.toolBar = QToolBar(self.tr("Default"), self)
        self.toolBar.setObjectName("DB_Manager_ToolBar")
        self.addToolBar(self.toolBar)

        # create menus' actions

        # menu DATABASE
        sep = self.menuDb.addSeparator()
        sep.setObjectName("DB_Manager_DbMenu_placeholder")
        sep.setVisible(False)

        self.actionRefresh = self.menuDb.addAction(QgsApplication.getThemeIcon("/mActionRefresh.svg"), self.tr("&Refresh"),
                                                   self.refreshActionSlot, QKeySequence("F5"))
        self.actionSqlWindow = self.menuDb.addAction(QIcon(":/db_manager/actions/sql_window"), self.tr("&SQL Window"),
                                                     self.runSqlWindow, QKeySequence("F2"))
        self.menuDb.addSeparator()
        self.actionClose = self.menuDb.addAction(QIcon(), self.tr("&Exit"), self.close, QKeySequence("CTRL+Q"))

        # menu SCHEMA
        sep = self.menuSchema.addSeparator()
        sep.setObjectName("DB_Manager_SchemaMenu_placeholder")
        sep.setVisible(False)

        actionMenuSchema.setVisible(False)

        # menu TABLE
        sep = self.menuTable.addSeparator()
        sep.setObjectName("DB_Manager_TableMenu_placeholder")
        sep.setVisible(False)

        self.actionImport = self.menuTable.addAction(QIcon(":/db_manager/actions/import"),
                                                     QApplication.translate("DBManager", "&Import Layer/File…"),
                                                     self.importActionSlot)
        self.actionExport = self.menuTable.addAction(QIcon(":/db_manager/actions/export"),
                                                     QApplication.translate("DBManager", "&Export to File…"),
                                                     self.exportActionSlot)
        self.menuTable.addSeparator()
        # self.actionShowSystemTables = self.menuTable.addAction(self.tr("Show system tables/views"), self.showSystemTables)
        # self.actionShowSystemTables.setCheckable(True)
        # self.actionShowSystemTables.setChecked(True)
        actionMenuTable.setVisible(False)

        # add actions to the toolbar
        self.toolBar.addAction(self.actionRefresh)
        self.toolBar.addAction(self.actionSqlWindow)
        self.toolBar.addSeparator()
        self.toolBar.addAction(self.actionImport)
        self.toolBar.addAction(self.actionExport)
