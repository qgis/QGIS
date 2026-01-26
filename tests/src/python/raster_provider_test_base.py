"""QGIS Unit test utils for raster provider tests.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.core import Qgis


class RasterProviderTestCase:
    """
    This is a collection of tests for raster data providers and kept generic.
    To make use of it, subclass it and implement
    self.get_layer(test_id: str) -> QgsRasterLayer
    to a provider you want to test.
    """

    def test_no_fixed_size_provider(self):
        """
        Test property values for a provider which does not have the
        fixed size capability
        """
        l = self.get_layer("not_fixed_size")
        if l.dataProvider().capabilities() & Qgis.RasterInterfaceCapability.Size:
            return

        # non-fixed size raster providers should return 0 for size
        self.assertEqual(l.width(), 0)
        self.assertEqual(l.height(), 0)
        # and 1 for raster units per pixel
        self.assertEqual(l.rasterUnitsPerPixelX(), 1)
        self.assertEqual(l.rasterUnitsPerPixelY(), 1)

    def test_fixed_size_provider(self):
        """
        Test property values for a provider which does has the
        fixed size capability
        """
        l = self.get_layer("fixed_size")
        if not l.dataProvider().capabilities() & Qgis.RasterInterfaceCapability.Size:
            return

        # TODO -- adjust these values after we've got a generic "reference" fixed size
        # raster which is appropriate for these tests
        self.assertEqual(l.width(), 3)
        self.assertEqual(l.height(), 4)
        # and 1 for raster units per pixel
        self.assertAlmostEqual(l.rasterUnitsPerPixelX(), 0.000642531091049392, 8)
        self.assertAlmostEqual(l.rasterUnitsPerPixelY(), 0.00048366498497820487, 8)

    def test_provider_clone(self):
        """
        Test that cloning layer works
        """
        l = self.get_layer("clone")
        self.assertTrue(l.isValid())

        l2 = l.clone()
        # expect a new instance of the data provider
        self.assertNotEqual(l.dataProvider(), l2.dataProvider())
        # but should be same provider class
        self.assertEqual(l.dataProvider().name(), l2.dataProvider().name())
        # and same provider URI
        self.assertEqual(l.dataProvider().uri(), l2.dataProvider().uri())
