# -*- coding: utf-8 -*-

"""
/***************************************************************************
Name                 : DB Manager
Description          : Database manager plugin for QGIS
Date                 : Oct 14 2016
copyright            : (C) 2016 by Even Rouault
                       (C) 2011 by Giuseppe Sucameli
email                : even.rouault at spatialys.com

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
from builtins import str

from functools import cmp_to_key

from qgis.PyQt.QtWidgets import QApplication
from qgis.PyQt.QtCore import QThread

from ..connector import DBConnector
from ..plugin import ConnectionError, DbError, Table

from qgis.utils import spatialite_connect
from qgis.core import (
    QgsApplication,
    QgsProviderRegistry,
    QgsAbstractDatabaseProviderConnection,
    QgsProviderConnectionException,
    QgsWkbTypes,
)

import sqlite3

from osgeo import gdal, ogr, osr


def classFactory():
    return GPKGDBConnector


class GPKGDBConnector(DBConnector):

    def __init__(self, uri, connection):
        """Creates a new GPKG connector

        :param uri: data source URI
        :type uri: QgsDataSourceUri
        :param connection: the GPKGDBPlugin parent instance
        :type connection: GPKGDBPlugin
        """

        DBConnector.__init__(self, uri)
        self.dbname = uri.database()
        self.connection = connection
        self._current_thread = None
        md = QgsProviderRegistry.instance().providerMetadata(connection.providerName())
        # QgsAbstractDatabaseProviderConnection instance
        self.core_connection = md.findConnection(connection.connectionName())
        if self.core_connection is None:
            self.core_connection = md.createConnection(uri.uri(), {})
        self.has_raster = False
        self.mapSridToName = {}
        # To be removed when migration to new API is completed
        self._opendb()

    def _opendb(self):

        # Keep this explicit assignment to None to make sure the file is
        # properly closed before being re-opened
        self.gdal_ds = None
        self.gdal_ds = gdal.OpenEx(self.dbname, gdal.OF_UPDATE)
        if self.gdal_ds is None:
            self.gdal_ds = gdal.OpenEx(self.dbname)
        if self.gdal_ds is None:
            raise ConnectionError(QApplication.translate("DBManagerPlugin", '"{0}" not found').format(self.dbname))
        if self.gdal_ds.GetDriver().ShortName != 'GPKG':
            raise ConnectionError(QApplication.translate("DBManagerPlugin", '"{dbname}" not recognized as GPKG ({shortname} reported instead.)').format(dbname=self.dbname, shortname=self.gdal_ds.GetDriver().ShortName))
        self.has_raster = self.gdal_ds.RasterCount != 0 or self.gdal_ds.GetMetadata('SUBDATASETS') is not None
        self.connection = None
        self._current_thread = None

    @property
    def connection(self):
        """Creates and returns a spatialite connection, if
        the existing connection was created in another thread
        invalidates it and create a new one.
        """

        if self._connection is None or self._current_thread != int(QThread.currentThreadId()):
            self._current_thread = int(QThread.currentThreadId())
            try:
                self._connection = spatialite_connect(str(self.dbname))
            except self.connection_error_types() as e:
                raise ConnectionError(e)
        return self._connection

    @connection.setter
    def connection(self, conn):
        self._connection = conn

    def unquoteId(self, quotedId):
        if len(quotedId) <= 2 or quotedId[0] != '"' or quotedId[len(quotedId) - 1] != '"':
            return quotedId
        unquoted = ''
        i = 1
        while i < len(quotedId) - 1:
            if quotedId[i] == '"' and quotedId[i + 1] == '"':
                unquoted += '"'
                i += 2
            else:
                unquoted += quotedId[i]
                i += 1
        return unquoted

    def _fetchOne(self, sql):

        return self.core_connection.executeSql(sql)

    def _fetchAll(self, sql, include_fid_and_geometry=False):

        return self.core_connection.executeSql(sql)

    def _fetchAllFromLayer(self, table):

        lyr = self.gdal_ds.GetLayerByName(table.name)
        if lyr is None:
            return []

        lyr.ResetReading()
        ret = []
        while True:
            f = lyr.GetNextFeature()
            if f is None:
                break
            else:
                field_vals = [f.GetFID()]
                if lyr.GetLayerDefn().GetGeomType() != ogr.wkbNone:
                    geom = f.GetGeometryRef()
                    if geom is not None:
                        geom = geom.ExportToWkt()
                    field_vals += [geom]
                field_vals += [f.GetField(i) for i in range(f.GetFieldCount())]
                ret.append(field_vals)
        return ret

    def _execute_and_commit(self, sql):
        sql_lyr = self.gdal_ds.ExecuteSQL(sql)
        self.gdal_ds.ReleaseResultSet(sql_lyr)

    def _execute(self, cursor, sql):

        if self.connection is None:
            # Needed when evaluating a SQL query
            try:
                self.connection = spatialite_connect(str(self.dbname))
            except self.connection_error_types() as e:
                raise ConnectionError(e)

        return DBConnector._execute(self, cursor, sql)

    def _commit(self):
        if self.connection is None:
            return

        try:
            self.connection.commit()

        except self.connection_error_types() as e:
            raise ConnectionError(e)

        except self.execution_error_types() as e:
            # do the rollback to avoid a "current transaction aborted, commands ignored" errors
            self._rollback()
            raise DbError(e)

    def cancel(self):
        if self.connection:
            self.connection.interrupt()

    @classmethod
    def isValidDatabase(cls, path):
        if hasattr(gdal, 'OpenEx'):
            ds = gdal.OpenEx(path)
            if ds is None or ds.GetDriver().ShortName != 'GPKG':
                return False
        else:
            ds = ogr.Open(path)
            if ds is None or ds.GetDriver().GetName() != 'GPKG':
                return False
        return True

    def getInfo(self):
        return None

    def getSpatialInfo(self):
        return None

    def hasSpatialSupport(self):
        return True

    # Used by DlgTableProperties
    def canAddGeometryColumn(self, table):
        _, tablename = self.getSchemaTableName(table)
        lyr = self.gdal_ds.GetLayerByName(tablename)
        if lyr is None:
            return False
        return lyr.GetGeomType() == ogr.wkbNone

    # Used by DlgTableProperties
    def canAddSpatialIndex(self, table):
        _, tablename = self.getSchemaTableName(table)
        lyr = self.gdal_ds.GetLayerByName(tablename)
        if lyr is None or lyr.GetGeometryColumn() == '':
            return False
        return not self.hasSpatialIndex(table,
                                        lyr.GetGeometryColumn())

    def hasRasterSupport(self):
        return self.has_raster

    def hasCustomQuerySupport(self):
        return True

    def hasTableColumnEditingSupport(self):
        return True

    def hasCreateSpatialViewSupport(self):
        return False

    def fieldTypes(self):
        # From "Table 1. GeoPackage Data Types" (http://www.geopackage.org/spec/)
        return [
            "TEXT",
            "MEDIUMINT",
            "INTEGER",
            "TINYINT",
            "SMALLINT",
            "DOUBLE",
            "FLOAT"
            "DATE",
            "DATETIME",
            "BOOLEAN",
        ]

    def getSchemas(self):
        return None

    def getTables(self, schema=None, add_sys_tables=False):
        """ get list of tables """
        items = []

        try:
            vectors = self.getVectorTables(schema)
            for tbl in vectors:
                items.append(tbl)
        except DbError:
            pass

        try:
            rasters = self.getRasterTables(schema)
            for tbl in rasters:
                items.append(tbl)
        except DbError:
            pass

        for i, tbl in enumerate(items):
            tbl.insert(3, False)  # not system table

        return sorted(items, key=cmp_to_key(lambda x, y: (x[1] > y[1]) - (x[1] < y[1])))

    def getVectorTables(self, schema=None):
        """Returns a list of vector table information
        """

        items = []
        for table in self.core_connection.tables(schema, QgsAbstractDatabaseProviderConnection.Vector | QgsAbstractDatabaseProviderConnection.Aspatial):
            if not (table.flags() & QgsAbstractDatabaseProviderConnection.Aspatial):
                geom_type = table.geometryColumnTypes()[0]
                # Use integer PG code for SRID
                srid = geom_type.crs.postgisSrid()
                geomtype_flatten = QgsWkbTypes.flatType(geom_type.wkbType)
                geomname = 'GEOMETRY'
                if geomtype_flatten == QgsWkbTypes.Point:
                    geomname = 'POINT'
                elif geomtype_flatten == QgsWkbTypes.LineString:
                    geomname = 'LINESTRING'
                elif geomtype_flatten == QgsWkbTypes.Polygon:
                    geomname = 'POLYGON'
                elif geomtype_flatten == QgsWkbTypes.MultiPoint:
                    geomname = 'MULTIPOINT'
                elif geomtype_flatten == QgsWkbTypes.MultiLineString:
                    geomname = 'MULTILINESTRING'
                elif geomtype_flatten == QgsWkbTypes.MultiPolygon:
                    geomname = 'MULTIPOLYGON'
                elif geomtype_flatten == QgsWkbTypes.GeometryCollection:
                    geomname = 'GEOMETRYCOLLECTION'
                elif geomtype_flatten == QgsWkbTypes.CircularString:
                    geomname = 'CIRCULARSTRING'
                elif geomtype_flatten == QgsWkbTypes.CompoundCurve:
                    geomname = 'COMPOUNDCURVE'
                elif geomtype_flatten == QgsWkbTypes.CurvePolygon:
                    geomname = 'CURVEPOLYGON'
                elif geomtype_flatten == QgsWkbTypes.MultiCurve:
                    geomname = 'MULTICURVE'
                elif geomtype_flatten == QgsWkbTypes.MultiSurface:
                    geomname = 'MULTISURFACE'
                geomdim = 'XY'
                if QgsWkbTypes.hasZ(geom_type.wkbType):
                    geomdim += 'Z'
                if QgsWkbTypes.hasM(geom_type.wkbType):
                    geomdim += 'M'
                item = [
                    Table.VectorType,
                    table.tableName(),
                    bool(table.flags() & QgsAbstractDatabaseProviderConnection.View),  # is_view
                    table.tableName(),
                    table.geometryColumn(),
                    geomname,
                    geomdim,
                    srid
                ]
                self.mapSridToName[srid] = geom_type.crs.description()
            else:
                item = [
                    Table.TableType,
                    table.tableName(),
                    bool(table.flags() & QgsAbstractDatabaseProviderConnection.View),
                ]

            items.append(item)

        return items

    def getRasterTables(self, schema=None):
        """ get list of table with a geometry column
                it returns:
                        name (table name)
                        type = 'view' (is a view?)
                        geometry_column:
                                r.table_name (the prefix table name, use this to load the layer)
                                r.geometry_column
                                srid
        """

        items = []
        for table in self.core_connection.tables(schema, QgsAbstractDatabaseProviderConnection.Raster):
            geom_type = table.geometryColumnTypes()[0]
            # Use integer PG code for SRID
            srid = geom_type.crs.postgisSrid()
            item = [
                Table.RasterType,
                table.tableName(),
                bool(table.flags() & QgsAbstractDatabaseProviderConnection.View),
                table.tableName(),
                table.geometryColumn(),
                srid,
            ]
            self.mapSridToName[srid] = geom_type.crs.description()
            items.append(item)

        return items

    def getTableRowCount(self, table):
        lyr = self.gdal_ds.GetLayerByName(self.getSchemaTableName(table)[1])
        return lyr.GetFeatureCount() if lyr is not None else None

    def getTableFields(self, table):
        """ return list of columns in table """
        sql = "PRAGMA table_info(%s)" % (self.quoteId(table))
        ret = self._fetchAll(sql)
        if ret is None:
            ret = []
        return ret

    def getTableIndexes(self, table):
        """ get info about table's indexes """
        sql = "PRAGMA index_list(%s)" % (self.quoteId(table))
        indexes = self._fetchAll(sql)
        if indexes is None:
            return []

        for i, idx in enumerate(indexes):
            # sqlite has changed the number of columns returned by index_list since 3.8.9
            # I am not using self.getInfo() here because this behavior
            # can be changed back without notice as done for index_info, see:
            # http://repo.or.cz/sqlite.git/commit/53555d6da78e52a430b1884b5971fef33e9ccca4
            if len(idx) == 3:
                num, name, unique = idx
            if len(idx) == 5:
                num, name, unique, createdby, partial = idx
            sql = "PRAGMA index_info(%s)" % (self.quoteId(name))

            idx = [num, name, unique]
            cols = [
                cid
                for seq, cid, cname in self._fetchAll(sql)
            ]
            idx.append(cols)
            indexes[i] = idx

        return indexes

    def getTableConstraints(self, table):
        return None

    def getTableTriggers(self, table):

        _, tablename = self.getSchemaTableName(table)
        # Do not list rtree related triggers as we don't want them to be dropped
        sql = "SELECT name, sql FROM sqlite_master WHERE tbl_name = %s AND type = 'trigger'" % (self.quoteString(tablename))
        if self.isVectorTable(table):
            sql += " AND name NOT LIKE 'rtree_%%'"
        elif self.isRasterTable(table):
            sql += " AND name NOT LIKE '%%_zoom_insert'"
            sql += " AND name NOT LIKE '%%_zoom_update'"
            sql += " AND name NOT LIKE '%%_tile_column_insert'"
            sql += " AND name NOT LIKE '%%_tile_column_update'"
            sql += " AND name NOT LIKE '%%_tile_row_insert'"
            sql += " AND name NOT LIKE '%%_tile_row_update'"
        return self._fetchAll(sql)

    def deleteTableTrigger(self, trigger, table=None):
        """Deletes trigger """
        sql = "DROP TRIGGER %s" % self.quoteId(trigger)
        self._execute_and_commit(sql)

    def getTableExtent(self, table, geom, force=False):
        """ find out table extent """
        _, tablename = self.getSchemaTableName(table)

        if self.isRasterTable(table):

            md = self.gdal_ds.GetMetadata('SUBDATASETS')
            if md is None or len(md) == 0:
                ds = self.gdal_ds
            else:
                subdataset_name = 'GPKG:%s:%s' % (self.gdal_ds.GetDescription(), tablename)
                ds = gdal.Open(subdataset_name)
            if ds is None:
                return None
            gt = ds.GetGeoTransform()
            minx = gt[0]
            maxx = gt[0] + gt[1] * ds.RasterYSize
            maxy = gt[3]
            miny = gt[3] + gt[5] * ds.RasterYSize

            return (minx, miny, maxx, maxy)

        lyr = self.gdal_ds.GetLayerByName(tablename)
        if lyr is None:
            return None
        ret = lyr.GetExtent(force=force, can_return_null=True)
        if ret is None:
            return None
        minx, maxx, miny, maxy = ret
        return (minx, miny, maxx, maxy)

    def getViewDefinition(self, view):
        """ returns definition of the view """
        return None

    def getSpatialRefInfo(self, srid):
        if srid in self.mapSridToName:
            return self.mapSridToName[srid]

        sql = "SELECT srs_name FROM gpkg_spatial_ref_sys WHERE srs_id = %s" % self.quoteString(srid)
        res = self._fetchOne(sql)
        if res is not None and len(res) > 0:
            res = res[0]
        self.mapSridToName[srid] = res
        return res

    def isVectorTable(self, table):

        _, tablename = self.getSchemaTableName(table)
        return self.gdal_ds.GetLayerByName(tablename) is not None

    def isRasterTable(self, table):
        if self.has_raster and not self.isVectorTable(table):
            _, tablename = self.getSchemaTableName(table)
            md = self.gdal_ds.GetMetadata('SUBDATASETS')
            if md is None or len(md) == 0:
                sql = "SELECT COUNT(*) FROM gpkg_contents WHERE data_type = 'tiles' AND table_name = %s" % self.quoteString(tablename)
                ret = self._fetchOne(sql)
                return ret != [] and ret[0][0] == 1
            else:
                subdataset_name = 'GPKG:%s:%s' % (self.gdal_ds.GetDescription(), tablename)
                for key in md:
                    if md[key] == subdataset_name:
                        return True

        return False

    def getOGRFieldTypeFromSQL(self, sql_type):
        ogr_type = ogr.OFTString
        ogr_subtype = ogr.OFSTNone
        width = 0
        if not sql_type.startswith('TEXT ('):
            pos = sql_type.find(' (')
            if pos >= 0:
                sql_type = sql_type[0:pos]
        if sql_type == 'BOOLEAN':
            ogr_type = ogr.OFTInteger
            ogr_subtype = ogr.OFSTBoolean
        elif sql_type in ('TINYINT', 'SMALLINT', 'MEDIUMINT'):
            ogr_type = ogr.OFTInteger
        elif sql_type == 'INTEGER':
            ogr_type = ogr.OFTInteger64
        elif sql_type == 'FLOAT':
            ogr_type = ogr.OFTReal
            ogr_subtype = ogr.OFSTFloat32
        elif sql_type == 'DOUBLE':
            ogr_type = ogr.OFTReal
        elif sql_type == 'DATE':
            ogr_type = ogr.OFTDate
        elif sql_type == 'DATETIME':
            ogr_type = ogr.OFTDateTime
        elif sql_type.startswith('TEXT (') and sql_type.endswith(')'):
            width = int(sql_type[len('TEXT ('):-1])
        return (ogr_type, ogr_subtype, width)

    def createOGRFieldDefnFromSQL(self, sql_fielddef):
        f_split = sql_fielddef.split(' ')
        quoted_name = f_split[0]
        name = self.unquoteId(quoted_name)
        sql_type = f_split[1].upper()
        if len(f_split) >= 3 and f_split[2].startswith('(') and f_split[2].endswith(')'):
            sql_type += ' ' + f_split[2]
            f_split = [f for f in f_split[3:]]
        else:
            f_split = [f for f in f_split[2:]]
        ogr_type, ogr_subtype, width = self.getOGRFieldTypeFromSQL(sql_type)
        fld_defn = ogr.FieldDefn(name, ogr_type)
        fld_defn.SetSubType(ogr_subtype)
        fld_defn.SetWidth(width)
        if len(f_split) >= 2 and f_split[0] == 'NOT' and f_split[1] == 'NULL':
            fld_defn.SetNullable(False)
            f_split = [f for f in f_split[2:]]
        elif len(f_split) >= 1:
            f_split = [f for f in f_split[1:]]
        if len(f_split) >= 2 and f_split[0] == 'DEFAULT':
            new_default = f_split[1]
            if new_default == '':
                fld_defn.SetDefault(None)
            elif new_default == 'NULL' or ogr_type in (ogr.OFTInteger, ogr.OFTReal):
                fld_defn.SetDefault(new_default)
            elif new_default.startswith("'") and new_default.endswith("'"):
                fld_defn.SetDefault(new_default)
            else:
                fld_defn.SetDefault(self.quoteString(new_default))
        return fld_defn

    def createTable(self, table, field_defs, pkey):
        """Creates ordinary table
                        'fields' is array containing field definitions
                        'pkey' is the primary key name
        """
        if len(field_defs) == 0:
            return False

        options = []
        if pkey is not None and pkey != "":
            options += ['FID=' + pkey]
        _, tablename = self.getSchemaTableName(table)
        lyr = self.gdal_ds.CreateLayer(tablename, geom_type=ogr.wkbNone, options=options)
        if lyr is None:
            return False
        for field_def in field_defs:
            fld_defn = self.createOGRFieldDefnFromSQL(field_def)
            if fld_defn.GetName() == pkey:
                continue
            if lyr.CreateField(fld_defn) != 0:
                return False

        return True

    def deleteTable(self, table):
        """Deletes table from the database """
        if self.isRasterTable(table):
            return False

        _, tablename = self.getSchemaTableName(table)
        for i in range(self.gdal_ds.GetLayerCount()):
            if self.gdal_ds.GetLayer(i).GetName() == tablename:
                return self.gdal_ds.DeleteLayer(i) == 0
        return False

    def emptyTable(self, table):
        """Deletes all rows from table """
        if self.isRasterTable(table):
            return False

        sql = "DELETE FROM %s" % self.quoteId(table)
        self._execute_and_commit(sql)

    def renameTable(self, table, new_table):
        """Renames the table

        :param table: tuple with schema and table names
        :type table: tuple (str, str)
        :param new_table: new table name
        :type new_table: str
        :return: true on success
        :rtype: bool
        """
        try:
            name = table[1]  # 0 is schema
            vector_table_names = [t.tableName() for t in self.core_connection.tables('', QgsAbstractDatabaseProviderConnection.Vector)]
            if name in vector_table_names:
                self.core_connection.renameVectorTable('', name, new_table)
            else:
                self.core_connection.renameRasterTable('', name, new_table)
            return True
        except QgsProviderConnectionException:
            return False

    def moveTable(self, table, new_table, new_schema=None):
        return self.renameTable(table, new_table)

    def runVacuum(self):
        """ run vacuum on the db """
        self._execute_and_commit("VACUUM")

    def addTableColumn(self, table, field_def):
        """Adds a column to table """

        _, tablename = self.getSchemaTableName(table)
        lyr = self.gdal_ds.GetLayerByName(tablename)
        if lyr is None:
            return False
        fld_defn = self.createOGRFieldDefnFromSQL(field_def)
        return lyr.CreateField(fld_defn) == 0

    def deleteTableColumn(self, table, column):
        """Deletes column from a table """
        if self.isGeometryColumn(table, column):
            return False

        _, tablename = self.getSchemaTableName(table)
        lyr = self.gdal_ds.GetLayerByName(tablename)
        if lyr is None:
            return False
        idx = lyr.GetLayerDefn().GetFieldIndex(column)
        if idx >= 0:
            return lyr.DeleteField(idx) == 0
        return False

    def updateTableColumn(self, table, column, new_name, new_data_type=None, new_not_null=None, new_default=None, comment=None):
        if self.isGeometryColumn(table, column):
            return False

        _, tablename = self.getSchemaTableName(table)
        lyr = self.gdal_ds.GetLayerByName(tablename)
        if lyr is None:
            return False
        if lyr.TestCapability(ogr.OLCAlterFieldDefn) == 0:
            return False
        idx = lyr.GetLayerDefn().GetFieldIndex(column)
        if idx >= 0:
            old_fielddefn = lyr.GetLayerDefn().GetFieldDefn(idx)
            flag = 0
            if new_name is not None:
                flag |= ogr.ALTER_NAME_FLAG
            else:
                new_name = column
            if new_data_type is None:
                ogr_type = old_fielddefn.GetType()
                ogr_subtype = old_fielddefn.GetSubType()
                width = old_fielddefn.GetWidth()
            else:
                flag |= ogr.ALTER_TYPE_FLAG
                flag |= ogr.ALTER_WIDTH_PRECISION_FLAG
                ogr_type, ogr_subtype, width = self.getOGRFieldTypeFromSQL(new_data_type)
            new_fielddefn = ogr.FieldDefn(new_name, ogr_type)
            new_fielddefn.SetSubType(ogr_subtype)
            new_fielddefn.SetWidth(width)
            if new_default is not None:
                flag |= ogr.ALTER_DEFAULT_FLAG
                if new_default == '':
                    new_fielddefn.SetDefault(None)
                elif new_default == 'NULL' or ogr_type in (ogr.OFTInteger, ogr.OFTReal):
                    new_fielddefn.SetDefault(str(new_default))
                elif new_default.startswith("'") and new_default.endswith("'"):
                    new_fielddefn.SetDefault(str(new_default))
                else:
                    new_fielddefn.SetDefault(self.quoteString(new_default))
            else:
                new_fielddefn.SetDefault(old_fielddefn.GetDefault())
            if new_not_null is not None:
                flag |= ogr.ALTER_NULLABLE_FLAG
                new_fielddefn.SetNullable(not new_not_null)
            else:
                new_fielddefn.SetNullable(old_fielddefn.IsNullable())
            return lyr.AlterFieldDefn(idx, new_fielddefn, flag) == 0

        return False

    def isGeometryColumn(self, table, column):

        _, tablename = self.getSchemaTableName(table)
        lyr = self.gdal_ds.GetLayerByName(tablename)
        if lyr is None:
            return False
        return column == lyr.GetGeometryColumn()

    def addGeometryColumn(self, table, geom_column='geometry', geom_type='POINT', srid=-1, dim=2):

        _, tablename = self.getSchemaTableName(table)
        lyr = self.gdal_ds.GetLayerByName(tablename)
        if lyr is None:
            return False
        ogr_type = ogr.wkbUnknown
        if geom_type == 'POINT':
            ogr_type = ogr.wkbPoint
        elif geom_type == 'LINESTRING':
            ogr_type = ogr.wkbLineString
        elif geom_type == 'POLYGON':
            ogr_type = ogr.wkbPolygon
        elif geom_type == 'MULTIPOINT':
            ogr_type = ogr.wkbMultiPoint
        elif geom_type == 'MULTILINESTRING':
            ogr_type = ogr.wkbMultiLineString
        elif geom_type == 'MULTIPOLYGON':
            ogr_type = ogr.wkbMultiPolygon
        elif geom_type == 'GEOMETRYCOLLECTION':
            ogr_type = ogr.wkbGeometryCollection

        if dim == 3:
            ogr_type = ogr_type | ogr.wkb25DBit
        elif dim == 4:
            if hasattr(ogr, 'GT_HasZ'):
                ogr_type = ogr.GT_SetZ(ogr_type)
            else:
                ogr_type = ogr_type | ogr.wkb25DBit
            if hasattr(ogr, 'GT_HasM'):
                ogr_type = ogr.GT_SetM(ogr_type)

        geom_field_defn = ogr.GeomFieldDefn(self.unquoteId(geom_column), ogr_type)
        if srid > 0:
            sr = osr.SpatialReference()
            if sr.ImportFromEPSG(srid) == 0:
                geom_field_defn.SetSpatialRef(sr)

        if lyr.CreateGeomField(geom_field_defn) != 0:
            return False
        self._opendb()
        return True

    def deleteGeometryColumn(self, table, geom_column):
        return False  # not supported

    def addTableUniqueConstraint(self, table, column):
        """Adds a unique constraint to a table """
        return False  # constraints not supported

    def deleteTableConstraint(self, table, constraint):
        """Deletes constraint in a table """
        return False  # constraints not supported

    def addTablePrimaryKey(self, table, column):
        """Adds a primery key (with one column) to a table """
        sql = "ALTER TABLE %s ADD PRIMARY KEY (%s)" % (self.quoteId(table), self.quoteId(column))
        self._execute_and_commit(sql)

    def createTableIndex(self, table, name, column, unique=False):
        """Creates index on one column using default options """
        unique_str = "UNIQUE" if unique else ""
        sql = "CREATE %s INDEX %s ON %s (%s)" % (
            unique_str, self.quoteId(name), self.quoteId(table), self.quoteId(column))
        self._execute_and_commit(sql)

    def deleteTableIndex(self, table, name):
        schema, tablename = self.getSchemaTableName(table)
        sql = "DROP INDEX %s" % self.quoteId((schema, name))
        self._execute_and_commit(sql)

    def createSpatialIndex(self, table, geom_column):
        if self.isRasterTable(table):
            return False
        _, tablename = self.getSchemaTableName(table)
        sql = "SELECT CreateSpatialIndex(%s, %s)" % (
            self.quoteId(tablename), self.quoteId(geom_column))
        try:
            res = self._fetchOne(sql)
        except QgsProviderConnectionException:
            return False
        return res is not None and res[0][0] == 1

    def deleteSpatialIndex(self, table, geom_column):
        if self.isRasterTable(table):
            return False
        _, tablename = self.getSchemaTableName(table)
        sql = "SELECT DisableSpatialIndex(%s, %s)" % (
            self.quoteId(tablename), self.quoteId(geom_column))
        res = self._fetchOne(sql)
        return len(res) > 0 and len(res[0]) > 0 and res[0][0] == 1

    def hasSpatialIndex(self, table, geom_column):
        if self.isRasterTable(table) or geom_column is None:
            return False
        _, tablename = self.getSchemaTableName(table)

        # (only available in >= 2.1.2)
        sql = "SELECT HasSpatialIndex(%s, %s)" % (self.quoteString(tablename), self.quoteString(geom_column))
        gdal.PushErrorHandler()
        ret = self._fetchOne(sql)
        gdal.PopErrorHandler()

        if len(ret) == 0:
            # might be the case for GDAL < 2.1.2
            sql = "SELECT COUNT(*) FROM sqlite_master WHERE type = 'table' AND name LIKE %s" % self.quoteString("%%rtree_" + tablename + "_%%")
            ret = self._fetchOne(sql)
        if len(ret) == 0:
            return False
        else:
            return ret[0][0] >= 1

    def execution_error_types(self):
        return sqlite3.Error, sqlite3.ProgrammingError, sqlite3.Warning

    def connection_error_types(self):
        return sqlite3.InterfaceError, sqlite3.OperationalError

    def getSqlDictionary(self):
        from .sql_dictionary import getSqlDictionary

        sql_dict = getSqlDictionary()

        items = []
        for tbl in self.getTables():
            items.append(tbl[1])  # table name

            for fld in self.getTableFields(tbl[0]):
                items.append(fld[1])  # field name

        sql_dict["identifier"] = items
        return sql_dict

    def getQueryBuilderDictionary(self):
        from .sql_dictionary import getQueryBuilderDictionary

        return getQueryBuilderDictionary()
