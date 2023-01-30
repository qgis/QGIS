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
from builtins import str

from qgis.PyQt.QtCore import QTime
from qgis.core import QgsMessageLog
from ..data_model import (TableDataModel,
                          SqlResultModel,
                          SqlResultModelAsync,
                          SqlResultModelTask,
                          BaseTableModel)
from ..plugin import DbError
from ..plugin import BaseError


class ORTableDataModel(TableDataModel):

    def __init__(self, table, parent=None):
        self.cursor = None
        TableDataModel.__init__(self, table, parent)

        if not self.table.rowCount:
            self.table.refreshRowCount()

        self.table.aboutToChange.connect(self._deleteCursor)
        self._createCursor()

    def _createCursor(self):
        fields_txt = ", ".join(self.fields)
        table_txt = self.db.quoteId(
            (self.table.schemaName(), self.table.name))

        self.cursor = self.db._get_cursor()
        sql = "SELECT {0} FROM {1}".format(fields_txt, table_txt)

        self.db._execute(self.cursor, sql)

    def _sanitizeTableField(self, field):
        # get fields, ignore geometry columns
        if field.dataType.upper() == "SDO_GEOMETRY":
            return ("CASE WHEN {0} IS NULL THEN NULL ELSE 'GEOMETRY'"
                    "END AS {0}".format(
                        self.db.quoteId(field.name)))
        if field.dataType.upper() == "DATE":
            return "CAST({} AS VARCHAR2(8))".format(
                self.db.quoteId(field.name))
        if "TIMESTAMP" in field.dataType.upper():
            return "TO_CHAR({}, 'YYYY-MM-DD HH:MI:SS.FF')".format(
                self.db.quoteId(field.name))
        if field.dataType.upper() == "NUMBER":
            if not field.charMaxLen:
                return "CAST({} AS VARCHAR2(135))".format(
                    self.db.quoteId(field.name))
            elif field.modifier:
                nbChars = 2 + int(field.charMaxLen) + \
                    int(field.modifier)
                return "CAST({} AS VARCHAR2({}))".format(
                    self.db.quoteId(field.name),
                    str(nbChars))

        return "CAST({0} As VARCHAR2({1}))".format(
            self.db.quoteId(field.name), field.charMaxLen)

    def _deleteCursor(self):
        self.db._close_cursor(self.cursor)
        self.cursor = None

    def __del__(self):
        self.table.aboutToChange.disconnect(self._deleteCursor)
        self._deleteCursor()

    def getData(self, row, col):
        if (row < self.fetchedFrom or
                row >= self.fetchedFrom + self.fetchedCount):
            margin = self.fetchedCount / 2
            if row + margin >= self.rowCount():
                start = int(self.rowCount() - margin)
            else:
                start = int(row - margin)
            if start < 0:
                start = 0
            self.fetchMoreData(start)

        # For some improbable cases
        if row - self.fetchedFrom >= len(self.resdata):
            return None

        return self.resdata[row - self.fetchedFrom][col]

    def fetchMoreData(self, row_start):
        if not self.cursor:
            self._createCursor()

        self.cursor.scroll(row_start - 1)

        self.resdata = self.cursor.fetchmany(self.fetchedCount)
        self.fetchedFrom = row_start


class ORSqlResultModelTask(SqlResultModelTask):

    def __init__(self, db, sql, parent):
        super().__init__(db, sql, parent)

    def run(self):
        try:
            self.model = ORSqlResultModel(self.db, self.sql, None)
        except BaseError as e:
            self.error = e
            QgsMessageLog.logMessage(e.msg)
            return False

        return True

    def cancel(self):
        self.db.connector.cancel()
        SqlResultModelTask.cancel(self)


class ORSqlResultModelAsync(SqlResultModelAsync):

    def __init__(self, db, sql, parent):
        super().__init__()

        self.task = ORSqlResultModelTask(db, sql, parent)
        self.task.taskCompleted.connect(self.modelDone)
        self.task.taskTerminated.connect(self.modelDone)


class ORSqlResultModel(SqlResultModel):

    def __init__(self, db, sql, parent=None):
        self.db = db.connector

        t = QTime()
        t.start()
        c = self.db._execute(None, str(sql))

        self._affectedRows = 0
        data = []
        header = self.db._get_cursor_columns(c)
        if not header:
            header = []

        try:
            if len(header) > 0:
                data = self.db._fetchall(c)
            self._affectedRows = len(data)
        except DbError:
            # nothing to fetch!
            data = []
            header = []

        self._secs = t.elapsed() / 1000.0
        del t

        BaseTableModel.__init__(self, header, data, parent)

        # commit before closing the cursor to make sure that the
        # changes are stored
        self.db._commit()
        c.close()
        del c
