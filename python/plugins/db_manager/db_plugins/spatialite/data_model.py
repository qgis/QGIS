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

from qgis.core import QgsMessageLog
from ..plugin import BaseError
from ..data_model import (TableDataModel,
                          SqlResultModel,
                          SqlResultModelAsync,
                          SqlResultModelTask)
from .plugin import SLDatabase


class SLTableDataModel(TableDataModel):

    def __init__(self, table, parent=None):
        TableDataModel.__init__(self, table, parent)

        fields_txt = ", ".join(self.fields)
        table_txt = self.db.quoteId((self.table.schemaName(), self.table.name))

        # run query and get results
        sql = "SELECT %s FROM %s" % (fields_txt, table_txt)
        c = self.db._get_cursor()
        self.db._execute(c, sql)

        self.resdata = self.db._fetchall(c)
        c.close()
        del c

        self.fetchedFrom = 0
        self.fetchedCount = len(self.resdata)

    def _sanitizeTableField(self, field):
        # get fields, ignore geometry columns
        dataType = field.dataType.upper()
        if dataType[:5] == "MULTI":
            dataType = dataType[5:]
        if dataType[-3:] == "25D":
            dataType = dataType[:-3]
        if dataType[-10:] == "COLLECTION":
            dataType = dataType[:-10]
        if dataType in ["POINT", "LINESTRING", "POLYGON", "GEOMETRY"]:
            return 'GeometryType(%s)' % self.db.quoteId(field.name)
        return self.db.quoteId(field.name)

    def rowCount(self, index=None):
        return self.fetchedCount


class SLSqlResultModelTask(SqlResultModelTask):

    def __init__(self, db, sql, parent):
        super().__init__(db, sql, parent)
        self.clone = None

    def run(self):
        try:
            self.clone = SLDatabase(None, self.db.connector.uri())
            self.model = SLSqlResultModel(self.clone, self.sql, None)
        except BaseError as e:
            self.error = e
            QgsMessageLog.logMessage(e.msg)
            return False

        return True

    def cancel(self):
        if self.clone:
            self.clone.connector.cancel()
        SqlResultModelTask.cancel(self)


class SLSqlResultModelAsync(SqlResultModelAsync):

    def __init__(self, db, sql, parent):
        super().__init__()

        self.task = SLSqlResultModelTask(db, sql, parent)
        self.task.taskCompleted.connect(self.modelDone)
        self.task.taskTerminated.connect(self.modelDone)


class SLSqlResultModel(SqlResultModel):
    pass
