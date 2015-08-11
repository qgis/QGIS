# -*- coding: utf-8 -*-

"""
/***************************************************************************
Name                 : DB Manager
Description          : Database manager plugin for QGIS (Oracle)
Date                 : Aug 27, 2014
copyright            : (C) 2014 by Médéric RIBREUX
email                : mederic.ribreux@gmail.com

The content of this file is based on
- PG_Manager by Martin Dobias <wonder.sk@gmail.com> (GPLv2 license)
- DB Manager by Giuseppe Sucameli <brush.tyler@gmail.com> (GPLv2 license)
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
from PyQt4.QtSql import QSqlDatabase

from ..connector import DBConnector
from ..plugin import ConnectionError, DbError, Table

import os
from qgis.core import QGis, QgsApplication
import QtSqlDB
import sqlite3


def classFactory():
    if QSqlDatabase.isDriverAvailable("QOCISPATIAL"):
        return OracleDBConnector
    else:
        return None


class OracleDBConnector(DBConnector):

    ORGeomTypes = {
        2001: QGis.WKBPoint,
        2002: QGis.WKBLineString,
        2003: QGis.WKBPolygon,
        2005: QGis.WKBMultiPoint,
        2006: QGis.WKBMultiLineString,
        2007: QGis.WKBMultiPolygon,
        3001: QGis.WKBPoint25D,
        3002: QGis.WKBLineString25D,
        3003: QGis.WKBPolygon25D,
        3005: QGis.WKBMultiPoint25D,
        3006: QGis.WKBMultiLineString25D,
        3007: QGis.WKBMultiPolygon25D
    }

    def __init__(self, uri, connName):
        DBConnector.__init__(self, uri)

        self.connName = connName
        self.user = uri.username() or os.environ.get('USER')
        self.passwd = uri.password()
        self.host = uri.host()

        if self.host != "":
            self.dbname = self.host
            if uri.port() != "" and uri.port() != "1521":
                self.dbname += ":" + uri.port()
            if uri.database() != "":
                self.dbname += "/" + uri.database()
        elif uri.database() != "":
            self.dbname = uri.database()

        # Connection options
        self.useEstimatedMetadata = uri.useEstimatedMetadata()
        self.userTablesOnly = uri.param('userTablesOnly').lower() == "true"
        self.geometryColumnsOnly = uri.param(
            'geometryColumnsOnly').lower() == "true"
        self.allowGeometrylessTables = uri.param(
            'allowGeometrylessTables').lower() == "true"
        self.onlyExistingTypes = uri.param(
            'onlyExistingTypes').lower() == "true"

        # For refreshing
        self.populated = False
        try:
            self.connection = QtSqlDB.connect(
                "QOCISPATIAL", self.dbname, self.user, self.passwd)

        except self.connection_error_types(), e:
            raise ConnectionError(e)

        # Find if we can connect to data_sources_cache.db
        sqlite_cache_file = os.path.join(
            QgsApplication.qgisSettingsDirPath(), u"data_sources_cache.db")
        if (os.path.isfile(sqlite_cache_file)):
            try:
                self.cache_connection = sqlite3.connect(sqlite_cache_file)
            except sqlite3.Error as e:

                self.cache_connection = False

        # Find if there is cache for our connection:
        if self.cache_connection:
            try:
                cache_c = self.cache_connection.cursor()
                query = (u"SELECT COUNT(*) FROM meta_oracle WHERE"
                         u" conn = '{}'".format(self.connName))
                cache_c.execute(query)
                has_cached = cache_c.fetchone()[0]
                cache_c.close()
                if not has_cached:
                    self.cache_connection = False

            except sqlite3.Error as e:
                self.cache_connection = False

        self._checkSpatial()
        self._checkGeometryColumnsTable()

    def _connectionInfo(self):
        return unicode(self._uri.connectionInfo())

    def _checkSpatial(self):
        """Check whether Oracle Spatial is present in catalog."""
        query = (u"SELECT count(*) FROM v$option WHERE parameter = "
                 u" 'Spatial' AND value = 'TRUE'")
        c = self._execute(None, query)
        self.has_spatial = self._fetchone(c)[0] > 0
        c.close()

        return self.has_spatial

    def _checkGeometryColumnsTable(self):
        """Check if user can read *_SDO_GEOM_METADATA view."""
        # First check if user can read ALL_SDO_GEOM_METADATA
        privs = self.getRawTablePrivileges('ALL_SDO_GEOM_METADATA',
                                           'MDSYS', 'PUBLIC')
        # Otherwise, try with USER_SDO_GEOM_METADATA
        if not privs[0]:
            privs = self.getRawTablePrivileges('USER_SDO_GEOM_METADATA',
                                               'MDSYS', 'PUBLIC')

        if privs[0]:
            self.has_geometry_columns = True
            self.has_geometry_columns_access = True
            self.is_geometry_columns_view = True
            return True
        else:
            self.has_geometry_columns = False
            self.has_geometry_columns_access = False
            self.is_geometry_columns_view = False
            return False

    def getInfo(self):
        """Returns Oracle Database server version."""
        c = self._execute(None, u"SELECT * FROM V$VERSION WHERE ROWNUM < 2")
        res = self._fetchone(c)
        c.close()
        return res

    def hasCache(self):
        """Returns self.cache_connection."""
        if self.cache_connection:
            return True
        return False

    def getSpatialInfo(self):
        """Returns Oracle Spatial version."""
        if not self.has_spatial:
            return

        try:
            c = self._execute(None, u"SELECT SDO_VERSION FROM DUAL")
        except DbError:
            return
        res = self._fetchone(c)
        c.close()

        return res

    def hasSpatialSupport(self):
        """Find if there is Spatial support."""
        return self.has_spatial

    def hasRasterSupport(self):
        """No raster support for the moment !"""
        # return self.has_raster
        return False

    def hasCustomQuerySupport(self):
        """From QGis v2.2 Oracle custom queries are supported."""
        return QGis.QGIS_VERSION_INT >= 20200

    def hasTableColumnEditingSupport(self):
        """Tables can always be edited."""
        return True

    def hasCreateSpatialViewSupport(self):
        """We can create Spatial Views."""
        return True

    def fieldTypes(self):
        """From
        http://docs.oracle.com/cd/B28359_01/server.111/b28318/datatype.htm#CNCPT1828
        """
        return [
            "number", "number(9)",  # integers
            "number(9,2)", "number(*,4)", "binary_float",
            "binary_double",  # floats
            "varchar2(255)", "char(20)", "nvarchar2(255)",
            "nchar(20)",  # strings
            "date", "timestamp"  # date/time
        ]

    def getSchemaPrivileges(self, schema):
        """
        Schema privileges:
        (can create new objects, can access objects in schema)
        """
        # TODO: find the best way in Oracle do determine schema privileges
        schema = self.user if not schema else schema

        # In Oracle world, rights seems quite simple: only schema_owner can
        # create table in the schema
        if schema == self.user:
            return (True, True)
        # getSchemas request only extract schemas where user has access
        return (False, True)

    def getRawTablePrivileges(self, table, owner, grantee):
        """
        Retrieve privileges on a table in a schema for a specific
        user.
        """
        result = [False, False, False, False]
        if owner != self.user:
            prefix = u"USER"
        # Inspect in all tab privs
        sql = u"""
        SELECT DISTINCT PRIVILEGE
        FROM ALL_TAB_PRIVS_RECD
        WHERE PRIVILEGE IN ('SELECT','INSERT','UPDATE','DELETE')
          AND TABLE_NAME = {0}
          AND OWNER = {1}
          AND GRANTEE IN ({2}, {3})
        """.format(self.quoteString(table),
                   self.quoteString(owner),
                   self.quoteString(grantee),
                   self.quoteString(grantee.upper()))

        c = self._execute(None, sql)
        res = self._fetchall(c)
        c.close()

        # Find which privilege is returned
        for line in res:
            if line[0] == u"SELECT":
                result[0] = True
            if line[0] == u"INSERT":
                result[1] = True
            if line[0] == u"UPDATE":
                result[2] = True
            if line[0] == u"DELETE":
                result[3] = True

        return result

    def getTablePrivileges(self, table):
        """Retrieve table privileges: (select, insert, update, delete)."""

        schema, tablename = self.getSchemaTableName(table)
        if self.user == schema:
            return [True, True, True, True]
        return self.getRawTablePrivileges(tablename, schema, self.user)

    def getSchemasCache(self):
        """Get the list of schemas from the cache."""
        sql = u"""
        SELECT DISTINCT ownername
        FROM "oracle_{}"
        ORDER BY ownername
        """.format(self.connName)
        c = self.cache_connection.cursor()
        c.execute(sql)
        res = c.fetchall()
        c.close()

        return res

    def getSchemas(self):
        """Get list of schemas in tuples:
        (oid, name, owner, perms, comment).
        """
        if self.userTablesOnly:
            return [(self.user,)]

        if self.hasCache():
            return self.getSchemasCache()

        # Use cache if avalaible:
        metatable = (u"all_objects WHERE object_type IN "
                     u"('TABLE','VIEW','SYNONYM')")
        if self.geometryColumnsOnly:
            metatable = u"all_sdo_geom_metadata"

        sql = u"""SELECT DISTINCT owner FROM {} ORDER BY owner""".format(
            metatable)

        c = self._execute(None, sql)
        res = self._fetchall(c)
        c.close()

        return res

    def getTables(self, schema=None, add_sys_tables=False):
        """Get list of tables."""
        if self.hasCache() and not self.populated:
            self.populated = True
            return self.getTablesCache(schema)

        tablenames = []
        items = []

        try:
            vectors = self.getVectorTables(schema)
            for tbl in vectors:
                tablenames.append((tbl[2], tbl[1]))
                items.append(tbl)
        except DbError:
            pass

        if self.allowGeometrylessTables:
            # get all non geographic tables and views
            prefix = u"ALL"
            owner = u"o.owner"
            metatable = u"tab_columns"
            where = u""
            if self.userTablesOnly:
                prefix = u"USER"
                owner = u"user As OWNER"
            if schema and not self.userTablesOnly:
                where = u"AND o.owner = {} ".format(
                    self.quoteString(schema))

            sql = u"""
            SELECT o.OBJECT_NAME, {0},
                   CASE o.OBJECT_TYPE
                   WHEN 'VIEW' THEN 1
                   ELSE 0 END As isView
            FROM {1}_OBJECTS o
            WHERE o.object_type IN ('TABLE','VIEW','SYNONYM')
            {2} {3}
            ORDER BY o.OBJECT_NAME
            """.format(owner, prefix, where,
                       u"" if add_sys_tables
                       else u"AND o.OBJECT_NAME NOT LIKE 'MDRT_%'")

            c = self._execute(None, sql)
            for tbl in self._fetchall(c):
                if tablenames.count((tbl[1], tbl[0])) <= 0:
                    item = list(tbl)
                    item.insert(0, Table.TableType)
                    items.append(item)

            c.close()

        self.populated = True

        listTables = sorted(items, cmp=lambda x, y: cmp((x[2], x[1]),
                                                        (y[2], y[1])))
        if self.hasCache():
            self.updateCache(listTables, schema)
            return self.getTablesCache(schema)

        return listTables

    def getTablesCache(self, schema=None):
        """Get list of tables from SQLite cache."""

        tablenames = []
        items = []

        try:
            vectors = self.getVectorTablesCache(schema)
            for tbl in vectors:
                tablenames.append((tbl[2], tbl[1]))
                items.append(tbl)
        except DbError:
            pass

        if not self.allowGeometrylessTables:
            return sorted(items,
                          cmp=lambda x, y: cmp((x[2], x[1]),
                                               (y[2], y[1])))

        # get all non geographic tables and views
        schema_where = u""
        if self.userTablesOnly:
            schema_where = u"AND ownername = '{}'".format(
                self.user)
        if schema and not self.userTablesOnly:
            schema_where = u"AND ownername = '{}'".format(
                schema)

        sql = u"""
        SELECT tablename, ownername, isview
        FROM "oracle_{0}"
        WHERE geometrycolname IS '' {1}
        ORDER BY tablename
        """.format(self.connName, schema_where)

        c = self.cache_connection.cursor()
        c.execute(sql)
        for tbl in c.fetchall():
            if tablenames.count((tbl[1], tbl[0])) <= 0:
                item = list(tbl)
                item.insert(0, Table.TableType)
                items.append(item)
        c.close()

        return sorted(items, cmp=lambda x, y: cmp((x[2], x[1]), (y[2], y[1])))

    def updateCache(self, tableList, schema=None):
        """Update the SQLite cache of table list for a schema."""

        data = []
        # First, we treat the list
        for table in tableList:
            line = ()
            # if the table is a view bring pkCols
            pkCols = None
            if int(table[3]) == 1:
                pkCols = self.pkCols((schema, table[1]))
            # Deals with non-geographic tables
            if table[0] == Table.TableType:
                line = (table[1], table[2], int(table[3]),
                        u"",
                        u",".join(pkCols) if pkCols else u"",
                        100, 0, u"")
            # Deals with vector tables
            elif table[0] == Table.VectorType:
                line = (table[1], table[2], int(table[3]),
                        table[4],
                        u",".join(pkCols) if pkCols else u"",
                        table[9],
                        table[8] if table[10] == u"-1" else table[10],
                        u"")
            else:
                continue
            data.append(line)

        # Then, empty the cache list
        sql = u"""
        DELETE FROM "oracle_{0}" {1}
        """.format(self.connName,
                   u"WHERE ownername = '{}'".format(schema) if schema else u"")
        self.cache_connection.execute(sql)
        self.cache_connection.commit()

        # Then we insert into SQLite database
        sql = u"""
        INSERT INTO "oracle_{}"(tablename, ownername, isview,
        geometrycolname, pkcols, geomtypes, geomsrids, sql)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
        """.format(self.connName)
        c = self.cache_connection.cursor()
        c.executemany(sql, data)
        c.close()
        self.cache_connection.commit()

    def singleGeomTypes(self, geomtypes, srids):
        """Intelligent wkbtype grouping (multi with non multi)"""
        if (QGis.WKBPolygon in geomtypes
                and QGis.WKBMultiPolygon in geomtypes):
            srids.pop(geomtypes.index(QGis.WKBPolygon))
            geomtypes.pop(geomtypes.index(QGis.WKBPolygon))
        if (QGis.WKBPoint in geomtypes
                and QGis.WKBMultiPoint in geomtypes):
            srids.pop(geomtypes.index(QGis.WKBPoint))
            geomtypes.pop(geomtypes.index(QGis.WKBPoint))
        if (QGis.WKBLineString in geomtypes
                and QGis.WKBMultiLineString in geomtypes):
            srids.pop(geomtypes.index(QGis.WKBLineString))
            geomtypes.pop(geomtypes.index(QGis.WKBLineString))
        if QGis.WKBUnknown in geomtypes and len(geomtypes) > 1:
            srids.pop(geomtypes.index(QGis.WKBUnknown))
            geomtypes.pop(geomtypes.index(QGis.WKBUnknown))

        return geomtypes, srids

    def getVectorTablesCache(self, schema=None):
        """Get list of table with a geometry column from SQLite cache
        it returns:
        name (table name)
        namespace (schema)
        type = 'view' (is a view?)
        geometry_column
        geometry_types (as WKB type)
        srids
        """
        schema_where = u""
        if self.userTablesOnly:
            schema_where = u"AND ownername = '{}'".format(
                self.user)
        if schema and not self.userTablesOnly:
            schema_where = u"AND ownername = '{}'".format(
                schema)

        sql = u"""
        SELECT tablename, ownername, isview,
               geometrycolname,
               geomtypes, geomsrids
        FROM "oracle_{0}"
        WHERE geometrycolname IS NOT '' {1}
        ORDER BY tablename
        """.format(self.connName, schema_where)

        items = []

        c = self.cache_connection.cursor()
        c.execute(sql)
        lst_tables = c.fetchall()
        c.close()

        # Handle multiple geometries tables
        for i, tbl in enumerate(lst_tables):
            item = list(tbl)
            srids = item.pop()
            geomtypes = item.pop()
            item.insert(0, Table.VectorType)
            if len(geomtypes) > 0 and len(srids) > 0:
                geomtypes = [int(l) for l in unicode(geomtypes).split(u",")]
                srids = [int(l) for l in unicode(srids).split(u",")]
                geomtypes, srids = self.singleGeomTypes(geomtypes, srids)
                for j in range(len(geomtypes)):
                    buf = list(item)
                    geomtype = geomtypes[j]
                    srid = srids[j]
                    datatype = QGis.featureType(QGis.singleType(geomtype))
                    geo = datatype[3:].upper().strip(u"25D")
                    buf.append(geo)
                    buf.append(geomtype)
                    buf.append(QGis.wkbDimensions(geomtype))  # Dimensions
                    buf.append(srid)
                    buf.append(None)  # To respect ORTableVector row
                    buf.append(None)  # To respect ORTableVector row
                    items.append(buf)

        return items

    def getVectorTables(self, schema=None):
        """Get list of table with a geometry column
        it returns a table of tuples:
            name (table name)
            namespace (schema/owner)
            isView (is a view?)
            geometry_column
            srid
        """
        if not self.has_spatial:
            return []

        # discovery of all geographic tables
        prefix = u"all"
        owner = u"c.owner"
        where = None

        if not self.geometryColumnsOnly:
            where = u"WHERE c.data_type = 'SDO_GEOMETRY'"
        if schema and not self.userTablesOnly:
            where = u"{0} c.owner = {1}".format(
                u"{} AND".format(where) if where else u"WHERE",
                self.quoteString(schema))

        if self.userTablesOnly:
            prefix = u"user"
            owner = u"user As owner"
            if self.geometryColumnsOnly:
                where = u""

        sql = u"""
        SELECT c.table_name, {0},
               CASE o.OBJECT_TYPE
               WHEN 'VIEW' THEN 1
               ELSE 0 END As isView,
               c.column_name,
               {1}
        FROM {2}_{3} c
        JOIN {2}_objects o ON c.table_name = o.object_name
             AND o.object_type IN ('TABLE','VIEW','SYNONYM') {4} {5}
        ORDER BY TABLE_NAME
        """.format(owner,
                   u"c.srid" if self.geometryColumnsOnly
                   else u"NULL as srid",
                   prefix,
                   u"sdo_geom_metadata" if self.geometryColumnsOnly
                   else u"tab_columns",
                   u"" if self.userTablesOnly
                   else u"AND c.owner = o.owner",
                   where)

        # For each table, get all of the details
        items = []

        c = self._execute(None, sql)
        lst_tables = self._fetchall(c)
        c.close()

        for i, tbl in enumerate(lst_tables):
            item = list(tbl)
            detectedSrid = item.pop()
            if isinstance(detectedSrid, QPyNullVariant):
                detectedSrid = u"-1"
            else:
                detectedSrid = int(detectedSrid)

            if schema:
                table_name = u"{0}.{1}".format(self.quoteId(schema),
                                               self.quoteId(item[0]))
            else:
                table_name = self.quoteId(item[0])
            geocol = self.quoteId(item[3])
            geomMultiTypes, multiSrids = self.getTableGeomTypes(
                table_name, geocol)
            geomtypes = list(geomMultiTypes)
            srids = list(multiSrids)
            item.insert(0, Table.VectorType)

            geomtypes, srids = self.singleGeomTypes(geomtypes, srids)

            for j in range(len(geomtypes)):
                buf = list(item)
                geomtype = geomtypes[j]
                datatype = QGis.featureType(QGis.singleType(geomtype))
                geo = datatype[3:].upper().strip(u"25D")
                buf.append(geo)  # Geometry type as String
                buf.append(geomtype)  # QGis.WkbType
                buf.append(QGis.wkbDimensions(geomtype))  # Dimensions
                buf.append(detectedSrid)  # srid
                if not self.onlyExistingTypes:
                    geomMultiTypes.append(0)
                    multiSrids.append(multiSrids[0])
                buf.append(u",".join([unicode(x) for x in
                                      geomMultiTypes]))
                buf.append(u",".join([unicode(x) for x in multiSrids]))
                items.append(buf)

            if self.allowGeometrylessTables and buf[-6] != u"UNKNOWN":
                copybuf = list(buf)
                copybuf[4] = u""
                copybuf[-6] = u"UNKNOWN"
                copybuf[-5] = QGis.WKBNoGeometry
                copybuf[-2] = QGis.WKBNoGeometry
                copybuf[-1] = u"0"
                items.append(copybuf)

        return items

    def getTableComment(self, table, objectType):
        """Return the general comment for the object"""

        schema, tablename = self.getSchemaTableName(table)
        data_prefix = u"ALL" if schema else u"USER"
        where = u"AND OWNER = {}".format(
            self.quoteString(schema)) if schema else u""
        if objectType in [u"TABLE", u"VIEW"]:
            data_table = u"{}_TAB_COMMENTS"
            table = u"TABLE"
        elif objectType == u"MATERIALIZED VIEW":
            data_table = u"{}_MVIEW_COMMENTS"
            table = u"MVIEW"
        else:
            return None

        data_table = data_table.format(data_prefix)
        sql = u"""
        SELECT COMMENTS FROM {0} WHERE {1}_NAME = {2}
        {3}
        """.format(data_table, table,
                   self.quoteString(tablename),
                   where)

        c = self._execute(None, sql)
        res = self._fetchone(c)
        c.close()

        if res:
            return res[0]

        return None

    def getTableType(self, table):
        """Return the type of a table between the following:
        * Table
        * View
        * Materialized view
        """

        schema, tablename = self.getSchemaTableName(table)
        sql = u"""
        SELECT OBJECT_TYPE FROM {0} WHERE OBJECT_NAME = {1} {2}
        """
        if schema:
            sql = sql.format(u"ALL_OBJECTS",
                             self.quoteString(tablename),
                             u"AND OWNER = {}".format(
                                 self.quoteString(schema)))
        else:
            sql = sql.format(u"USER_OBJECTS",
                             self.quoteString(tablename),
                             u"")

        c = self._execute(None, sql)
        res = self._fetchall(c)
        c.close()

        # Analyse return values
        if not res:
            return False
        else:
            types = [x[0] for x in res]
            if u"MATERIALIZED VIEW" in types:
                return u"MATERIALIZED VIEW"
            elif u"VIEW" in types:
                return u"VIEW"
            else:
                return u"TABLE"

    def pkCols(self, table):
        """Return the primary keys candidates for a view."""
        schema, tablename = self.getSchemaTableName(table)
        sql = u"""
        SELECT column_name
        FROM all_tab_columns
        WHERE owner={0}
        AND table_name={1}
        ORDER BY column_id
        """.format(self.quoteString(schema) if schema else self.user,
                   self.quoteString(tablename))
        c = self._execute(None, sql)
        res = self._fetchall(c)
        c.close()

        return [x[0] for x in res] if res else None

    def getTableGeomTypes(self, table, geomCol):
        """Return all the wkbTypes for a table by requesting geometry
        column.
        """

        estimated = u""
        if self.useEstimatedMetadata:
            estimated = u"AND ROWNUM < 100"

        # Grab all of geometry types from the layer
        query = u"""
        SELECT DISTINCT a.{0}.SDO_GTYPE As gtype,
                        a.{0}.SDO_SRID
        FROM {1} a
        WHERE a.{0} IS NOT NULL {2}
        ORDER BY a.{0}.SDO_GTYPE
        """.format(geomCol, table, estimated)

        try:
            c = self._execute(None, query)
        except DbError, e:  # handle error views or other problems
            return [QGis.WKBUnknown], [-1]

        rows = self._fetchall(c)
        c.close()

        # Handle results
        if len(rows) == 0:
            return [QGis.WKBUnknown], [-1]

        # A dict to store the geomtypes
        geomtypes = []
        srids = []
        for row in rows:
            if isinstance(row[1], QPyNullVariant):
                srids.append(-1)
            else:
                srids.append(int(row[1]))
            if int(row[0]) in OracleDBConnector.ORGeomTypes.keys():
                geomtypes.append(OracleDBConnector.ORGeomTypes[int(row[0])])
            else:
                geomtypes.append(QGis.WKBUnknown)

        return geomtypes, srids

    def getTableMainGeomType(self, table, geomCol):
        """Return the best wkbType for a table by requesting geometry
        column.
        """

        geomTypes, srids = self.getTableGeomTypes(table, geomCol)

        # Make the decision:
        wkbType = QGis.WKBUnknown
        srid = -1
        order = [QGis.WKBMultiPolygon25D, QGis.WKBPolygon25D,
                 QGis.WKBMultiPolygon, QGis.WKBPolygon,
                 QGis.WKBMultiLineString25D, QGis.WKBLineString25D,
                 QGis.WKBMultiLineString, QGis.WKBLineString,
                 QGis.WKBMultiPoint25D, QGis.WKBPoint25D,
                 QGis.WKBMultiPoint, QGis.WKBPoint]
        for geomType in order:
            if geomType in geomTypes:
                wkbType = geomType
                srid = srids[geomTypes.index(geomType)]
                break

        return wkbType, srid

    def getTableRowEstimation(self, table):
        """ Find the estimated number of rows of a table. """
        schema, tablename = self.getSchemaTableName(table)
        prefix = u"ALL" if schema else u"USER"
        where = u"AND OWNER = {}".format(
            self.quoteString(schema)) if schema else u""

        sql = u"""
        SELECT NUM_ROWS FROM {0}_ALL_TABLES
        WHERE TABLE_NAME = {1}
        {2}
        """.format(prefix, self.quoteString(tablename), where)

        c = self._execute(None, sql)
        res = self._fetchone(c)
        c.close()

        if not res or isinstance(res[0], QPyNullVariant):
            return 0
        else:
            return int(res[0])

    def getTableDates(self, table):
        """ Returns the modification/creation dates of an object"""
        schema, tablename = self.getSchemaTableName(table)
        prefix = u"ALL" if schema else u"USER"
        where = u"AND OWNER = {}".format(
            self.quoteString(schema)) if schema else u""

        sql = u"""
        SELECT CREATED, LAST_DDL_TIME FROM {0}_OBJECTS
        WHERE OBJECT_NAME = {1}
        {2}
        """.format(prefix, self.quoteString(tablename), where)

        c = self._execute(None, sql)
        res = self._fetchone(c)
        c.close()

        if not res:
            return None, None

        return res[0], res[1]

    def getTableRowCount(self, table):
        """Returns the number of rows of the table."""
        c = self._execute(
            None, u"SELECT COUNT(*) FROM {}".format(self.quoteId(table)))
        res = self._fetchone(c)[0]
        c.close()

        return res

    def getTableFields(self, table):
        """Returns list of columns in table."""

        schema, tablename = self.getSchemaTableName(table)
        schema_where = u" AND a.OWNER={}".format(
            self.quoteString(schema) if schema else "")
        sql = u"""
        SELECT a.COLUMN_ID As ordinal_position,
               a.COLUMN_NAME As column_name,
               a.DATA_TYPE As data_type,
               CASE a.DATA_TYPE
                 WHEN 'NUMBER' THEN a.DATA_PRECISION
                 ELSE a.DATA_LENGTH END As char_max_len,
               a.DATA_SCALE As modifier,
               a.NULLABLE As nullable,
               a.DEFAULT_LENGTH As hasdefault,
               a.DATA_DEFAULT As default_value,
               a.DATA_TYPE As formatted_type,
               c.COMMENTS
        FROM ALL_TAB_COLUMNS a
            JOIN ALL_COL_COMMENTS c ON
                a.TABLE_NAME = c.TABLE_NAME
                AND a.COLUMN_NAME = c.COLUMN_NAME
                AND a.OWNER = c.OWNER
        WHERE a.TABLE_NAME = {0} {1}
        ORDER BY a.COLUMN_ID
        """.format(self.quoteString(tablename), schema_where)

        c = self._execute(None, sql)
        res = self._fetchall(c)
        c.close()
        return res

    def getSpatialFields(self, table):
        """Returns the list of geometric columns"""
        fields = self.getTableFields(table)
        geomFields = []
        for field in fields:
            if field[2] == u"SDO_GEOMETRY":
                geomFields.append(field[1])

        return geomFields

    def getTableIndexes(self, table):
        """Get info about table's indexes."""
        schema, tablename = self.getSchemaTableName(table)
        schema_where = u" AND i.OWNER = {} ".format(
            self.quoteString(schema) if schema else "")

        sql = u"""
        SELECT i.INDEX_NAME, c.COLUMN_NAME, i.ITYP_NAME,
               i.STATUS, i.LAST_ANALYZED, i.COMPRESSION,
               i.UNIQUENESS
        FROM ALL_INDEXES i
        INNER JOIN ALL_IND_COLUMNS c ON i.index_name = c.index_name
        WHERE i.table_name = {0} {1}
        """.format(self.quoteString(tablename), schema_where)

        c = self._execute(None, sql)
        res = self._fetchall(c)
        c.close()

        return res

    def getMViewInfo(self, table):
        """Find some information about materialized views"""
        schema, tablename = self.getSchemaTableName(table)
        where = u" AND a.OWNER = {} ".format(
            self.quoteString(schema)) if schema else u""
        prefix = u"ALL" if schema else u"USER"
        sql = u"""
        SELECT a.REFRESH_MODE,
               a.REFRESH_METHOD, a.BUILD_MODE, a.FAST_REFRESHABLE,
               a.LAST_REFRESH_TYPE, a.LAST_REFRESH_DATE, a.STALENESS,
               a.STALE_SINCE, a.COMPILE_STATE, a.USE_NO_INDEX
        FROM {0}_MVIEWS a
        WHERE MVIEW_NAME = {1}
        {2}
        """.format(prefix, self.quoteString(tablename), where)

        c = self._execute(None, sql)
        res = self._fetchone(c)
        c.close()

        return res

    def getTableConstraints(self, table):
        """Find all the constraints for a table."""
        schema, tablename = self.getSchemaTableName(table)
        schema_where = u" AND c.OWNER={} ".format(
            self.quoteString(schema)) if schema else u""

        sql = u"""
        SELECT a.CONSTRAINT_NAME, a.CONSTRAINT_TYPE,
               c.COLUMN_NAME, a.VALIDATED, a.GENERATED, a.STATUS,
               a.SEARCH_CONDITION, a.DELETE_RULE,
               CASE WHEN b.TABLE_NAME IS NULL THEN NULL
                    ELSE b.OWNER || '.' || b.TABLE_NAME END
               As F_TABLE, b.COLUMN_NAME As F_COLUMN
        FROM ALL_CONS_COLUMNS c
        INNER JOIN ALL_CONSTRAINTS a ON
                   a.CONSTRAINT_NAME = c.CONSTRAINT_NAME
        LEFT OUTER JOIN ALL_CONS_COLUMNS b ON
                        b.CONSTRAINT_NAME = a.R_CONSTRAINT_NAME
                        AND a.R_OWNER = b.OWNER
                        AND b.POSITION = c.POSITION
        WHERE c.TABLE_NAME = {0} {1}
        """.format(self.quoteString(tablename), schema_where)

        c = self._execute(None, sql)
        res = self._fetchall(c)
        c.close()

        return res

    def getTableTriggers(self, table):
        """Find all the triggers of the table."""
        schema, tablename = self.getSchemaTableName(table)

        sql = u"""
        SELECT TRIGGER_NAME, TRIGGERING_EVENT, TRIGGER_TYPE, STATUS
        FROM ALL_TRIGGERS
        WHERE TABLE_OWNER = {0}
        AND TABLE_NAME = {1}
        """.format(self.quoteString(schema), self.quoteString(tablename))

        c = self._execute(None, sql)
        res = self._fetchall(c)
        c.close()

        return res

    def enableAllTableTriggers(self, enable, table):
        """Enable or disable all triggers on table."""
        triggers = [l[0] for l in self.getTableTriggers(table)]
        for trigger in triggers:
            self.enableTableTrigger(trigger, enable, table)

    def enableTableTrigger(self, trigger, enable, table):
        """Enable or disable one trigger on table."""
        schema, tablename = self.getSchemaTableName(table)
        trigger = u".".join([self.quoteId(schema), self.quoteId(trigger)])
        sql = u"ALTER TRIGGER {0} {1}".format(trigger, "ENABLE" if
                                              enable else "DISABLE")
        self._execute_and_commit(sql)

    def deleteTableTrigger(self, trigger, table):
        """Delete the trigger on a table."""
        schema, tablename = self.getSchemaTableName(table)
        trigger = u".".join([self.quoteId(schema), self.quoteId(trigger)])
        sql = u"DROP TRIGGER {}".format(trigger)
        self._execute_and_commit(sql)

    def canUpdateMetadata(self, table):
        """Verify if user can update metadata table
        returns False or metadata table name.
        """
        schema, tablename = self.getSchemaTableName(table)
        metadata = False
        # User can only update in USER_SDO_GEOM_METADATA
        if self.getRawTablePrivileges('USER_SDO_GEOM_METADATA', 'MDSYS',
                                      'PUBLIC')[2]:
            tbQuery = u"""
            SELECT COUNT(*) FROM USER_SDO_GEOM_METADATA
            WHERE TABLE_NAME = {0}
            """.format(self.quoteString(tablename))
            c = self._execute(None, tbQuery)
            res = self._fetchone(c)
            c.close()

            if res:
                if res[0] > 0:
                    metadata = True

        return metadata

    def getTableExtent(self, table, geom):
        """Calculate the real table extent."""
        schema, tablename = self.getSchemaTableName(table)
        tableQuote = u"'{0}.{1}'".format(schema, tablename)
        # Extent calculation without spatial index
        extentFunction = u"""SDO_AGGR_MBR("{0}")""".format(geom)
        fromTable = u'"{0}"."{1}"'.format(schema, tablename)

        # if table as spatial index:
        indexes = self.getTableIndexes(table)
        if indexes:
            if u"SPATIAL_INDEX" in [f[2] for f in indexes]:
                extentFunction = u"SDO_TUNE.EXTENT_OF({0}, {1})".format(
                    tableQuote, self.quoteString(geom))
                fromTable = u"DUAL"

        sql = u"""
        SELECT
        SDO_GEOM.SDO_MIN_MBR_ORDINATE({0}, 1),
        SDO_GEOM.SDO_MIN_MBR_ORDINATE({0}, 2),
        SDO_GEOM.SDO_MAX_MBR_ORDINATE({0}, 1),
        SDO_GEOM.SDO_MAX_MBR_ORDINATE({0}, 2)
        FROM {1}
        """.format(extentFunction, fromTable)

        try:
            c = self._execute(None, sql)
        except DbError, e:  # no spatial index on table, try aggregation
            return None

        res = self._fetchone(c)
        c.close()

        if not res:
            res = None

        return res if res else None

    def getTableEstimatedExtent(self, table, geom):
        """Find out estimated extent (from metadata view)."""
        res = []
        schema, tablename = self.getSchemaTableName(table)
        where = u"""
        WHERE TABLE_NAME = {}
        AND COLUMN_NAME = {}
        """.format(self.quoteString(tablename),
                   self.quoteString(geom))
        if schema:
            where = u"{} AND OWNER = {}".format(
                where, self.quoteString(schema))

        request = u"""
        SELECT SDO_LB, SDO_UB
        FROM ALL_SDO_GEOM_METADATA m,
             TABLE(m.DIMINFO)
        {0}
        AND SDO_DIMNAME = '{1}'
        """
        for dimension in [u"X", u"Y"]:
            sql = request.format(where, dimension)
            try:
                c = self._execute(None, sql)
            except DbError, e:  # no statistics for the current table
                return None

            res_d = self._fetchone(c)
            c.close()

            if not res_d or len(res_d) < 2:
                return None
            elif isinstance(res_d[0], QPyNullVariant):
                return None
            else:
                res.extend(res_d)

        return [res[0], res[2], res[1], res[3]]

    def getDefinition(self, view, objectType):
        """Returns definition of the view."""

        schema, tablename = self.getSchemaTableName(view)
        where = u""
        if schema:
            where = u" AND OWNER={} ".format(
                self.quoteString(schema))

        # Query to grab a view definition
        if objectType == u"VIEW":
            sql = u"""
            SELECT TEXT FROM ALL_VIEWS WHERE VIEW_NAME = {0} {1}
            """.format(self.quoteString(tablename), where)
        elif objectType == u"MATERIALIZED VIEW":
            sql = u"""
            SELECT QUERY FROM ALL_MVIEWS WHERE MVIEW_NAME = {0} {1}
            """.format(self.quoteString(tablename), where)
        else:
            return None

        c = self._execute(None, sql)
        res = self._fetchone(c)
        c.close()

        return res[0] if res else None

    def getSpatialRefInfo(self, srid):
        """Returns human name from an srid as describe in Oracle sys
        table.
        """
        if not self.has_spatial:
            return

        try:
            c = self._execute(
                None,
                (u"SELECT CS_NAME FROM MDSYS.CS_SRS WHERE"
                 u" SRID = {}".format(srid)))
        except DbError, e:
            return
        sr = self._fetchone(c)
        c.close()

        return sr[0] if sr else None

    def isVectorTable(self, table):
        """Determine if a table is a vector one by looking into
        metadata view.
        """
        if self.has_geometry_columns and self.has_geometry_columns_access:
            schema, tablename = self.getSchemaTableName(table)
            where = u"WHERE TABLE_NAME = {}".format(
                self.quoteString(tablename))
            if schema:
                where = u"{} AND OWNER = {}".format(where,
                                                    self.quoteString(schema))
            sql = u"""
            SELECT COUNT(*)
            FROM ALL_SDO_GEOM_METADATA
            {}
            """.format(where)

            c = self._execute(None, sql)
            res = self._fetchone(c)
            c.close()
            return res is not None and res[0] > 0

        return False

    def createTable(self, table, field_defs, pkey):
        """Create ordinary table
        'fields' is array containing field definitions
        'pkey' is the primary key name
        """
        if len(field_defs) == 0:
            return False

        sql = "CREATE TABLE {} (".format(self.quoteId(table))
        sql += u", ".join(field_defs)
        if pkey:
            sql += u", PRIMARY KEY ({})".format(self.quoteId(pkey))
        sql += ")"

        self._execute_and_commit(sql)
        return True

    def deleteTable(self, table):
        """Delete table and its reference in sdo_geom_metadata."""

        schema, tablename = self.getSchemaTableName(table)
        schema_part = u"AND owner = {} ".format(
            self.quoteString(schema)) if schema else ""

        if self.isVectorTable(table):
            self.deleteMetadata(table)

        sql = u"DROP TABLE {}".format(self.quoteId(table))
        self._execute_and_commit(sql)

    def emptyTable(self, table):
        """Delete all the rows of a table."""

        sql = u"TRUNCATE TABLE {}".format(self.quoteId(table))
        self._execute_and_commit(sql)

    def renameTable(self, table, new_table):
        """Rename a table inside the database."""
        schema, tablename = self.getSchemaTableName(table)
        if new_table == tablename:
            return

        c = self._get_cursor()

        # update geometry_columns if Spatial is enabled
        if self.isVectorTable(table):
            self.updateMetadata(table, None, new_table=new_table)

        sql = u"RENAME {0} TO {1}".format(
            self.quoteId(tablename), self.quoteId(new_table))
        self._execute(c, sql)

        self._commit()

    def createView(self, view, query):
        """Create a view as defined."""
        sql = u"CREATE VIEW {0} AS {1}".format(self.quoteId(view),
                                               query)
        self._execute_and_commit(sql)

    def createSpatialView(self, view, query):
        """Creates a spatial view and update metadata table."""
        # What is the view name ?
        if len(view.split(u".")) > 1:
            schema, view = view.split(u".")
        else:
            schema = self.user
        view = (schema, view)

        # First create the view
        self.createView(view, query)

        # Grab the geometric column(s)
        fields = self.getSpatialFields(view)
        if not fields:
            return False

        for geoCol in fields:
            # Grab SRID
            geomTypes, srids = self.getTableGeomTypes(view, geoCol)

            # Calculate the extent
            extent = self.getTableExtent(view, geoCol)

            # Insert information into metadata table
            self.insertMetadata(view, geoCol, extent, srids[0])

        return True

    def deleteView(self, view):
        """Delete a view."""
        schema, tablename = self.getSchemaTableName(view)
        schema_part = u"AND owner = {} ".format(
            self.quoteString(schema)) if schema else ""

        if self.isVectorTable(view):
            self.deleteMetadata(view)

        sql = u"DROP VIEW {}".format(self.quoteId(view))
        self._execute_and_commit(sql)

    def createSchema(self, schema):
        """Create a new empty schema in database."""
        # Not tested
        sql = u"CREATE SCHEMA AUTHORIZATION {}".format(
            self.quoteId(schema))
        self._execute_and_commit(sql)

    def deleteSchema(self, schema):
        """Drop (empty) schema from database."""
        sql = u"DROP USER {} CASCADE".format(self.quoteId(schema))
        self._execute_and_commit(sql)

    def renameSchema(self, schema, new_schema):
        """Rename a schema in the database."""
        # Unsupported in Oracle
        pass

    def addTableColumn(self, table, field_def):
        """Add a column to a table."""
        sql = u"ALTER TABLE {0} ADD {1}".format(self.quoteId(table),
                                                field_def)
        self._execute_and_commit(sql)

    def deleteTableColumn(self, table, column):
        """Delete column from a table."""
        # Delete all the constraints for this column
        constraints = [f[0] for f in self.getTableConstraints(table)
                       if f[2] == column]
        for constraint in constraints:
            self.deleteTableConstraint(table, constraint)

        # Delete all the indexes for this column
        indexes = [f[0] for f in self.getTableIndexes(table) if f[1] == column]
        for ind in indexes:
            self.deleteTableIndex(table, ind)

        # Delete metadata is we have a geo column
        if self.isGeometryColumn(table, column):
            self.deleteMetadata(table, column)

        sql = u"ALTER TABLE {0} DROP COLUMN {1}".format(
            self.quoteId(table), self.quoteId(column))
        self._execute_and_commit(sql)

    def updateTableColumn(self, table, column, new_name=None,
                          data_type=None, not_null=None,
                          default=None):
        """Update properties of a column in a table."""

        schema, tablename = self.getSchemaTableName(table)

        c = self._get_cursor()

        # update column definition
        col_actions = []
        if data_type:
            col_actions.append(u"{}".format(data_type))
        if default:
            col_actions.append(u"DEFAULT {}".format(default))
        else:
            col_actions.append(u"DEFAULT NULL")

        if not_null:
            col_actions.append(u"NOT NULL")
        if not_null is None:
            col_actions.append(u"NULL")

        if col_actions:
            sql = u"ALTER TABLE {0} MODIFY ( {1} {2} )".format(
                self.quoteId(table), self.quoteId(column),
                u" ".join(col_actions))
            self._execute(c, sql)

        # rename the column
        if new_name and new_name != column:
            isGeo = self.isGeometryColumn(table, column)
            sql = u"ALTER TABLE {0} RENAME COLUMN {1} TO {2}".format(
                self.quoteId(table), self.quoteId(column),
                self.quoteId(new_name))
            self._execute(c, sql)

            # update geometry_columns if Spatial is enabled
            if isGeo:
                self.updateMetadata(table, column, new_name)

        self._commit()

    def renameTableColumn(self, table, column, new_name):
        """Rename column in a table."""
        return self.updateTableColumn(table, column, new_name)

    def setTableColumnType(self, table, column, data_type):
        """Change column type."""
        return self.updateTableColumn(table, column, None, data_type)

    def setTableColumnNull(self, table, column, is_null):
        """Change whether column can contain null values."""
        return self.updateTableColumn(table, column, None, None, not is_null)

    def setTableColumnDefault(self, table, column, default):
        """Change column's default value.
        If default=None or an empty string drop default value.
        """
        return self.updateTableColumn(table, column, None, None, None, default)

    def isGeometryColumn(self, table, column):
        """Find if a column is geometric."""
        schema, tablename = self.getSchemaTableName(table)
        prefix = u"ALL" if schema else u"USER"
        where = u"AND owner = {} ".format(
            self.quoteString(schema)) if schema else ""

        sql = u"""
        SELECT COUNT(*)
        FROM {0}_SDO_GEOM_METADATA
        WHERE TABLE_NAME = {1}
              AND COLUMN_NAME = {2} {3}
        """.format(prefix, self.quoteString(tablename),
                   self.quoteString(column.upper()), where)

        c = self._execute(None, sql)
        res = self._fetchone(c)[0] > 0

        c.close()
        return res

    def refreshMView(self, table):
        """Refresh an MVIEW"""
        schema, tablename = self.getSchemaTableName(table)
        mview = u"{}.{}".format(schema, tablename) if schema else tablename
        sql = u"""
        BEGIN
          DBMS_MVIEW.REFRESH({},'?');
        END;
        """.format(self.quoteString(mview))

        self._execute_and_commit(sql)

    def deleteMetadata(self, table, geom_column=None):
        """Delete the metadata entry for a table"""
        schema, tablename = self.getSchemaTableName(table)
        if not (self.getRawTablePrivileges('USER_SDO_GEOM_METADATA',
                                           'MDSYS',
                                           'PUBLIC')[3]
                and schema == self.user):
            return False

        where = u"WHERE TABLE_NAME = {}".format(self.quoteString(tablename))
        if geom_column:
            where = (u"{} AND COLUMN_NAME = "
                     u"{}".format(where,
                                  self.quoteString(geom_column)))
        sql = u"DELETE FROM USER_SDO_GEOM_METADATA {}".format(where)

        self._execute_and_commit(sql)

    def updateMetadata(self, table, geom_column, new_geom_column=None,
                       new_table=None, extent=None, srid=None):
        """update the metadata table with the new information"""

        schema, tablename = self.getSchemaTableName(table)
        if not (self.getRawTablePrivileges('USER_SDO_GEOM_METADATA',
                                           'MDSYS',
                                           'PUBLIC')[2]
                and schema == self.user):
            return False

        where = u"WHERE TABLE_NAME = {}".format(self.quoteString(tablename))
        if geom_column:
            # in Metadata view, geographic column is always in uppercase
            where = (u"{} AND COLUMN_NAME = "
                     u"{}".format(where,
                                  self.quoteString(geom_column.upper())))

        update = u"SET"
        if srid == 0:
            srid = -1

        if srid:
            update = u"{} SRID = {}".format(update, srid)
        if extent:
            if len(extent) == 4:
                if update != u"SET":
                    update = u"{},".format(update)
                update = u"""{4} DIMINFO = MDSYS.SDO_DIM_ARRAY(
                MDSYS.SDO_DIM_ELEMENT('X', {0:.9f}, {1:.9f}, 0.005),
                MDSYS.SDO_DIM_ELEMENT('Y', {2:.9f}, {3:.9f}, 0.005))
                """.format(extent[0], extent[2], extent[1],
                           extent[3], update)
        if new_geom_column:
            if update != u"SET":
                update = u"{},".format(update)
            # in Metadata view, geographic column is always in uppercase
            update = (u"{} COLUMN_NAME = "
                      u"{}".format(update,
                                   self.quoteString(new_geom_column.upper())))

        if new_table:
            if update != u"SET":
                update = u"{},".format(update)
            update = (u"{} TABLE_NAME = "
                      u"{}".format(update,
                                   self.quoteString(new_table)))

        sql = u"UPDATE USER_SDO_GEOM_METADATA {0} {1}".format(update, where)

        self._execute_and_commit(sql)

    def insertMetadata(self, table, geom_column, extent, srid, dim=2):
        """ Insert a line for the table in Oracle Metadata table."""
        schema, tablename = self.getSchemaTableName(table)
        if not (self.getRawTablePrivileges('USER_SDO_GEOM_METADATA',
                                           'MDSYS',
                                           'PUBLIC')[1]
                and schema == self.user):
            return False

        # in Metadata view, geographic column is always in uppercase
        geom_column = geom_column.upper()
        if srid == 0:
            srid = -1

        if len(extent) != 4:
            return False
        dims = [u'X', u'Y', u'Z', u'T']
        extentParts = []
        for i in range(dim):
            extentParts.append(
                u"""MDSYS.SDO_DIM_ELEMENT(
                '{0}', {1:.9f}, {2:.9f}, 0.005)""".format(dims[i],
                                                          extent[i],
                                                          extent[i+1]))
        extentParts = u",".join(extentParts)
        sqlExtent = u"""MDSYS.SDO_DIM_ARRAY(
                {})
                """.format(extentParts)

        sql = u"""
        INSERT INTO USER_SDO_GEOM_METADATA (TABLE_NAME,
                                           COLUMN_NAME, DIMINFO,
                                           SRID)
        VALUES({0}, {1},
               {2},
               {3})
            """.format(self.quoteString(tablename),
                       self.quoteString(geom_column),
                       sqlExtent, str(srid))

        self._execute_and_commit(sql)

    def addGeometryColumn(self, table, geom_column='GEOM',
                          geom_type=None, srid=-1, dim=2):
        """Add a geometry column and update Oracle Spatial
        metadata.
        """

        schema, tablename = self.getSchemaTableName(table)
        # in Metadata view, geographic column is always in uppercase
        geom_column = geom_column.upper()

        # Add the column to the table
        sql = u"ALTER TABLE {0} ADD {1} SDO_GEOMETRY".format(
            self.quoteId(table), self.quoteId(geom_column))

        self._execute_and_commit(sql)

        # Then insert the metadata
        extent = []
        for i in range(dim):
            extent.extend([-100000, 10000])
        self.insertMetadata(table, geom_column,
                            [-100000, 100000, -10000, 10000],
                            srid, dim)

    def deleteGeometryColumn(self, table, geom_column):
        """Delete a geometric column."""
        return self.deleteTableColumn(table, geom_column)

    def addTableUniqueConstraint(self, table, column):
        """Add a unique constraint to a table."""
        sql = u"ALTER TABLE {0} ADD UNIQUE ({1})".format(
            self.quoteId(table), self.quoteId(column))
        self._execute_and_commit(sql)

    def deleteTableConstraint(self, table, constraint):
        """Delete constraint in a table."""
        sql = u"ALTER TABLE {0} DROP CONSTRAINT {1}".format(
            self.quoteId(table), self.quoteId(constraint))
        self._execute_and_commit(sql)

    def addTablePrimaryKey(self, table, column):
        """Add a primary key (with one column) to a table."""
        sql = u"ALTER TABLE {0} ADD PRIMARY KEY ({1})".format(
            self.quoteId(table), self.quoteId(column))
        self._execute_and_commit(sql)

    def createTableIndex(self, table, name, column):
        """Create index on one column using default options."""
        sql = u"CREATE INDEX {0} ON {1} ({2})".format(
            self.quoteId(name), self.quoteId(table),
            self.quoteId(column))
        self._execute_and_commit(sql)

    def rebuildTableIndex(self, table, name):
        """Rebuild a table index"""
        schema, tablename = self.getSchemaTableName(table)
        sql = u"ALTER INDEX {} REBUILD".format(self.quoteId((schema, name)))
        self._execute_and_commit(sql)

    def deleteTableIndex(self, table, name):
        """Delete an index on a table."""
        schema, tablename = self.getSchemaTableName(table)
        sql = u"DROP INDEX {}".format(self.quoteId((schema, name)))
        self._execute_and_commit(sql)

    def createSpatialIndex(self, table, geom_column='GEOM'):
        """Create a spatial index on a geometric column."""
        geom_column = geom_column.upper()
        schema, tablename = self.getSchemaTableName(table)
        idx_name = self.quoteId(u"sidx_{0}_{1}".format(tablename,
                                                       geom_column))
        sql = u"""
        CREATE INDEX {0}
        ON {1}({2})
        INDEXTYPE IS MDSYS.SPATIAL_INDEX
        PARAMETERS ('TABLESPACE={3} SDO_DML_BATCH_SIZE = 1')
        """.format(idx_name, self.quoteId(table),
                   self.quoteId(geom_column),
                   u"{}_INDEX".format(schema))
        self._execute_and_commit(sql)

    def deleteSpatialIndex(self, table, geom_column='GEOM'):
        """Delete a spatial index of a geometric column."""
        schema, tablename = self.getSchemaTableName(table)
        idx_name = self.quoteId(u"sidx_{0}_{1}".format(tablename,
                                                       geom_column))
        return self.deleteTableIndex(table, idx_name)

    def execution_error_types(self):
        return QtSqlDB.ExecError

    def connection_error_types(self):
        return QtSqlDB.ConnectionError

    def error_types(self):
        return self.connection_error_types(), self.execution_error_types()

    def _close_cursor(self, c):
        """new implementation of _close_cursor (because c.closed is
        psycopg2 specific and not DB API 2.0
        """
        try:
            if c:
                c.close()

        except self.error_types(), e:
            pass

        return

    # moved into the parent class: DbConnector._execute()
    # def _execute(self, cursor, sql):
    #     pass

    # moved into the parent class: DbConnector._execute_and_commit()
    # def _execute_and_commit(self, sql):
    #     pass

    # moved into the parent class: DbConnector._get_cursor()
    # def _get_cursor(self, name=None):
    #     pass

    # moved into the parent class: DbConnector._fetchall()
    # def _fetchall(self, c):
    #     pass

    # moved into the parent class: DbConnector._fetchone()
    # def _fetchone(self, c):
    #     pass

    # moved into the parent class: DbConnector._commit()
    # def _commit(self):
    #     pass

    # moved into the parent class: DbConnector._rollback()
    # def _rollback(self):
    #     pass

    # moved into the parent class: DbConnector._get_cursor_columns()
    # def _get_cursor_columns(self, c):
    #     pass

    def getQueryBuilderDictionary(self):
        from .sql_dictionary import getQueryBuilderDictionary

        return getQueryBuilderDictionary()

    def getSqlDictionary(self):
        """Returns the dictionary for SQL dialog."""
        from .sql_dictionary import getSqlDictionary
        sql_dict = getSqlDictionary()

        # get schemas, tables and field names
        items = []

        # First look into the cache if available
        if self.hasCache():
            sql = u"""
            SELECT DISTINCT tablename FROM "oracle_{0}"
            UNION
            SELECT DISTINCT ownername FROM "oracle_{0}"
            """.format(self.connName)
            if self.userTablesOnly:
                sql = u"""
                SELECT DISTINCT tablename
                FROM "oracle_{0}" WHERE ownername = '{1}'
                UNION
                SELECT DISTINCT ownername
                FROM "oracle_{0}" WHERE ownername = '{1}'
                """.format(self.connName, self.user, self.connName,
                           self.user)

            c = self.cache_connection.cursor()
            c.execute(sql)
            for row in c.fetchall():
                items.append(row[0])
            c.close()

        if self.hasCache():
            sql = u"""
            SELECT DISTINCT COLUMN_NAME FROM {}_TAB_COLUMNS
            """.format(u"USER" if self.userTablesOnly else
                       u"ALL")
        elif self.userTablesOnly:
            sql = u"""
            SELECT DISTINCT TABLE_NAME FROM USER_ALL_TABLES
            UNION
            SELECT USER FROM DUAL
            UNION
            SELECT DISTINCT COLUMN_NAME FROM USER_TAB_COLUMNS
            """
        else:
            sql = u"""
            SELECT TABLE_NAME FROM ALL_ALL_TABLES
            UNION
            SELECT DISTINCT OWNER FROM ALL_ALL_TABLES
            UNION
            SELECT DISTINCT COLUMN_NAME FROM ALL_TAB_COLUMNS
            """

        c = self._execute(None, sql)
        for row in self._fetchall(c):
            items.append(row[0])
        c.close()

        sql_dict["identifier"] = items
        return sql_dict

    def getQueryBuilderDictionary(self):
        from .sql_dictionary import getQueryBuilderDictionary

        return getQueryBuilderDictionary()
