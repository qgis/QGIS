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

from functools import partial
from qgis.PyQt.QtCore import Qt, QObject, qDebug, QByteArray, QMimeData, QDataStream, QIODevice, QFileInfo, QAbstractItemModel, QModelIndex, pyqtSignal
from qgis.PyQt.QtWidgets import QApplication, QMessageBox
from qgis.PyQt.QtGui import QIcon

from .db_plugins import supportedDbTypes, createDbPlugin
from .db_plugins.plugin import BaseError, Table, Database
from .dlg_db_error import DlgDbError
from .gui_utils import GuiUtils

from qgis.core import (
    QgsApplication,
    QgsDataSourceUri,
    QgsVectorLayer,
    QgsRasterLayer,
    QgsMimeDataUtils,
    QgsProviderConnectionException,
    QgsProviderRegistry,
    QgsAbstractDatabaseProviderConnection,
    QgsMessageLog,
)

from qgis.utils import OverrideCursor

try:
    from qgis.core import QgsVectorLayerExporter  # NOQA

    isImportVectorAvail = True
except:
    isImportVectorAvail = False


class TreeItem(QObject):
    deleted = pyqtSignal()
    changed = pyqtSignal()

    def __init__(self, data, parent=None):
        QObject.__init__(self, parent)
        self.populated = False
        self.itemData = data
        self.childItems = []
        if parent:
            parent.appendChild(self)

    def childRemoved(self):
        self.itemChanged()

    def itemChanged(self):
        self.changed.emit()

    def itemDeleted(self):
        self.deleted.emit()

    def populate(self):
        self.populated = True
        return True

    def getItemData(self):
        return self.itemData

    def appendChild(self, child):
        self.childItems.append(child)
        child.deleted.connect(self.childRemoved)

    def child(self, row):
        return self.childItems[row]

    def removeChild(self, row):
        if row >= 0 and row < len(self.childItems):
            self.childItems[row].itemData.deleteLater()
            self.childItems[row].deleted.disconnect(self.childRemoved)
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
            pathList.extend(self.parent().path())
        pathList.append(self.data(0))
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
        return [self.getItemData().typeName()]


class ConnectionItem(TreeItem):

    def __init__(self, connection, parent=None):
        TreeItem.__init__(self, connection, parent)
        connection.changed.connect(self.itemChanged)
        connection.deleted.connect(self.itemDeleted)

        # load (shared) icon with first instance of table item
        if not hasattr(ConnectionItem, 'connectedIcon'):
            ConnectionItem.connectedIcon = GuiUtils.get_icon("plugged")
            ConnectionItem.disconnectedIcon = GuiUtils.get_icon("unplugged")

    def data(self, column):
        if column == 0:
            return self.getItemData().connectionName()
        return None

    def icon(self):
        return self.getItemData().connectionIcon()

    def populate(self):
        if self.populated:
            return True

        connection = self.getItemData()
        if connection.database() is None:
            # connect to database
            try:
                if not connection.connect():
                    return False

            except BaseError as e:
                DlgDbError.showError(e, None)
                return False

        database = connection.database()
        database.changed.connect(self.itemChanged)
        database.deleted.connect(self.itemDeleted)

        schemas = database.schemas()
        if schemas is not None:
            for s in schemas:
                SchemaItem(s, self)
        else:
            tables = database.tables()
            for t in tables:
                TableItem(t, self)

        self.populated = True
        return True

    def isConnected(self):
        return self.getItemData().database() is not None

        # def icon(self):
        #       return self.connectedIcon if self.isConnected() else self.disconnectedIcon


class SchemaItem(TreeItem):

    def __init__(self, schema, parent):
        TreeItem.__init__(self, schema, parent)
        schema.changed.connect(self.itemChanged)
        schema.deleted.connect(self.itemDeleted)

        # load (shared) icon with first instance of schema item
        if not hasattr(SchemaItem, 'schemaIcon'):
            SchemaItem.schemaIcon = GuiUtils.get_icon("namespace")

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
        table.changed.connect(self.itemChanged)
        table.deleted.connect(self.itemDeleted)
        self.populate()

        # load (shared) icon with first instance of table item
        if not hasattr(TableItem, 'tableIcon'):
            TableItem.tableIcon = QgsApplication.getThemeIcon("/mIconTableLayer.svg")
            TableItem.viewIcon = GuiUtils.get_icon("view")
            TableItem.viewMaterializedIcon = GuiUtils.get_icon("view_materialized")
            TableItem.layerPointIcon = QgsApplication.getThemeIcon("/mIconPointLayer.svg")
            TableItem.layerLineIcon = QgsApplication.getThemeIcon("/mIconLineLayer.svg")
            TableItem.layerPolygonIcon = QgsApplication.getThemeIcon("/mIconPolygonLayer.svg")
            TableItem.layerRasterIcon = QgsApplication.getThemeIcon("/mIconRasterLayer.svg")
            TableItem.layerUnknownIcon = GuiUtils.get_icon("layer_unknown")

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
                elif geom_type.find('LINESTRING') != -1 or geom_type in ('CIRCULARSTRING', 'COMPOUNDCURVE', 'MULTICURVE'):
                    return self.layerLineIcon
                elif geom_type.find('POLYGON') != -1 or geom_type == 'MULTISURFACE':
                    return self.layerPolygonIcon
                return self.layerUnknownIcon

        elif self.getItemData().type == Table.RasterType:
            return self.layerRasterIcon

        if self.getItemData().isView:
            if hasattr(self.getItemData(), '_relationType') and self.getItemData()._relationType == 'm':
                return self.viewMaterializedIcon
            else:
                return self.viewIcon
        return self.tableIcon

    def path(self):
        pathList = []
        if self.parent():
            pathList.extend(self.parent().path())

        if self.getItemData().type == Table.VectorType:
            pathList.append("%s::%s" % (self.data(0), self.getItemData().geomColumn))
        else:
            pathList.append(self.data(0))

        return pathList


class DBModel(QAbstractItemModel):
    importVector = pyqtSignal(QgsVectorLayer, Database, QgsDataSourceUri, QModelIndex)
    notPopulated = pyqtSignal(QModelIndex)

    def __init__(self, parent=None):
        global isImportVectorAvail

        QAbstractItemModel.__init__(self, parent)
        self.treeView = parent
        self.header = [self.tr('Databases')]

        if isImportVectorAvail:
            self.importVector.connect(self.vectorImport)

        self.hasSpatialiteSupport = "spatialite" in supportedDbTypes()
        self.hasGPKGSupport = "gpkg" in supportedDbTypes()

        self.rootItem = TreeItem(None, None)
        for dbtype in supportedDbTypes():
            dbpluginclass = createDbPlugin(dbtype)
            item = PluginItem(dbpluginclass, self.rootItem)
            item.changed.connect(partial(self.refreshItem, item))

    def refreshItem(self, item):
        if isinstance(item, TreeItem):
            # find the index for the tree item using the path
            index = self._rPath2Index(item.path())
        else:
            # find the index for the db item
            index = self._rItem2Index(item)
        if index.isValid():
            self._refreshIndex(index)
        else:
            qDebug("invalid index")

    def _rItem2Index(self, item, parent=None):
        if parent is None:
            parent = QModelIndex()
        if item == self.getItem(parent):
            return parent

        if not parent.isValid() or parent.internalPointer().populated:
            for i in range(self.rowCount(parent)):
                index = self.index(i, 0, parent)
                index = self._rItem2Index(item, index)
                if index.isValid():
                    return index

        return QModelIndex()

    def _rPath2Index(self, path, parent=None, n=0):
        if parent is None:
            parent = QModelIndex()
        if path is None or len(path) == 0:
            return parent

        for i in range(self.rowCount(parent)):
            index = self.index(i, 0, parent)
            if self._getPath(index)[n] == path[0]:
                return self._rPath2Index(path[1:], index, n + 1)

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

        if role == Qt.ItemDataRole.DecorationRole and index.column() == 0:
            icon = index.internalPointer().icon()
            if icon:
                return icon

        if role != Qt.ItemDataRole.DisplayRole and role != Qt.ItemDataRole.EditRole:
            return None

        retval = index.internalPointer().data(index.column())
        return retval

    def flags(self, index):
        global isImportVectorAvail

        if not index.isValid():
            return Qt.ItemFlag.NoItemFlags

        flags = Qt.ItemFlag.ItemIsEnabled | Qt.ItemFlag.ItemIsSelectable

        if index.column() == 0:
            item = index.internalPointer()

            if isinstance(item, SchemaItem) or isinstance(item, TableItem):
                flags |= Qt.ItemFlag.ItemIsEditable

            if isinstance(item, TableItem):
                flags |= Qt.ItemFlag.ItemIsDragEnabled

            # vectors/tables can be dropped on connected databases to be imported
            if isImportVectorAvail:
                if isinstance(item, ConnectionItem) and item.populated:
                    flags |= Qt.ItemFlag.ItemIsDropEnabled

                if isinstance(item, (SchemaItem, TableItem)):
                    flags |= Qt.ItemFlag.ItemIsDropEnabled

            # SL/Geopackage db files can be dropped everywhere in the tree
            if self.hasSpatialiteSupport or self.hasGPKGSupport:
                flags |= Qt.ItemFlag.ItemIsDropEnabled

        return flags

    def headerData(self, section, orientation, role):
        if orientation == Qt.Orientation.Horizontal and role == Qt.ItemDataRole.DisplayRole and section < len(self.header):
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
            self._refreshIndex(parent, True)
        return parentItem.childCount()

    def hasChildren(self, parent):
        parentItem = parent.internalPointer() if parent.isValid() else self.rootItem
        return parentItem.childCount() > 0 or not parentItem.populated

    def setData(self, index, value, role):
        if role != Qt.ItemDataRole.EditRole or index.column() != 0:
            return False

        item = index.internalPointer()
        new_value = str(value)

        if isinstance(item, SchemaItem) or isinstance(item, TableItem):
            obj = item.getItemData()

            # rename schema or table or view
            if new_value == obj.name:
                return False

            with OverrideCursor(Qt.CursorShape.WaitCursor):
                try:
                    obj.rename(new_value)
                    self._onDataChanged(index)
                except BaseError as e:
                    DlgDbError.showError(e, self.treeView)
                    return False
                else:
                    return True

        return False

    def removeRows(self, row, count, parent):
        self.beginRemoveRows(parent, row, count + row - 1)
        item = parent.internalPointer()
        for i in range(row, count + row):
            item.removeChild(row)
        self.endRemoveRows()

    def _refreshIndex(self, index, force=False):
        with OverrideCursor(Qt.CursorShape.WaitCursor):
            try:
                item = index.internalPointer() if index.isValid() else self.rootItem
                prevPopulated = item.populated
                if prevPopulated:
                    self.removeRows(0, self.rowCount(index), index)
                    item.populated = False
                if prevPopulated or force:
                    if item.populate():
                        for child in item.childItems:
                            child.changed.connect(partial(self.refreshItem, child))
                        self._onDataChanged(index)
                    else:
                        self.notPopulated.emit(index)

            except BaseError:
                item.populated = False

    def _onDataChanged(self, indexFrom, indexTo=None):
        if indexTo is None:
            indexTo = indexFrom
        self.dataChanged.emit(indexFrom, indexTo)

    QGIS_URI_MIME = "application/x-vnd.qgis.qgis.uri"

    def mimeTypes(self):
        return ["text/uri-list", self.QGIS_URI_MIME]

    def mimeData(self, indexes):
        mimeData = QMimeData()
        encodedData = QByteArray()

        stream = QDataStream(encodedData, QIODevice.OpenModeFlag.WriteOnly)

        for index in indexes:
            if not index.isValid():
                continue
            if not isinstance(index.internalPointer(), TableItem):
                continue
            table = self.getItem(index)
            stream.writeQString(table.mimeUri())

        mimeData.setData(self.QGIS_URI_MIME, encodedData)
        return mimeData

    def dropMimeData(self, data, action, row, column, parent):
        global isImportVectorAvail

        if action == Qt.DropAction.IgnoreAction:
            return True

        # vectors/tables to be imported must be dropped on connected db, schema or table
        canImportLayer = isImportVectorAvail and parent.isValid() and \
            (isinstance(parent.internalPointer(), (SchemaItem, TableItem)) or
             (isinstance(parent.internalPointer(), ConnectionItem) and parent.internalPointer().populated))

        added = 0

        if data.hasUrls():
            for u in data.urls():
                filename = u.toLocalFile()
                if filename == "":
                    continue

                if self.hasSpatialiteSupport:
                    from .db_plugins.spatialite.connector import SpatiaLiteDBConnector

                    if SpatiaLiteDBConnector.isValidDatabase(filename):
                        # retrieve the SL plugin tree item using its path
                        index = self._rPath2Index(["spatialite"])
                        if not index.isValid():
                            continue
                        item = index.internalPointer()

                        conn_name = QFileInfo(filename).fileName()
                        uri = QgsDataSourceUri()
                        uri.setDatabase(filename)
                        item.getItemData().addConnection(conn_name, uri)
                        item.changed.emit()
                        added += 1
                        continue

                if canImportLayer:
                    if QgsRasterLayer.isValidRasterFileName(filename):
                        layerType = 'raster'
                        providerKey = 'gdal'
                    else:
                        layerType = 'vector'
                        providerKey = 'ogr'

                    layerName = QFileInfo(filename).completeBaseName()
                    if self.importLayer(layerType, providerKey, layerName, filename, parent):
                        added += 1

        if data.hasFormat(self.QGIS_URI_MIME):
            for uri in QgsMimeDataUtils.decodeUriList(data):
                if canImportLayer:
                    if self.importLayer(uri.layerType, uri.providerKey, uri.name, uri.uri, parent):
                        added += 1

        return added > 0

    def importLayer(self, layerType, providerKey, layerName, uriString, parent):
        global isImportVectorAvail

        if not isImportVectorAvail:
            return False

        if layerType == 'raster':
            return False  # not implemented yet
            inLayer = QgsRasterLayer(uriString, layerName, providerKey)
        else:
            inLayer = QgsVectorLayer(uriString, layerName, providerKey)

        if not inLayer.isValid():
            # invalid layer
            QMessageBox.warning(None, self.tr("Invalid layer"), self.tr("Unable to load the layer {0}").format(inLayer.name()))
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
            schema = outSchema.name if outDb.schemas() is not None and outSchema is not None else ""
            pkCol = geomCol = ""

            # default pk and geom field name value
            if providerKey in ['postgres', 'spatialite']:
                inUri = QgsDataSourceUri(inLayer.source())
                pkCol = inUri.keyColumn()
                geomCol = inUri.geometryColumn()

            outUri = outDb.uri()
            outUri.setDataSource(schema, layerName, geomCol, "", pkCol)

            self.importVector.emit(inLayer, outDb, outUri, toIndex)
            return True

        return False

    def vectorImport(self, inLayer, outDb, outUri, parent):
        global isImportVectorAvail

        if not isImportVectorAvail:
            return False

        try:
            from .dlg_import_vector import DlgImportVector

            dlg = DlgImportVector(inLayer, outDb, outUri)
            QApplication.restoreOverrideCursor()
            if dlg.exec():
                self._refreshIndex(parent)
        finally:
            inLayer.deleteLater()
