"""QGIS Unit tests for QgsRasterVectorFieldRenderer

From build dir, run:
ctest -R PyQgsRasterVectorFieldRenderer -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import os
import unittest

from qgis.core import (
    Qgis,
    QgsRasterLayer,
    QgsRasterVectorFieldRenderer,
)
from qgis.testing import QgisTestCase, start_app
from utilities import unitTestDataPath

start_app()


class TestQgsRasterVectorFieldRenderer(QgisTestCase):
    def layer(self):
        path = os.path.join(unitTestDataPath(), "raster", "vector_field_uv.tif")
        layer = QgsRasterLayer(path, "vector field")
        self.assertTrue(layer.isValid(), f"Raster not loaded: {path}")
        return layer

    def test_bands(self):
        layer = self.layer()
        renderer = QgsRasterVectorFieldRenderer(layer.dataProvider())

        # two bands are used, so there is no single input band
        self.assertEqual(renderer.inputBand(), -1)
        self.assertFalse(renderer.setInputBand(1))

        self.assertEqual(renderer.xBand(), 1)
        self.assertEqual(renderer.yBand(), 2)
        self.assertEqual(renderer.usesBands(), [1, 2])

        self.assertTrue(renderer.setXBand(2))
        self.assertTrue(renderer.setYBand(1))
        self.assertEqual(renderer.usesBands(), [2, 1])

        # the raster only holds two bands
        self.assertFalse(renderer.setXBand(0))
        self.assertFalse(renderer.setYBand(3))
        self.assertEqual(renderer.usesBands(), [2, 1])

    def direction_layer(self):
        path = os.path.join(unitTestDataPath(), "raster", "flow_direction_d8.tif")
        layer = QgsRasterLayer(path, "flow direction")
        self.assertTrue(layer.isValid(), f"Raster not loaded: {path}")
        return layer

    def test_encoded_direction_bands(self):
        layer = self.direction_layer()
        renderer = QgsRasterVectorFieldRenderer(layer.dataProvider())

        self.assertEqual(renderer.directionBand(), 1)

        renderer.setSourceMode(Qgis.RasterVectorFieldSourceMode.EncodedDirection)
        self.assertEqual(renderer.usesBands(), [1])

        # the raster only holds one band
        self.assertFalse(renderer.setDirectionBand(2))
        self.assertFalse(renderer.setDirectionBand(0))
        self.assertEqual(renderer.directionBand(), 1)

    def test_invalid_layer(self):
        """
        Test vector field band handling with a broken layer path
        """
        renderer = QgsRasterVectorFieldRenderer(None)

        self.assertEqual(renderer.xBand(), 1)
        self.assertEqual(renderer.yBand(), 2)
        self.assertEqual(renderer.directionBand(), 1)

        # the renderer input is broken, we don't know what bands are valid, so all positive bands should be accepted
        self.assertTrue(renderer.setXBand(10))
        self.assertTrue(renderer.setYBand(11))
        self.assertTrue(renderer.setDirectionBand(12))
        self.assertEqual(renderer.usesBands(), [10, 11])

        renderer.setSourceMode(Qgis.RasterVectorFieldSourceMode.EncodedDirection)
        self.assertEqual(renderer.usesBands(), [12])

        self.assertFalse(renderer.setXBand(0))
        self.assertFalse(renderer.setDirectionBand(0))
        self.assertEqual(renderer.xBand(), 10)
        self.assertEqual(renderer.directionBand(), 12)


if __name__ == "__main__":
    unittest.main()
