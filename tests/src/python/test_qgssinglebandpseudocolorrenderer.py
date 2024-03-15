"""QGIS Unit tests for QgsSingleBandPseudoColorRenderer.

From build dir, run:
ctest -R PyQgsSingleBandPseudoColorRenderer -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import os

from qgis.PyQt.QtCore import QFileInfo
from qgis.core import (
    QgsRasterLayer,
    QgsSingleBandPseudoColorRenderer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

# Convenience instances in case you may need them
# not used in this test
start_app()


class TestQgsSingleBandPseudoColorRenderer(QgisTestCase):

    def test_renderer(self):
        path = os.path.join(unitTestDataPath(),
                            'landsat.tif')
        info = QFileInfo(path)
        base_name = info.baseName()
        layer = QgsRasterLayer(path, base_name)
        self.assertTrue(layer.isValid(), f'Raster not loaded: {path}')

        renderer = QgsSingleBandPseudoColorRenderer(layer.dataProvider(),
                                                    1)

        self.assertEqual(renderer.inputBand(), 1)

        self.assertFalse(renderer.setInputBand(0))
        self.assertEqual(renderer.inputBand(), 1)
        self.assertFalse(renderer.setInputBand(10))
        self.assertEqual(renderer.inputBand(), 1)
        self.assertTrue(renderer.setInputBand(2))
        self.assertEqual(renderer.inputBand(), 2)

    def test_invalid_layer(self):
        """
        Test renderer band with a broken layer path
        """
        renderer = QgsSingleBandPseudoColorRenderer(None,
                                                    11)

        self.assertEqual(renderer.inputBand(), 11)

        # the renderer input is broken, we don't know what bands are valid, so all should be accepted
        self.assertTrue(renderer.setInputBand(10))
        self.assertEqual(renderer.inputBand(), 10)


if __name__ == '__main__':
    unittest.main()
