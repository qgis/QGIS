# -*- coding: utf-8 -*-

"""
***************************************************************************
    spatialite.py
    ---------------------
    Date                 : November 2015
    Copyright            : (C) 2015 by René-Luc Dhont
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'René-Luc Dhont'
__date__ = 'November 2015'
__copyright__ = '(C) 2015, René-Luc Dhont'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.utils import spatialite_connect
import sqlite3 as sqlite
import re


class DbError(Exception):

    def __init__(self, message, query=None):
        # Save error. funny that the variables are in utf-8
        self.message = str(message)
        self.query = (str(query) if query is not None else None)

    def __str__(self):
        return 'MESSAGE: %s\nQUERY: %s' % (self.message, self.query)


class GeoDB:

    def __init__(self, uri=None):
        self.uri = uri
        self.dbname = uri.database()

        try:
            self.con = spatialite_connect(self.con_info())

        except (sqlite.InterfaceError, sqlite.OperationalError) as e:
            raise DbError(str(e))

        self.has_spatialite = self.check_spatialite()
        if not self.has_spatialite:
            self.has_spatialite = self.init_spatialite()

    def con_info(self):
        return str(self.dbname)

    def init_spatialite(self):
        # Get SpatiaLite version
        c = self.con.cursor()
        try:
            self._exec_sql(c, u'SELECT spatialite_version()')
            rep = c.fetchall()
            v = [int(x) if x.isdigit() else x for x in re.findall(r"\d+|[a-zA-Z]+", rep[0][0])]

            # Add SpatiaLite support
            if v >= [4, 1, 0]:
                # 4.1 and above
                sql = "SELECT initspatialmetadata(1)"
            else:
                # Under 4.1
                sql = "SELECT initspatialmetadata()"
            self._exec_sql_and_commit(sql)
        except:
            return False
        finally:
            self.con.close()

        try:
            self.con = spatialite_connect(self.con_info())

        except (sqlite.InterfaceError, sqlite.OperationalError) as e:
            raise DbError(str(e))

        return self.check_spatialite()

    def check_spatialite(self):
        try:
            c = self.con.cursor()
            self._exec_sql(c, u"SELECT CheckSpatialMetaData()")
            v = c.fetchone()[0]
            self.has_geometry_columns = v == 1 or v == 3
            self.has_spatialite4 = v == 3
        except Exception:
            self.has_geometry_columns = False
            self.has_spatialite4 = False

        self.has_geometry_columns_access = self.has_geometry_columns
        return self.has_geometry_columns

    def _exec_sql(self, cursor, sql):
        try:
            cursor.execute(sql)
        except (sqlite.Error, sqlite.ProgrammingError, sqlite.Warning, sqlite.InterfaceError, sqlite.OperationalError) as e:
            raise DbError(str(e), sql)

    def _exec_sql_and_commit(self, sql):
        """Tries to execute and commit some action, on error it rolls
        back the change.
        """

        try:
            c = self.con.cursor()
            self._exec_sql(c, sql)
            self.con.commit()
        except DbError:
            self.con.rollback()
            raise

    def create_spatial_index(self, table, geom_column='the_geom'):
        sql = u"SELECT CreateSpatialIndex(%s, %s)" % (self._quote(table), self._quote(geom_column))
        self._exec_sql_and_commit(sql)

    def _quote(self, identifier):
        """Quote identifier."""

        # quote identifier, and double the double-quotes
        return u"'%s'" % identifier.replace("'", "''")
