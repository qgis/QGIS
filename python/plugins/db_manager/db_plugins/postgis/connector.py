"""
/***************************************************************************
Name                 : DB Manager
Description          : Database manager plugin for QGIS
Date                 : May 23, 2011
copyright            : (C) 2011 by Giuseppe Sucameli
email                : brush.tyler@gmail.com

The content of this file is based on
- PG_Manager by Martin Dobias <wonder.sk@gmail.com> (GPLv2 license)
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

from functools import cmp_to_key

from qgis.PyQt.QtCore import (
    QRegularExpression,
    QFile,
    QVariant,
    QDateTime,
    QTime,
    QDate,
    Qt,
)
from qgis.core import (
    Qgis,
    QgsCoordinateReferenceSystem,
    QgsVectorLayer,
    QgsDataSourceUri,
    QgsProviderRegistry,
    QgsProviderConnectionException,
    QgsFeedback,
)

from ..connector import DBConnector
from ..plugin import DbError, Table

import os
import re


def classFactory():
    return PostGisDBConnector


class CursorAdapter:

    def _debug(self, msg):
        pass
        # print("XXX CursorAdapter[" + hex(id(self)) + "]: " + msg)

    def __init__(self, connection, sql=None, feedback=None):
        self._debug("Created with sql: " + str(sql))
        self.connection = connection
        self.sql = sql
        self.result = None
        self.cursor = 0
        self.feedback = feedback
        self.closed = False
        if self.sql is not None:
            self._execute()

    def _toStrResultSet(self, res):
        newres = []
        for rec in res:
            newrec = []
            for col in rec:
                if type(col) == type(QVariant(None)):  # noqa
                    if str(col) == "NULL":
                        col = None
                    else:
                        col = str(col)  # force to string
                if (
                    isinstance(col, QDateTime)
                    or isinstance(col, QDate)
                    or isinstance(col, QTime)
                ):
                    col = col.toString(Qt.DateFormat.ISODate)
                newrec.append(col)
            newres.append(newrec)
        return newres

    def _execute(self, sql=None):
        if (sql is None or self.sql == sql) and self.result is not None:
            return
        if sql is not None:
            self.sql = sql
        if self.sql is None:
            return
        self._debug("execute called with sql " + self.sql)
        try:
            result = self.connection.execSql(self.sql, feedback=self.feedback)
            self._description = []  # reset description
            self.result = self._toStrResultSet(result.rows())
            for c in result.columns():
                self._description.append(
                    [
                        c,  # name
                        "",  # type_code
                        -1,  # display_size
                        -1,  # internal_size
                        -1,  # precision
                        None,  # scale
                        True,  # null_ok
                    ]
                )

        except QgsProviderConnectionException as e:
            self._description = None
            raise DbError(e, self.sql)
        self._debug("execute returned " + str(len(self.result)) + " rows")
        self.cursor = 0

    @property
    def description(self):
        """Returns columns description, it should be already set by _execute"""

        if self._description is None:

            self._description = []

            if re.match("^SHOW", self.sql.strip().upper()):
                try:
                    count = len(self.connection.executeSql(self.sql)[0])
                except QgsProviderConnectionException:
                    count = 1
                for i in range(count):
                    self._description.append(
                        [
                            "",  # name
                            "",  # type_code
                            -1,  # display_size
                            -1,  # internal_size
                            -1,  # precision
                            None,  # scale
                            True,  # null_ok
                        ]
                    )
            else:
                uri = QgsDataSourceUri(self.connection.uri())

                # TODO: make this part provider-agnostic
                sql = (
                    self.sql
                    if self.sql.upper().find(" LIMIT ") >= 0
                    else self.sql + " LIMIT 1 "
                )
                uri.setTable(
                    "(SELECT row_number() OVER () AS __rid__, * FROM ("
                    + sql
                    + ") as foo)"
                )
                uri.setKeyColumn("__rid__")
                uri.setParam("checkPrimaryKeyUnicity", "0")
                # TODO: fetch provider name from connection (QgsAbstractConnectionProvider)
                # TODO: re-use the VectorLayer for fetching rows in batch mode
                vl = QgsVectorLayer(uri.uri(False), "dbmanager_cursor", "postgres")

                fields = vl.fields()

                for i in range(1, len(fields)):  # skip first field (__rid__)
                    f = fields[i]
                    self._description.append(
                        [
                            f.name(),  # name
                            f.type(),  # type_code
                            f.length(),  # display_size
                            f.length(),  # internal_size
                            f.precision(),  # precision
                            None,  # scale
                            True,  # null_ok
                        ]
                    )

            self._debug(
                "get_description returned " + str(len(self._description)) + " cols"
            )

        return self._description

    def fetchone(self):
        self._execute()
        if len(self.result) - self.cursor:
            res = self.result[self.cursor]
            self.cursor += 1
            return res
        return None

    def fetchmany(self, size):
        self._execute()
        if self.result is None:
            self._debug(
                "fetchmany: none result after _execute (self.sql is "
                + str(self.sql)
                + ", returning []"
            )
            return []
        leftover = len(self.result) - self.cursor
        self._debug(
            "fetchmany: cursor: "
            + str(self.cursor)
            + " leftover: "
            + str(leftover)
            + " requested: "
            + str(size)
        )
        if leftover < 1:
            return []
        if size > leftover:
            size = leftover
        stop = self.cursor + size
        res = self.result[self.cursor : stop]
        self.cursor = stop
        self._debug(
            "fetchmany: new cursor: "
            + str(self.cursor)
            + " reslen: "
            + str(len(self.result))
        )
        return res

    def fetchall(self):
        self._execute()
        res = self.result[self.cursor :]
        self.cursor = len(self.result)
        return res

    def scroll(self, pos, mode="relative"):
        self._execute()
        if pos < 0:
            self._debug("scroll pos is negative: " + str(pos))
        if mode == "relative":
            self.cursor = self.cursor + pos
        elif mode == "absolute":
            self.cursor = pos

    def close(self):
        self.result = None
        self.closed = True


class PostGisDBConnector(DBConnector):

    def __init__(self, uri, connection):
        """Creates a new PostgreSQL connector

        :param uri: data source URI
        :type uri: QgsDataSourceUri
        :param connection: the plugin parent instance
        :type connection: PostGisDBPlugin
        """
        DBConnector.__init__(self, uri)

        username = uri.username() or os.environ.get("PGUSER")

        # Do not get db and user names from the env if service is used
        if not uri.service():
            if username is None:
                username = os.environ.get("USER")
            self.dbname = uri.database() or os.environ.get("PGDATABASE") or username
            uri.setDatabase(self.dbname)

        # self.connName = connName
        # self.user = uri.username() or os.environ.get('USER')
        # self.passwd = uri.password()
        self.host = uri.host()

        md = QgsProviderRegistry.instance().providerMetadata(connection.providerName())
        # QgsAbstractDatabaseProviderConnection instance
        self.core_connection = md.findConnection(connection.connectionName())
        if self.core_connection is None:
            self.core_connection = md.createConnection(uri.uri(), {})

        c = self._execute(None, "SELECT current_user,current_database()")
        self.user, self.dbname = self._fetchone(c)
        self._close_cursor(c)

        self._checkSpatial()
        self._checkRaster()
        self._checkGeometryColumnsTable()
        self._checkRasterColumnsTable()

        self.feedback = None

    def _connectionInfo(self):
        return str(self.uri().connectionInfo(True))

    def _clearSslTempCertsIfAny(self, connectionInfo):
        # remove certs (if any) of the connectionInfo
        expandedUri = QgsDataSourceUri(connectionInfo)

        def removeCert(certFile):
            certFile = certFile.replace("'", "")
            file = QFile(certFile)
            # set permission to allow removing on Win.
            # On linux and Mac if file is set with QFile::>ReadUser
            # does not create problem removing certs
            if not file.setPermissions(QFile.Permission.WriteOwner):
                raise Exception(
                    f"Cannot change permissions on {file.fileName()}: error code: {file.error()}"
                )
            if not file.remove():
                raise Exception(
                    f"Cannot remove {file.fileName()}: error code: {file.error()}"
                )

        sslCertFile = expandedUri.param("sslcert")
        if sslCertFile:
            removeCert(sslCertFile)

        sslKeyFile = expandedUri.param("sslkey")
        if sslKeyFile:
            removeCert(sslKeyFile)

        sslCAFile = expandedUri.param("sslrootcert")
        if sslCAFile:
            removeCert(sslCAFile)

    def _checkSpatial(self):
        """check whether postgis_version is present in catalog"""
        c = self._execute(
            None, "SELECT COUNT(*) FROM pg_proc WHERE proname = 'postgis_version'"
        )
        self.has_spatial = self._fetchone(c)[0] > 0
        self._close_cursor(c)
        return self.has_spatial

    def _checkRaster(self):
        """check whether postgis_version is present in catalog"""
        c = self._execute(
            None,
            "SELECT COUNT(*) FROM pg_proc WHERE proname = 'postgis_raster_lib_version'",
        )
        self.has_raster = self._fetchone(c)[0] > 0
        self._close_cursor(c)
        return self.has_raster

    def _checkGeometryColumnsTable(self):
        c = self._execute(
            None,
            "SELECT relkind = 'v' OR relkind = 'm' FROM pg_class WHERE relname = 'geometry_columns' AND relkind IN ('v', 'r', 'm', 'p')",
        )
        res = self._fetchone(c)
        self._close_cursor(c)
        self.has_geometry_columns = res is not None and len(res) != 0

        if not self.has_geometry_columns:
            self.has_geometry_columns_access = self.is_geometry_columns_view = False
        else:
            self.is_geometry_columns_view = res[0]
            # find out whether has privileges to access geometry_columns table
            priv = self.getTablePrivileges("geometry_columns")
            self.has_geometry_columns_access = priv[0]
        return self.has_geometry_columns

    def _checkRasterColumnsTable(self):
        c = self._execute(
            None,
            "SELECT relkind = 'v' OR relkind = 'm' FROM pg_class WHERE relname = 'raster_columns' AND relkind IN ('v', 'r', 'm', 'p')",
        )
        res = self._fetchone(c)
        self._close_cursor(c)
        self.has_raster_columns = res is not None and len(res) != 0

        if not self.has_raster_columns:
            self.has_raster_columns_access = self.is_raster_columns_view = False
        else:
            self.is_raster_columns_view = res[0]
            # find out whether has privileges to access geometry_columns table
            self.has_raster_columns_access = self.getTablePrivileges("raster_columns")[
                0
            ]
        return self.has_raster_columns

    def cancel(self):
        if self.connection:
            self.connection.cancel()
        if self.core_connection:
            self.feedback.cancel()

    def getInfo(self):
        c = self._execute(None, "SELECT version()")
        res = self._fetchone(c)
        self._close_cursor(c)
        return res

    def getPsqlVersion(self):
        regex = r"^PostgreSQL\s([0-9]{1,2})"
        match = re.match(regex, self.getInfo()[0])
        if match:
            return int(match.group(1))
        raise DbError(f"Unknown PostgreSQL version: {self.getInfo()[0]}")

    def getSpatialInfo(self):
        """returns tuple about PostGIS support:
        - lib version
        - geos version
        - proj version
        - installed scripts version
        - released scripts version
        """
        if not self.has_spatial:
            return

        try:
            c = self._execute(
                None,
                "SELECT postgis_lib_version(), postgis_geos_version(), postgis_proj_version(), postgis_scripts_installed(), postgis_scripts_released()",
            )
        except DbError:
            return
        res = self._fetchone(c)
        self._close_cursor(c)
        return res

    def hasSpatialSupport(self):
        return self.has_spatial

    def hasRasterSupport(self):
        return self.has_raster

    def hasCustomQuerySupport(self):
        return Qgis.QGIS_VERSION[0:3] >= "1.5"

    def hasTableColumnEditingSupport(self):
        return True

    def hasCreateSpatialViewSupport(self):
        return True

    def fieldTypes(self):
        return [
            "integer",
            "bigint",
            "smallint",  # integers
            "serial",
            "bigserial",  # auto-incrementing ints
            "real",
            "double precision",
            "numeric",  # floats
            "varchar",
            "varchar(255)",
            "char(20)",
            "text",  # strings
            "date",
            "time",
            "timestamp",  # date/time
            "boolean",  # bool
        ]

    def getDatabasePrivileges(self):
        """db privileges: (can create schemas, can create temp. tables)"""
        sql = "SELECT has_database_privilege(current_database(), 'CREATE'), has_database_privilege(current_database(), 'TEMP')"
        c = self._execute(None, sql)
        res = self._fetchone(c)
        self._close_cursor(c)
        return res

    def getSchemaPrivileges(self, schema):
        """schema privileges: (can create new objects, can access objects in schema)"""
        schema = "current_schema()" if schema is None else self.quoteString(schema)
        sql = "SELECT has_schema_privilege({s}, 'CREATE'), has_schema_privilege({s}, 'USAGE')".format(
            s=schema
        )
        c = self._execute(None, sql)
        res = self._fetchone(c)
        self._close_cursor(c)
        return res

    def getTablePrivileges(self, table):
        """table privileges: (select, insert, update, delete)"""

        schema, tablename = self.getSchemaTableName(table)
        schema_priv = self.getSchemaPrivileges(schema)
        if not schema_priv[1]:
            return

        t = self.quoteId(table)
        sql = """SELECT has_table_privilege({t}, 'SELECT'), has_table_privilege({t}, 'INSERT'),
                                has_table_privilege({t}, 'UPDATE'), has_table_privilege({t}, 'DELETE')""".format(
            t=self.quoteString(t)
        )
        c = self._execute(None, sql)
        res = self._fetchone(c)
        self._close_cursor(c)
        return res

    def getSchemas(self):
        """get list of schemas in tuples: (oid, name, owner, perms)"""
        sql = "SELECT oid, nspname, pg_get_userbyid(nspowner), nspacl, pg_catalog.obj_description(oid) FROM pg_namespace WHERE nspname !~ '^pg_' AND nspname != 'information_schema' ORDER BY nspname"

        c = self._execute(None, sql)
        res = self._fetchall(c)
        self._close_cursor(c)
        return res

    def getTables(self, schema=None, add_sys_tables=False):
        """get list of tables"""
        tablenames = []
        items = []

        sys_tables = [
            "spatial_ref_sys",
            "geography_columns",
            "geometry_columns",
            "raster_columns",
            "raster_overviews",
        ]

        try:
            vectors = self.getVectorTables(schema)
            for tbl in vectors:
                if (
                    not add_sys_tables
                    and tbl[1] in sys_tables
                    and tbl[2] in ["", "public"]
                ):
                    continue
                tablenames.append((tbl[2], tbl[1]))
                items.append(tbl)
        except DbError:
            pass

        try:
            rasters = self.getRasterTables(schema)
            for tbl in rasters:
                if (
                    not add_sys_tables
                    and tbl[1] in sys_tables
                    and tbl[2] in ["", "public"]
                ):
                    continue
                tablenames.append((tbl[2], tbl[1]))
                items.append(tbl)
        except DbError:
            pass

        sys_tables = [
            "spatial_ref_sys",
            "geography_columns",
            "geometry_columns",
            "raster_columns",
            "raster_overviews",
        ]

        if schema:
            schema_where = " AND nspname = %s " % self.quoteString(schema)
        else:
            schema_where = (
                " AND (nspname != 'information_schema' AND nspname !~ 'pg_') "
            )

        # get all tables and views
        sql = (
            """SELECT
                                                cla.relname, nsp.nspname, cla.relkind,
                                                pg_get_userbyid(relowner), reltuples, relpages,
                                                pg_catalog.obj_description(cla.oid)
                                        FROM pg_class AS cla
                                        JOIN pg_namespace AS nsp ON nsp.oid = cla.relnamespace
                                        WHERE cla.relkind IN ('v', 'r', 'm', 'p') """
            + schema_where
            + """
                                        ORDER BY nsp.nspname, cla.relname"""
        )

        c = self._execute(None, sql)
        for tbl in self._fetchall(c):
            if tablenames.count((tbl[1], tbl[0])) <= 0:
                item = list(tbl)
                item.insert(0, Table.TableType)
                items.append(item)
        self._close_cursor(c)

        return sorted(items, key=cmp_to_key(lambda x, y: (x[1] > y[1]) - (x[1] < y[1])))

    def getVectorTables(self, schema=None):
        """get list of table with a geometry column
        it returns:
                name (table name)
                namespace (schema)
                type = 'view' (is a view?)
                owner
                tuples
                pages
                geometry_column:
                        f_geometry_column (or pg_attribute.attname, the geometry column name)
                        type (or pg_attribute.atttypid::regtype, the geometry column type name)
                        coord_dimension
                        srid
        """

        if not self.has_spatial:
            return []

        if schema:
            schema_where = " AND nspname = %s " % self.quoteString(schema)
        else:
            schema_where = (
                " AND (nspname != 'information_schema' AND nspname !~ 'pg_') "
            )

        geometry_column_from = ""
        geometry_fields_select = """att.attname,
                                                textin(regtypeout(att.atttypid::regtype)),
                                                NULL, NULL"""
        if self.has_geometry_columns and self.has_geometry_columns_access:
            geometry_column_from = """LEFT OUTER JOIN geometry_columns AS geo ON
                                                cla.relname = geo.f_table_name AND nsp.nspname = f_table_schema AND
                                                lower(att.attname) = lower(f_geometry_column)"""
            geometry_fields_select = """CASE WHEN geo.f_geometry_column IS NOT NULL THEN geo.f_geometry_column ELSE att.attname END,
                                                CASE WHEN geo.type IS NOT NULL THEN geo.type ELSE textin(regtypeout(att.atttypid::regtype)) END,
                                                geo.coord_dimension, geo.srid"""

        # discovery of all tables and whether they contain a geometry column
        sql = (
            """SELECT
                                                cla.relname, nsp.nspname, cla.relkind,
                                                pg_get_userbyid(relowner), cla.reltuples, cla.relpages,
                                                pg_catalog.obj_description(cla.oid),
                                                """
            + geometry_fields_select
            + """

                                        FROM pg_class AS cla
                                        JOIN pg_namespace AS nsp ON
                                                nsp.oid = cla.relnamespace

                                        JOIN pg_attribute AS att ON
                                                att.attrelid = cla.oid AND
                                                att.atttypid = 'geometry'::regtype OR
                                                att.atttypid IN (SELECT oid FROM pg_type WHERE typbasetype='geometry'::regtype )

                                        """
            + geometry_column_from
            + """

                                        WHERE cla.relkind IN ('v', 'r', 'm', 'p') """
            + schema_where
            + """
                                        ORDER BY nsp.nspname, cla.relname, att.attname"""
        )

        items = []

        c = self._execute(None, sql)
        for i, tbl in enumerate(self._fetchall(c)):
            item = list(tbl)
            item.insert(0, Table.VectorType)
            items.append(item)
        self._close_cursor(c)

        return items

    def getRasterTables(self, schema=None):
        """get list of table with a raster column
        it returns:
                name (table name)
                namespace (schema)
                type = 'view' (is a view?)
                owner
                tuples
                pages
                raster_column:
                        r_raster_column (or pg_attribute.attname, the raster column name)
                        pixel type
                        block size
                        internal or external
                        srid
        """

        if not self.has_spatial:
            return []
        if not self.has_raster:
            return []

        if schema:
            schema_where = " AND nspname = %s " % self.quoteString(schema)
        else:
            schema_where = (
                " AND (nspname != 'information_schema' AND nspname !~ 'pg_') "
            )

        raster_column_from = ""
        raster_fields_select = """att.attname, NULL, NULL, NULL, NULL, NULL"""
        if self.has_raster_columns and self.has_raster_columns_access:
            raster_column_from = """LEFT OUTER JOIN raster_columns AS rast ON
                                                cla.relname = rast.r_table_name AND nsp.nspname = r_table_schema AND
                                                lower(att.attname) = lower(r_raster_column)"""
            raster_fields_select = """CASE WHEN rast.r_raster_column IS NOT NULL THEN rast.r_raster_column ELSE att.attname END,
                                                rast.pixel_types,
                                                rast.scale_x,
                                                rast.scale_y,
                                                rast.out_db,
                                                rast.srid"""

        # discovery of all tables and whether they contain a raster column
        sql = (
            """SELECT
                                                cla.relname, nsp.nspname, cla.relkind,
                                                pg_get_userbyid(relowner), cla.reltuples, cla.relpages,
                                                pg_catalog.obj_description(cla.oid),
                                                """
            + raster_fields_select
            + """

                                        FROM pg_class AS cla
                                        JOIN pg_namespace AS nsp ON
                                                nsp.oid = cla.relnamespace

                                        JOIN pg_attribute AS att ON
                                                att.attrelid = cla.oid AND
                                                att.atttypid = 'raster'::regtype OR
                                                att.atttypid IN (SELECT oid FROM pg_type WHERE typbasetype='raster'::regtype )

                                        """
            + raster_column_from
            + """

                                        WHERE cla.relkind IN ('v', 'r', 'm', 'p') """
            + schema_where
            + """
                                        ORDER BY nsp.nspname, cla.relname, att.attname"""
        )

        items = []

        c = self._execute(None, sql)
        for i, tbl in enumerate(self._fetchall(c)):
            item = list(tbl)
            item.insert(0, Table.RasterType)
            items.append(item)
        self._close_cursor(c)

        return items

    def getTableRowCount(self, table):
        c = self._execute(None, "SELECT COUNT(*) FROM %s" % self.quoteId(table))
        res = self._fetchone(c)[0]
        self._close_cursor(c)
        return res

    def getTableFields(self, table):
        """return list of columns in table"""

        schema, tablename = self.getSchemaTableName(table)
        schema_where = (
            " AND nspname=%s " % self.quoteString(schema) if schema is not None else ""
        )

        version_number = self.getPsqlVersion()
        ad_col_name = "adsrc" if version_number < 12 else "adbin"

        sql = """SELECT a.attnum AS ordinal_position,
                                a.attname AS column_name,
                                t.typname AS data_type,
                                a.attlen AS char_max_len,
                                a.atttypmod AS modifier,
                                a.attnotnull AS notnull,
                                a.atthasdef AS hasdefault,
                                adef.{} AS default_value,
                                pg_catalog.format_type(a.atttypid,a.atttypmod) AS formatted_type
                        FROM pg_class c
                        JOIN pg_attribute a ON a.attrelid = c.oid
                        JOIN pg_type t ON a.atttypid = t.oid
                        JOIN pg_namespace nsp ON c.relnamespace = nsp.oid
                        LEFT JOIN pg_attrdef adef ON adef.adrelid = a.attrelid AND adef.adnum = a.attnum
                        WHERE
                          a.attnum > 0 AND c.relname={} {}
                        ORDER BY a.attnum""".format(
            ad_col_name, self.quoteString(tablename), schema_where
        )

        c = self._execute(None, sql)
        res = self._fetchall(c)
        self._close_cursor(c)
        return res

    def getTableIndexes(self, table):
        """get info about table's indexes. ignore primary key constraint index, they get listed in constraints"""
        schema, tablename = self.getSchemaTableName(table)
        schema_where = (
            " AND nspname=%s " % self.quoteString(schema) if schema is not None else ""
        )

        sql = """SELECT idxcls.relname, indkey, indisunique = 't'
                                                FROM pg_index JOIN pg_class ON pg_index.indrelid=pg_class.oid
                                                JOIN pg_class AS idxcls ON pg_index.indexrelid=idxcls.oid
                                                JOIN pg_namespace nsp ON pg_class.relnamespace = nsp.oid
                                                        WHERE pg_class.relname={} {}
                                                        AND indisprimary != 't' """.format(
            self.quoteString(tablename), schema_where
        )
        c = self._execute(None, sql)
        res = self._fetchall(c)
        self._close_cursor(c)
        return res

    def getTableConstraints(self, table):

        schema, tablename = self.getSchemaTableName(table)
        schema_where = (
            " AND nspname=%s " % self.quoteString(schema) if schema is not None else ""
        )

        version_number = self.getPsqlVersion()
        con_col_name = "consrc" if version_number < 12 else "conbin"

        # In the query below, we exclude rows where pg_constraint.contype whose values are equal to 't'
        # because 't' describes a CONSTRAINT TRIGGER, which is not really a constraint in the traditional
        # sense, but a special type of trigger, and an extension to the SQL standard.
        sql = """SELECT c.conname, c.contype, c.condeferrable, c.condeferred, array_to_string(c.conkey, ' '), c.{},
                         t2.relname, c.confupdtype, c.confdeltype, c.confmatchtype, array_to_string(c.confkey, ' ') FROM pg_constraint c
                  LEFT JOIN pg_class t ON c.conrelid = t.oid
                        LEFT JOIN pg_class t2 ON c.confrelid = t2.oid
                        JOIN pg_namespace nsp ON t.relnamespace = nsp.oid
                        WHERE c.contype <> 't' AND t.relname = {} {} """.format(
            con_col_name, self.quoteString(tablename), schema_where
        )

        c = self._execute(None, sql)
        res = self._fetchall(c)
        self._close_cursor(c)
        return res

    def getTableTriggers(self, table):

        schema, tablename = self.getSchemaTableName(table)
        schema_where = (
            " AND nspname=%s " % self.quoteString(schema) if schema is not None else ""
        )

        sql = """SELECT tgname, proname, tgtype, tgenabled NOT IN ('f', 'D')  FROM pg_trigger trig
                          LEFT JOIN pg_class t ON trig.tgrelid = t.oid
                                                        LEFT JOIN pg_proc p ON trig.tgfoid = p.oid
                                                        JOIN pg_namespace nsp ON t.relnamespace = nsp.oid
                                                        WHERE t.relname = {} {} """.format(
            self.quoteString(tablename), schema_where
        )

        c = self._execute(None, sql)
        res = self._fetchall(c)
        self._close_cursor(c)
        return res

    def enableAllTableTriggers(self, enable, table):
        """enable or disable all triggers on table"""
        self.enableTableTrigger(None, enable, table)

    def enableTableTrigger(self, trigger, enable, table):
        """enable or disable one trigger on table"""
        trigger = self.quoteId(trigger) if trigger is not None else "ALL"
        sql = "ALTER TABLE {} {} TRIGGER {}".format(
            self.quoteId(table), "ENABLE" if enable else "DISABLE", trigger
        )
        self._execute_and_commit(sql)

    def deleteTableTrigger(self, trigger, table):
        """Deletes trigger on table"""
        sql = f"DROP TRIGGER {self.quoteId(trigger)} ON {self.quoteId(table)}"
        self._execute_and_commit(sql)

    def getTableRules(self, table):

        schema, tablename = self.getSchemaTableName(table)
        schema_where = (
            " AND schemaname=%s " % self.quoteString(schema)
            if schema is not None
            else ""
        )

        sql = """SELECT rulename, definition FROM pg_rules
                                        WHERE tablename={} {} """.format(
            self.quoteString(tablename), schema_where
        )

        c = self._execute(None, sql)
        res = self._fetchall(c)
        self._close_cursor(c)
        return res

    def deleteTableRule(self, rule, table):
        """Deletes rule on table"""
        sql = f"DROP RULE {self.quoteId(rule)} ON {self.quoteId(table)}"
        self._execute_and_commit(sql)

    def getTableExtent(self, table, geom):
        """find out table extent"""
        subquery = "SELECT st_extent({}) AS extent FROM {}".format(
            self.quoteId(geom), self.quoteId(table)
        )
        sql = (
            "SELECT st_xmin(extent), st_ymin(extent), st_xmax(extent), st_ymax(extent) FROM (%s) AS subquery"
            % subquery
        )

        c = self._execute(None, sql)
        res = self._fetchone(c)
        self._close_cursor(c)
        return res

    def getTableEstimatedExtent(self, table, geom):
        """find out estimated extent (from the statistics)"""
        if self.isRasterTable(table):
            return

        schema, tablename = self.getSchemaTableName(table)
        schema_part = "%s," % self.quoteString(schema) if schema is not None else ""

        pgis_versions = self.getSpatialInfo()[0].split(".")
        pgis_major_version = int(pgis_versions[0])
        pgis_minor_version = int(pgis_versions[1])
        pgis_old = False
        if pgis_major_version < 2:
            pgis_old = True
        elif pgis_major_version == 2 and pgis_minor_version < 1:
            pgis_old = True
        subquery = "SELECT {}({}{},{}) AS extent".format(
            "st_estimated_extent" if pgis_old else "st_estimatedextent",
            schema_part,
            self.quoteString(tablename),
            self.quoteString(geom),
        )
        sql = (
            """SELECT st_xmin(extent), st_ymin(extent), st_xmax(extent), st_ymax(extent) FROM (%s) AS subquery """
            % subquery
        )

        try:
            c = self._execute(None, sql)
        except DbError:  # No statistics for the current table
            return
        res = self._fetchone(c)
        self._close_cursor(c)
        return res

    def getViewDefinition(self, view):
        """returns definition of the view"""

        schema, tablename = self.getSchemaTableName(view)
        schema_where = (
            " AND nspname=%s " % self.quoteString(schema) if schema is not None else ""
        )

        sql = """SELECT pg_get_viewdef(c.oid) FROM pg_class c
                                                JOIN pg_namespace nsp ON c.relnamespace = nsp.oid
                        WHERE relname={} {} AND (relkind='v' OR relkind='m') """.format(
            self.quoteString(tablename), schema_where
        )

        c = self._execute(None, sql)
        res = self._fetchone(c)
        self._close_cursor(c)
        return res[0] if res is not None else None

    def getCrs(self, srid):
        if not self.has_spatial:
            return QgsCoordinateReferenceSystem()

        try:
            c = self._execute(
                None, "SELECT proj4text FROM spatial_ref_sys WHERE srid = '%d'" % srid
            )
        except DbError:
            return QgsCoordinateReferenceSystem()
        res = self._fetchone(c)
        self._close_cursor(c)
        if res is None:
            return QgsCoordinateReferenceSystem()

        proj4text = res[0]
        crs = QgsCoordinateReferenceSystem.fromProj(proj4text)
        return crs

    def getSpatialRefInfo(self, srid):
        if not self.has_spatial:
            return

        try:
            c = self._execute(
                None, "SELECT srtext FROM spatial_ref_sys WHERE srid = '%d'" % srid
            )
        except DbError:
            return
        sr = self._fetchone(c)
        self._close_cursor(c)
        if sr is None:
            return

        srtext = sr[0]
        # try to extract just SR name (should be quoted in double quotes)
        regex = QRegularExpression('"([^"]+)"')
        match = regex.match(srtext)
        if match.hasMatch():
            srtext = match.captured(1)
        return srtext

    def isVectorTable(self, table):
        if self.has_geometry_columns and self.has_geometry_columns_access:
            schema, tablename = self.getSchemaTableName(table)
            sql = "SELECT count(*) FROM geometry_columns WHERE f_table_schema = {} AND f_table_name = {}".format(
                self.quoteString(schema), self.quoteString(tablename)
            )

            c = self._execute(None, sql)
            res = self._fetchone(c)
            self._close_cursor(c)
            return res is not None and res[0] > 0

        return False

    def isRasterTable(self, table):
        if self.has_raster_columns and self.has_raster_columns_access:
            schema, tablename = self.getSchemaTableName(table)
            sql = "SELECT count(*) FROM raster_columns WHERE r_table_schema = {} AND r_table_name = {}".format(
                self.quoteString(schema), self.quoteString(tablename)
            )

            c = self._execute(None, sql)
            res = self._fetchone(c)
            self._close_cursor(c)
            return res is not None and res[0] > 0

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
        """Deletes table and its reference in either geometry_columns or raster_columns"""
        schema, tablename = self.getSchemaTableName(table)
        schema_part = "%s, " % self.quoteString(schema) if schema is not None else ""
        if self.isVectorTable(table):
            sql = "SELECT DropGeometryTable({}{})".format(
                schema_part, self.quoteString(tablename)
            )
        elif self.isRasterTable(table):
            # Fix #8521: delete raster table and references from raster_columns table
            sql = "DROP TABLE %s" % self.quoteId(table)
        else:
            sql = "DROP TABLE %s" % self.quoteId(table)
        self._execute_and_commit(sql)

    def emptyTable(self, table):
        """Deletes all rows from table"""
        sql = "TRUNCATE %s" % self.quoteId(table)
        self._execute_and_commit(sql)

    def renameTable(self, table, new_table):
        """Renames a table in database"""
        schema, tablename = self.getSchemaTableName(table)
        if new_table == tablename:
            return

        sql = "ALTER TABLE {} RENAME  TO {}".format(
            self.quoteId(table), self.quoteId(new_table)
        )
        self._executeSql(sql)

        # update geometry_columns if PostGIS is enabled
        if self.has_geometry_columns and not self.is_geometry_columns_view:
            schema_where = (
                " AND f_table_schema=%s " % self.quoteString(schema)
                if schema is not None
                else ""
            )
            sql = "UPDATE geometry_columns SET f_table_name={} WHERE f_table_name={} {}".format(
                self.quoteString(new_table), self.quoteString(tablename), schema_where
            )
            self._executeSql(sql)

    def renameSchema(self, schema, new_schema):
        try:
            self.core_connection.renameSchema(schema, new_schema)
            return True
        except QgsProviderConnectionException:
            return False

    def commentTable(self, schema, tablename, comment=None):
        if comment is None:
            self._execute(None, f'COMMENT ON TABLE "{schema}"."{tablename}" IS NULL;')
        else:
            self._execute(
                None,
                f'COMMENT ON TABLE "{schema}"."{tablename}" IS $escape${comment}$escape$;',
            )

    def getComment(self, tablename, field):
        """Returns the comment for a field"""
        # SQL Query checking if a comment exists for the field
        sql_cpt = "Select count(*) from pg_description pd, pg_class pc, pg_attribute pa where relname = '{}' and attname = '{}' and pa.attrelid = pc.oid and pd.objoid = pc.oid and pd.objsubid = pa.attnum".format(
            tablename, field
        )
        # SQL Query that return the comment of the field
        sql = "Select pd.description from pg_description pd, pg_class pc, pg_attribute pa where relname = '{}' and attname = '{}' and pa.attrelid = pc.oid and pd.objoid = pc.oid and pd.objsubid = pa.attnum".format(
            tablename, field
        )
        c = self._execute(None, sql_cpt)  # Execute Check query
        res = self._fetchone(c)[0]  # Store result
        if res == 1:
            # When a comment exists
            c = self._execute(None, sql)  # Execute query
            res = self._fetchone(c)[0]  # Store result
            self._close_cursor(c)  # Close cursor
            return res  # Return comment
        else:
            return ""

    def moveTableToSchema(self, table, new_schema):
        schema, tablename = self.getSchemaTableName(table)
        if new_schema == schema:
            return

        c = self._get_cursor()

        sql = "ALTER TABLE {} SET SCHEMA {}".format(
            self.quoteId(table), self.quoteId(new_schema)
        )
        self._execute(c, sql)

        # update geometry_columns if PostGIS is enabled
        if self.has_geometry_columns and not self.is_geometry_columns_view:
            schema, tablename = self.getSchemaTableName(table)
            schema_where = (
                " AND f_table_schema=%s " % self.quoteString(schema)
                if schema is not None
                else ""
            )
            sql = "UPDATE geometry_columns SET f_table_schema={} WHERE f_table_name={} {}".format(
                self.quoteString(new_schema), self.quoteString(tablename), schema_where
            )
            self._execute(c, sql)

        self._commit()

    def moveTable(self, table, new_table, new_schema=None):
        schema, tablename = self.getSchemaTableName(table)
        if new_schema == schema and new_table == tablename:
            return
        if new_schema == schema:
            return self.renameTable(table, new_table)
        if new_table == table:
            return self.moveTableToSchema(table, new_schema)

        c = self._get_cursor()
        t = "__new_table__"

        sql = "ALTER TABLE {} RENAME  TO {}".format(
            self.quoteId(table), self.quoteId(t)
        )
        self._execute(c, sql)

        sql = "ALTER TABLE {} SET SCHEMA {}".format(
            self.quoteId((schema, t)), self.quoteId(new_schema)
        )
        self._execute(c, sql)

        sql = "ALTER TABLE {} RENAME  TO {}".format(
            self.quoteId((new_schema, t)), self.quoteId(table)
        )
        self._execute(c, sql)

        # update geometry_columns if PostGIS is enabled
        if self.has_geometry_columns and not self.is_geometry_columns_view:
            schema, tablename = self.getSchemaTableName(table)
            schema_where = (
                " f_table_schema=%s AND " % self.quoteString(schema)
                if schema is not None
                else ""
            )
            schema_part = (
                " f_table_schema=%s, " % self.quoteString(new_schema)
                if schema is not None
                else ""
            )
            sql = "UPDATE geometry_columns SET {} f_table_name={} WHERE {} f_table_name={}".format(
                schema_part,
                self.quoteString(new_table),
                schema_where,
                self.quoteString(tablename),
            )
            self._execute(c, sql)

        self._commit()

    def createView(self, view, query):
        view_name_parts = view.split(".")

        if len(view_name_parts) > 2:
            # Raise an error when more than one period is used.
            raise DbError(
                "Invalid view name: Please use the format 'schema.viewname', or enter only the viewname for the public schema."
            )
        elif len(view_name_parts) == 2:  # To allow view creation into specified schema
            schema, view_name = view_name_parts
            sql = "CREATE VIEW {} AS {}".format(
                self.quoteId([schema, view_name]), query
            )
        else:  # No specific schema specified
            sql = f"CREATE VIEW {self.quoteId(view)} AS {query}"
        self._execute_and_commit(sql)

    def createSpatialView(self, view, query):
        self.createView(view, query)

    def deleteView(self, view, isMaterialized=False):
        sql = "DROP {} VIEW {}".format(
            "MATERIALIZED" if isMaterialized else "", self.quoteId(view)
        )
        self._execute_and_commit(sql)

    def renameView(self, view, new_name):
        """Renames view in database"""
        self.renameTable(view, new_name)

    def createSchema(self, schema):
        """Creates a new empty schema in database"""
        sql = "CREATE SCHEMA %s" % self.quoteId(schema)
        self._execute_and_commit(sql)

    def deleteSchema(self, schema):
        """Drops (empty) schema from database"""
        sql = "DROP SCHEMA %s" % self.quoteId(schema)
        self._execute_and_commit(sql)

    def renamesSchema(self, schema, new_schema):
        """Renames a schema in database"""
        sql = "ALTER SCHEMA {} RENAME  TO {}".format(
            self.quoteId(schema), self.quoteId(new_schema)
        )
        self._execute_and_commit(sql)

    def runVacuum(self):
        """Runs vacuum on the db"""
        self._execute_and_commit("VACUUM")

    def runVacuumAnalyze(self, table):
        """Runs vacuum analyze on a table"""
        sql = "VACUUM ANALYZE %s" % self.quoteId(table)
        self._execute(None, sql)
        self._commit()

    def runRefreshMaterializedView(self, table):
        """Runs refresh materialized view on a table"""
        sql = "REFRESH MATERIALIZED VIEW %s" % self.quoteId(table)
        self._execute(None, sql)
        self._commit()

    def addTableColumn(self, table, field_def):
        """Adds a column to table"""
        sql = f"ALTER TABLE {self.quoteId(table)} ADD {field_def}"
        self._execute_and_commit(sql)

    def deleteTableColumn(self, table, column):
        """Deletes column from a table"""
        if self.isGeometryColumn(table, column):
            # use PostGIS function to delete geometry column correctly
            schema, tablename = self.getSchemaTableName(table)
            schema_part = "%s, " % self.quoteString(schema) if schema else ""
            sql = "SELECT DropGeometryColumn({}{}, {})".format(
                schema_part, self.quoteString(tablename), self.quoteString(column)
            )
        else:
            sql = "ALTER TABLE {} DROP {}".format(
                self.quoteId(table), self.quoteId(column)
            )
        self._execute_and_commit(sql)

    def updateTableColumn(
        self,
        table,
        column,
        new_name=None,
        data_type=None,
        not_null=None,
        default=None,
        comment=None,
        test=None,
    ):
        if (
            new_name is None
            and data_type is None
            and not_null is None
            and default is None
            and comment is None
        ):
            return

        c = self._get_cursor()

        # update column definition
        col_actions = []
        if data_type is not None:
            col_actions.append("TYPE %s" % data_type)
        if not_null is not None:
            col_actions.append("SET NOT NULL" if not_null else "DROP NOT NULL")
        if default is not None:
            if default and default != "":
                col_actions.append("SET DEFAULT %s" % default)
            else:
                col_actions.append("DROP DEFAULT")
        if len(col_actions) > 0:
            sql = "ALTER TABLE %s" % self.quoteId(table)
            alter_col_str = "ALTER %s" % self.quoteId(column)
            for a in col_actions:
                sql += f" {alter_col_str} {a},"
            self._execute(c, sql[:-1])

        # Renames the column
        if new_name is not None and new_name != column:
            sql = "ALTER TABLE {} RENAME  {} TO {}".format(
                self.quoteId(table), self.quoteId(column), self.quoteId(new_name)
            )
            self._execute(c, sql)

            # update geometry_columns if PostGIS is enabled
            if self.has_geometry_columns and not self.is_geometry_columns_view:
                schema, tablename = self.getSchemaTableName(table)
                schema_where = (
                    " f_table_schema=%s AND " % self.quoteString(schema)
                    if schema is not None
                    else ""
                )
                sql = "UPDATE geometry_columns SET f_geometry_column={} WHERE {} f_table_name={} AND f_geometry_column={}".format(
                    self.quoteString(new_name),
                    schema_where,
                    self.quoteString(tablename),
                    self.quoteString(column),
                )
                self._execute(c, sql)

        # comment the column
        if comment is not None:
            schema, tablename = self.getSchemaTableName(table)
            column_name = (
                new_name if new_name is not None and new_name != column else column
            )
            sql = "COMMENT ON COLUMN {}.{}.{} IS '{}'".format(
                schema, tablename, column_name, comment
            )
            self._execute(c, sql)

        self._commit()

    def renamesTableColumn(self, table, column, new_name):
        """Renames column in a table"""
        return self.updateTableColumn(table, column, new_name)

    def setTableColumnType(self, table, column, data_type):
        """Changes column type"""
        return self.updateTableColumn(table, column, None, data_type)

    def setTableColumnNull(self, table, column, is_null):
        """Changes whether column can contain null values"""
        return self.updateTableColumn(table, column, None, None, not is_null)

    def setTableColumnDefault(self, table, column, default):
        """Changes column's default value.
        If default=None or an empty string drop default value"""
        return self.updateTableColumn(table, column, None, None, None, default)

    def isGeometryColumn(self, table, column):

        schema, tablename = self.getSchemaTableName(table)
        schema_where = (
            " f_table_schema=%s AND " % self.quoteString(schema)
            if schema is not None
            else ""
        )

        sql = "SELECT count(*) > 0 FROM geometry_columns WHERE {} f_table_name={} AND f_geometry_column={}".format(
            schema_where, self.quoteString(tablename), self.quoteString(column)
        )

        c = self._execute(None, sql)
        res = self._fetchone(c)[0] == "t"
        self._close_cursor(c)
        return res

    def addGeometryColumn(
        self, table, geom_column="geom", geom_type="POINT", srid=-1, dim=2
    ):
        schema, tablename = self.getSchemaTableName(table)
        schema_part = "%s, " % self.quoteString(schema) if schema else ""

        sql = "SELECT AddGeometryColumn(%s%s, %s, %d, %s, %d)" % (
            schema_part,
            self.quoteString(tablename),
            self.quoteString(geom_column),
            srid,
            self.quoteString(geom_type),
            dim,
        )
        self._execute_and_commit(sql)

    def deleteGeometryColumn(self, table, geom_column):
        return self.deleteTableColumn(table, geom_column)

    def addTableUniqueConstraint(self, table, column):
        """Adds a unique constraint to a table"""
        sql = "ALTER TABLE {} ADD UNIQUE ({})".format(
            self.quoteId(table), self.quoteId(column)
        )
        self._execute_and_commit(sql)

    def deleteTableConstraint(self, table, constraint):
        """Deletes constraint in a table"""
        sql = "ALTER TABLE {} DROP CONSTRAINT {}".format(
            self.quoteId(table), self.quoteId(constraint)
        )
        self._execute_and_commit(sql)

    def addTablePrimaryKey(self, table, column):
        """Adds a primery key (with one column) to a table"""
        sql = "ALTER TABLE {} ADD PRIMARY KEY ({})".format(
            self.quoteId(table), self.quoteId(column)
        )
        self._execute_and_commit(sql)

    def createTableIndex(self, table, name, column):
        """Creates index on one column using default options"""
        sql = "CREATE INDEX {} ON {} ({})".format(
            self.quoteId(name), self.quoteId(table), self.quoteId(column)
        )
        self._execute_and_commit(sql)

    def deleteTableIndex(self, table, name):
        schema, tablename = self.getSchemaTableName(table)
        sql = "DROP INDEX %s" % self.quoteId((schema, name))
        self._execute_and_commit(sql)

    def createSpatialIndex(self, table, geom_column="geom"):
        schema, tablename = self.getSchemaTableName(table)
        idx_name = self.quoteId(f"sidx_{tablename}_{geom_column}")
        sql = "CREATE INDEX {} ON {} USING GIST({})".format(
            idx_name, self.quoteId(table), self.quoteId(geom_column)
        )
        self._execute_and_commit(sql)

    def deleteSpatialIndex(self, table, geom_column="geom"):
        schema, tablename = self.getSchemaTableName(table)
        idx_name = self.quoteId(f"sidx_{tablename}_{geom_column}")
        return self.deleteTableIndex(table, idx_name)

    def _execute(self, cursor, sql):
        if cursor is not None:
            cursor._execute(sql)
            return cursor
        self.feedback = QgsFeedback()
        return CursorAdapter(self.core_connection, sql, feedback=self.feedback)

    def _executeSql(self, sql):
        return self.core_connection.executeSql(sql)

    def _get_cursor(self, name=None):
        # if name is not None:
        #   print("XXX _get_cursor called with a Name: " + name)
        return CursorAdapter(self.core_connection, name)

    def _commit(self):
        pass

    # moved into the parent class: DbConnector._rollback()
    def _rollback(self):
        pass

    # moved into the parent class: DbConnector._get_cursor_columns()
    # def _get_cursor_columns(self, c):
    #       pass

    def getSqlDictionary(self):
        from .sql_dictionary import getSqlDictionary

        sql_dict = getSqlDictionary()

        # get schemas, tables and field names
        sql = """SELECT nspname FROM pg_namespace WHERE nspname !~ '^pg_' AND nspname != 'information_schema'
UNION SELECT relname FROM pg_class WHERE relkind IN ('v', 'r', 'm', 'p')
UNION SELECT attname FROM pg_attribute WHERE attnum > 0"""
        c = self._execute(None, sql)
        items = [row[0] for row in self._fetchall(c)]
        self._close_cursor(c)

        sql_dict["identifier"] = items
        return sql_dict

    def getQueryBuilderDictionary(self):
        from .sql_dictionary import getQueryBuilderDictionary

        return getQueryBuilderDictionary()
