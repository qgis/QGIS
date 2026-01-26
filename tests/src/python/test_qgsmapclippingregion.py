"""QGIS Unit tests for QgsMapClippingRegion.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "2020-06"
__copyright__ = "Copyright 2020, The QGIS Project"


from qgis.core import QgsGeometry, QgsMapClippingRegion, QgsVectorLayer
from qgis.testing import unittest


class TestQgsMapClippingRegion(unittest.TestCase):

    def testGetSet(self):
        region = QgsMapClippingRegion(
            QgsGeometry.fromWkt("Polygon((0 0, 1 0, 1 1, 0 1, 0 0))")
        )
        self.assertEqual(
            region.geometry().asWkt(), "Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))"
        )

        region.setGeometry(
            QgsGeometry.fromWkt("Polygon((10 0, 11 0, 11 1, 10 1, 10 0))")
        )
        self.assertEqual(
            region.geometry().asWkt(), "Polygon ((10 0, 11 0, 11 1, 10 1, 10 0))"
        )

        self.assertEqual(
            region.featureClip(),
            QgsMapClippingRegion.FeatureClippingType.ClipToIntersection,
        )
        region.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.ClipPainterOnly)
        self.assertEqual(
            region.featureClip(),
            QgsMapClippingRegion.FeatureClippingType.ClipPainterOnly,
        )

        layer = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer", "addfeat", "memory"
        )
        layer2 = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer", "addfeat", "memory"
        )
        self.assertEqual(len(region.restrictedLayers()), 0)
        region.setRestrictedLayers([layer, layer2])
        self.assertCountEqual(region.restrictedLayers(), [layer, layer2])
        region.setRestrictToLayers(False)
        self.assertFalse(region.restrictToLayers())
        region.setRestrictToLayers(True)
        self.assertTrue(region.restrictToLayers())

    def testAppliesToLayer(self):
        layer = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer", "addfeat", "memory"
        )
        layer2 = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer", "addfeat", "memory"
        )
        layer3 = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer", "addfeat", "memory"
        )

        region = QgsMapClippingRegion(
            QgsGeometry.fromWkt("Polygon((0 0, 1 0, 1 1, 0 1, 0 0))")
        )
        # should apply to all layers by default
        self.assertTrue(region.appliesToLayer(layer))
        self.assertTrue(region.appliesToLayer(layer2))
        self.assertTrue(region.appliesToLayer(layer3))

        region.setRestrictedLayers([layer, layer2])
        self.assertTrue(region.appliesToLayer(layer))
        self.assertTrue(region.appliesToLayer(layer2))
        self.assertTrue(region.appliesToLayer(layer3))

        region.setRestrictToLayers(True)
        self.assertTrue(region.appliesToLayer(layer))
        self.assertTrue(region.appliesToLayer(layer2))
        self.assertFalse(region.appliesToLayer(layer3))

        region.setRestrictedLayers([])
        self.assertFalse(region.appliesToLayer(layer))
        self.assertFalse(region.appliesToLayer(layer2))
        self.assertFalse(region.appliesToLayer(layer3))


if __name__ == "__main__":
    unittest.main()
