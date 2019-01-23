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

from ..connector import DBConnector
from ..plugin import ConnectionError, DbError, Table

from qgis.utils import spatialite_connect
import sqlite3

from osgeo import gdal, ogr, osr


def classFactory():
    return GPKGDBConnector


class GPKGDBConnector(DBConnector):

    def __init__(self, uri):
        DBConnector.__init__(self, uri)

        self.dbname = uri.database()
        self.has_raster = False
        self.mapSridToName = {}
        self._opendb()

    def _opendb(self):

        # Keep this explicit assignment to None to make sure the file is
        # properly closed before being re-opened
        self.gdal_ds = None
        self.gdal_ds = gdal.OpenEx(self.dbname, gdal.OF_UPDATE)
        if self.gdal_ds is None:
            self.gdal_ds = gdal.OpenEx(self.dbname)
        if self.gdal_ds is None or self.gdal_ds.GetDriver().ShortName != 'GPKG':
            raise ConnectionError(QApplication.translate("DBManagerPlugin", '"{0}" not found').format(self.dbname))
        self.has_raster = self.gdal_ds.RasterCount != 0 or self.gdal_ds.GetMetadata('SUBDATASETS') is not None
        self.connection = None

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
        sql_lyr = self.gdal_ds.ExecuteSQL(sql)
        if sql_lyr is None:
            return None
        f = sql_lyr.GetNextFeature()
        if f is None:
            ret = None
        else:
            ret = [f.GetField(i) for i in range(f.GetFieldCount())]
        self.gdal_ds.ReleaseResultSet(sql_lyr)
        return ret

    def _fetchAll(self, sql, include_fid_and_geometry=False):
        sql_lyr = self.gdal_ds.ExecuteSQL(sql)
        if sql_lyr is None:
            return None
        ret = []
        while True:
            f = sql_lyr.GetNextFeature()
            if f is None:
                break
            else:
                if include_fid_and_geometry:
                    field_vals = [f.GetFID()]
                    if sql_lyr.GetLayerDefn().GetGeomType() != ogr.wkbNone:
                        geom = f.GetGeometryRef()
                        if geom is not None:
                            geom = geom.ExportToWkt()
                        field_vals += [geom]
                    field_vals += [f.GetField(i) for i in range(f.GetFieldCount())]
                    ret.append(field_vals)
                else:
                    ret.append([f.GetField(i) for i in range(f.GetFieldCount())])
        self.gdal_ds.ReleaseResultSet(sql_lyr)
        return ret

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

        items = []
        for i in range(self.gdal_ds.GetLayerCount()):
            lyr = self.gdal_ds.GetLayer(i)
            geomtype = lyr.GetGeomType()
            if hasattr(ogr, 'GT_Flatten'):
                geomtype_flatten = ogr.GT_Flatten(geomtype)
            else:
                geomtype_flatten = geomtype
            geomname = 'GEOMETRY'
            if geomtype_flatten == ogr.wkbPoint:
                geomname = 'POINT'
            elif geomtype_flatten == ogr.wkbLineString:
                geomname = 'LINESTRING'
            elif geomtype_flatten == ogr.wkbPolygon:
                geomname = 'POLYGON'
            elif geomtype_flatten == ogr.wkbMultiPoint:
                geomname = 'MULTIPOINT'
            elif geomtype_flatten == ogr.wkbMultiLineString:
                geomname = 'MULTILINESTRING'
            elif geomtype_flatten == ogr.wkbMultiPolygon:
                geomname = 'MULTIPOLYGON'
            elif geomtype_flatten == ogr.wkbGeometryCollection:
                geomname = 'GEOMETRYCOLLECTION'
            elif geomtype_flatten == ogr.wkbCircularString:
                geomname = 'CIRCULARSTRING'
            elif geomtype_flatten == ogr.wkbCompoundCurve:
                geomname = 'COMPOUNDCURVE'
            elif geomtype_flatten == ogr.wkbCurvePolygon:
                geomname = 'CURVEPOLYGON'
            elif geomtype_flatten == ogr.wkbMultiCurve:
                geomname = 'MULTICURVE'
            elif geomtype_flatten == ogr.wkbMultiSurface:
                geomname = 'MULTISURFACE'
            geomdim = 'XY'
            if hasattr(ogr, 'GT_HasZ') and ogr.GT_HasZ(lyr.GetGeomType()):
                geomdim += 'Z'
            if hasattr(ogr, 'GT_HasM') and ogr.GT_HasM(lyr.GetGeomType()):
                geomdim += 'M'
            srs = lyr.GetSpatialRef()
            srid = None
            if srs is not None:
                if srs.IsProjected():
                    name = srs.GetAttrValue('PROJCS', 0)
                elif srs.IsGeographic():
                    name = srs.GetAttrValue('GEOGCS', 0)
                else:
                    name = None
                srid = srs.GetAuthorityCode(None)
                if srid is not None:
                    srid = int(srid)
                else:
                    srid = self._fetchOne('SELECT srid FROM gpkg_spatial_ref_sys WHERE table_name = %s' % self.quoteString(lyr.GetName()))
                    if srid is not None:
                        srid = int(srid)
                self.mapSridToName[srid] = name

            if geomtype == ogr.wkbNone:
                item = list([Table.TableType,
                             lyr.GetName(),
                             False,  # is_view
                             ])
            else:
                item = list([Table.VectorType,
                             lyr.GetName(),
                             False,  # is_view
                             lyr.GetName(),
                             lyr.GetGeometryColumn(),
                             geomname,
                             geomdim,
                             srid])
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

        sql = u"""SELECT table_name, 0 AS is_view, table_name AS r_table_name, '' AS r_geometry_column, srs_id FROM gpkg_contents WHERE data_type = 'tiles'"""
        ret = self._fetchAll(sql)
        if ret is None:
            return []
        items = []
        for i, tbl in enumerate(ret):
            item = list(tbl)
            item.insert(0, Table.RasterType)
            items.append(item)
        return items

    def getTableRowCount(self, table):
        lyr = self.gdal_ds.GetLayerByName(self.getSchemaTableName(table)[1])
        return lyr.GetFeatureCount() if lyr is not None else None

    def getTableFields(self, table):
        """ return list of columns in table """
        sql = u"PRAGMA table_info(%s)" % (self.quoteId(table))
        ret = self._fetchAll(sql)
        if ret is None:
            ret = []
        return ret

    def getTableIndexes(self, table):
        """ get info about table's indexes """
        sql = u"PRAGMA index_list(%s)" % (self.quoteId(table))
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
            sql = u"PRAGMA index_info(%s)" % (self.quoteId(name))

            idx = [num, name, unique]
            cols = []
            for seq, cid, cname in self._fetchAll(sql):
                cols.append(cid)
            idx.append(cols)
            indexes[i] = idx

        return indexes

    def getTableConstraints(self, table):
        return None

    def getTableTriggers(self, table):

        _, tablename = self.getSchemaTableName(table)
        # Do not list rtree related triggers as we don't want them to be dropped
        sql = u"SELECT name, sql FROM sqlite_master WHERE tbl_name = %s AND type = 'trigger'" % (self.quoteString(tablename))
        if self.isVectorTable(table):
            sql += u" AND name NOT LIKE 'rtree_%%'"
        elif self.isRasterTable(table):
            sql += u" AND name NOT LIKE '%%_zoom_insert'"
            sql += u" AND name NOT LIKE '%%_zoom_update'"
            sql += u" AND name NOT LIKE '%%_tile_column_insert'"
            sql += u" AND name NOT LIKE '%%_tile_column_update'"
            sql += u" AND name NOT LIKE '%%_tile_row_insert'"
            sql += u" AND name NOT LIKE '%%_tile_row_update'"
        return self._fetchAll(sql)

    def deleteTableTrigger(self, trigger, table=None):
        """ delete trigger """
        sql = u"DROP TRIGGER %s" % self.quoteId(trigger)
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

        sql = u"SELECT srs_name FROM gpkg_spatial_ref_sys WHERE srs_id = %s" % self.quoteString(srid)
        res = self._fetchOne(sql)
        if res is not None:
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
                sql = u"SELECT COUNT(*) FROM gpkg_contents WHERE data_type = 'tiles' AND table_name = %s" % self.quoteString(tablename)
                ret = self._fetchOne(sql)
                return ret is not None and ret[0] == 1
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
        """ create ordinary table
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
        """ delete table from the database """
        if self.isRasterTable(table):
            return False

        _, tablename = self.getSchemaTableName(table)
        for i in range(self.gdal_ds.GetLayerCount()):
            if self.gdal_ds.GetLayer(i).GetName() == tablename:
                return self.gdal_ds.DeleteLayer(i) == 0
        return False

    def emptyTable(self, table):
        """ delete all rows from table """
        if self.isRasterTable(table):
            return False

        sql = u"DELETE FROM %s" % self.quoteId(table)
        self._execute_and_commit(sql)

    def renameTable(self, table, new_table):
        """ rename a table """

        if self.isRasterTable(table):
            return False

        _, tablename = self.getSchemaTableName(table)
        if new_table == tablename:
            return True

        if tablename.find('"') >= 0:
            tablename = self.quoteId(tablename)
        if new_table.find('"') >= 0:
            new_table = self.quoteId(new_table)

        gdal.ErrorReset()
        self.gdal_ds.ExecuteSQL('ALTER TABLE %s RENAME TO %s' % (tablename, new_table))
        if gdal.GetLastErrorMsg() != '':
            return False
        # we need to reopen after renaming since OGR doesn't update its
        # internal state
        self._opendb()
        return True

    def moveTable(self, table, new_table, new_schema=None):
        return self.renameTable(table, new_table)

    def runVacuum(self):
        """ run vacuum on the db """
        self._execute_and_commit("VACUUM")

    def addTableColumn(self, table, field_def):
        """ add a column to table """

        _, tablename = self.getSchemaTableName(table)
        lyr = self.gdal_ds.GetLayerByName(tablename)
        if lyr is None:
            return False
        fld_defn = self.createOGRFieldDefnFromSQL(field_def)
        return lyr.CreateField(fld_defn) == 0

    def deleteTableColumn(self, table, column):
        """ delete column from a table """
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

    def updateTableColumn(self, table, column, new_name, new_data_type=None, new_not_null=None, new_default=None, new_comment=None):
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
        """ add a unique constraint to a table """
        return False  # constraints not supported

    def deleteTableConstraint(self, table, constraint):
        """ delete constraint in a table """
        return False  # constraints not supported

    def addTablePrimaryKey(self, table, column):
        """ add a primery key (with one column) to a table """
        sql = u"ALTER TABLE %s ADD PRIMARY KEY (%s)" % (self.quoteId(table), self.quoteId(column))
        self._execute_and_commit(sql)

    def createTableIndex(self, table, name, column, unique=False):
        """ create index on one column using default options """
        unique_str = u"UNIQUE" if unique else ""
        sql = u"CREATE %s INDEX %s ON %s (%s)" % (
            unique_str, self.quoteId(name), self.quoteId(table), self.quoteId(column))
        self._execute_and_commit(sql)

    def deleteTableIndex(self, table, name):
        schema, tablename = self.getSchemaTableName(table)
        sql = u"DROP INDEX %s" % self.quoteId((schema, name))
        self._execute_and_commit(sql)

    def createSpatialIndex(self, table, geom_column):
        if self.isRasterTable(table):
            return False
        _, tablename = self.getSchemaTableName(table)
        sql = u"SELECT CreateSpatialIndex(%s, %s)" % (
            self.quoteId(tablename), self.quoteId(geom_column))
        res = self._fetchOne(sql)
        return res is not None and res[0] == 1

    def deleteSpatialIndex(self, table, geom_column):
        if self.isRasterTable(table):
            return False
        _, tablename = self.getSchemaTableName(table)
        sql = u"SELECT DisableSpatialIndex(%s, %s)" % (
            self.quoteId(tablename), self.quoteId(geom_column))
        res = self._fetchOne(sql)
        return res is not None and res[0] == 1

    def hasSpatialIndex(self, table, geom_column):
        if self.isRasterTable(table) or geom_column is None:
            return False
        _, tablename = self.getSchemaTableName(table)

        # (only available in >= 2.1.2)
        sql = u"SELECT HasSpatialIndex(%s, %s)" % (self.quoteString(tablename), self.quoteString(geom_column))
        gdal.PushErrorHandler()
        ret = self._fetchOne(sql)
        gdal.PopErrorHandler()

        if ret is None:
            # might be the case for GDAL < 2.1.2
            sql = u"SELECT COUNT(*) FROM sqlite_master WHERE type = 'table' AND name LIKE %s" % self.quoteString("%%rtree_" + tablename + "_%%")
            ret = self._fetchOne(sql)
        if ret is None:
            return False
        else:
            return ret[0] >= 1

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
