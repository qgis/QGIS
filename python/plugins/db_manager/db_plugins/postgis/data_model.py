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

from qgis.core import QgsMessageLog
from ..plugin import BaseError
from ..data_model import (
    TableDataModel,
    SqlResultModel,
    SqlResultModelAsync,
    SqlResultModelTask,
)


class PGTableDataModel(TableDataModel):

    def __init__(self, table, parent=None):
        self.cursor = None
        TableDataModel.__init__(self, table, parent)

        if self.table.rowCount is None:
            self.table.refreshRowCount()
            if self.table.rowCount is None:
                return

        self.table.aboutToChange.connect(self._deleteCursor)
        self._createCursor()

    def _createCursor(self):
        fields_txt = ", ".join(self.fields)
        table_txt = self.db.quoteId((self.table.schemaName(), self.table.name))

        self.cursor = self.db._get_cursor()
        sql = f"SELECT {fields_txt} FROM {table_txt}"
        self.db._execute(self.cursor, sql)

    def _sanitizeTableField(self, field):
        # get fields, ignore geometry columns
        if field.dataType.lower() == "geometry":
            return "CASE WHEN {fld} IS NULL THEN NULL ELSE GeometryType({fld}) END AS {fld}".format(
                fld=self.db.quoteId(field.name)
            )
        elif field.dataType.lower() == "raster":
            return (
                "CASE WHEN {fld} IS NULL THEN NULL ELSE 'RASTER' END AS {fld}".format(
                    fld=self.db.quoteId(field.name)
                )
            )
        return "%s::text" % self.db.quoteId(field.name)

    def _deleteCursor(self):
        self.db._close_cursor(self.cursor)
        self.cursor = None

    def __del__(self):
        self.table.aboutToChange.disconnect(self._deleteCursor)
        self._deleteCursor()
        pass  # print "PGTableModel.__del__"

    def fetchMoreData(self, row_start):
        if not self.cursor:
            self._createCursor()

        try:
            self.cursor.scroll(row_start, mode="absolute")
        except self.db.error_types():
            self._deleteCursor()
            return self.fetchMoreData(row_start)

        self.resdata = self.cursor.fetchmany(self.fetchedCount)
        self.fetchedFrom = row_start


class PGSqlResultModelTask(SqlResultModelTask):

    def __init__(self, db, sql, parent):
        super().__init__(db, sql, parent)

    def run(self):
        try:
            self.model = PGSqlResultModel(self.db, self.sql, None)
        except BaseError as e:
            self.error = e
            QgsMessageLog.logMessage(e.msg)
            return False
        return True

    def cancel(self):
        self.db.connector.cancel()
        SqlResultModelTask.cancel(self)


class PGSqlResultModelAsync(SqlResultModelAsync):

    def __init__(self, db, sql, parent):
        super().__init__()

        self.task = PGSqlResultModelTask(db, sql, parent)
        self.task.taskCompleted.connect(self.modelDone)
        self.task.taskTerminated.connect(self.modelDone)


class PGSqlResultModel(SqlResultModel):
    pass
