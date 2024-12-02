"""QGIS Unit tests for QgsElevationUtils

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import os

from qgis.core import (
    Qgis,
    QgsProject,
    QgsRasterLayer,
    QgsElevationUtils,
    QgsDoubleRange,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

# Convenience instances in case you may need them
# not used in this test
start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsElevationUtils(QgisTestCase):

    def test_z_range_for_project(self):
        """
        Test calculating z range for a project
        """
        project = QgsProject()
        self.assertEqual(
            QgsElevationUtils.calculateZRangeForProject(project), QgsDoubleRange()
        )

        raster_layer = QgsRasterLayer(
            os.path.join(unitTestDataPath(), "landsat_4326.tif")
        )
        self.assertTrue(raster_layer.isValid())
        project.addMapLayer(raster_layer)
        self.assertEqual(
            QgsElevationUtils.calculateZRangeForProject(project), QgsDoubleRange()
        )

        props = raster_layer.elevationProperties()
        props.setEnabled(True)
        props.setMode(Qgis.RasterElevationMode.FixedRangePerBand)
        props.setFixedRangePerBand(
            {
                1: QgsDoubleRange(103.1, 106.8),
                2: QgsDoubleRange(106.8, 116.8),
                3: QgsDoubleRange(116.8, 126.8),
            }
        )
        self.assertEqual(
            QgsElevationUtils.calculateZRangeForProject(project),
            QgsDoubleRange(103.1, 126.8),
        )

        raster_layer2 = QgsRasterLayer(
            os.path.join(unitTestDataPath(), "landsat_4326.tif")
        )
        self.assertTrue(raster_layer2.isValid())
        project.addMapLayer(raster_layer2)
        self.assertEqual(
            QgsElevationUtils.calculateZRangeForProject(project),
            QgsDoubleRange(103.1, 126.8),
        )

        props = raster_layer2.elevationProperties()
        props.setEnabled(True)
        props.setMode(Qgis.RasterElevationMode.FixedRangePerBand)
        props.setFixedRangePerBand(
            {
                1: QgsDoubleRange(103.1, 106.8),
                2: QgsDoubleRange(106.8, 116.8),
                3: QgsDoubleRange(126.8, 136.8),
            }
        )
        self.assertEqual(
            QgsElevationUtils.calculateZRangeForProject(project),
            QgsDoubleRange(103.1, 136.8),
        )

    def test_significant_z_values_for_project(self):
        """
        Test calculating significant z values for a project
        """
        project = QgsProject()
        self.assertFalse(QgsElevationUtils.significantZValuesForProject(project))
        self.assertFalse(QgsElevationUtils.significantZValuesForLayers([]))

        raster_layer = QgsRasterLayer(
            os.path.join(unitTestDataPath(), "landsat_4326.tif")
        )
        self.assertTrue(raster_layer.isValid())
        project.addMapLayer(raster_layer)
        self.assertFalse(QgsElevationUtils.significantZValuesForProject(project))
        self.assertFalse(QgsElevationUtils.significantZValuesForLayers([raster_layer]))

        props = raster_layer.elevationProperties()
        props.setEnabled(True)
        props.setMode(Qgis.RasterElevationMode.FixedRangePerBand)
        props.setFixedRangePerBand(
            {
                1: QgsDoubleRange(103.1, 106.8),
                2: QgsDoubleRange(106.8, 116.8),
                3: QgsDoubleRange(116.8, 126.8),
            }
        )
        self.assertEqual(
            QgsElevationUtils.significantZValuesForProject(project),
            [103.1, 106.8, 116.8, 126.8],
        )
        self.assertEqual(
            QgsElevationUtils.significantZValuesForLayers([raster_layer]),
            [103.1, 106.8, 116.8, 126.8],
        )

        raster_layer2 = QgsRasterLayer(
            os.path.join(unitTestDataPath(), "landsat_4326.tif")
        )
        self.assertTrue(raster_layer2.isValid())
        project.addMapLayer(raster_layer2)
        self.assertEqual(
            QgsElevationUtils.significantZValuesForProject(project),
            [103.1, 106.8, 116.8, 126.8],
        )
        self.assertEqual(
            QgsElevationUtils.significantZValuesForLayers(
                [raster_layer, raster_layer2]
            ),
            [103.1, 106.8, 116.8, 126.8],
        )

        props = raster_layer2.elevationProperties()
        props.setEnabled(True)
        props.setMode(Qgis.RasterElevationMode.FixedRangePerBand)
        props.setFixedRangePerBand(
            {
                1: QgsDoubleRange(103.1, 106.8),
                2: QgsDoubleRange(106.8, 116.8),
                3: QgsDoubleRange(126.8, 136.8),
            }
        )
        self.assertEqual(
            QgsElevationUtils.significantZValuesForProject(project),
            [103.1, 106.8, 116.8, 126.8, 136.8],
        )
        self.assertEqual(
            QgsElevationUtils.significantZValuesForLayers(
                [raster_layer, raster_layer2]
            ),
            [103.1, 106.8, 116.8, 126.8, 136.8],
        )


if __name__ == "__main__":
    unittest.main()
