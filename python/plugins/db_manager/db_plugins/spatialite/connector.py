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
from builtins import str

from functools import cmp_to_key

from qgis.core import Qgis, QgsSqliteUtils
from qgis.PyQt.QtCore import QFile
from qgis.PyQt.QtWidgets import QApplication

from ..connector import DBConnector
from ..plugin import ConnectionError, DbError, Table

from qgis.utils import spatialite_connect
import sqlite3 as sqlite


def classFactory():
    return SpatiaLiteDBConnector


class SpatiaLiteDBConnector(DBConnector):

    def __init__(self, uri):
        DBConnector.__init__(self, uri)

        self.dbname = uri.database()
        if not QFile.exists(self.dbname):
            raise ConnectionError(QApplication.translate("DBManagerPlugin", '"{0}" not found').format(self.dbname))

        try:
            self.connection = spatialite_connect(self._connectionInfo())

        except self.connection_error_types() as e:
            raise ConnectionError(e)

        self._checkSpatial()
        self._checkRaster()

    def _connectionInfo(self):
        return str(self.dbname)

    def cancel(self):
        # https://www.sqlite.org/c3ref/interrupt.html
        # This function causes any pending database operation to abort and return at its earliest opportunity.
        if self.connection:
            self.connection.interrupt()

    @classmethod
    def isValidDatabase(self, path):
        if not QFile.exists(path):
            return False
        try:
            conn = spatialite_connect(path)
        except self.connection_error_types():
            return False

        isValid = False

        try:
            c = conn.cursor()
            c.execute("SELECT count(*) FROM sqlite_master")
            c.fetchone()
            isValid = True
        except sqlite.DatabaseError:
            pass

        conn.close()
        return isValid

    def _checkSpatial(self):
        """ check if it's a valid SpatiaLite db """
        self.has_spatial = self._checkGeometryColumnsTable()
        return self.has_spatial

    def _checkRaster(self):
        """ check if it's a rasterite db """
        self.has_raster = self._checkRasterTables()
        return self.has_raster

    def _checkGeometryColumnsTable(self):
        try:
            c = self._get_cursor()
            self._execute(c, "SELECT CheckSpatialMetaData()")
            v = c.fetchone()[0]
            self.has_geometry_columns = v == 1 or v == 3
            self.has_spatialite4 = v == 3
        except Exception:
            self.has_geometry_columns = False
            self.has_spatialite4 = False

        self.has_geometry_columns_access = self.has_geometry_columns
        return self.has_geometry_columns

    def _checkRasterTables(self):
        c = self._get_cursor()
        sql = "SELECT count(*) = 3 FROM sqlite_master WHERE name IN ('layer_params', 'layer_statistics', 'raster_pyramids')"
        self._execute(c, sql)
        ret = c.fetchone()
        return ret and ret[0]

    def getInfo(self):
        c = self._get_cursor()
        self._execute(c, "SELECT sqlite_version()")
        return c.fetchone()

    def getSpatialInfo(self):
        """ returns tuple about SpatiaLite support:
                - lib version
                - geos version
                - proj version
        """
        if not self.has_spatial:
            return

        c = self._get_cursor()
        try:
            self._execute(c, "SELECT spatialite_version(), geos_version(), proj4_version()")
        except DbError:
            return

        return c.fetchone()

    def hasSpatialSupport(self):
        return self.has_spatial

    def hasRasterSupport(self):
        return self.has_raster

    def hasCustomQuerySupport(self):
        return Qgis.QGIS_VERSION[0:3] >= "1.6"

    def hasTableColumnEditingSupport(self):
        return False

    def hasCreateSpatialViewSupport(self):
        return True

    def fieldTypes(self):
        return [
            "integer", "bigint", "smallint",  # integers
            "real", "double", "float", "numeric",  # floats
            "varchar", "varchar(255)", "character(20)", "text",  # strings
            "date", "datetime"  # date/time
        ]

    def getSchemas(self):
        return None

    def getTables(self, schema=None, add_sys_tables=False):
        """ get list of tables """
        tablenames = []
        items = []

        sys_tables = QgsSqliteUtils.systemTables()

        try:
            vectors = self.getVectorTables(schema)
            for tbl in vectors:
                if not add_sys_tables and tbl[1] in sys_tables:
                    continue
                tablenames.append(tbl[1])
                items.append(tbl)
        except DbError:
            pass

        try:
            rasters = self.getRasterTables(schema)
            for tbl in rasters:
                if not add_sys_tables and tbl[1] in sys_tables:
                    continue
                tablenames.append(tbl[1])
                items.append(tbl)
        except DbError:
            pass

        c = self._get_cursor()

        if self.has_geometry_columns:
            # get the R*Tree tables
            sql = "SELECT f_table_name, f_geometry_column FROM geometry_columns WHERE spatial_index_enabled = 1"
            self._execute(c, sql)
            for idx_item in c.fetchall():
                sys_tables.append('idx_%s_%s' % idx_item)
                sys_tables.append('idx_%s_%s_node' % idx_item)
                sys_tables.append('idx_%s_%s_parent' % idx_item)
                sys_tables.append('idx_%s_%s_rowid' % idx_item)

        sql = "SELECT name, type = 'view' FROM sqlite_master WHERE type IN ('table', 'view')"
        self._execute(c, sql)

        for tbl in c.fetchall():
            if tablenames.count(tbl[0]) <= 0 and not tbl[0].startswith('idx_'):
                if not add_sys_tables and tbl[0] in sys_tables:
                    continue
                item = list(tbl)
                item.insert(0, Table.TableType)
                items.append(item)

        for i, tbl in enumerate(items):
            tbl.insert(3, tbl[1] in sys_tables)

        return sorted(items, key=cmp_to_key(lambda x, y: (x[1] > y[1]) - (x[1] < y[1])))

    def getVectorTables(self, schema=None):
        """ get list of table with a geometry column
                it returns:
                        name (table name)
                        type = 'view' (is a view?)
                        geometry_column:
                                f_table_name (the table name in geometry_columns may be in a wrong case, use this to load the layer)
                                f_geometry_column
                                type
                                coord_dimension
                                srid
        """

        if self.has_geometry_columns:
            if self.has_spatialite4:
                cols = """CASE geometry_type % 10
                                  WHEN 1 THEN 'POINT'
                                  WHEN 2 THEN 'LINESTRING'
                                  WHEN 3 THEN 'POLYGON'
                                  WHEN 4 THEN 'MULTIPOINT'
                                  WHEN 5 THEN 'MULTILINESTRING'
                                  WHEN 6 THEN 'MULTIPOLYGON'
                                  WHEN 7 THEN 'GEOMETRYCOLLECTION'
                                  END AS gtype,
                                  CASE geometry_type / 1000
                                  WHEN 0 THEN 'XY'
                                  WHEN 1 THEN 'XYZ'
                                  WHEN 2 THEN 'XYM'
                                  WHEN 3 THEN 'XYZM'
                                  ELSE NULL
                                  END AS coord_dimension"""
            else:
                cols = "g.type,g.coord_dimension"

            # get geometry info from geometry_columns if exists
            sql = """SELECT m.name, m.type = 'view', g.f_table_name, g.f_geometry_column, %s, g.srid
                                                FROM sqlite_master AS m JOIN geometry_columns AS g ON upper(m.name) = upper(g.f_table_name)
                                                WHERE m.type in ('table', 'view')
                                                ORDER BY m.name, g.f_geometry_column""" % cols

        else:
            return []

        c = self._get_cursor()
        self._execute(c, sql)

        items = []
        for tbl in c.fetchall():
            item = list(tbl)
            item.insert(0, Table.VectorType)
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

        if not self.has_geometry_columns:
            return []
        if not self.has_raster:
            return []

        c = self._get_cursor()

        # get geometry info from geometry_columns if exists
        sql = """SELECT r.table_name||'_rasters', m.type = 'view', r.table_name, r.geometry_column, g.srid
                                                FROM sqlite_master AS m JOIN geometry_columns AS g ON upper(m.name) = upper(g.f_table_name)
                                                JOIN layer_params AS r ON upper(REPLACE(m.name, '_metadata', '')) = upper(r.table_name)
                                                WHERE m.type in ('table', 'view') AND upper(m.name) = upper(r.table_name||'_metadata')
                                                ORDER BY r.table_name"""

        self._execute(c, sql)

        items = []
        for i, tbl in enumerate(c.fetchall()):
            item = list(tbl)
            item.insert(0, Table.RasterType)
            items.append(item)

        return items

    def getTableRowCount(self, table):
        c = self._get_cursor()
        self._execute(c, "SELECT COUNT(*) FROM %s" % self.quoteId(table))
        ret = c.fetchone()
        return ret[0] if ret is not None else None

    def getTableFields(self, table):
        """ return list of columns in table """
        c = self._get_cursor()
        sql = "PRAGMA table_info(%s)" % (self.quoteId(table))
        self._execute(c, sql)
        return c.fetchall()

    def getTableIndexes(self, table):
        """ get info about table's indexes """
        c = self._get_cursor()
        sql = "PRAGMA index_list(%s)" % (self.quoteId(table))
        self._execute(c, sql)
        indexes = c.fetchall()

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
            self._execute(c, sql)

            idx = [num, name, unique]
            cols = [
                cid
                for seq, cid, cname in c.fetchall()
            ]
            idx.append(cols)
            indexes[i] = idx

        return indexes

    def getTableConstraints(self, table):
        return None

    def getTableTriggers(self, table):
        c = self._get_cursor()
        schema, tablename = self.getSchemaTableName(table)
        sql = "SELECT name, sql FROM sqlite_master WHERE tbl_name = %s AND type = 'trigger'" % (
            self.quoteString(tablename))
        self._execute(c, sql)
        return c.fetchall()

    def deleteTableTrigger(self, trigger, table=None):
        """Deletes trigger """
        sql = "DROP TRIGGER %s" % self.quoteId(trigger)
        self._execute_and_commit(sql)

    def getTableExtent(self, table, geom):
        """ find out table extent """
        schema, tablename = self.getSchemaTableName(table)
        c = self._get_cursor()

        if self.isRasterTable(table):
            tablename = tablename.replace('_rasters', '_metadata')
            geom = 'geometry'

        sql = """SELECT Min(MbrMinX(%(geom)s)), Min(MbrMinY(%(geom)s)), Max(MbrMaxX(%(geom)s)), Max(MbrMaxY(%(geom)s))
                                                FROM %(table)s """ % {'geom': self.quoteId(geom),
                                                                      'table': self.quoteId(tablename)}
        self._execute(c, sql)
        return c.fetchone()

    def getViewDefinition(self, view):
        """ returns definition of the view """
        schema, tablename = self.getSchemaTableName(view)
        sql = "SELECT sql FROM sqlite_master WHERE type = 'view' AND name = %s" % self.quoteString(tablename)
        c = self._execute(None, sql)
        ret = c.fetchone()
        return ret[0] if ret is not None else None

    def getSpatialRefInfo(self, srid):
        sql = "SELECT ref_sys_name FROM spatial_ref_sys WHERE srid = %s" % self.quoteString(srid)
        c = self._execute(None, sql)
        ret = c.fetchone()
        return ret[0] if ret is not None else None

    def isVectorTable(self, table):
        if self.has_geometry_columns:
            schema, tablename = self.getSchemaTableName(table)
            sql = "SELECT count(*) FROM geometry_columns WHERE upper(f_table_name) = upper(%s)" % self.quoteString(
                tablename)
            c = self._execute(None, sql)
            ret = c.fetchone()
            return ret is not None and ret[0] > 0
        return True

    def isRasterTable(self, table):
        if self.has_geometry_columns and self.has_raster:
            schema, tablename = self.getSchemaTableName(table)
            if not tablename.endswith("_rasters"):
                return False

            sql = """SELECT count(*)
                                        FROM layer_params AS r JOIN geometry_columns AS g
                                                ON upper(r.table_name||'_metadata') = upper(g.f_table_name)
                                        WHERE upper(r.table_name) = upper(REPLACE(%s, '_rasters', ''))""" % self.quoteString(
                tablename)
            c = self._execute(None, sql)
            ret = c.fetchone()
            return ret is not None and ret[0] > 0

        return False

    def createTable(self, table, field_defs, pkey):
        """Creates ordinary table
                        'fields' is array containing field definitions
                        'pkey' is the primary key name
        """
        if len(field_defs) == 0:
            return False

        sql = "CREATE TABLE %s (" % self.quoteId(table)
        sql += ", ".join(field_defs)
        if pkey is not None and pkey != "":
            sql += ", PRIMARY KEY (%s)" % self.quoteId(pkey)
        sql += ")"

        self._execute_and_commit(sql)
        return True

    def deleteTable(self, table):
        """Deletes table from the database """
        if self.isRasterTable(table):
            return False

        c = self._get_cursor()
        sql = "DROP TABLE %s" % self.quoteId(table)
        self._execute(c, sql)
        schema, tablename = self.getSchemaTableName(table)
        sql = "DELETE FROM geometry_columns WHERE upper(f_table_name) = upper(%s)" % self.quoteString(tablename)
        self._execute(c, sql)
        self._commit()

        return True

    def emptyTable(self, table):
        """Deletes all rows from table """
        if self.isRasterTable(table):
            return False

        sql = "DELETE FROM %s" % self.quoteId(table)
        self._execute_and_commit(sql)

    def renameTable(self, table, new_table):
        """ rename a table """
        schema, tablename = self.getSchemaTableName(table)
        if new_table == tablename:
            return

        if self.isRasterTable(table):
            return False

        c = self._get_cursor()

        sql = "ALTER TABLE %s RENAME TO %s" % (self.quoteId(table), self.quoteId(new_table))
        self._execute(c, sql)

        # update geometry_columns
        if self.has_geometry_columns:
            sql = "UPDATE geometry_columns SET f_table_name = %s WHERE upper(f_table_name) = upper(%s)" % (
                self.quoteString(new_table), self.quoteString(tablename))
            self._execute(c, sql)

        self._commit()
        return True

    def moveTable(self, table, new_table, new_schema=None):
        return self.renameTable(table, new_table)

    def createView(self, view, query):
        sql = "CREATE VIEW %s AS %s" % (self.quoteId(view), query)
        self._execute_and_commit(sql)

    def deleteView(self, view):
        c = self._get_cursor()

        sql = "DROP VIEW %s" % self.quoteId(view)
        self._execute(c, sql)

        # update geometry_columns
        if self.has_geometry_columns:
            sql = "DELETE FROM geometry_columns WHERE f_table_name = %s" % self.quoteString(view)
            self._execute(c, sql)

        self._commit()

    def renameView(self, view, new_name):
        """ rename view """
        return self.renameTable(view, new_name)

    def createSpatialView(self, view, query):

        self.createView(view, query)
        # get type info about the view
        sql = "PRAGMA table_info(%s)" % self.quoteString(view)
        c = self._execute(None, sql)
        geom_col = None
        for r in c.fetchall():
            if r[2].upper() in ('POINT', 'LINESTRING', 'POLYGON',
                                'MULTIPOINT', 'MULTILINESTRING', 'MULTIPOLYGON'):
                geom_col = r[1]
                break
        if geom_col is None:
            return

        # get geometry type and srid
        sql = "SELECT geometrytype(%s), srid(%s) FROM %s LIMIT 1" % (self.quoteId(geom_col), self.quoteId(geom_col), self.quoteId(view))
        c = self._execute(None, sql)
        r = c.fetchone()
        if r is None:
            return

        gtype, gsrid = r
        gdim = 'XY'
        if ' ' in gtype:
            zm = gtype.split(' ')[1]
            gtype = gtype.split(' ')[0]
            gdim += zm
        try:
            wkbType = ('POINT', 'LINESTRING', 'POLYGON', 'MULTIPOINT', 'MULTILINESTRING', 'MULTIPOLYGON').index(gtype) + 1
        except:
            wkbType = 0
        if 'Z' in gdim:
            wkbType += 1000
        if 'M' in gdim:
            wkbType += 2000

        sql = """INSERT INTO geometry_columns (f_table_name, f_geometry_column, geometry_type, coord_dimension, srid, spatial_index_enabled)
                                        VALUES (%s, %s, %s, %s, %s, 0)""" % (self.quoteId(view), self.quoteId(geom_col), wkbType, len(gdim), gsrid)
        self._execute_and_commit(sql)

    def runVacuum(self):
        """ run vacuum on the db """
        # Workaround http://bugs.python.org/issue28518
        self.connection.isolation_level = None
        c = self._get_cursor()
        c.execute('VACUUM')
        self.connection.isolation_level = ''  # reset to default isolation

    def addTableColumn(self, table, field_def):
        """Adds a column to table """
        sql = "ALTER TABLE %s ADD %s" % (self.quoteId(table), field_def)
        self._execute(None, sql)

        sql = "SELECT InvalidateLayerStatistics(%s)" % (self.quoteId(table))
        self._execute(None, sql)

        sql = "SELECT UpdateLayerStatistics(%s)" % (self.quoteId(table))
        self._execute(None, sql)

        self._commit()
        return True

    def deleteTableColumn(self, table, column):
        """Deletes column from a table """
        if not self.isGeometryColumn(table, column):
            return False  # column editing not supported

        # delete geometry column correctly
        schema, tablename = self.getSchemaTableName(table)
        sql = "SELECT DiscardGeometryColumn(%s, %s)" % (self.quoteString(tablename), self.quoteString(column))
        self._execute_and_commit(sql)

    def updateTableColumn(self, table, column, new_name, new_data_type=None, new_not_null=None, new_default=None, comment=None):
        return False  # column editing not supported

    def renameTableColumn(self, table, column, new_name):
        """ rename column in a table """
        return False  # column editing not supported

    def setColumnType(self, table, column, data_type):
        """Changes column type """
        return False  # column editing not supported

    def setColumnDefault(self, table, column, default):
        """Changes column's default value. If default=None drop default value """
        return False  # column editing not supported

    def setColumnNull(self, table, column, is_null):
        """Changes whether column can contain null values """
        return False  # column editing not supported

    def isGeometryColumn(self, table, column):

        c = self._get_cursor()
        schema, tablename = self.getSchemaTableName(table)
        sql = "SELECT count(*) > 0 FROM geometry_columns WHERE upper(f_table_name) = upper(%s) AND upper(f_geometry_column) = upper(%s)" % (
            self.quoteString(tablename), self.quoteString(column))
        self._execute(c, sql)
        return c.fetchone()[0] == 't'

    def addGeometryColumn(self, table, geom_column='geometry', geom_type='POINT', srid=-1, dim=2):

        schema, tablename = self.getSchemaTableName(table)
        sql = "SELECT AddGeometryColumn(%s, %s, %d, %s, %s)" % (
            self.quoteString(tablename), self.quoteString(geom_column), srid, self.quoteString(geom_type), dim)
        self._execute_and_commit(sql)

    def deleteGeometryColumn(self, table, geom_column):
        return self.deleteTableColumn(table, geom_column)

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

    def createSpatialIndex(self, table, geom_column='geometry'):
        if self.isRasterTable(table):
            return False

        schema, tablename = self.getSchemaTableName(table)
        sql = "SELECT CreateSpatialIndex(%s, %s)" % (self.quoteString(tablename), self.quoteString(geom_column))
        self._execute_and_commit(sql)

    def deleteSpatialIndex(self, table, geom_column='geometry'):
        if self.isRasterTable(table):
            return False

        schema, tablename = self.getSchemaTableName(table)
        try:
            sql = "SELECT DiscardSpatialIndex(%s, %s)" % (self.quoteString(tablename), self.quoteString(geom_column))
            self._execute_and_commit(sql)
        except DbError:
            sql = "SELECT DeleteSpatialIndex(%s, %s)" % (self.quoteString(tablename), self.quoteString(geom_column))
            self._execute_and_commit(sql)
            # delete the index table
            idx_table_name = "idx_%s_%s" % (tablename, geom_column)
            self.deleteTable(idx_table_name)

    def hasSpatialIndex(self, table, geom_column='geometry'):
        if not self.has_geometry_columns or self.isRasterTable(table):
            return False
        c = self._get_cursor()
        schema, tablename = self.getSchemaTableName(table)
        sql = "SELECT spatial_index_enabled FROM geometry_columns WHERE upper(f_table_name) = upper(%s) AND upper(f_geometry_column) = upper(%s)" % (
            self.quoteString(tablename), self.quoteString(geom_column))
        self._execute(c, sql)
        row = c.fetchone()
        return row is not None and row[0] == 1

    def execution_error_types(self):
        return sqlite.Error, sqlite.ProgrammingError, sqlite.Warning

    def connection_error_types(self):
        return sqlite.InterfaceError, sqlite.OperationalError

    # moved into the parent class: DbConnector._execute()
    # def _execute(self, cursor, sql):
    #       pass

    # moved into the parent class: DbConnector._execute_and_commit()
    # def _execute_and_commit(self, sql):
    #       pass

    # moved into the parent class: DbConnector._get_cursor()
    # def _get_cursor(self, name=None):
    #       pass

    # moved into the parent class: DbConnector._fetchall()
    # def _fetchall(self, c):
    #       pass

    # moved into the parent class: DbConnector._fetchone()
    # def _fetchone(self, c):
    #       pass

    # moved into the parent class: DbConnector._commit()
    # def _commit(self):
    #       pass

    # moved into the parent class: DbConnector._rollback()
    # def _rollback(self):
    #       pass

    # moved into the parent class: DbConnector._get_cursor_columns()
    # def _get_cursor_columns(self, c):
    #       pass

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
