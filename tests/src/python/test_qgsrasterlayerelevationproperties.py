"""QGIS Unit tests for QgsRasterLayerElevationProperties

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '09/11/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import math
import os
import unittest

from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    Qgis,
    QgsFillSymbol,
    QgsLineSymbol,
    QgsRasterLayerElevationProperties,
    QgsReadWriteContext,
    QgsRasterLayer,
    QgsDoubleRange,
    QgsProperty
)
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()


class TestQgsRasterLayerElevationProperties(QgisTestCase):

    def test_basic_elevation_surface(self):
        """
        Basic tests for the class using the RepresentsElevationSurface mode
        """
        props = QgsRasterLayerElevationProperties(None)
        self.assertEqual(props.mode(),
                         Qgis.RasterElevationMode.RepresentsElevationSurface)
        self.assertEqual(props.zScale(), 1)
        self.assertEqual(props.zOffset(), 0)
        self.assertFalse(props.isEnabled())
        self.assertFalse(props.hasElevation())
        self.assertEqual(props.bandNumber(), 1)
        self.assertTrue(props.fixedRange().isInfinite())
        self.assertIsInstance(props.profileLineSymbol(), QgsLineSymbol)
        self.assertIsInstance(props.profileFillSymbol(), QgsFillSymbol)
        self.assertEqual(props.profileSymbology(),
                         Qgis.ProfileSurfaceSymbology.Line)

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
        self.assertEqual(props.profileSymbology(),
                         Qgis.ProfileSurfaceSymbology.FillBelow)
        self.assertEqual(props.elevationLimit(), 909)

        sym = QgsLineSymbol.createSimple(
            {'outline_color': '#ff4433', 'outline_width': 0.5})
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
        self.assertEqual(props2.mode(),
                         Qgis.RasterElevationMode.RepresentsElevationSurface)
        self.assertEqual(props2.zScale(), 2)
        self.assertEqual(props2.zOffset(), 0.5)
        self.assertTrue(props2.isEnabled())
        self.assertEqual(props2.bandNumber(), 2)
        self.assertEqual(props2.profileLineSymbol().color().name(), '#ff4433')
        self.assertEqual(props2.profileFillSymbol().color().name(), '#ff44ff')
        self.assertEqual(props2.profileSymbology(),
                         Qgis.ProfileSurfaceSymbology.FillBelow)
        self.assertEqual(props2.elevationLimit(), 909)

        props2 = props.clone()
        self.assertEqual(props2.mode(),
                         Qgis.RasterElevationMode.RepresentsElevationSurface)
        self.assertEqual(props2.zScale(), 2)
        self.assertEqual(props2.zOffset(), 0.5)
        self.assertTrue(props2.isEnabled())
        self.assertEqual(props2.bandNumber(), 2)
        self.assertEqual(props2.profileLineSymbol().color().name(), '#ff4433')
        self.assertEqual(props2.profileFillSymbol().color().name(), '#ff44ff')
        self.assertEqual(props2.profileSymbology(),
                         Qgis.ProfileSurfaceSymbology.FillBelow)
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
        self.assertEqual(props.calculateZRange(None),
                         QgsDoubleRange(103.1, 106.8))
        self.assertFalse(props.isVisibleInZRange(QgsDoubleRange(3.1, 6.8)))
        self.assertTrue(props.isVisibleInZRange(QgsDoubleRange(3.1, 104.8)))
        self.assertTrue(props.isVisibleInZRange(QgsDoubleRange(104.8, 114.8)))
        self.assertFalse(props.isVisibleInZRange(QgsDoubleRange(114.8, 124.8)))

        doc = QDomDocument("testdoc")
        elem = doc.createElement('test')
        props.writeXml(elem, doc, QgsReadWriteContext())

        props2 = QgsRasterLayerElevationProperties(None)
        props2.readXml(elem, QgsReadWriteContext())
        self.assertEqual(props2.mode(),
                         Qgis.RasterElevationMode.FixedElevationRange)
        self.assertEqual(props2.fixedRange(), QgsDoubleRange(103.1, 106.8))

        props2 = props.clone()
        self.assertEqual(props2.mode(),
                         Qgis.RasterElevationMode.FixedElevationRange)
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

    def test_basic_fixed_range_per_band(self):
        """
        Basic tests for the class using the FixedRangePerBand mode
        """
        props = QgsRasterLayerElevationProperties(None)
        self.assertFalse(props.fixedRangePerBand())

        props.setMode(Qgis.RasterElevationMode.FixedRangePerBand)
        props.setFixedRangePerBand({1: QgsDoubleRange(103.1, 106.8),
                                   2: QgsDoubleRange(106.8, 116.8),
                                   3: QgsDoubleRange(116.8, 126.8)})
        # fixed ranges should not be affected by scale/offset
        props.setZOffset(0.5)
        props.setZScale(2)
        self.assertEqual(props.fixedRangePerBand(), {1: QgsDoubleRange(103.1, 106.8),
                                                     2: QgsDoubleRange(106.8, 116.8),
                                                     3: QgsDoubleRange(116.8, 126.8)})
        self.assertEqual(props.calculateZRange(None),
                         QgsDoubleRange(103.1, 126.8))
        self.assertFalse(props.isVisibleInZRange(QgsDoubleRange(3.1, 6.8)))
        self.assertTrue(props.isVisibleInZRange(QgsDoubleRange(3.1, 104.8)))
        self.assertTrue(props.isVisibleInZRange(QgsDoubleRange(104.8, 114.8)))
        self.assertTrue(props.isVisibleInZRange(QgsDoubleRange(114.8, 124.8)))
        self.assertFalse(props.isVisibleInZRange(QgsDoubleRange(128.8, 134.8)))
        self.assertEqual(
            props.bandForElevationRange(None, QgsDoubleRange(1, 2)), -1)
        self.assertEqual(
            props.bandForElevationRange(None, QgsDoubleRange(103, 104)), 1)
        self.assertEqual(
            props.bandForElevationRange(None, QgsDoubleRange(104, 108)), 2)
        self.assertEqual(
            props.bandForElevationRange(None, QgsDoubleRange(112, 112)), 2)
        self.assertEqual(
            props.bandForElevationRange(None, QgsDoubleRange(112, 118)), 3)
        self.assertEqual(
            props.bandForElevationRange(None, QgsDoubleRange(118, 218)), 3)
        self.assertEqual(
            props.bandForElevationRange(None, QgsDoubleRange(212, 218)), -1)

        doc = QDomDocument("testdoc")
        elem = doc.createElement('test')
        props.writeXml(elem, doc, QgsReadWriteContext())

        props2 = QgsRasterLayerElevationProperties(None)
        props2.readXml(elem, QgsReadWriteContext())
        self.assertEqual(props2.mode(),
                         Qgis.RasterElevationMode.FixedRangePerBand)
        self.assertEqual(props2.fixedRangePerBand(), {1: QgsDoubleRange(103.1, 106.8),
                                                      2: QgsDoubleRange(106.8, 116.8),
                                                      3: QgsDoubleRange(116.8, 126.8)})

        props2 = props.clone()
        self.assertEqual(props2.mode(),
                         Qgis.RasterElevationMode.FixedRangePerBand)
        self.assertEqual(props2.fixedRangePerBand(), {1: QgsDoubleRange(103.1, 106.8),
                                                      2: QgsDoubleRange(106.8, 116.8),
                                                      3: QgsDoubleRange(116.8, 126.8)})

        # include lower, exclude upper
        props.setFixedRangePerBand({1: QgsDoubleRange(103.1, 106.8,
                                                      includeLower=True,
                                                      includeUpper=False)})
        elem = doc.createElement('test')
        props.writeXml(elem, doc, QgsReadWriteContext())

        props2 = QgsRasterLayerElevationProperties(None)
        props2.readXml(elem, QgsReadWriteContext())
        self.assertEqual(props2.fixedRangePerBand(), {1: QgsDoubleRange(103.1, 106.8,
                                                                        includeLower=True,
                                                                        includeUpper=False)})

        # exclude lower, include upper
        props.setFixedRangePerBand({1: QgsDoubleRange(103.1, 106.8,
                                                      includeLower=False,
                                                      includeUpper=True)})
        elem = doc.createElement('test')
        props.writeXml(elem, doc, QgsReadWriteContext())

        props2 = QgsRasterLayerElevationProperties(None)
        props2.readXml(elem, QgsReadWriteContext())
        self.assertEqual(props2.fixedRangePerBand(), {1: QgsDoubleRange(103.1, 106.8,
                                                                        includeLower=False,
                                                                        includeUpper=True)})

    def test_basic_dynamic_range_per_band(self):
        """
        Basic tests for the class using the DynamicRangePerBand mode
        """
        raster_layer = QgsRasterLayer(os.path.join(unitTestDataPath(), 'landsat_4326.tif'))
        self.assertTrue(raster_layer.isValid())

        props = QgsRasterLayerElevationProperties(None)

        props.setMode(Qgis.RasterElevationMode.DynamicRangePerBand)
        props.dataDefinedProperties().setProperty(
            QgsRasterLayerElevationProperties.Property.RasterPerBandLowerElevation,
            QgsProperty.fromExpression("@band*2"))
        props.dataDefinedProperties().setProperty(
            QgsRasterLayerElevationProperties.Property.RasterPerBandUpperElevation,
            QgsProperty.fromExpression("@band*2 + 1"))
        # fixed ranges should not be affected by scale/offset
        props.setZOffset(0.5)
        props.setZScale(2)
        self.assertEqual(props.dataDefinedProperties().property(
            QgsRasterLayerElevationProperties.Property.RasterPerBandLowerElevation).asExpression(),
            '@band*2')
        self.assertEqual(props.dataDefinedProperties().property(
            QgsRasterLayerElevationProperties.Property.RasterPerBandUpperElevation).asExpression(),
            '@band*2 + 1')

        # layer is required to calculated z range
        self.assertEqual(props.calculateZRange(None), QgsDoubleRange())
        self.assertEqual(props.calculateZRange(raster_layer), QgsDoubleRange(2, 19))

        self.assertFalse(props.isVisibleInZRange(QgsDoubleRange(3.1, 6.8)))
        self.assertTrue(props.isVisibleInZRange(QgsDoubleRange(3.1, 6.8), raster_layer))
        self.assertFalse(
            props.isVisibleInZRange(QgsDoubleRange(1.1, 1.9), raster_layer))
        self.assertFalse(props.isVisibleInZRange(QgsDoubleRange(104.8, 114.8)))

        self.assertEqual(
            props.bandForElevationRange(None, QgsDoubleRange(1, 2)), -1)
        self.assertEqual(
            props.bandForElevationRange(None, QgsDoubleRange(3.1, 6.8)), -1)
        self.assertEqual(
            props.bandForElevationRange(raster_layer, QgsDoubleRange(3.1, 6.8)), 3)
        self.assertEqual(
            props.bandForElevationRange(raster_layer, QgsDoubleRange(13.1, 16.8)), 8)
        self.assertEqual(
            props.bandForElevationRange(raster_layer, QgsDoubleRange(113.1, 116.8)), -1)

        doc = QDomDocument("testdoc")
        elem = doc.createElement('test')
        props.writeXml(elem, doc, QgsReadWriteContext())

        props2 = QgsRasterLayerElevationProperties(None)
        props2.readXml(elem, QgsReadWriteContext())
        self.assertEqual(props2.mode(),
                         Qgis.RasterElevationMode.DynamicRangePerBand)
        self.assertEqual(props2.dataDefinedProperties().property(
            QgsRasterLayerElevationProperties.Property.RasterPerBandLowerElevation).asExpression(),
            '@band*2')
        self.assertEqual(props2.dataDefinedProperties().property(
            QgsRasterLayerElevationProperties.Property.RasterPerBandUpperElevation).asExpression(),
            '@band*2 + 1')

        props2 = props.clone()
        self.assertEqual(props2.mode(),
                         Qgis.RasterElevationMode.DynamicRangePerBand)
        self.assertEqual(
            props2.dataDefinedProperties().property(
                QgsRasterLayerElevationProperties.Property.RasterPerBandLowerElevation).asExpression(),
            '@band*2')

        self.assertEqual(props2.dataDefinedProperties().property(
            QgsRasterLayerElevationProperties.Property.RasterPerBandUpperElevation).asExpression(),
            '@band*2 + 1')

    def test_looks_like_dem(self):
        layer = QgsRasterLayer(
            os.path.join(unitTestDataPath(), 'landsat.tif'), 'i am not a dem')
        self.assertTrue(layer.isValid())

        # not like a dem, the layer has multiple bands
        self.assertFalse(
            QgsRasterLayerElevationProperties.layerLooksLikeDem(layer))

        # layer data type doesn't look like a dem
        layer = QgsRasterLayer(
            os.path.join(unitTestDataPath(),
                         'raster/band1_byte_ct_epsg4326.tif'),
            'i am not a dem')
        self.assertTrue(layer.isValid())
        self.assertFalse(
            QgsRasterLayerElevationProperties.layerLooksLikeDem(layer))

        layer = QgsRasterLayer(
            os.path.join(unitTestDataPath(), 'landsat-f32-b1.tif'), 'my layer')
        self.assertTrue(layer.isValid())

        # not like a dem, the layer name doesn't hint this to
        self.assertFalse(
            QgsRasterLayerElevationProperties.layerLooksLikeDem(layer))
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
        raster_layer = QgsRasterLayer(os.path.join(unitTestDataPath(), 'landsat_4326.tif'))
        self.assertTrue(raster_layer.isValid())
        props = QgsRasterLayerElevationProperties(raster_layer)

        self.assertEqual(
            props.elevationRangeForPixelValue(layer=None, band=1, pixelValue=3),
            QgsDoubleRange())
        props.setEnabled(True)
        self.assertEqual(
            props.elevationRangeForPixelValue(layer=None, band=1, pixelValue=3),
            QgsDoubleRange(3, 3))
        self.assertEqual(
            props.elevationRangeForPixelValue(layer=None, band=1, pixelValue=math.nan),
            QgsDoubleRange())

        # check that band number is respected
        props.setBandNumber(2)
        self.assertEqual(
            props.elevationRangeForPixelValue(layer=None, band=1, pixelValue=3),
            QgsDoubleRange())
        self.assertEqual(
            props.elevationRangeForPixelValue(layer=None, band=2, pixelValue=3),
            QgsDoubleRange(3, 3))

        # check that offset/scale is respected
        props.setZOffset(0.5)
        props.setZScale(2)
        self.assertEqual(
            props.elevationRangeForPixelValue(layer=None, band=2, pixelValue=3),
            QgsDoubleRange(6.5, 6.5))

        # with fixed range mode
        props.setMode(Qgis.RasterElevationMode.FixedElevationRange)
        props.setFixedRange(QgsDoubleRange(11, 15))
        self.assertEqual(
            props.elevationRangeForPixelValue(layer=None, band=1, pixelValue=math.nan),
            QgsDoubleRange())
        self.assertEqual(
            props.elevationRangeForPixelValue(layer=None, band=1, pixelValue=3),
            QgsDoubleRange(11, 15))

        # with fixed range per band mode
        props.setMode(Qgis.RasterElevationMode.FixedRangePerBand)
        props.setFixedRangePerBand({1: QgsDoubleRange(11, 15),
                                   2: QgsDoubleRange(16, 25)})
        self.assertEqual(
            props.elevationRangeForPixelValue(layer=None, band=1, pixelValue=math.nan),
            QgsDoubleRange())
        self.assertEqual(
            props.elevationRangeForPixelValue(layer=None, band=1, pixelValue=3),
            QgsDoubleRange(11, 15))
        self.assertEqual(
            props.elevationRangeForPixelValue(layer=None, band=2, pixelValue=3),
            QgsDoubleRange(16, 25))

        # with dynamic range per band mode
        props.setMode(Qgis.RasterElevationMode.DynamicRangePerBand)
        props.dataDefinedProperties().setProperty(
            QgsRasterLayerElevationProperties.Property.RasterPerBandLowerElevation,
            QgsProperty.fromExpression("@band*2"))
        props.dataDefinedProperties().setProperty(
            QgsRasterLayerElevationProperties.Property.RasterPerBandUpperElevation,
            QgsProperty.fromExpression("@band*2 + 1"))
        self.assertEqual(
            props.elevationRangeForPixelValue(layer=None, band=1, pixelValue=math.nan),
            QgsDoubleRange())
        self.assertEqual(
            props.elevationRangeForPixelValue(layer=raster_layer, band=1, pixelValue=3),
            QgsDoubleRange(2, 3))
        self.assertEqual(
            props.elevationRangeForPixelValue(layer=raster_layer, band=2, pixelValue=3),
            QgsDoubleRange(4, 5))
        self.assertEqual(
            props.elevationRangeForPixelValue(layer=raster_layer, band=100, pixelValue=3),
            QgsDoubleRange())


if __name__ == '__main__':
    unittest.main()
