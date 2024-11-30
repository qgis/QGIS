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

# this will disable the dbplugin if the connector raise an ImportError
from .connector import SpatiaLiteDBConnector

from qgis.PyQt.QtCore import Qt, QFileInfo, QCoreApplication
from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtWidgets import QApplication, QAction, QFileDialog
from qgis.core import Qgis, QgsApplication, QgsDataSourceUri, QgsSettings
from qgis.gui import QgsMessageBar

from ..plugin import (
    DBPlugin,
    Database,
    Table,
    VectorTable,
    RasterTable,
    TableField,
    TableIndex,
    TableTrigger,
    InvalidDataException,
)


def classFactory():
    return SpatiaLiteDBPlugin


class SpatiaLiteDBPlugin(DBPlugin):

    @classmethod
    def icon(self):
        return QgsApplication.getThemeIcon("/mIconSpatialite.svg")

    @classmethod
    def typeName(self):
        return "spatialite"

    @classmethod
    def typeNameString(self):
        return QCoreApplication.translate("db_manager", "SpatiaLite")

    @classmethod
    def providerName(self):
        return "spatialite"

    @classmethod
    def connectionSettingsKey(self):
        return "/SpatiaLite/connections"

    def databasesFactory(self, connection, uri):
        return SLDatabase(connection, uri)

    def connect(self, parent=None):
        conn_name = self.connectionName()
        settings = QgsSettings()
        settings.beginGroup(f"/{self.connectionSettingsKey()}/{conn_name}")

        if not settings.contains("sqlitepath"):  # non-existent entry?
            raise InvalidDataException(
                self.tr('There is no defined database connection "{0}".').format(
                    conn_name
                )
            )

        database = settings.value("sqlitepath")

        uri = QgsDataSourceUri()
        uri.setDatabase(database)
        return self.connectToUri(uri)

    @classmethod
    def addConnection(self, conn_name, uri):
        settings = QgsSettings()
        settings.beginGroup(f"/{self.connectionSettingsKey()}/{conn_name}")
        settings.setValue("sqlitepath", uri.database())
        return True

    @classmethod
    def addConnectionActionSlot(self, item, action, parent, index):
        QApplication.restoreOverrideCursor()
        try:
            filename, selected_filter = QFileDialog.getOpenFileName(
                parent, "Choose SQLite/SpatiaLite file"
            )
            if not filename:
                return
        finally:
            QApplication.setOverrideCursor(Qt.CursorShape.WaitCursor)

        conn_name = QFileInfo(filename).fileName()
        uri = QgsDataSourceUri()
        uri.setDatabase(filename)
        self.addConnection(conn_name, uri)
        index.internalPointer().itemChanged()


class SLDatabase(Database):

    def __init__(self, connection, uri):
        Database.__init__(self, connection, uri)

    def connectorsFactory(self, uri):
        return SpatiaLiteDBConnector(uri)

    def dataTablesFactory(self, row, db, schema=None):
        return SLTable(row, db, schema)

    def vectorTablesFactory(self, row, db, schema=None):
        return SLVectorTable(row, db, schema)

    def rasterTablesFactory(self, row, db, schema=None):
        return SLRasterTable(row, db, schema)

    def info(self):
        from .info_model import SLDatabaseInfo

        return SLDatabaseInfo(self)

    def sqlResultModel(self, sql, parent):
        from .data_model import SLSqlResultModel

        return SLSqlResultModel(self, sql, parent)

    def sqlResultModelAsync(self, sql, parent):
        from .data_model import SLSqlResultModelAsync

        return SLSqlResultModelAsync(self, sql, parent)

    def registerDatabaseActions(self, mainWindow):
        action = QAction(self.tr("Run &Vacuum"), self)
        mainWindow.registerAction(
            action, self.tr("&Database"), self.runVacuumActionSlot
        )

        Database.registerDatabaseActions(self, mainWindow)

    def runVacuumActionSlot(self, item, action, parent):
        QApplication.restoreOverrideCursor()
        try:
            if not isinstance(item, (DBPlugin, Table)) or item.database() is None:
                parent.infoBar.pushMessage(
                    self.tr("No database selected or you are not connected to it."),
                    Qgis.MessageLevel.Info,
                    parent.iface.messageTimeout(),
                )
                return
        finally:
            QApplication.setOverrideCursor(Qt.CursorShape.WaitCursor)

        self.runVacuum()

    def runVacuum(self):
        self.database().aboutToChange.emit()
        self.database().connector.runVacuum()
        self.database().refresh()

    def runAction(self, action):
        action = str(action)

        if action.startswith("vacuum/"):
            if action == "vacuum/run":
                self.runVacuum()
                return True

        return Database.runAction(self, action)

    def uniqueIdFunction(self):
        return None

    def explicitSpatialIndex(self):
        return True

    def spatialIndexClause(self, src_table, src_column, dest_table, dest_column):
        return """ "{}".ROWID IN (\nSELECT ROWID FROM SpatialIndex WHERE f_table_name='{}' AND search_frame="{}"."{}") """.format(
            src_table, src_table, dest_table, dest_column
        )

    def supportsComment(self):
        return False


class SLTable(Table):

    def __init__(self, row, db, schema=None):
        Table.__init__(self, db, None)
        self.name, self.isView, self.isSysTable = row

    def ogrUri(self):
        ogrUri = f"{self.uri().database()}|layername={self.name}"
        return ogrUri

    def mimeUri(self):
        return Table.mimeUri(self)

    def toMapLayer(self, geometryType=None, crs=None):
        from qgis.core import QgsVectorLayer

        provider = self.database().dbplugin().providerName()
        uri = self.uri().uri()

        return QgsVectorLayer(uri, self.name, provider)

    def tableFieldsFactory(self, row, table):
        return SLTableField(row, table)

    def tableIndexesFactory(self, row, table):
        return SLTableIndex(row, table)

    def tableTriggersFactory(self, row, table):
        return SLTableTrigger(row, table)

    def tableDataModel(self, parent):
        from .data_model import SLTableDataModel

        return SLTableDataModel(self, parent)


class SLVectorTable(SLTable, VectorTable):

    def __init__(self, row, db, schema=None):
        SLTable.__init__(self, row[:-5], db, schema)
        VectorTable.__init__(self, db, schema)
        # SpatiaLite does case-insensitive checks for table names, but the
        # SL provider didn't do the same in QGIS < 1.9, so self.geomTableName
        # stores the table name like stored in the geometry_columns table
        self.geomTableName, self.geomColumn, self.geomType, self.geomDim, self.srid = (
            row[-5:]
        )

    def uri(self):
        uri = self.database().uri()
        uri.setDataSource("", self.geomTableName, self.geomColumn)
        return uri

    def hasSpatialIndex(self, geom_column=None):
        geom_column = geom_column if geom_column is not None else self.geomColumn
        return self.database().connector.hasSpatialIndex(
            (self.schemaName(), self.name), geom_column
        )

    def createSpatialIndex(self, geom_column=None):
        self.aboutToChange.emit()
        ret = VectorTable.createSpatialIndex(self, geom_column)
        if ret is not False:
            self.database().refresh()
        return ret

    def deleteSpatialIndex(self, geom_column=None):
        self.aboutToChange.emit()
        ret = VectorTable.deleteSpatialIndex(self, geom_column)
        if ret is not False:
            self.database().refresh()
        return ret

    def refreshTableEstimatedExtent(self):
        return

    def runAction(self, action):
        if SLTable.runAction(self, action):
            return True
        return VectorTable.runAction(self, action)


class SLRasterTable(SLTable, RasterTable):

    def __init__(self, row, db, schema=None):
        SLTable.__init__(self, row[:-3], db, schema)
        RasterTable.__init__(self, db, schema)
        self.prefixName, self.geomColumn, self.srid = row[-3:]
        self.geomType = "RASTER"

        # def info(self):
        # from .info_model import SLRasterTableInfo
        # return SLRasterTableInfo(self)

    def rasterliteGdalUri(self):
        gdalUri = "RASTERLITE:{},table={}".format(
            self.uri().database(), self.prefixName
        )
        return gdalUri

    def mimeUri(self):
        # QGIS has no provider to load rasters, let's use GDAL
        uri = f"raster:gdal:{self.name}:{self.uri().database()}"
        return uri

    def toMapLayer(self, geometryType=None, crs=None):
        from qgis.core import QgsRasterLayer, QgsContrastEnhancement

        # QGIS has no provider to load Rasterlite rasters, let's use GDAL
        uri = self.rasterliteGdalUri()

        rl = QgsRasterLayer(uri, self.name)
        if rl.isValid():
            rl.setContrastEnhancement(
                QgsContrastEnhancement.ContrastEnhancementAlgorithm.StretchToMinimumMaximum
            )
        return rl


class SLTableField(TableField):

    def __init__(self, row, table):
        TableField.__init__(self, table)
        (
            self.num,
            self.name,
            self.dataType,
            self.notNull,
            self.default,
            self.primaryKey,
        ) = row
        self.hasDefault = self.default


class SLTableIndex(TableIndex):

    def __init__(self, row, table):
        TableIndex.__init__(self, table)
        self.num, self.name, self.isUnique, self.columns = row


class SLTableTrigger(TableTrigger):

    def __init__(self, row, table):
        TableTrigger.__init__(self, table)
        self.name, self.function = row
