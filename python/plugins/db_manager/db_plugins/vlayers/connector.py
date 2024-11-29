"""
/***************************************************************************
Name                 : Virtual layers plugin for DB Manager
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

from qgis.PyQt.QtCore import QUrl, QTemporaryFile

from ..connector import DBConnector
from ..plugin import Table

from qgis.core import (
    QgsDataSourceUri,
    QgsVirtualLayerDefinition,
    QgsProject,
    QgsMapLayerType,
    QgsVectorLayer,
    QgsCoordinateReferenceSystem,
    QgsWkbTypes,
)

import sqlite3


class sqlite3_connection:

    def __init__(self, sqlite_file):
        self.conn = sqlite3.connect(sqlite_file)

    def __enter__(self):
        return self.conn

    def __exit__(self, ex_type, value, traceback):
        self.conn.close()
        return ex_type is None


def getQueryGeometryName(sqlite_file):
    # introspect the file
    with sqlite3_connection(sqlite_file) as conn:
        c = conn.cursor()
        for r in c.execute("SELECT url FROM _meta"):
            d = QgsVirtualLayerDefinition.fromUrl(QUrl(r[0]))
            if d.hasDefinedGeometry():
                return d.geometryField()
        return None


def classFactory():
    return VLayerConnector


# Tables in DB Manager are identified by their display names
# This global registry maps a display name with a layer id
# It is filled when getVectorTables is called
class VLayerRegistry:
    _instance = None

    @classmethod
    def instance(cls):
        if cls._instance is None:
            cls._instance = VLayerRegistry()
        return cls._instance

    def __init__(self):
        self.layers = {}

    def reset(self):
        self.layers = {}

    def has(self, k):
        return k in self.layers

    def get(self, k):
        return self.layers.get(k)

    def __getitem__(self, k):
        return self.get(k)

    def set(self, k, l):
        self.layers[k] = l

    def __setitem__(self, k, l):
        self.set(k, l)

    def items(self):
        return list(self.layers.items())

    def getLayer(self, l):
        lid = self.layers.get(l)
        if lid is None:
            return lid
        if lid not in QgsProject.instance().mapLayers().keys():
            self.layers.pop(l)
            return None
        return QgsProject.instance().mapLayer(lid)


class VLayerConnector(DBConnector):

    def __init__(self, uri):
        self.mapSridToName = {}

    def _execute(self, cursor, sql):
        # This is only used to get list of fields
        class DummyCursor:

            def __init__(self, sql):
                self.sql = sql

            def close(self):
                pass

        return DummyCursor(sql)

    def _get_cursor(self, name=None):
        # fix_print_with_import
        print(("_get_cursor_", name))

    def _get_cursor_columns(self, c):
        tf = QTemporaryFile()
        tf.open()
        tmp = tf.fileName()
        tf.close()

        df = QgsVirtualLayerDefinition()
        df.setFilePath(tmp)
        df.setQuery(c.sql)
        p = QgsVectorLayer(df.toString(), "vv", "virtual")
        if not p.isValid():
            return []
        f = [f.name() for f in p.fields()]
        if p.geometryType() != QgsWkbTypes.GeometryType.NullGeometry:
            gn = getQueryGeometryName(tmp)
            if gn:
                f += [gn]
        return f

    def uri(self):
        return QgsDataSourceUri("qgis")

    def getInfo(self):
        return "info"

    def getSpatialInfo(self):
        return None

    def hasSpatialSupport(self):
        return True

    def hasRasterSupport(self):
        return False

    def hasCustomQuerySupport(self):
        return True

    def hasTableColumnEditingSupport(self):
        return False

    def fieldTypes(self):
        return [
            "integer",
            "bigint",
            "smallint",  # integers
            "real",
            "double",
            "float",
            "numeric",  # floats
            "varchar",
            "varchar(255)",
            "character(20)",
            "text",  # strings
            "date",
            "datetime",  # date/time
        ]

    def getSchemas(self):
        return None

    def getTables(self, schema=None, add_sys_tables=False):
        """get list of tables"""
        return self.getVectorTables()

    def getVectorTables(self, schema=None):
        """get list of table with a geometry column
        it returns:
                name (table name)
                is_system_table
                type = 'view' (is a view?)
                geometry_column:
                        f_table_name (the table name in geometry_columns may be in a wrong case, use this to load the layer)
                        f_geometry_column
                        type
                        coord_dimension
                        srid
        """
        reg = VLayerRegistry.instance()
        VLayerRegistry.instance().reset()
        lst = []
        for _, l in QgsProject.instance().mapLayers().items():
            if l.type() == QgsMapLayerType.VectorLayer:

                lname = l.name()
                # if there is already a layer with this name, use the layer id
                # as name
                if reg.has(lname):
                    lname = l.id()
                VLayerRegistry.instance().set(lname, l.id())

                geomType = None
                dim = None
                if l.isSpatial():
                    g = l.dataProvider().wkbType()
                    g_flat = QgsWkbTypes.flatType(g)
                    geomType = QgsWkbTypes.displayString(g_flat).upper()
                    if geomType:
                        dim = "XY"
                        if QgsWkbTypes.hasZ(g):
                            dim += "Z"
                        if QgsWkbTypes.hasM(g):
                            dim += "M"
                    srid = l.crs().postgisSrid()
                    if srid not in self.mapSridToName:
                        self.mapSridToName[srid] = l.crs().description()
                    lst.append(
                        (
                            Table.VectorType,
                            lname,
                            False,
                            False,
                            l.id(),
                            "geometry",
                            geomType,
                            dim,
                            srid,
                        )
                    )
                else:
                    lst.append((Table.TableType, lname, False, False))
        return lst

    def getRasterTables(self, schema=None):
        return []

    def getTableRowCount(self, table):
        t = table[1]
        l = VLayerRegistry.instance().getLayer(t)
        if not l or not l.isValid():
            return None
        return l.featureCount()

    def getTableFields(self, table):
        """return list of columns in table"""
        t = table[1]
        l = VLayerRegistry.instance().getLayer(t)
        if not l or not l.isValid():
            return []
        # id, name, type, nonnull, default, pk
        n = l.dataProvider().fields().size()
        f = [
            (i, f.name(), f.typeName(), False, None, False)
            for i, f in enumerate(l.dataProvider().fields())
        ]
        if l.isSpatial():
            f += [(n, "geometry", "geometry", False, None, False)]
        return f

    def getTableIndexes(self, table):
        return []

    def getTableConstraints(self, table):
        return None

    def getTableTriggers(self, table):
        return []

    def deleteTableTrigger(self, trigger, table=None):
        return

    def getTableExtent(self, table, geom):
        is_id, t = table
        if is_id:
            l = QgsProject.instance().mapLayer(t)
        else:
            l = VLayerRegistry.instance().getLayer(t)
        if not l or not l.isValid():
            return None
        e = l.extent()
        r = (e.xMinimum(), e.yMinimum(), e.xMaximum(), e.yMaximum())
        return r

    def getViewDefinition(self, view):
        print("**unimplemented** getViewDefinition")

    def getSpatialRefInfo(self, srid):
        return self.mapSridToName.get(srid, "")

    def isVectorTable(self, table):
        return True

    def isRasterTable(self, table):
        return False

    def createTable(self, table, field_defs, pkey):
        print("**unimplemented** createTable")
        return False

    def deleteTable(self, table):
        print("**unimplemented** deleteTable")
        return False

    def emptyTable(self, table):
        print("**unimplemented** emptyTable")
        return False

    def renameTable(self, table, new_table):
        print("**unimplemented** renameTable")
        return False

    def moveTable(self, table, new_table, new_schema=None):
        print("**unimplemented** moveTable")
        return False

    def createView(self, view, query):
        print("**unimplemented** createView")
        return False

    def deleteView(self, view):
        print("**unimplemented** deleteView")
        return False

    def renameView(self, view, new_name):
        print("**unimplemented** renameView")
        return False

    def runVacuum(self):
        print("**unimplemented** runVacuum")
        return False

    def addTableColumn(self, table, field_def):
        print("**unimplemented** addTableColumn")
        return False

    def deleteTableColumn(self, table, column):
        print("**unimplemented** deleteTableColumn")

    def updateTableColumn(
        self,
        table,
        column,
        new_name,
        new_data_type=None,
        new_not_null=None,
        new_default=None,
        comment=None,
    ):
        print("**unimplemented** updateTableColumn")

    def renameTableColumn(self, table, column, new_name):
        print("**unimplemented** renameTableColumn")
        return False

    def setColumnType(self, table, column, data_type):
        print("**unimplemented** setColumnType")
        return False

    def setColumnDefault(self, table, column, default):
        print("**unimplemented** setColumnDefault")
        return False

    def setColumnNull(self, table, column, is_null):
        print("**unimplemented** setColumnNull")
        return False

    def isGeometryColumn(self, table, column):
        print("**unimplemented** isGeometryColumn")
        return False

    def addGeometryColumn(
        self, table, geom_column="geometry", geom_type="POINT", srid=-1, dim=2
    ):
        print("**unimplemented** addGeometryColumn")
        return False

    def deleteGeometryColumn(self, table, geom_column):
        print("**unimplemented** deleteGeometryColumn")
        return False

    def addTableUniqueConstraint(self, table, column):
        print("**unimplemented** addTableUniqueConstraint")
        return False

    def deleteTableConstraint(self, table, constraint):
        print("**unimplemented** deleteTableConstraint")
        return False

    def addTablePrimaryKey(self, table, column):
        print("**unimplemented** addTablePrimaryKey")
        return False

    def createTableIndex(self, table, name, column, unique=False):
        print("**unimplemented** createTableIndex")
        return False

    def deleteTableIndex(self, table, name):
        print("**unimplemented** deleteTableIndex")
        return False

    def createSpatialIndex(self, table, geom_column="geometry"):
        print("**unimplemented** createSpatialIndex")
        return False

    def deleteSpatialIndex(self, table, geom_column="geometry"):
        print("**unimplemented** deleteSpatialIndex")
        return False

    def hasSpatialIndex(self, table, geom_column="geometry"):
        print("**unimplemented** hasSpatialIndex")
        return False

    def execution_error_types(self):
        print("**unimplemented** execution_error_types")
        return False

    def connection_error_types(self):
        print("**unimplemented** connection_error_types")
        return False

    def getSqlDictionary(self):
        from .sql_dictionary import getSqlDictionary

        sql_dict = getSqlDictionary()

        items = []
        for tbl in self.getTables():
            items.append(tbl[1])  # table name

            for fld in self.getTableFields((None, tbl[1])):
                items.append(fld[1])  # field name

        sql_dict["identifier"] = items
        return sql_dict

    def getQueryBuilderDictionary(self):
        from .sql_dictionary import getQueryBuilderDictionary

        return getQueryBuilderDictionary()
