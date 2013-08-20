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

from .db_plugins import supportedDbTypes, createDbPlugin
from .db_plugins.plugin import BaseError, DbError, Table
from .dlg_db_error import DlgDbError

import qgis.core

try:
	from . import resources_rc
except ImportError:
	pass

class TreeItem(QObject):
	def __init__(self, data, parent=None):
		QObject.__init__(self, parent)
		self.populated = False
		self.itemData = data
		self.childItems = []
		if parent:
			parent.appendChild(self)

	def childRemoved(self, child):
		self.itemChanged()

	def itemChanged(self):
		self.emit( SIGNAL("itemChanged"), self )

	def itemRemoved(self):
		self.emit( SIGNAL("itemRemoved"), self )

	def populate(self):
		self.populated = True
		return True

	def getItemData(self):
		return self.itemData

	def appendChild(self, child):
		self.childItems.append(child)
		self.connect(child, SIGNAL("itemRemoved"), self.childRemoved)

	def child(self, row):
		return self.childItems[row]

	def removeChild(self, row):
		if row >= 0 and row < len(self.childItems):
			self.childItems[row].itemData.deleteLater()
			self.disconnect(self.childItems[row], SIGNAL("itemRemoved"), self.childRemoved)
			del self.childItems[row]

	def childCount(self):
		return len(self.childItems)

	def columnCount(self):
		return 1

	def row(self):
		if self.parent():
			for row, item in enumerate(self.parent().childItems):
				if item is self:
					return row
		return 0

	def data(self, column):
		return "" if column == 0 else None

	def icon(self):
		return None

	def path(self):
		pathList = []
		if self.parent():
			pathList.extend( self.parent().path() )
		pathList.append( self.data(0) )
		return pathList


class PluginItem(TreeItem):
	def __init__(self, dbplugin, parent=None):
		TreeItem.__init__(self, dbplugin, parent)

	def populate(self):
		if self.populated:
			return True

		# create items for connections
		for c in self.getItemData().connections():
			ConnectionItem(c, self)

		self.populated = True
		return True


	def data(self, column):
		if column == 0:
			return self.getItemData().typeNameString()
		return None

	def icon(self):
		return self.getItemData().icon()

	def path(self):
		return [ self.getItemData().typeName() ]

class ConnectionItem(TreeItem):
	def __init__(self, connection, parent=None):
		TreeItem.__init__(self, connection, parent)

		# load (shared) icon with first instance of table item
		if not hasattr(ConnectionItem, 'connectedIcon'):
			ConnectionItem.connectedIcon = QIcon(":/db_manager/icons/plugged.png")
			ConnectionItem.disconnectedIcon = QIcon(":/db_manager/icons/unplugged.png")

	def data(self, column):
		if column == 0:
			return self.getItemData().connectionName()
		return None

	def populate(self):
		if self.populated:
			return True

		connection = self.getItemData()
		if connection.database() == None:
			# connect to database
			try:
				if not connection.connect():
					return False

			except BaseError, e:
				QMessageBox.warning( None, self.tr("Unable to connect"), unicode(e) )
				return False

		database = connection.database()
		self.connect(database, SIGNAL("changed"), self.itemChanged)
		self.connect(database, SIGNAL("deleted"), self.itemRemoved)

		schemas = database.schemas()
		if schemas != None:
			for s in schemas:
				SchemaItem(s, self)
		else:
			tables = database.tables()
			for t in tables:
				TableItem(t, self)

		self.populated = True
		return True

	def isConnected(self):
		return self.getItemData().database() != None

	#def icon(self):
	#	return self.connectedIcon if self.isConnected() else self.disconnectedIcon

class SchemaItem(TreeItem):
	def __init__(self, schema, parent):
		TreeItem.__init__(self, schema, parent)
		self.connect(schema, SIGNAL("changed"), self.itemChanged)
		self.connect(schema, SIGNAL("deleted"), self.itemRemoved)

		# load (shared) icon with first instance of schema item
		if not hasattr(SchemaItem, 'schemaIcon'):
			SchemaItem.schemaIcon = QIcon(":/db_manager/icons/namespace.png")

	def data(self, column):
		if column == 0:
			return self.getItemData().name
		return None

	def icon(self):
		return self.schemaIcon

	def populate(self):
		if self.populated:
			return True

		for t in self.getItemData().tables():
			TableItem(t, self)

		self.populated = True
		return True


class TableItem(TreeItem):
	def __init__(self, table, parent):
		TreeItem.__init__(self, table, parent)
		self.connect(table, SIGNAL("changed"), self.itemChanged)
		self.connect(table, SIGNAL("deleted"), self.itemRemoved)
		self.populate()

		# load (shared) icon with first instance of table item
		if not hasattr(TableItem, 'tableIcon'):
			TableItem.tableIcon = QIcon(":/db_manager/icons/table.png")
			TableItem.viewIcon = QIcon(":/db_manager/icons/view.png")
			TableItem.layerPointIcon = QIcon(":/db_manager/icons/layer_point.png")
			TableItem.layerLineIcon = QIcon(":/db_manager/icons/layer_line.png")
			TableItem.layerPolygonIcon = QIcon(":/db_manager/icons/layer_polygon.png")
			TableItem.layerRasterIcon = QIcon(":/db_manager/icons/layer_raster.png")
			TableItem.layerUnknownIcon = QIcon(":/db_manager/icons/layer_unknown.png")

	def data(self, column):
		if column == 0:
			return self.getItemData().name
		elif column == 1:
			if self.getItemData().type == Table.VectorType:
				return self.getItemData().geomType
		return None

	def icon(self):
		if self.getItemData().type == Table.VectorType:
			geom_type = self.getItemData().geomType
			if geom_type is not None:
				if geom_type.find('POINT') != -1:
					return self.layerPointIcon
				elif geom_type.find('LINESTRING') != -1:
					return self.layerLineIcon
				elif geom_type.find('POLYGON') != -1:
					return self.layerPolygonIcon
				return self.layerUnknownIcon

		elif self.getItemData().type == Table.RasterType:
			return self.layerRasterIcon

		if self.getItemData().isView:
			return self.viewIcon
		return self.tableIcon

	def path(self):
		pathList = []
		if self.parent():
			pathList.extend( self.parent().path() )

		if self.getItemData().type == Table.VectorType:
			pathList.append( "%s::%s" % ( self.data(0), self.getItemData().geomColumn ) )
		else:
			pathList.append( self.data(0) )

		return pathList


class DBModel(QAbstractItemModel):
	def __init__(self, parent=None):
		QAbstractItemModel.__init__(self, parent)
		self.treeView = parent
		self.header = [self.tr('Databases')]

		self.isImportVectorAvail = hasattr(qgis.core, 'QgsVectorLayerImport')
		if self.isImportVectorAvail:
			self.connect(self, SIGNAL("importVector"), self.importVector)

		self.rootItem = TreeItem(None, None)
		for dbtype in supportedDbTypes():
			dbpluginclass = createDbPlugin( dbtype )
			PluginItem( dbpluginclass, self.rootItem )


	def refreshItem(self, item):
		if isinstance(item, TreeItem):
			# find the index for the tree item using the path
			index = self._rPath2Index( item.path() )
		else:
			# find the index for the db item
			index = self._rItem2Index(item)
		if index.isValid():
			self._refreshIndex(index)
		else:
			qDebug( "invalid index" )

	def _rItem2Index(self, item, parent=None):
		if parent == None:
			parent = QModelIndex()
		if item == self.getItem(parent):
			return parent

		if not parent.isValid() or parent.internalPointer().populated:
			for i in range( self.rowCount(parent) ):
				index = self.index(i, 0, parent)
				index = self._rItem2Index(item, index)
				if index.isValid():
					return index

		return QModelIndex()

	def _rPath2Index(self, path, parent=None, n=0):
		if parent == None:
			parent = QModelIndex()
		if path == None or len(path) == 0:
			return parent

		for i in range( self.rowCount(parent) ):
			index = self.index(i, 0, parent)
			if self._getPath(index)[n] == path[0]:
				return self._rPath2Index( path[1:], index, n+1 )

		return parent

	def getItem(self, index):
		if not index.isValid():
			return None
		return index.internalPointer().getItemData()

	def _getPath(self, index):
		if not index.isValid():
			return None
		return index.internalPointer().path()


	def columnCount(self, parent):
		return 1

	def data(self, index, role):
		if not index.isValid():
			return None

		if role == Qt.DecorationRole and index.column() == 0:
			icon = index.internalPointer().icon()
			if icon: return icon

		if role != Qt.DisplayRole and role != Qt.EditRole:
			return None

		retval = index.internalPointer().data(index.column())
		return retval

	def flags(self, index):
		if not index.isValid():
			return Qt.NoItemFlags

		flags = Qt.ItemIsEnabled | Qt.ItemIsSelectable

		if index.column() == 0:
			item = index.internalPointer()
			if isinstance(item, SchemaItem) or isinstance(item, TableItem):
				flags |= Qt.ItemIsEditable

			if isinstance(item, TableItem):
				flags |= Qt.ItemIsDragEnabled

			if self.isImportVectorAvail:	# allow to import a vector layer
				if isinstance(item, ConnectionItem) and item.populated:
					flags |= Qt.ItemIsDropEnabled

				if isinstance(item, SchemaItem) or isinstance(item, TableItem):
					flags |= Qt.ItemIsDropEnabled

		return flags

	def headerData(self, section, orientation, role):
		if orientation == Qt.Horizontal and role == Qt.DisplayRole and section < len(self.header):
			return self.header[section]
		return None

	def index(self, row, column, parent):
		if not self.hasIndex(row, column, parent):
			return QModelIndex()

		parentItem = parent.internalPointer() if parent.isValid() else self.rootItem
		childItem = parentItem.child(row)
		if childItem:
			return self.createIndex(row, column, childItem)
		return QModelIndex()

	def parent(self, index):
		if not index.isValid():
			return QModelIndex()

		childItem = index.internalPointer()
		parentItem = childItem.parent()

		if parentItem == self.rootItem:
			return QModelIndex()

		return self.createIndex(parentItem.row(), 0, parentItem)


	def rowCount(self, parent):
		parentItem = parent.internalPointer() if parent.isValid() else self.rootItem
		if not parentItem.populated:
			self._refreshIndex( parent, True )
		return parentItem.childCount()

	def hasChildren(self, parent):
		parentItem = parent.internalPointer() if parent.isValid() else self.rootItem
		return parentItem.childCount() > 0 or not parentItem.populated


	def setData(self, index, value, role):
		if role != Qt.EditRole or index.column() != 0:
			return False

		item = index.internalPointer()
		new_value = unicode(value)

		if isinstance(item, SchemaItem) or isinstance(item, TableItem):
			obj = item.getItemData()

			# rename schema or table or view
			if new_value == obj.name:
				return False

			QApplication.setOverrideCursor(Qt.WaitCursor)
			try:
				obj.rename(new_value)
				self._onDataChanged(index)
			except BaseError, e:
				DlgDbError.showError(e, self.treeView)
				return False
			finally:
				QApplication.restoreOverrideCursor()

			return True

		return False

	def removeRows(self, row, count, parent):
		self.beginRemoveRows(parent, row, count+row-1)
		item = parent.internalPointer()
		for i in range(row, count+row):
			item.removeChild(row)
		self.endRemoveRows()

	def _refreshIndex(self, index, force=False):
		QApplication.setOverrideCursor(Qt.WaitCursor)
		try:
			item = index.internalPointer() if index.isValid() else self.rootItem
			prevPopulated = item.populated
			if prevPopulated:
				self.removeRows(0, self.rowCount(index), index)
				item.populated = False
			if prevPopulated or force:
				if item.populate():
					for child in item.childItems:
						self.connect(child, SIGNAL("itemChanged"), self.refreshItem)
					self._onDataChanged( index )
				else:
					self.emit( SIGNAL("notPopulated"), index )

		except BaseError, e:
			item.populated = False
			return

		finally:
			QApplication.restoreOverrideCursor()

	def _onDataChanged(self, indexFrom, indexTo=None):
		if indexTo == None: indexTo = indexFrom
		self.emit( SIGNAL('dataChanged(const QModelIndex &, const QModelIndex &)'), indexFrom, indexTo)


	QGIS_URI_MIME = "application/x-vnd.qgis.qgis.uri"

	def mimeTypes(self):
		return ["text/uri-list", self.QGIS_URI_MIME]

	def mimeData(self, indexes):
		mimeData = QMimeData()
		encodedData = QByteArray()

		stream = QDataStream(encodedData, QIODevice.WriteOnly)

		for index in indexes:
			if not index.isValid():
				continue
			if not isinstance(index.internalPointer(), TableItem):
				continue
			table = self.getItem( index )
			stream.writeQString( table.mimeUri() )

		mimeData.setData(self.QGIS_URI_MIME, encodedData)
		return mimeData


	def dropMimeData(self, data, action, row, column, parent):
		if action == Qt.IgnoreAction:
			return True

		if not self.isImportVectorAvail:
			return False

		added = 0

		if data.hasUrls():
			for u in data.urls():
				filename = u.toLocalFile()
				if filename == "":
					continue

				if qgis.core.QgsRasterLayer.isValidRasterFileName( filename ):
					layerType = 'raster'
					providerKey = 'gdal'
				else:
					layerType = 'vector'
					providerKey = 'ogr'

				layerName = QFileInfo(filename).completeBaseName()

				if self.importLayer( layerType, providerKey, layerName, filename, parent ):
					added += 1

		if data.hasFormat(self.QGIS_URI_MIME):
			for uri in qgis.core.QgsMimeDataUtils.decodeUriList( data ):
				if self.importLayer( uri.layerType, uri.providerKey, uri.name, uri.uri, parent ):
					added += 1

		return added > 0


	def importLayer(self, layerType, providerKey, layerName, uriString, parent):
		if not self.isImportVectorAvail:
			return False

		if layerType == 'raster':
			return False	# not implemented yet
			inLayer = qgis.core.QgsRasterLayer(uriString, layerName, providerKey)
		else:
			inLayer = qgis.core.QgsVectorLayer(uriString, layerName, providerKey)

		if not inLayer.isValid():
			# invalid layer
			QMessageBox.warning(None, self.tr("Invalid layer"), self.tr("Unable to load the layer %s") % inLayer.name)
			return False

		# retrieve information about the new table's db and schema
		outItem = parent.internalPointer()
		outObj = outItem.getItemData()
		outDb = outObj.database()
		outSchema = None
		if isinstance(outItem, SchemaItem):
			outSchema = outObj
		elif isinstance(outItem, TableItem):
			outSchema = outObj.schema()

		# toIndex will point to the parent item of the new table
		toIndex = parent
		if isinstance(toIndex.internalPointer(), TableItem):
			toIndex = toIndex.parent()

		if inLayer.type() == inLayer.VectorLayer:
			# create the output uri
			schema = outSchema.name if outDb.schemas() != None and outSchema != None else ""
			pkCol = geomCol = ""

			# default pk and geom field name value
			if providerKey in ['postgres', 'spatialite']:
				inUri = qgis.core.QgsDataSourceURI( inLayer.source() )
				pkCol = inUri.keyColumn()
				geomCol = inUri.geometryColumn()

			outUri = outDb.uri()
			outUri.setDataSource( schema, layerName, geomCol, "", pkCol )

			self.emit( SIGNAL("importVector"), inLayer, outDb, outUri, toIndex )
			return True

		return False

	def importVector(self, inLayer, outDb, outUri, parent):
		if not self.isImportVectorAvail:
			return False

		try:
			from dlg_import_vector import DlgImportVector
			dlg = DlgImportVector(inLayer, outDb, outUri)
			QApplication.restoreOverrideCursor()
			if dlg.exec_():
				self._refreshIndex( parent )
		finally:
			inLayer.deleteLater()
