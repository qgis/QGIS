"""QGIS Unit tests for QgsRasterRendererRegistry

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
from qgis.core import (
    Qgis,
    QgsRasterRendererRegistry
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

# Convenience instances in case you may need them
# not used in this test
start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsRasterRendererRegistry(QgisTestCase):

    def test_registered(self):
        """
        Test that standard renderers are registered
        """
        registry = QgsRasterRendererRegistry()
        self.assertIn('multibandcolor', registry.renderersList())
        self.assertIn('singlebandgray', registry.renderersList())
        self.assertIn('singlebandpseudocolor', registry.renderersList())
        self.assertIn('singlebandcolordata', registry.renderersList())
        self.assertIn('hillshade', registry.renderersList())
        self.assertIn('paletted', registry.renderersList())
        self.assertIn('contour', registry.renderersList())

    def test_capabilities(self):
        """
        Test retrieving renderer capabilities
        """
        registry = QgsRasterRendererRegistry()
        self.assertFalse(registry.rendererCapabilities('not a renderer'))
        self.assertEqual(registry.rendererCapabilities('multibandcolor'),
                         Qgis.RasterRendererCapability.UsesMultipleBands)
        self.assertEqual(registry.rendererCapabilities('singlebandgray'),
                         Qgis.RasterRendererCapabilities())


if __name__ == '__main__':
    unittest.main()
