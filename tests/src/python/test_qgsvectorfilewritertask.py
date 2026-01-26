"""QGIS Unit tests for QgsVectorFileWriterTask.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "12/02/2017"
__copyright__ = "Copyright 2017, The QGIS Project"

import os

from qgis.PyQt.QtCore import QCoreApplication, QDir
from qgis.core import (
    QgsApplication,
    QgsFeature,
    QgsGeometry,
    QgsPointXY,
    QgsVectorFileWriter,
    QgsVectorFileWriterTask,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


def create_temp_filename(base_file):
    return os.path.join(str(QDir.tempPath()), base_file)


class TestQgsVectorFileWriterTask(QgisTestCase):

    def setUp(self):
        self.new_filename = ""
        self.new_layer = ""
        self.success = False
        self.fail = False

    def onSuccess(self):
        self.success = True

    def onComplete(self, filename, layer):
        self.success = True
        self.new_filename = filename
        self.new_layer = layer

    def onFail(self):
        self.fail = True

    def createLayer(self):
        layer = QgsVectorLayer(
            (
                "Point?crs=epsg:4326&field=name:string(20)&"
                "field=age:integer&field=size:double&index=yes"
            ),
            "test",
            "memory",
        )

        self.assertIsNotNone(layer, "Provider not initialized")
        provider = layer.dataProvider()
        self.assertIsNotNone(provider)

        ft = QgsFeature()
        ft.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(10, 10)))
        ft.setAttributes(["Johny", 20, 0.3])
        provider.addFeatures([ft])
        return layer

    def testSuccess(self):
        """test successfully writing a layer"""
        self.layer = self.createLayer()
        options = QgsVectorFileWriter.SaveVectorOptions()
        tmp = create_temp_filename("successlayer.shp")
        task = QgsVectorFileWriterTask(self.layer, tmp, options)

        task.writeComplete.connect(self.onSuccess)
        task.errorOccurred.connect(self.onFail)

        QgsApplication.taskManager().addTask(task)
        while not self.success and not self.fail:
            QCoreApplication.processEvents()

        self.assertTrue(self.success)
        self.assertFalse(self.fail)

    def testSuccessWithLayer(self):
        """test successfully writing to a layer-enabled format"""
        self.layer = self.createLayer()
        options = QgsVectorFileWriter.SaveVectorOptions()
        options.driverName = "KML"
        options.layerName = "test-dashes"
        tmp = create_temp_filename("successlayer.kml")
        task = QgsVectorFileWriterTask(self.layer, tmp, options)

        task.completed.connect(self.onComplete)
        task.errorOccurred.connect(self.onFail)

        QgsApplication.taskManager().addTask(task)
        while not self.success and not self.fail:
            QCoreApplication.processEvents()

        self.assertEqual(self.new_layer, "test_dashes")
        self.assertTrue(self.success)
        self.assertFalse(self.fail)

    def testLayerRemovalBeforeRun(self):
        """test behavior when layer is removed before task begins"""
        self.layer = self.createLayer()
        options = QgsVectorFileWriter.SaveVectorOptions()
        tmp = create_temp_filename("fail.shp")
        task = QgsVectorFileWriterTask(self.layer, tmp, options)

        task.writeComplete.connect(self.onSuccess)
        task.errorOccurred.connect(self.onFail)

        # remove layer
        self.layer = None

        QgsApplication.taskManager().addTask(task)
        while not self.success and not self.fail:
            QCoreApplication.processEvents()

        self.assertTrue(self.success)
        self.assertFalse(self.fail)

    def testNoLayer(self):
        """test that failure (and not crash) occurs when no layer set"""

        options = QgsVectorFileWriter.SaveVectorOptions()
        tmp = create_temp_filename("fail.shp")
        task = QgsVectorFileWriterTask(None, tmp, options)
        task.writeComplete.connect(self.onSuccess)
        task.errorOccurred.connect(self.onFail)

        QgsApplication.taskManager().addTask(task)
        while not self.success and not self.fail:
            QCoreApplication.processEvents()

        self.assertFalse(self.success)
        self.assertTrue(self.fail)

    def testFieldValueConverter(self):
        """test no crash when fieldValueConverter is used"""
        self.layer = self.createLayer()
        options = QgsVectorFileWriter.SaveVectorOptions()
        converter = QgsVectorFileWriter.FieldValueConverter()
        options.fieldValueConverter = converter
        tmp = create_temp_filename("converter.shp")
        task = QgsVectorFileWriterTask(self.layer, tmp, options)

        task.writeComplete.connect(self.onSuccess)
        task.errorOccurred.connect(self.onFail)

        del converter

        QgsApplication.taskManager().addTask(task)
        while not self.success and not self.fail:
            QCoreApplication.processEvents()

        self.assertTrue(self.success)
        self.assertFalse(self.fail)


if __name__ == "__main__":
    unittest.main()
