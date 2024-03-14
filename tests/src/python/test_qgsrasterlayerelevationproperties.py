"""QGIS Unit tests for QgsRasterLayerElevationProperties

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '09/11/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import os
import math

from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    Qgis,
    QgsFillSymbol,
    QgsLineSymbol,
    QgsRasterLayerElevationProperties,
    QgsReadWriteContext,
    QgsRasterLayer,
    QgsDoubleRange
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()


class TestQgsRasterLayerElevationProperties(QgisTestCase):

    def test_basic_elevation_surface(self):
        """
        Basic tests for the class using the RepresentsElevationSurface mode
        """
        props = QgsRasterLayerElevationProperties(None)
        self.assertEqual(props.mode(), Qgis.RasterElevationMode.RepresentsElevationSurface)
        self.assertEqual(props.zScale(), 1)
        self.assertEqual(props.zOffset(), 0)
        self.assertFalse(props.isEnabled())
        self.assertFalse(props.hasElevation())
        self.assertEqual(props.bandNumber(), 1)
        self.assertTrue(props.fixedRange().isInfinite())
        self.assertIsInstance(props.profileLineSymbol(), QgsLineSymbol)
        self.assertIsInstance(props.profileFillSymbol(), QgsFillSymbol)
        self.assertEqual(props.profileSymbology(), Qgis.ProfileSurfaceSymbology.Line)

        props.setZOffset(0.5)
        props.setZScale(2)
        props.setBandNumber(2)
        props.setEnabled(True)
        props.setProfileSymbology(Qgis.ProfileSurfaceSymbology.FillBelow)
        props.setElevationLimit(909)
        self.assertEqual(props.zScale(), 2)
        self.assertEqual(props.zOffset(), 0.5)
        self.assertTrue(props.isEnabled())
        self.assertEqual(props.bandNumber(), 2)
        self.assertTrue(props.hasElevation())
        self.assertEqual(props.profileSymbology(), Qgis.ProfileSurfaceSymbology.FillBelow)
        self.assertEqual(props.elevationLimit(), 909)

        sym = QgsLineSymbol.createSimple({'outline_color': '#ff4433', 'outline_width': 0.5})
        props.setProfileLineSymbol(sym)
        self.assertEqual(props.profileLineSymbol().color().name(), '#ff4433')

        sym = QgsFillSymbol.createSimple({'color': '#ff44ff'})
        props.setProfileFillSymbol(sym)
        self.assertEqual(props.profileFillSymbol().color().name(), '#ff44ff')

        doc = QDomDocument("testdoc")
        elem = doc.createElement('test')
        props.writeXml(elem, doc, QgsReadWriteContext())

        props2 = QgsRasterLayerElevationProperties(None)
        props2.readXml(elem, QgsReadWriteContext())
        self.assertEqual(props2.mode(), Qgis.RasterElevationMode.RepresentsElevationSurface)
        self.assertEqual(props2.zScale(), 2)
        self.assertEqual(props2.zOffset(), 0.5)
        self.assertTrue(props2.isEnabled())
        self.assertEqual(props2.bandNumber(), 2)
        self.assertEqual(props2.profileLineSymbol().color().name(), '#ff4433')
        self.assertEqual(props2.profileFillSymbol().color().name(), '#ff44ff')
        self.assertEqual(props2.profileSymbology(), Qgis.ProfileSurfaceSymbology.FillBelow)
        self.assertEqual(props2.elevationLimit(), 909)

        props2 = props.clone()
        self.assertEqual(props2.mode(), Qgis.RasterElevationMode.RepresentsElevationSurface)
        self.assertEqual(props2.zScale(), 2)
        self.assertEqual(props2.zOffset(), 0.5)
        self.assertTrue(props2.isEnabled())
        self.assertEqual(props2.bandNumber(), 2)
        self.assertEqual(props2.profileLineSymbol().color().name(), '#ff4433')
        self.assertEqual(props2.profileFillSymbol().color().name(), '#ff44ff')
        self.assertEqual(props2.profileSymbology(), Qgis.ProfileSurfaceSymbology.FillBelow)
        self.assertEqual(props2.elevationLimit(), 909)

    def test_basic_fixed_range(self):
        """
        Basic tests for the class using the FixedElevationRange mode
        """
        props = QgsRasterLayerElevationProperties(None)
        self.assertTrue(props.fixedRange().isInfinite())

        props.setMode(Qgis.RasterElevationMode.FixedElevationRange)
        props.setFixedRange(QgsDoubleRange(103.1, 106.8))
        # fixed ranges should not be affected by scale/offset
        props.setZOffset(0.5)
        props.setZScale(2)
        self.assertEqual(props.fixedRange(), QgsDoubleRange(103.1, 106.8))
        self.assertEqual(props.calculateZRange(None), QgsDoubleRange(103.1, 106.8))
        self.assertFalse(props.isVisibleInZRange(QgsDoubleRange(3.1, 6.8)))
        self.assertTrue(props.isVisibleInZRange(QgsDoubleRange(3.1, 104.8)))
        self.assertTrue(props.isVisibleInZRange(QgsDoubleRange(104.8, 114.8)))
        self.assertFalse(props.isVisibleInZRange(QgsDoubleRange(114.8, 124.8)))

        doc = QDomDocument("testdoc")
        elem = doc.createElement('test')
        props.writeXml(elem, doc, QgsReadWriteContext())

        props2 = QgsRasterLayerElevationProperties(None)
        props2.readXml(elem, QgsReadWriteContext())
        self.assertEqual(props2.mode(), Qgis.RasterElevationMode.FixedElevationRange)
        self.assertEqual(props2.fixedRange(), QgsDoubleRange(103.1, 106.8))

        props2 = props.clone()
        self.assertEqual(props2.mode(), Qgis.RasterElevationMode.FixedElevationRange)
        self.assertEqual(props2.fixedRange(), QgsDoubleRange(103.1, 106.8))

        # include lower, exclude upper
        props.setFixedRange(QgsDoubleRange(103.1, 106.8,
                                           includeLower=True,
                                           includeUpper=False))
        elem = doc.createElement('test')
        props.writeXml(elem, doc, QgsReadWriteContext())

        props2 = QgsRasterLayerElevationProperties(None)
        props2.readXml(elem, QgsReadWriteContext())
        self.assertEqual(props2.fixedRange(), QgsDoubleRange(103.1, 106.8,
                                                             includeLower=True,
                                                             includeUpper=False))

        # exclude lower, include upper
        props.setFixedRange(QgsDoubleRange(103.1, 106.8,
                                           includeLower=False,
                                           includeUpper=True))
        elem = doc.createElement('test')
        props.writeXml(elem, doc, QgsReadWriteContext())

        props2 = QgsRasterLayerElevationProperties(None)
        props2.readXml(elem, QgsReadWriteContext())
        self.assertEqual(props2.fixedRange(), QgsDoubleRange(103.1, 106.8,
                                                             includeLower=False,
                                                             includeUpper=True))

    def test_looks_like_dem(self):
        layer = QgsRasterLayer(
            os.path.join(unitTestDataPath(), 'landsat.tif'), 'i am not a dem')
        self.assertTrue(layer.isValid())

        # not like a dem, the layer has multiple bands
        self.assertFalse(
            QgsRasterLayerElevationProperties.layerLooksLikeDem(layer))

        # layer data type doesn't look like a dem
        layer = QgsRasterLayer(
            os.path.join(unitTestDataPath(), 'raster/band1_byte_ct_epsg4326.tif'), 'i am not a dem')
        self.assertTrue(layer.isValid())
        self.assertFalse(
            QgsRasterLayerElevationProperties.layerLooksLikeDem(layer))

        layer = QgsRasterLayer(
            os.path.join(unitTestDataPath(), 'landsat-f32-b1.tif'), 'my layer')
        self.assertTrue(layer.isValid())

        # not like a dem, the layer name doesn't hint this to
        self.assertFalse(QgsRasterLayerElevationProperties.layerLooksLikeDem(layer))
        layer.setName('i am a DEM')
        self.assertTrue(
            QgsRasterLayerElevationProperties.layerLooksLikeDem(layer))
        layer.setName('i am a raster')
        self.assertFalse(
            QgsRasterLayerElevationProperties.layerLooksLikeDem(layer))

        layer.setName('i am a aster satellite layer')
        self.assertTrue(
            QgsRasterLayerElevationProperties.layerLooksLikeDem(layer))

    def test_elevation_range_for_pixel_value(self):
        """
        Test transforming pixel values to elevation ranges
        """
        props = QgsRasterLayerElevationProperties(None)

        self.assertEqual(props.elevationRangeForPixelValue(band=1, pixelValue=3),
                         QgsDoubleRange())
        props.setEnabled(True)
        self.assertEqual(props.elevationRangeForPixelValue(band=1, pixelValue=3),
                         QgsDoubleRange(3,3))
        self.assertEqual(props.elevationRangeForPixelValue(band=1, pixelValue=math.nan),
                         QgsDoubleRange())

        # check that band number is respected
        props.setBandNumber(2)
        self.assertEqual(props.elevationRangeForPixelValue(band=1, pixelValue=3),
                         QgsDoubleRange())
        self.assertEqual(props.elevationRangeForPixelValue(band=2, pixelValue=3),
                         QgsDoubleRange(3,3))

        # check that offset/scale is respected
        props.setZOffset(0.5)
        props.setZScale(2)
        self.assertEqual(props.elevationRangeForPixelValue(band=2, pixelValue=3),
                         QgsDoubleRange(6.5, 6.5))

        # with fixed range mode
        props.setMode(Qgis.RasterElevationMode.FixedElevationRange)
        props.setFixedRange(QgsDoubleRange(11, 15))
        self.assertEqual(props.elevationRangeForPixelValue(band=1, pixelValue=math.nan),
                         QgsDoubleRange())
        self.assertEqual(props.elevationRangeForPixelValue(band=1, pixelValue=3),
                         QgsDoubleRange(11, 15))


if __name__ == '__main__':
    unittest.main()
