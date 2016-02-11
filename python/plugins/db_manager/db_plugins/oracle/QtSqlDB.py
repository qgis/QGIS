# -*- coding: utf-8 -*-

"""
/***************************************************************************
Name                 : QtSqlDB
Description          : DB API 2.0 interface for QtSql
Date                 : June 6, 2015
Copyright            : (C) 2015 by Jürgen E. Fischer
email                : jef at norbit dot de

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

from PyQt4.QtCore import QVariant, QDate, QTime, QDateTime, QByteArray
from PyQt4.QtSql import QSqlDatabase, QSqlQuery, QSqlField

paramstyle = "qmark"
threadsafety = 1
apilevel = "2.0"

import time
import datetime


def Date(year, month, day):
    return datetime.date(year, month, day)


def Time(hour, minute, second):
    return datetime.time(hour, minute, second)


def Timestamp(year, month, day, hour, minute, second):
    return datetime.datetime(year, month, day, hour, minute, second)


def DateFromTicks(ticks):
    return Date(*time.localtime(ticks)[:3])


def TimeFromTicks(ticks):
    return Time(*time.localtime(ticks)[3:6])


def TimestampFromTicks(ticks):
    return Timestamp(*time.localtime(ticks)[:6])


class ConnectionError(Exception):

    def __init__(self, *args, **kwargs):
        super(Exception, self).__init__(*args, **kwargs)


class ExecError(Exception):

    def __init__(self, *args, **kwargs):
        super(Exception, self).__init__(*args, **kwargs)


class QtSqlDBCursor:

    def __init__(self, conn):
        self.qry = QSqlQuery(conn)
        self.description = None
        self.rowcount = -1
        self.arraysize = 1

    def close(self):
        self.qry.finish()

    def execute(self, operation, parameters=[]):
        if len(parameters) == 0:
            if not self.qry.exec_(operation):
                raise ExecError(self.qry.lastError().databaseText())
        else:
            if not self.qry.prepare(operation):
                raise ExecError(self.qry.lastError().databaseText())

            for i in range(len(parameters)):
                self.qry.bindValue(i, parameters[i])

            if not self.qry.exec_():
                raise ExecError(self.qry.lastError().databaseText())

        self.rowcount = self.qry.size()
        self.description = []
        for c in range(self.qry.record().count()):
            f = self.qry.record().field(c)

            if f.type() == QVariant.Date:
                t = Date
            elif f.type() == QVariant.Time:
                t = Time
            elif f.type() == QVariant.DateTime:
                t = Timestamp
            elif f.type() == QVariant.Double:
                t = float
            elif f.type() == QVariant.Int:
                t = int
            elif f.type() == QVariant.String:
                t = unicode
            elif f.type() == QVariant.ByteArray:
                t = unicode
            else:
                continue

            self.description.append([
                f.name(),                                 # name
                t,                                        # type_code
                f.length(),                               # display_size
                f.length(),                               # internal_size
                f.precision(),                            # precision
                None,                                     # scale
                f.requiredStatus() != QSqlField.Required  # null_ok
            ])

    def executemany(self, operation, seq_of_parameters):
        if len(seq_of_parameters) == 0:
            return

        if not self.qry.prepare(operation):
            raise ExecError(self.qry.lastError().databaseText())

        for r in seq_of_parameters:
            for i in range(len(r)):
                self.qry.bindValue(i, r[i])

            if not self.qry.exec_():
                raise ExecError(self.qry.lastError().databaseText())

    def scroll(self, row):
        return self.qry.seek(row)

    def fetchone(self):
        if not self.qry.next():
            return None

        row = []
        for i in range(len(self.description)):
            value = self.qry.value(i)
            if (isinstance(value, QDate)
                    or isinstance(value, QTime)
                    or isinstance(value, QDateTime)):
                value = value.toString()
            elif isinstance(value, QByteArray):
                value = u"GEOMETRY"
                # value = value.toHex()

            row.append(value)

        return row

    def fetchmany(self, size=10):
        rows = []
        while len(rows) < size:
            row = self.fetchone()
            if row is None:
                break
            rows.append(row)

        return rows

    def fetchall(self):
        rows = []
        while True:
            row = self.fetchone()
            if row is None:
                break
            rows.append(row)

        return rows

    def setinputsize(self, sizes):
        raise ExecError("nyi")

    def setoutputsize(self, size, column=None):
        raise ExecError("nyi")


class QtSqlDBConnection:
    connections = 0

    def __init__(self, driver, dbname, user, passwd):
        self.conn = QSqlDatabase.addDatabase(
            driver, "qtsql_%d" % QtSqlDBConnection.connections)
        QtSqlDBConnection.connections += 1
        self.conn.setDatabaseName(dbname)
        self.conn.setUserName(user)
        self.conn.setPassword(passwd)

        if not self.conn.open():
            raise ConnectionError(self.conn.lastError().databaseText())

    def close(self):
        self.conn.close()

    def commit(self):
        self.conn.commit()

    def rollback(self):
        self.conn.rollback()

    def cursor(self):
        return QtSqlDBCursor(self.conn)


def connect(driver, dbname, user, passwd):
    return QtSqlDBConnection(driver, dbname, user, passwd)
