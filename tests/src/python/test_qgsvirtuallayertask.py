# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsVirtualLayerTask.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Paul Blottiere'
__date__ = '28/02/2018'
__copyright__ = 'Copyright 2018, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA
import os

from qgis.core import (
    QgsProject,
    QgsVectorLayer,
    QgsApplication,
    QgsVirtualLayerDefinition,
    QgsVirtualLayerTask
)
from qgis.PyQt.QtCore import QCoreApplication
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()


class TestQgsVirtualLayerTask(unittest.TestCase):

    def setUp(self):
        self.testDataDir = unitTestDataPath()
        self.success = False
        self.fail = False
        self.ids = None
        self.task = None

    def onSuccess(self):
        self.success = True
        self.ids = [f.id() for f in self.task.layer().getFeatures()]

    def onFail(self):
        self.fail = True

    def test(self):
        l1 = QgsVectorLayer(os.path.join(self.testDataDir, "france_parts.shp"), "françéà", "ogr", QgsVectorLayer.LayerOptions(False))
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
        while not self.success and not self.fail:
            QCoreApplication.processEvents()

        self.assertTrue(self.success)
        self.assertFalse(self.fail)

        self.assertEqual(len(self.ids), 4)


if __name__ == '__main__':
    unittest.main()
