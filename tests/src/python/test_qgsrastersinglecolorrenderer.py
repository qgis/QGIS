"""QGIS Unit tests for QgsRasterSingleColorRenderer.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import os

from qgis.PyQt.QtCore import QFileInfo
from qgis.PyQt.QtGui import QColor
from qgis.core import (
    QgsRasterLayer,
    QgsRasterSingleColorRenderer,
    QgsMapSettings,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

# Convenience instances in case you may need them
# not used in this test
start_app()


class TestQgsRasterSingleBandGrayRenderer(QgisTestCase):

    def testRenderer(self):
        path = os.path.join(unitTestDataPath(), "landsat-int16-b1.tif")
        info = QFileInfo(path)
        base_name = info.baseName()
        layer = QgsRasterLayer(path, base_name)
        self.assertTrue(layer.isValid(), f"Raster not loaded: {path}")

        renderer = QgsRasterSingleColorRenderer(
            layer.dataProvider(), 1, QColor(255, 0, 0)
        )

        self.assertEqual(renderer.inputBand(), 1)
        self.assertEqual(renderer.usesBands(), [1])
        self.assertEqual(renderer.color(), QColor(255, 0, 0))
        renderer.setColor(QColor(0, 255, 0))
        self.assertEqual(renderer.color(), QColor(0, 255, 0))

        layer.setRenderer(renderer)
        ms = QgsMapSettings()
        ms.setLayers([layer])
        ms.setExtent(layer.extent())

        self.assertTrue(
            self.render_map_settings_check(
                "raster_single_color_renderer", "raster_single_color_renderer", ms
            )
        )


if __name__ == "__main__":
    unittest.main()
