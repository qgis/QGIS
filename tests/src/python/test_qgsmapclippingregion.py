# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsMapClippingRegion.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2020-06'
__copyright__ = 'Copyright 2020, The QGIS Project'


import qgis  # NOQA

from qgis.testing import unittest
from qgis.core import (
    QgsMapClippingRegion,
    QgsGeometry
)


class TestQgsMapClippingRegion(unittest.TestCase):

    def testGetSet(self):
        region = QgsMapClippingRegion(QgsGeometry.fromWkt('Polygon((0 0, 1 0, 1 1, 0 1, 0 0))'))
        self.assertEqual(region.geometry().asWkt(), 'Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))')

        region.setGeometry(QgsGeometry.fromWkt('Polygon((10 0, 11 0, 11 1, 10 1, 10 0))'))
        self.assertEqual(region.geometry().asWkt(), 'Polygon ((10 0, 11 0, 11 1, 10 1, 10 0))')

        self.assertEqual(region.featureClip(), QgsMapClippingRegion.FeatureClippingType.Intersect)
        region.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.PainterClip)
        self.assertEqual(region.featureClip(), QgsMapClippingRegion.FeatureClippingType.PainterClip)


if __name__ == '__main__':
    unittest.main()
