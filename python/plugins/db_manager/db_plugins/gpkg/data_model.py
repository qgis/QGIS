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

from ..data_model import (TableDataModel,
                          SqlResultModel,
                          SqlResultModelAsync,
                          SqlResultModelTask)
from ..plugin import BaseError


class GPKGTableDataModel(TableDataModel):

    def __init__(self, table, parent=None):
        TableDataModel.__init__(self, table, parent)

        # fields_txt = ", ".join(self.fields)
        # table_txt = self.db.quoteId((self.table.schemaName(), self.table.name))

        # run query and get results
        # sql = "SELECT %s FROM %s" % (fields_txt, table_txt)
        # self.resdata = self.db._fetchAll(sql, include_fid_and_geometry = True)

        self.resdata = self.db._fetchAllFromLayer(table)

        self.fetchedFrom = 0
        self.fetchedCount = len(self.resdata)

    def _sanitizeTableField(self, field):
        return self.db.quoteId(field.name)

    def rowCount(self, index=None):
        return self.fetchedCount


class GPKGSqlResultModelTask(SqlResultModelTask):

    def __init__(self, db, sql, parent):
        super().__init__(db, sql, parent)

    def run(self):
        try:
            self.model = GPKGSqlResultModel(self.db, self.sql, None)
        except BaseError as e:
            self.error = e
            QgsMessageLog.logMessage(e.msg)
            return False
        return True

    def cancel(self):
        self.db.connector.cancel()
        SqlResultModelTask.cancel(self)


class GPKGSqlResultModelAsync(SqlResultModelAsync):

    def __init__(self, db, sql, parent):
        super().__init__()

        self.task = GPKGSqlResultModelTask(db, sql, parent)
        self.task.taskCompleted.connect(self.modelDone)
        self.task.taskTerminated.connect(self.modelDone)


class GPKGSqlResultModel(SqlResultModel):
    pass
