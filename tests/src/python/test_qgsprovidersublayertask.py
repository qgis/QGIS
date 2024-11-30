"""QGIS Unit tests for QgsProviderSublayerTask.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "30/06/2021"
__copyright__ = "Copyright 2021, The QGIS Project"

from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import (
    QgsApplication,
    QgsMapLayerType,
    QgsProviderSublayerTask,
    QgsWkbTypes,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

app = start_app()


class TestQgsProviderSublayerTask(QgisTestCase):

    def test_query(self):
        """
        Test querying sublayers using the task
        """
        task = QgsProviderSublayerTask(uri=unitTestDataPath() + "/mixed_types.TAB")

        def completed():
            completed.results = task.results()

        completed.results = None

        task.taskCompleted.connect(completed)
        spy = QSignalSpy(task.taskCompleted)

        QgsApplication.taskManager().addTask(task)
        if completed.results is None:
            spy.wait()

        self.assertEqual(completed.results[0].layerNumber(), 0)
        self.assertEqual(completed.results[0].name(), "mixed_types")
        self.assertEqual(completed.results[0].description(), "")
        self.assertEqual(
            completed.results[0].uri(),
            f"{unitTestDataPath()}/mixed_types.TAB|geometrytype=Point",
        )
        self.assertEqual(completed.results[0].providerKey(), "ogr")
        self.assertEqual(completed.results[0].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(completed.results[0].featureCount(), 4)
        self.assertEqual(completed.results[0].wkbType(), QgsWkbTypes.Type.Point)
        self.assertEqual(completed.results[0].geometryColumnName(), "")

        self.assertEqual(completed.results[1].layerNumber(), 0)
        self.assertEqual(completed.results[1].name(), "mixed_types")
        self.assertEqual(completed.results[1].description(), "")
        self.assertEqual(
            completed.results[1].uri(),
            f"{unitTestDataPath()}/mixed_types.TAB|geometrytype=LineString",
        )
        self.assertEqual(completed.results[1].providerKey(), "ogr")
        self.assertEqual(completed.results[1].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(completed.results[1].featureCount(), 4)
        self.assertEqual(completed.results[1].wkbType(), QgsWkbTypes.Type.LineString)
        self.assertEqual(completed.results[1].geometryColumnName(), "")

        self.assertEqual(completed.results[2].layerNumber(), 0)
        self.assertEqual(completed.results[2].name(), "mixed_types")
        self.assertEqual(completed.results[2].description(), "")
        self.assertEqual(
            completed.results[2].uri(),
            f"{unitTestDataPath()}/mixed_types.TAB|geometrytype=Polygon",
        )
        self.assertEqual(completed.results[2].providerKey(), "ogr")
        self.assertEqual(completed.results[2].type(), QgsMapLayerType.VectorLayer)
        self.assertEqual(completed.results[2].featureCount(), 3)
        self.assertEqual(completed.results[2].wkbType(), QgsWkbTypes.Type.Polygon)
        self.assertEqual(completed.results[2].geometryColumnName(), "")


if __name__ == "__main__":
    unittest.main()
