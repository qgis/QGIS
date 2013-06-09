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

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from qgis.core import QgsMapLayerRegistry

from .db_model import DBModel
from .db_plugins.plugin import DBPlugin, Schema, Table

class DBTree(QTreeView):
  def __init__(self, mainWindow):
    QTreeView.__init__(self, mainWindow)
    self.mainWindow = mainWindow

    self.setModel( DBModel(self) )
    self.setHeaderHidden(True)
    self.setEditTriggers(QTreeView.EditKeyPressed|QTreeView.SelectedClicked)

    self.setDragEnabled(True)
    self.setAcceptDrops(True)
    self.setDropIndicatorShown(True)

    self.connect(self.selectionModel(), SIGNAL("currentChanged(const QModelIndex&, const QModelIndex&)"), self.currentItemChanged)
    self.connect(self, SIGNAL("expanded(const QModelIndex&)"), self.itemChanged)
    self.connect(self, SIGNAL("collapsed(const QModelIndex&)"), self.itemChanged)
    self.connect(self.model(), SIGNAL("dataChanged(const QModelIndex&, const QModelIndex&)"), self.modelDataChanged)
    self.connect(self.model(), SIGNAL("notPopulated"), self.collapse)

  def refreshItem(self, item=None):
    if item == None:
      item = self.currentItem()
      if item == None: return
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
    if item == None: return

    if isinstance(item, (DBPlugin, Schema, Table)):
      return item.database()
    return None

  def currentSchema(self):
    item = self.currentItem()
    if item == None: return

    if isinstance(item, (Schema, Table)):
      return item.schema()
    return None

  def currentTable(self):
    item = self.currentItem()
    if item == None: return

    if isinstance(item, Table):
      return item
    return None


  def itemChanged(self, index):
    self.setCurrentIndex(index)
    self.emit( SIGNAL('selectedItemChanged'), self.currentItem() )

  def modelDataChanged(self, indexFrom, indexTo):
    self.itemChanged(indexTo)

  def currentItemChanged(self, current, previous):
    self.itemChanged(current)

  def contextMenuEvent(self, ev):
    index = self.indexAt( ev.pos() )
    if not index.isValid():
      return

    if index != self.currentIndex():
      self.itemChanged(index)

    item = self.currentItem()

    menu = QMenu(self)

    if isinstance(item, (Table, Schema)):
      menu.addAction(self.tr("Rename"), self.rename)
      menu.addAction(self.tr("Delete"), self.delete)

      if isinstance(item, Table):
        menu.addSeparator()
        menu.addAction(self.tr("Add to QGis canvas"), self.addLayer)

    elif isinstance(item, DBPlugin) and item.database() is not None:
      menu.addAction(self.tr("Re-connect"), self.reconnect)

    if not menu.isEmpty():
      menu.exec_(ev.globalPos())

    menu.deleteLater()

  def rename(self):
    index = self.currentIndex()
    item = self.model().getItem(index)
    if isinstance(item, (Table, Schema)):
      self.edit( index )

  def delete(self):
    item = self.currentItem()
    if isinstance(item, (Table, Schema)):
      self.mainWindow.invokeCallback(item.database().deleteActionSlot)

  def addLayer(self):
    table = self.currentTable()
    if table is not None:
      QgsMapLayerRegistry.instance().addMapLayers([table.toMapLayer()])

  def reconnect(self):
    db = self.currentDatabase()
    if db is not None:
      self.mainWindow.invokeCallback(db.reconnectActionSlot)

