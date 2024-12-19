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

from ..data_model import (TableDataModel,
                          BaseTableModel,
                          SqlResultModelAsync,
                          SqlResultModelTask)

from .connector import VLayerRegistry, getQueryGeometryName
from .plugin import LVectorTable
from ..plugin import DbError, BaseError

from qgis.PyQt.QtCore import QElapsedTimer, QTemporaryFile
from qgis.core import (QgsVectorLayer,
                       QgsWkbTypes,
                       QgsVirtualLayerDefinition,
                       QgsVirtualLayerTask,
                       QgsTask)


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
                a.append(QgsWkbTypes.displayString(f.geometry().wkbType()))
            else:
                a.append('None')
            self.resdata.append(a)

        self.fetchedFrom = 0
        self.fetchedCount = len(self.resdata)

    def rowCount(self, index=None):
        if self.layer:
            return self.layer.featureCount()
        return 0


class LSqlResultModelTask(SqlResultModelTask):

    def __init__(self, db, sql, parent):
        super().__init__(db, sql, parent)

        tf = QTemporaryFile()
        tf.open()
        path = tf.fileName()
        tf.close()

        df = QgsVirtualLayerDefinition()
        df.setFilePath(path)
        df.setQuery(sql)

        self.subtask = QgsVirtualLayerTask(df)
        self.addSubTask(self.subtask, [], QgsTask.SubTaskDependency.ParentDependsOnSubTask)

    def run(self):
        try:
            path = self.subtask.definition().filePath()
            sql = self.subtask.definition().query()
            self.model = LSqlResultModel(self.db, sql, None, self.subtask.layer(), path)
        except Exception as e:
            self.error = BaseError(str(e))
            return False
        return True

    def cancel(self):
        SqlResultModelTask.cancel(self)


class LSqlResultModelAsync(SqlResultModelAsync):

    def __init__(self, db, sql, parent=None):
        super().__init__()

        self.task = LSqlResultModelTask(db, sql, parent)
        self.task.taskCompleted.connect(self.modelDone)
        self.task.taskTerminated.connect(self.modelDone)

    def modelDone(self):
        self.status = self.task.status
        self.model = self.task.model
        if self.task.subtask.exceptionText():
            self.error = BaseError(self.task.subtask.exceptionText())
        self.done.emit()


class LSqlResultModel(BaseTableModel):

    def __init__(self, db, sql, parent=None, layer=None, path=None):
        t = QElapsedTimer()
        t.start()

        if not layer:
            tf = QTemporaryFile()
            tf.open()
            path = tf.fileName()
            tf.close()

            df = QgsVirtualLayerDefinition()
            df.setFilePath(path)
            df.setQuery(sql)
            layer = QgsVectorLayer(df.toString(), "vv", "virtual")
            self._secs = t.elapsed() / 1000.0

        data = []
        header = []

        if not layer.isValid():
            raise DbError(layer.dataProvider().error().summary(), sql)
        else:
            header = [f.name() for f in layer.fields()]
            has_geometry = False
            if layer.geometryType() != QgsWkbTypes.GeometryType.NullGeometry:
                gn = getQueryGeometryName(path)
                if gn:
                    has_geometry = True
                    header += [gn]

            for f in layer.getFeatures():
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
