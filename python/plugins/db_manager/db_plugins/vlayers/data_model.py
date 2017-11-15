# -*- coding: utf-8 -*-

"""
/***************************************************************************
Name                 : Virtual layers plugin for DB Manager
Date                 : December 2015
copyright            : (C) 2015 by Hugo Mercier
email                : hugo dot mercier at oslandia dot com

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

from ..data_model import TableDataModel, BaseTableModel

from .connector import VLayerRegistry, getQueryGeometryName
from .plugin import LVectorTable
from ..plugin import DbError

from qgis.PyQt.QtCore import QTime, QTemporaryFile
from qgis.core import Qgis, QgsVectorLayer, QgsWkbTypes, QgsVirtualLayerDefinition


class LTableDataModel(TableDataModel):

    def __init__(self, table, parent=None):
        TableDataModel.__init__(self, table, parent)

        self.layer = None

        if isinstance(table, LVectorTable):
            self.layer = VLayerRegistry.instance().getLayer(table.name)
        else:
            self.layer = VLayerRegistry.instance().getLayer(table)

        if not self.layer:
            return
        # populate self.resdata
        self.resdata = []
        for f in self.layer.getFeatures():
            a = f.attributes()
            # add the geometry type
            if f.hasGeometry():
                a.append(QgsWkbTypes.displayString(Qgis.fromOldWkbType(f.geometry().wkbType())))
            else:
                a.append('None')
            self.resdata.append(a)

        self.fetchedFrom = 0
        self.fetchedCount = len(self.resdata)

    def rowCount(self, index=None):
        if self.layer:
            return self.layer.featureCount()
        return 0


class LSqlResultModel(BaseTableModel):
    # BaseTableModel

    def __init__(self, db, sql, parent=None):
        # create a virtual layer with non-geometry results
        t = QTime()
        t.start()

        tf = QTemporaryFile()
        tf.open()
        tmp = tf.fileName()
        tf.close()

        df = QgsVirtualLayerDefinition()
        df.setFilePath(tmp)
        df.setQuery(sql)
        p = QgsVectorLayer(df.toString(), "vv", "virtual")
        self._secs = t.elapsed() / 1000.0

        if not p.isValid():
            data = []
            header = []
            raise DbError(p.dataProvider().error().summary(), sql)
        else:
            header = [f.name() for f in p.fields()]
            has_geometry = False
            if p.geometryType() != QgsWkbTypes.NullGeometry:
                gn = getQueryGeometryName(tmp)
                if gn:
                    has_geometry = True
                    header += [gn]

            data = []
            for f in p.getFeatures():
                a = f.attributes()
                if has_geometry:
                    if f.hasGeometry():
                        a += [f.geometry().asWkt()]
                    else:
                        a += [None]
                data += [a]

        self._secs = 0
        self._affectedRows = len(data)

        BaseTableModel.__init__(self, header, data, parent)

    def secs(self):
        return self._secs

    def affectedRows(self):
        return self._affectedRows
