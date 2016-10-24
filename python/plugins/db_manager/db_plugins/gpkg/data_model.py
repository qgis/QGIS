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

from ..data_model import TableDataModel, SqlResultModel


class GPKGTableDataModel(TableDataModel):

    def __init__(self, table, parent=None):
        TableDataModel.__init__(self, table, parent)

        #fields_txt = u", ".join(self.fields)
        #table_txt = self.db.quoteId((self.table.schemaName(), self.table.name))

        # run query and get results
        #sql = u"SELECT %s FROM %s" % (fields_txt, table_txt)
        #self.resdata = self.db._fetchAll(sql, include_fid_and_geometry = True)

        self.resdata = self.db._fetchAllFromLayer(table)

        self.fetchedFrom = 0
        self.fetchedCount = len(self.resdata)

    def _sanitizeTableField(self, field):
        return self.db.quoteId(field.name)

    def rowCount(self, index=None):
        return self.fetchedCount


class GPKGSqlResultModel(SqlResultModel):
    pass
