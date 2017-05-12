# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsRasterFileWriterTask.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '12/02/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA
import os

from qgis.core import (
    QgsApplication,
    QgsRasterLayer,
    QgsRasterPipe,
    QgsRasterFileWriter,
    QgsRasterFileWriterTask
)
from qgis.PyQt.QtCore import QCoreApplication, QDir
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()


def create_temp_filename(base_file):
    return os.path.join(str(QDir.tempPath()), base_file)


class TestQgsRasterFileWriterTask(unittest.TestCase):

    def setUp(self):
        self.success = False
        self.fail = False

    def onSuccess(self):
        self.success = True

    def onFail(self):
        self.fail = True

    def testSuccess(self):
        """test successfully writing a layer"""
        path = os.path.join(unitTestDataPath(), 'raster', 'with_color_table.tif')
        raster_layer = QgsRasterLayer(path, "test")
        self.assertTrue(raster_layer.isValid())

        pipe = QgsRasterPipe()
        self.assertTrue(pipe.set(raster_layer.dataProvider().clone()))

        tmp = create_temp_filename('success.tif')
        writer = QgsRasterFileWriter(tmp)

        task = QgsRasterFileWriterTask(writer, pipe, 100, 100, raster_layer.extent(), raster_layer.crs())

        task.writeComplete.connect(self.onSuccess)
        task.errorOccurred.connect(self.onFail)

        QgsApplication.taskManager().addTask(task)
        while not self.success and not self.fail:
            QCoreApplication.processEvents()

        self.assertTrue(self.success)
        self.assertFalse(self.fail)
        self.assertTrue(os.path.exists(tmp))

    def testLayerRemovalBeforeRun(self):
        """test behavior when layer is removed before task begins"""
        path = os.path.join(unitTestDataPath(), 'raster', 'with_color_table.tif')
        raster_layer = QgsRasterLayer(path, "test")
        self.assertTrue(raster_layer.isValid())

        pipe = QgsRasterPipe()
        self.assertTrue(pipe.set(raster_layer.dataProvider().clone()))

        tmp = create_temp_filename('remove_layer.tif')
        writer = QgsRasterFileWriter(tmp)

        task = QgsRasterFileWriterTask(writer, pipe, 100, 100, raster_layer.extent(), raster_layer.crs())

        task.writeComplete.connect(self.onSuccess)
        task.errorOccurred.connect(self.onFail)

        # remove layer
        raster_layer = None

        QgsApplication.taskManager().addTask(task)
        while not self.success and not self.fail:
            QCoreApplication.processEvents()

        # in this case will still get a positive result - since the pipe is cloned before the task
        # begins the task is no longer dependent on the original layer
        self.assertTrue(self.success)
        self.assertFalse(self.fail)
        self.assertTrue(os.path.exists(tmp))


if __name__ == '__main__':
    unittest.main()
