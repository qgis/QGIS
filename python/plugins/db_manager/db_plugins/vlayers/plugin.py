"""
/***************************************************************************
Name                 : DB Manager plugin for virtual layers
Date                 : December 2015
copyright            : (C) 2015 by Hugo Mercier
email                : hugo dot mercier at oslandia dot com

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
from .connector import VLayerConnector

from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtGui import QIcon
from qgis.core import (
    QgsApplication,
    QgsVectorLayer,
    QgsProject,
    QgsVirtualLayerDefinition,
)

from ..plugin import DBPlugin, Database, Table, VectorTable, TableField


def classFactory():
    return VLayerDBPlugin


class VLayerDBPlugin(DBPlugin):

    @classmethod
    def icon(self):
        return QgsApplication.getThemeIcon("/mIconVirtualLayer.svg")

    def connectionIcon(self):
        return QgsApplication.getThemeIcon("/providerQgis.svg")

    @classmethod
    def typeName(self):
        return "vlayers"

    @classmethod
    def typeNameString(self):
        return QCoreApplication.translate("db_manager", "Virtual Layers")

    @classmethod
    def providerName(self):
        return "virtual"

    @classmethod
    def connectionSettingsKey(self):
        return "vlayers"

    @classmethod
    def connections(self):
        return [
            VLayerDBPlugin(QCoreApplication.translate("db_manager", "Project layers"))
        ]

    def databasesFactory(self, connection, uri):
        return FakeDatabase(connection, uri)

    def database(self):
        return self.db

    # def info( self ):

    def connect(self, parent=None):
        self.connectToUri("qgis")
        return True


class FakeDatabase(Database):

    def __init__(self, connection, uri):
        Database.__init__(self, connection, uri)

    def connectorsFactory(self, uri):
        return VLayerConnector(uri)

    def dataTablesFactory(self, row, db, schema=None):
        return LTable(row, db, schema)

    def vectorTablesFactory(self, row, db, schema=None):
        return LVectorTable(row, db, schema)

    def rasterTablesFactory(self, row, db, schema=None):
        return None

    def info(self):
        from .info_model import LDatabaseInfo

        return LDatabaseInfo(self)

    def sqlResultModel(self, sql, parent):
        from .data_model import LSqlResultModel

        return LSqlResultModel(self, sql, parent)

    def sqlResultModelAsync(self, sql, parent):
        from .data_model import LSqlResultModelAsync

        return LSqlResultModelAsync(self, sql, parent)

    def toSqlLayer(
        self,
        sql,
        geomCol,
        uniqueCol,
        layerName="QueryLayer",
        layerType=None,
        avoidSelectById=False,
        _filter="",
    ):
        df = QgsVirtualLayerDefinition()
        df.setQuery(sql)
        if uniqueCol is not None:
            uniqueCol = uniqueCol.strip('"').replace('""', '"')
            df.setUid(uniqueCol)
        if geomCol is not None:
            df.setGeometryField(geomCol)
        vl = QgsVectorLayer(df.toString(), layerName, "virtual")
        if _filter:
            vl.setSubsetString(_filter)
        return vl

    def registerDatabaseActions(self, mainWindow):
        return

    def runAction(self, action):
        return

    def uniqueIdFunction(self):
        return None

    def explicitSpatialIndex(self):
        return True

    def spatialIndexClause(self, src_table, src_column, dest_table, dest_column):
        return '"{}"._search_frame_ = "{}"."{}"'.format(
            src_table, dest_table, dest_column
        )

    def supportsComment(self):
        return False


class LTable(Table):

    def __init__(self, row, db, schema=None):
        Table.__init__(self, db, None)
        self.name, self.isView, self.isSysTable = row

    def tableFieldsFactory(self, row, table):
        return LTableField(row, table)

    def tableDataModel(self, parent):
        from .data_model import LTableDataModel

        return LTableDataModel(self, parent)

    def canBeAddedToCanvas(self):
        return False


class LVectorTable(LTable, VectorTable):

    def __init__(self, row, db, schema=None):
        LTable.__init__(self, row[:-5], db, schema)
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
        return True

    def createSpatialIndex(self, geom_column=None):
        return

    def deleteSpatialIndex(self, geom_column=None):
        return

    def refreshTableEstimatedExtent(self):
        self.extent = self.database().connector.getTableExtent(
            ("id", self.geomTableName), None
        )

    def runAction(self, action):
        return

    def toMapLayer(self, geometryType=None, crs=None):
        return QgsProject.instance().mapLayer(self.geomTableName)


class LTableField(TableField):

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
