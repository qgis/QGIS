"""QGIS Unit tests for QgsVirtualLayerTask.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Paul Blottiere"
__date__ = "28/02/2018"
__copyright__ = "Copyright 2018, The QGIS Project"

import os

from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import (
    QgsApplication,
    QgsProject,
    QgsVectorLayer,
    QgsVirtualLayerDefinition,
    QgsVirtualLayerTask,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()


class TestQgsVirtualLayerTask(QgisTestCase):

    def setUp(self):
        self.testDataDir = unitTestDataPath()
        self._success = False
        self._fail = False
        self.ids = None
        self.task = None

    def onSuccess(self):
        self._success = True
        self.ids = [f.id() for f in self.task.layer().getFeatures()]

    def onFail(self):
        self._fail = True
        self._exceptionText = self.task.exceptionText()

    def test(self):
        l1 = QgsVectorLayer(
            os.path.join(self.testDataDir, "france_parts.shp"),
            "françéà",
            "ogr",
            QgsVectorLayer.LayerOptions(False),
        )
        self.assertEqual(l1.isValid(), True)
        QgsProject.instance().addMapLayer(l1)

        df = QgsVirtualLayerDefinition()
        df.setQuery('select * from "françéà"')
        self.task = QgsVirtualLayerTask(df)

        ids = [f.id() for f in self.task.layer().getFeatures()]
        self.assertEqual(len(ids), 0)

        self.task.taskCompleted.connect(self.onSuccess)
        self.task.taskTerminated.connect(self.onFail)

        QgsApplication.taskManager().addTask(self.task)
        while not self._success and not self._fail:
            QCoreApplication.processEvents()

        self.assertTrue(self._success)
        self.assertFalse(self._fail)

        self.assertEqual(len(self.ids), 4)

        # Test exception
        self._success = False
        self._fail = False
        df.setQuery("select *")
        self.task = QgsVirtualLayerTask(df)
        self.task.taskCompleted.connect(self.onSuccess)
        self.task.taskTerminated.connect(self.onFail)
        QgsApplication.taskManager().addTask(self.task)
        while not self._success and not self._fail:
            QCoreApplication.processEvents()

        self.assertFalse(self._success)
        self.assertTrue(self._fail)
        self.assertEqual(
            self._exceptionText,
            "Query preparation error on PRAGMA table_info(_tview): no tables specified",
            self._exceptionText,
        )


if __name__ == "__main__":
    unittest.main()
