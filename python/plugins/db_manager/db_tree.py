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

from qgis.PyQt.QtCore import pyqtSignal
from qgis.PyQt.QtWidgets import QWidget, QTreeView, QMenu, QLabel

from qgis.core import QgsMapLayerRegistry, QgsMessageLog
from qgis.gui import QgsMessageBar, QgsMessageBarItem

from .db_model import DBModel, PluginItem
from .db_plugins.plugin import DBPlugin, Schema, Table


class DBTree(QTreeView):
    selectedItemChanged = pyqtSignal(object)

    def __init__(self, mainWindow):
        QTreeView.__init__(self, mainWindow)
        self.mainWindow = mainWindow

        self.setModel(DBModel(self))
        self.setHeaderHidden(True)
        self.setEditTriggers(QTreeView.EditKeyPressed | QTreeView.SelectedClicked)

        self.setDragEnabled(True)
        self.setAcceptDrops(True)
        self.setDropIndicatorShown(True)

        self.doubleClicked.connect(self.addLayer)
        self.selectionModel().currentChanged.connect(self.currentItemChanged)
        self.expanded.connect(self.itemChanged)
        self.collapsed.connect(self.itemChanged)
        self.model().dataChanged.connect(self.modelDataChanged)
        self.model().notPopulated.connect(self.collapse)

    def refreshItem(self, item=None):
        if item is None:
            item = self.currentItem()
            if item is None:
                return
        self.model().refreshItem(item)

    def showSystemTables(self, show):
        pass

    def currentItem(self):
        indexes = self.selectedIndexes()
        if len(indexes) <= 0:
            return
        return self.model().getItem(indexes[0])

    def currentDatabase(self):
        item = self.currentItem()
        if item is None:
            return

        if isinstance(item, (DBPlugin, Schema, Table)):
            return item.database()
        return None

    def currentSchema(self):
        item = self.currentItem()
        if item is None:
            return

        if isinstance(item, (Schema, Table)):
            return item.schema()
        return None

    def currentTable(self):
        item = self.currentItem()
        if item is None:
            return

        if isinstance(item, Table):
            return item
        return None

    def newConnection(self):
        index = self.currentIndex()
        if not index.isValid() or not isinstance(index.internalPointer(), PluginItem):
            return
        item = self.currentItem()
        self.mainWindow.invokeCallback(item.addConnectionActionSlot, index)

    def itemChanged(self, index):
        self.setCurrentIndex(index)
        self.selectedItemChanged.emit(self.currentItem())

    def modelDataChanged(self, indexFrom, indexTo):
        self.itemChanged(indexTo)

    def currentItemChanged(self, current, previous):
        self.itemChanged(current)

    def contextMenuEvent(self, ev):
        index = self.indexAt(ev.pos())
        if not index.isValid():
            return

        if index != self.currentIndex():
            self.itemChanged(index)

        item = self.currentItem()

        menu = QMenu(self)

        if isinstance(item, (Table, Schema)):
            menu.addAction(self.tr("Rename"), self.rename)
            menu.addAction(self.tr("Delete"), self.delete)

            if isinstance(item, Table) and item.canBeAddedToCanvas():
                menu.addSeparator()
                menu.addAction(self.tr("Add to canvas"), self.addLayer)

        elif isinstance(item, DBPlugin):
            if item.database() is not None:
                menu.addAction(self.tr("Re-connect"), self.reconnect)
            menu.addAction(self.tr("Remove"), self.delete)

        elif not index.parent().isValid() and item.typeName() == "spatialite":
            menu.addAction(self.tr("New Connection..."), self.newConnection)

        if not menu.isEmpty():
            menu.exec_(ev.globalPos())

        menu.deleteLater()

    def rename(self):
        item = self.currentItem()
        if isinstance(item, (Table, Schema)):
            self.edit(self.currentIndex())

    def delete(self):
        item = self.currentItem()
        if isinstance(item, (Table, Schema)):
            self.mainWindow.invokeCallback(item.database().deleteActionSlot)
        elif isinstance(item, DBPlugin):
            self.mainWindow.invokeCallback(item.removeActionSlot)

    def addLayer(self):
        table = self.currentTable()
        if table is not None:
            layer = table.toMapLayer()
            layers = QgsMapLayerRegistry.instance().addMapLayers([layer])
            if len(layers) != 1:
                QgsMessageLog.logMessage(
                    self.tr("%1 is an invalid layer - not loaded").replace("%1", layer.publicSource()))
                msgLabel = QLabel(self.tr(
                    "%1 is an invalid layer and cannot be loaded. Please check the <a href=\"#messageLog\">message log</a> for further info.").replace(
                    "%1", layer.publicSource()), self.mainWindow.infoBar)
                msgLabel.setWordWrap(True)
                msgLabel.linkActivated.connect(self.mainWindow.iface.mainWindow().findChild(QWidget, "MessageLog").show)
                msgLabel.linkActivated.connect(self.mainWindow.iface.mainWindow().raise_)
                self.mainWindow.infoBar.pushItem(QgsMessageBarItem(msgLabel, QgsMessageBar.WARNING))

    def reconnect(self):
        db = self.currentDatabase()
        if db is not None:
            self.mainWindow.invokeCallback(db.reconnectActionSlot)
