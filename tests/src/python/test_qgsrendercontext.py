# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsRenderContext.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '16/01/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import qgis  # NOQA

from qgis.core import (QgsRenderContext,
                       QgsMapSettings,
                       QgsDistanceArea,
                       QgsRectangle, QgsPointXY,
                       QgsCoordinateReferenceSystem,
                       QgsMapUnitScale,
                       QgsUnitTypes,
                       QgsProject,
                       QgsRectangle,
                       QgsVectorSimplifyMethod,
                       QgsRenderedFeatureHandlerInterface,
                       QgsDateTimeRange,
                       QgsMapClippingRegion,
                       QgsGeometry,
                       QgsDoubleRange,
                       Qgis)
from qgis.PyQt.QtCore import QSize, QDateTime
from qgis.PyQt.QtGui import QPainter, QImage
from qgis.testing import start_app, unittest
import math

# Convenience instances in case you may need them
# to find the srs.db
start_app()


class TestFeatureHandler(QgsRenderedFeatureHandlerInterface):

    def handleRenderedFeature(self, feature, geometry, context):
        pass


class TestQgsRenderContext(unittest.TestCase):

    def testGettersSetters(self):
        """
        Basic getter/setter tests
        """
        c = QgsRenderContext()

        c.setTextRenderFormat(QgsRenderContext.TextFormatAlwaysText)
        self.assertEqual(c.textRenderFormat(), QgsRenderContext.TextFormatAlwaysText)
        c.setTextRenderFormat(QgsRenderContext.TextFormatAlwaysOutlines)
        self.assertEqual(c.textRenderFormat(), QgsRenderContext.TextFormatAlwaysOutlines)

        c.setMapExtent(QgsRectangle(1, 2, 3, 4))
        self.assertEqual(c.mapExtent(), QgsRectangle(1, 2, 3, 4))

        self.assertTrue(c.zRange().isInfinite())
        c.setZRange(QgsDoubleRange(1, 10))
        self.assertEqual(c.zRange(), QgsDoubleRange(1, 10))

        self.assertEqual(c.symbologyReferenceScale(), -1)
        c.setSymbologyReferenceScale(1000)
        self.assertEqual(c.symbologyReferenceScale(), 1000)

        self.assertTrue(c.outputSize().isEmpty())
        c.setOutputSize(QSize(100, 200))
        self.assertEqual(c.outputSize(), QSize(100, 200))

        self.assertEqual(c.devicePixelRatio(), 1)
        c.setDevicePixelRatio(2)
        self.assertEqual(c.devicePixelRatio(), 2)
        self.assertEqual(c.deviceOutputSize(), QSize(200, 400))

        c.setImageFormat(QImage.Format_Alpha8)
        self.assertEqual(c.imageFormat(), QImage.Format_Alpha8)

        # should have an invalid mapToPixel by default
        self.assertFalse(c.mapToPixel().isValid())

        self.assertEqual(c.frameRate(), -1)
        c.setFrameRate(30)
        self.assertEqual(c.frameRate(), 30)

        self.assertEqual(c.currentFrame(), -1)
        c.setCurrentFrame(6)
        self.assertEqual(c.currentFrame(), 6)

    def testCopyConstructor(self):
        """
        Test the copy constructor
        """
        c1 = QgsRenderContext()

        c1.setTextRenderFormat(QgsRenderContext.TextFormatAlwaysText)
        c1.setMapExtent(QgsRectangle(1, 2, 3, 4))
        c1.setZRange(QgsDoubleRange(1, 10))
        c1.setSymbologyReferenceScale(1000)
        c1.setOutputSize(QSize(100, 200))
        c1.setImageFormat(QImage.Format_Alpha8)
        c1.setDevicePixelRatio(2)
        c1.setFrameRate(30)
        c1.setCurrentFrame(6)

        c2 = QgsRenderContext(c1)
        self.assertEqual(c2.textRenderFormat(), QgsRenderContext.TextFormatAlwaysText)
        self.assertEqual(c2.mapExtent(), QgsRectangle(1, 2, 3, 4))
        self.assertEqual(c2.zRange(), QgsDoubleRange(1, 10))
        self.assertEqual(c2.symbologyReferenceScale(), 1000)
        self.assertEqual(c2.outputSize(), QSize(100, 200))
        self.assertEqual(c2.imageFormat(), QImage.Format_Alpha8)
        self.assertEqual(c2.devicePixelRatio(), 2)
        self.assertEqual(c2.deviceOutputSize(), QSize(200, 400))
        self.assertEqual(c2.frameRate(), 30)
        self.assertEqual(c2.currentFrame(), 6)

        c1.setTextRenderFormat(QgsRenderContext.TextFormatAlwaysOutlines)
        c2 = QgsRenderContext(c1)
        self.assertEqual(c2.textRenderFormat(), QgsRenderContext.TextFormatAlwaysOutlines)

        c1.setIsTemporal(True)
        c1.setTemporalRange(QgsDateTimeRange(QDateTime(2020, 1, 1, 0, 0), QDateTime(2010, 12, 31, 23, 59)))
        c2 = QgsRenderContext(c1)

        self.assertEqual(c2.isTemporal(), True)
        self.assertEqual(c2.temporalRange(),
                         QgsDateTimeRange(QDateTime(2020, 1, 1, 0, 0), QDateTime(2010, 12, 31, 23, 59)))

    def testFromQPainter(self):
        """ test QgsRenderContext.fromQPainter """

        # no painter
        c = QgsRenderContext.fromQPainter(None)
        self.assertFalse(c.painter())
        # assuming 88 dpi as fallback
        self.assertAlmostEqual(c.scaleFactor(), 88 / 25.4, 3)

        # no painter destination
        p = QPainter()
        c = QgsRenderContext.fromQPainter(p)
        self.assertEqual(c.painter(), p)
        self.assertEqual(c.testFlag(QgsRenderContext.Antialiasing), False)
        self.assertEqual(c.testFlag(QgsRenderContext.LosslessImageRendering), False)
        self.assertAlmostEqual(c.scaleFactor(), 88 / 25.4, 3)

        # should have an invalid mapToPixel by default
        self.assertFalse(c.mapToPixel().isValid())

        im = QImage(1000, 600, QImage.Format_RGB32)
        dots_per_m = int(300 / 25.4 * 1000)  # 300 dpi to dots per m
        im.setDotsPerMeterX(dots_per_m)
        im.setDotsPerMeterY(dots_per_m)
        p = QPainter(im)
        p.setRenderHint(QPainter.Antialiasing)
        try:
            p.setRenderHint(QPainter.LosslessImageRendering)
            supports_lossless = True
        except AttributeError:
            supports_lossless = False

        c = QgsRenderContext.fromQPainter(p)
        self.assertEqual(c.painter(), p)
        self.assertEqual(c.testFlag(QgsRenderContext.Antialiasing), True)
        self.assertEqual(c.testFlag(QgsRenderContext.LosslessImageRendering), supports_lossless)
        self.assertAlmostEqual(c.scaleFactor(), dots_per_m / 1000, 3)  # scaleFactor should be pixels/mm

    def testFromMapSettings(self):
        """
        test QgsRenderContext.fromMapSettings()
        """
        ms = QgsMapSettings()
        ms.setOutputSize(QSize(1000, 1000))
        ms.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:3111'))
        ms.setExtent(QgsRectangle(10000, 20000, 30000, 40000))
        ms.setFlag(QgsMapSettings.Antialiasing, True)
        ms.setFlag(QgsMapSettings.LosslessImageRendering, True)
        ms.setFlag(QgsMapSettings.Render3DMap, True)
        ms.setZRange(QgsDoubleRange(1, 10))
        ms.setOutputSize(QSize(100, 100))
        ms.setDevicePixelRatio(2)
        ms.setOutputImageFormat(QImage.Format_Alpha8)
        ms.setFrameRate(30)
        ms.setCurrentFrame(6)

        ms.setTextRenderFormat(QgsRenderContext.TextFormatAlwaysText)
        rc = QgsRenderContext.fromMapSettings(ms)
        self.assertEqual(rc.textRenderFormat(), QgsRenderContext.TextFormatAlwaysText)
        self.assertTrue(rc.testFlag(QgsRenderContext.Antialiasing))
        self.assertTrue(rc.testFlag(QgsRenderContext.LosslessImageRendering))
        self.assertTrue(rc.testFlag(QgsRenderContext.Render3DMap))
        self.assertEqual(ms.zRange(), QgsDoubleRange(1, 10))
        self.assertEqual(rc.symbologyReferenceScale(), -1)
        self.assertEqual(rc.outputSize(), QSize(100, 100))
        self.assertEqual(rc.devicePixelRatio(), 2)
        self.assertEqual(rc.deviceOutputSize(), QSize(200, 200))
        self.assertEqual(rc.imageFormat(), QImage.Format_Alpha8)
        self.assertEqual(rc.frameRate(), 30)
        self.assertEqual(rc.currentFrame(), 6)

        # should have an valid mapToPixel
        self.assertTrue(rc.mapToPixel().isValid())

        ms.setTextRenderFormat(QgsRenderContext.TextFormatAlwaysOutlines)
        ms.setZRange(QgsDoubleRange())
        rc = QgsRenderContext.fromMapSettings(ms)
        self.assertEqual(rc.textRenderFormat(), QgsRenderContext.TextFormatAlwaysOutlines)
        self.assertTrue(ms.zRange().isInfinite())

        self.assertEqual(rc.mapExtent(), QgsRectangle(10000, 20000, 30000, 40000))

        ms.setIsTemporal(True)
        rc = QgsRenderContext.fromMapSettings(ms)
        self.assertEqual(rc.isTemporal(), True)

        ms.setTemporalRange(QgsDateTimeRange(QDateTime(2020, 1, 1, 0, 0), QDateTime(2010, 12, 31, 23, 59)))
        rc = QgsRenderContext.fromMapSettings(ms)
        self.assertEqual(rc.temporalRange(),
                         QgsDateTimeRange(QDateTime(2020, 1, 1, 0, 0), QDateTime(2010, 12, 31, 23, 59)))

        ms.setDpiTarget(111.1)
        rc = QgsRenderContext.fromMapSettings(ms)
        self.assertEqual(rc.dpiTarget(), 111.1)

    def testVectorSimplification(self):
        """
        Test vector simplification hints, ensure they are copied correctly from map settings
        """
        rc = QgsRenderContext()
        self.assertEqual(rc.vectorSimplifyMethod().simplifyHints(), QgsVectorSimplifyMethod.NoSimplification)

        ms = QgsMapSettings()

        rc = QgsRenderContext.fromMapSettings(ms)
        self.assertEqual(rc.vectorSimplifyMethod().simplifyHints(), QgsVectorSimplifyMethod.NoSimplification)
        rc2 = QgsRenderContext(rc)
        self.assertEqual(rc2.vectorSimplifyMethod().simplifyHints(), QgsVectorSimplifyMethod.NoSimplification)

        method = QgsVectorSimplifyMethod()
        method.setSimplifyHints(QgsVectorSimplifyMethod.GeometrySimplification)
        ms.setSimplifyMethod(method)

        rc = QgsRenderContext.fromMapSettings(ms)
        self.assertEqual(rc.vectorSimplifyMethod().simplifyHints(), QgsVectorSimplifyMethod.GeometrySimplification)

        rc2 = QgsRenderContext(rc)
        self.assertEqual(rc2.vectorSimplifyMethod().simplifyHints(), QgsVectorSimplifyMethod.GeometrySimplification)

    def testRenderedFeatureHandlers(self):
        rc = QgsRenderContext()
        self.assertFalse(rc.renderedFeatureHandlers())
        self.assertFalse(rc.hasRenderedFeatureHandlers())

        ms = QgsMapSettings()
        rc = QgsRenderContext.fromMapSettings(ms)
        self.assertFalse(rc.renderedFeatureHandlers())
        self.assertFalse(rc.hasRenderedFeatureHandlers())

        handler = TestFeatureHandler()
        handler2 = TestFeatureHandler()
        ms.addRenderedFeatureHandler(handler)
        ms.addRenderedFeatureHandler(handler2)

        rc = QgsRenderContext.fromMapSettings(ms)
        self.assertEqual(rc.renderedFeatureHandlers(), [handler, handler2])
        self.assertTrue(rc.hasRenderedFeatureHandlers())

        rc2 = QgsRenderContext(rc)
        self.assertEqual(rc2.renderedFeatureHandlers(), [handler, handler2])
        self.assertTrue(rc2.hasRenderedFeatureHandlers())

    def testRenderMetersInMapUnits(self):
        crs_wsg84 = QgsCoordinateReferenceSystem.fromOgcWmsCrs('EPSG:4326')
        rt_extent = QgsRectangle(13.37768985634235, 52.51625705830762, 13.37771931686235, 52.51628651882762)
        point_berlin_wsg84 = QgsPointXY(13.37770458660236, 52.51627178856762)
        length_wsg84_mapunits = 0.00001473026350140572
        meters_test = 2.40
        da_wsg84 = QgsDistanceArea()
        da_wsg84.setSourceCrs(crs_wsg84, QgsProject.instance().transformContext())
        if (da_wsg84.sourceCrs().isGeographic()):
            da_wsg84.setEllipsoid(da_wsg84.sourceCrs().ellipsoidAcronym())
        meters_test_mapunits = meters_test * length_wsg84_mapunits
        ms = QgsMapSettings()
        ms.setDestinationCrs(crs_wsg84)
        ms.setExtent(rt_extent)
        ms.setOutputSize(QSize(50, 50))
        r = QgsRenderContext.fromMapSettings(ms)
        r.setExtent(rt_extent)
        self.assertEqual(r.extent().center().toString(7), point_berlin_wsg84.toString(7))
        c = QgsMapUnitScale()
        r.setDistanceArea(da_wsg84)
        result_test_painterunits = r.convertToPainterUnits(meters_test, QgsUnitTypes.RenderMetersInMapUnits, c)
        self.assertAlmostEqual(result_test_painterunits, 60.0203759, 1)
        result_test_painterunits = r.convertToPainterUnits(-meters_test, QgsUnitTypes.RenderMetersInMapUnits, c)
        self.assertAlmostEqual(result_test_painterunits, -60.0203759, 1)
        result_test_mapunits = r.convertToMapUnits(meters_test, QgsUnitTypes.RenderMetersInMapUnits, c)
        self.assertEqual(QgsDistanceArea.formatDistance(result_test_mapunits, 7, QgsUnitTypes.DistanceDegrees, True),
                         QgsDistanceArea.formatDistance(meters_test_mapunits, 7, QgsUnitTypes.DistanceDegrees, True))
        result_test_mapunits = r.convertToMapUnits(-meters_test, QgsUnitTypes.RenderMetersInMapUnits, c)
        self.assertEqual(QgsDistanceArea.formatDistance(result_test_mapunits, 7, QgsUnitTypes.DistanceDegrees, True),
                         QgsDistanceArea.formatDistance(-meters_test_mapunits, 7, QgsUnitTypes.DistanceDegrees, True))
        result_test_meters = r.convertFromMapUnits(meters_test_mapunits, QgsUnitTypes.RenderMetersInMapUnits)
        self.assertEqual(QgsDistanceArea.formatDistance(result_test_meters, 1, QgsUnitTypes.DistanceMeters, True),
                         QgsDistanceArea.formatDistance(meters_test, 1, QgsUnitTypes.DistanceMeters, True))

        # attempting to convert to meters in map units when no extent is available should fallback to a very
        # approximate degrees -> meters conversion
        r.setExtent(QgsRectangle())
        self.assertAlmostEqual(r.convertToPainterUnits(5555, QgsUnitTypes.RenderMetersInMapUnits), 84692, -10)

    def testConvertSingleUnit(self):
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 100, 100))
        ms.setOutputSize(QSize(100, 50))
        ms.setOutputDpi(300)
        r = QgsRenderContext.fromMapSettings(ms)

        # renderer scale should be about 1:291937841

        # start with no min/max scale
        c = QgsMapUnitScale()
        # self.assertEqual(r.scaleFactor(),666)

        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(sf, 0.5, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(sf, 11.8110236, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(sf, 4.166666665625, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(sf, 300.0, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(sf, 1.0, places=5)

        # minimum scale greater than the renderer scale, so should be limited to minScale
        c.minScale = 150000000.0
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(sf, 3.89250455, places=5)
        # only conversion from mapunits should be affected
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(sf, 11.8110236, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(sf, 4.166666665625, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(sf, 300.0, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(sf, 1.0, places=5)
        c.minScale = 0

        # maximum scale less than the renderer scale, so should be limited to maxScale
        c.maxScale = 350000000.0
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(sf, 0.5, places=5)
        # only conversion from mapunits should be affected
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(sf, 11.8110236, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(sf, 4.166666665625, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(sf, 300.0, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(sf, 1.0, places=5)

        # with symbologyReferenceScale set
        c = QgsMapUnitScale()
        r.setSymbologyReferenceScale(1000)
        r.setRendererScale(1000)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(sf, 0.5, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(sf, 11.8110236, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(sf, 4.166666665625, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(sf, 300.0, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(sf, 1.0, places=5)

        r.setRendererScale(2000)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(sf, 0.5 / 2, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(sf, 11.8110236 / 2, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(sf, 4.166666665625 / 2, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(sf, 300.0 / 2, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(sf, 1.0 / 2, places=5)

        r.setRendererScale(500)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(sf, 0.5 * 2, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(sf, 11.8110236 * 2, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(sf, 4.166666665625 * 2, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(sf, 300.0 * 2, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(sf, 1.0 * 2, places=5)

    def testConvertToPainterUnits(self):
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 100, 100))
        ms.setOutputSize(QSize(100, 50))
        ms.setOutputDpi(300)
        r = QgsRenderContext.fromMapSettings(ms)

        # renderer scale should be about 1:291937841

        # start with no min/max scale
        c = QgsMapUnitScale()

        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(size, 1.0, places=5)
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(size, 23.622047, places=5)
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(size, 8.33333333125, places=5)
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(size, 600.0, places=5)
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(size, 2.0, places=5)

        # minimum size greater than the calculated size, so size should be limited to minSizeMM
        c.minSizeMM = 5
        c.minSizeMMEnabled = True
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(size, 59.0551181, places=5)
        # only conversion from mapunits should be affected
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(size, 23.622047, places=5)
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(size, 8.33333333125, places=5)
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(size, 600.0, places=5)
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(size, 2.0, places=5)
        c.minSizeMMEnabled = False

        # maximum size less than the calculated size, so size should be limited to maxSizeMM
        c.maxSizeMM = 0.1
        c.maxSizeMMEnabled = True
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(size, 1.0, places=5)
        # only conversion from mapunits should be affected
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(size, 23.622047, places=5)
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(size, 8.33333333125, places=5)
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(size, 600.0, places=5)
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(size, 2.0, places=5)

    def testConvertToPainterUnitsNoMapToPixel(self):
        """
        Test converting map unit based sizes to painter units when render context has NO map to pixel set
        """
        r = QgsRenderContext()
        r.setScaleFactor(300 / 25.4)  # 300 dpi, to match above test

        # start with no min/max scale
        c = QgsMapUnitScale()

        # since we have no map scale to work with, this makes the gross assumption that map units == points. It's magic, but
        # what else can we do?
        size = r.convertToPainterUnits(10, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(size, 41.66666, places=3)
        size = r.convertToPainterUnits(10, QgsUnitTypes.RenderMetersInMapUnits, c)
        self.assertAlmostEqual(size, 41.66666, places=3)

        # sizes should be clamped to reasonable range -- we don't want to treat 2000m map unit sizes as 10 million pixels!
        size = r.convertToPainterUnits(2000, QgsUnitTypes.RenderMapUnits, c)
        self.assertEqual(size, 100.0)
        size = r.convertToPainterUnits(2000, QgsUnitTypes.RenderMetersInMapUnits, c)
        self.assertEqual(size, 100.0)
        size = r.convertToPainterUnits(0.0002, QgsUnitTypes.RenderMapUnits, c)
        self.assertEqual(size, 10.0)
        size = r.convertToPainterUnits(0.0002, QgsUnitTypes.RenderMetersInMapUnits, c)
        self.assertEqual(size, 10.0)

        # normal units, should not be affected
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(size, 23.622047, places=5)
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(size, 8.33333333125, places=5)
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(size, 600.0, places=5)
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(size, 2.0, places=5)

        # minimum size greater than the calculated size, so size should be limited to minSizeMM
        c.minSizeMM = 5
        c.minSizeMMEnabled = True
        size = r.convertToPainterUnits(2, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(size, 59.0551181, places=5)

        # maximum size less than the calculated size, so size should be limited to maxSizeMM
        c.maxSizeMM = 6
        c.maxSizeMMEnabled = True
        size = r.convertToPainterUnits(26, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(size, 70.866, places=2)

    def testConvertToPainterUnitsSpecialCases(self):
        """
        Tests special cases for convertToPainterUnits
        """
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 100, 100))
        ms.setOutputSize(QSize(100, 50))
        ms.setOutputDpi(300)
        r = QgsRenderContext.fromMapSettings(ms)
        c = QgsMapUnitScale()
        size = r.convertToPainterUnits(10, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(size, 118.11023622047244, places=5)

        r.setFlag(Qgis.RenderContextFlag.RenderSymbolPreview, False)
        size = r.convertToPainterUnits(10, QgsUnitTypes.RenderMillimeters, c, Qgis.RenderSubcomponentProperty.BlurSize)
        self.assertAlmostEqual(size, 118.11023622047244, places=5)
        size = r.convertToPainterUnits(10, QgsUnitTypes.RenderMillimeters, c, Qgis.RenderSubcomponentProperty.ShadowOffset)
        self.assertAlmostEqual(size, 118.11023622047244, places=5)
        size = r.convertToPainterUnits(10, QgsUnitTypes.RenderMillimeters, c, Qgis.RenderSubcomponentProperty.GlowSpread)
        self.assertAlmostEqual(size, 118.11023622047244, places=5)

        # subcomponents which should be size limited in symbol previews
        r.setFlag(Qgis.RenderContextFlag.RenderSymbolPreview, True)
        size = r.convertToPainterUnits(10, QgsUnitTypes.RenderMillimeters, c, Qgis.RenderSubcomponentProperty.BlurSize)
        self.assertAlmostEqual(size, 30.0, places=5)
        size = r.convertToPainterUnits(10, QgsUnitTypes.RenderMillimeters, c, Qgis.RenderSubcomponentProperty.ShadowOffset)
        self.assertAlmostEqual(size, 100.0, places=5)
        size = r.convertToPainterUnits(10, QgsUnitTypes.RenderMillimeters, c, Qgis.RenderSubcomponentProperty.GlowSpread)
        self.assertAlmostEqual(size, 50.0, places=5)

    def testConvertToMapUnits(self):
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 100, 100))
        ms.setOutputSize(QSize(100, 50))
        ms.setOutputDpi(300)
        r = QgsRenderContext.fromMapSettings(ms)

        # renderer scale should be about 1:291937841

        # start with no min/max scale
        c = QgsMapUnitScale()

        size = r.convertToMapUnits(2, QgsUnitTypes.RenderMapUnits, c)
        self.assertEqual(size, 2.0)
        self.assertEqual(r.convertFromMapUnits(size, QgsUnitTypes.RenderMapUnits), 2)
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(size, 47.244094, places=5)
        self.assertEqual(r.convertFromMapUnits(size, QgsUnitTypes.RenderMillimeters), 2)
        size = r.convertToMapUnits(5.66929, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(size, 47.2440833, places=5)
        self.assertAlmostEqual(r.convertFromMapUnits(size, QgsUnitTypes.RenderPoints), 5.66929, 4)
        size = r.convertToMapUnits(5.66929, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(size, 3401.574, places=5)
        self.assertAlmostEqual(r.convertFromMapUnits(size, QgsUnitTypes.RenderInches), 5.66929, 4)
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(size, 4.0, places=5)
        self.assertAlmostEqual(r.convertFromMapUnits(size, QgsUnitTypes.RenderPixels), 2, 4)

        # minimum size greater than the calculated size, so size should be limited to minSizeMM
        c.minSizeMM = 5
        c.minSizeMMEnabled = True
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(size, 118.1102362, places=5)
        # only conversion from mapunits should be affected
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(size, 47.244094, places=5)
        size = r.convertToMapUnits(5.66929, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(size, 47.2440833, places=5)
        size = r.convertToMapUnits(5.66929, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(size, 3401.574, places=5)
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(size, 4.0, places=5)
        c.minSizeMMEnabled = False

        # maximum size less than the calculated size, so size should be limited to maxSizeMM
        c.maxSizeMM = 0.05
        c.maxSizeMMEnabled = True
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(size, 1.1811023622047245, places=5)
        # only conversion from mapunits should be affected
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(size, 47.244094, places=5)
        size = r.convertToMapUnits(5.66929, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(size, 47.2440833, places=5)
        size = r.convertToMapUnits(5.66929, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(size, 3401.574, places=5)
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(size, 4.0, places=5)
        c.maxSizeMMEnabled = False

        # test with minimum scale set
        c.minScale = 150000000.0
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(size, 15.57001821, places=5)
        # only conversion from mapunits should be affected
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(size, 47.244094, places=5)
        size = r.convertToMapUnits(5.66929, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(size, 47.2440833, places=5)
        size = r.convertToMapUnits(5.66929, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(size, 3401.574, places=5)
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(size, 4.0, places=5)
        c.minScale = 0

        # test with maximum scale set
        c.maxScale = 1550000000.0
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(size, 1.50677595625, places=5)
        # only conversion from mapunits should be affected
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(size, 47.244094, places=5)
        size = r.convertToMapUnits(5.66929, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(size, 47.2440833, places=5)
        size = r.convertToMapUnits(5.66929, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(size, 3401.574, places=5)
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(size, 4.0, places=5)
        c.maxScale = 0

        # with symbology reference scale
        c = QgsMapUnitScale()
        r.setSymbologyReferenceScale(1000)
        r.setRendererScale(1000)

        size = r.convertToMapUnits(2, QgsUnitTypes.RenderMapUnits, c)
        self.assertEqual(size, 2.0)
        self.assertEqual(r.convertFromMapUnits(size, QgsUnitTypes.RenderMapUnits), 2)
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(size, 47.244094, places=5)
        self.assertEqual(r.convertFromMapUnits(size, QgsUnitTypes.RenderMillimeters), 2)
        size = r.convertToMapUnits(5.66929, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(size, 47.2440833, places=5)
        self.assertAlmostEqual(r.convertFromMapUnits(size, QgsUnitTypes.RenderPoints), 5.66929, 4)
        size = r.convertToMapUnits(5.66929, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(size, 3401.574, places=5)
        self.assertAlmostEqual(r.convertFromMapUnits(size, QgsUnitTypes.RenderInches), 5.66929, 4)
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(size, 4.0, places=5)
        self.assertAlmostEqual(r.convertFromMapUnits(size, QgsUnitTypes.RenderPixels), 2, 4)

        r.setRendererScale(2000)
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderMapUnits, c)
        self.assertEqual(size, 2.0)
        self.assertEqual(r.convertFromMapUnits(size, QgsUnitTypes.RenderMapUnits), 2)
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(size, 47.244094 * 2, places=5)
        self.assertEqual(r.convertFromMapUnits(size, QgsUnitTypes.RenderMillimeters), 2)
        size = r.convertToMapUnits(5.66929, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(size, 47.2440833 * 2, places=5)
        self.assertAlmostEqual(r.convertFromMapUnits(size, QgsUnitTypes.RenderPoints), 5.66929, 4)
        size = r.convertToMapUnits(5.66929, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(size, 3401.574 * 2, places=5)
        self.assertAlmostEqual(r.convertFromMapUnits(size, QgsUnitTypes.RenderInches), 5.66929, 4)
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(size, 4.0 * 2, places=5)
        self.assertAlmostEqual(r.convertFromMapUnits(size, QgsUnitTypes.RenderPixels), 2, 4)

        r.setRendererScale(500)
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderMapUnits, c)
        self.assertEqual(size, 2.0)
        self.assertEqual(r.convertFromMapUnits(size, QgsUnitTypes.RenderMapUnits), 2)
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(size, 47.244094 / 2, places=5)
        self.assertEqual(r.convertFromMapUnits(size, QgsUnitTypes.RenderMillimeters), 2)
        size = r.convertToMapUnits(5.66929, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(size, 47.2440833 / 2, places=5)
        self.assertAlmostEqual(r.convertFromMapUnits(size, QgsUnitTypes.RenderPoints), 5.66929, 4)
        size = r.convertToMapUnits(5.66929, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(size, 3401.574 / 2, places=5)
        self.assertAlmostEqual(r.convertFromMapUnits(size, QgsUnitTypes.RenderInches), 5.66929, 4)
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(size, 4.0 / 2, places=5)
        self.assertAlmostEqual(r.convertFromMapUnits(size, QgsUnitTypes.RenderPixels), 2, 4)

    def testPixelSizeScaleFactor(self):
        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 100, 100))
        ms.setOutputSize(QSize(100, 50))
        ms.setOutputDpi(300)
        r = QgsRenderContext.fromMapSettings(ms)

        # renderer scale should be about 1:291937841

        # start with no min/max scale
        c = QgsMapUnitScale()

        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(sf, 0.5, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(sf, 11.8110236, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(sf, 4.166666665625, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(sf, 300.0, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(sf, 1.0, places=5)

        # minimum scale greater than the renderer scale, so should be limited to minScale
        c.minScale = 150000000.0
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(sf, 3.8925045, places=5)
        # only conversion from mapunits should be affected
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(sf, 11.811023, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(sf, 4.166666665625, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(sf, 300.0, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(sf, 1.0, places=5)
        c.minScale = 0

        # maximum scale less than the renderer scale, so should be limited to maxScale
        c.maxScale = 350000000.0
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(sf, 0.5, places=5)
        # only conversion from mapunits should be affected
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(sf, 11.8110236, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(sf, 4.166666665625, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(sf, 300.0, places=5)
        sf = r.convertToPainterUnits(1, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(sf, 1.0, places=5)

    def testMapUnitScaleFactor(self):
        # test QgsSymbolLayerUtils::mapUnitScaleFactor() using QgsMapUnitScale

        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 100, 100))
        ms.setOutputSize(QSize(100, 50))
        ms.setOutputDpi(300)
        r = QgsRenderContext.fromMapSettings(ms)

        # renderer scale should be about 1:291937841

        c = QgsMapUnitScale()
        sf = r.convertToMapUnits(1, QgsUnitTypes.RenderMapUnits, c)
        self.assertAlmostEqual(sf, 1.0, places=5)
        sf = r.convertToMapUnits(1, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(sf, 23.622047, places=5)
        sf = r.convertToMapUnits(1, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(sf, 8.33333324723, places=5)
        sf = r.convertToMapUnits(1, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(sf, 600.0, places=5)
        sf = r.convertToMapUnits(1, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(sf, 2.0, places=5)

    def testCustomRenderingFlags(self):
        rc = QgsRenderContext()
        rc.setCustomRenderingFlag('myexport', True)
        rc.setCustomRenderingFlag('omitgeometries', 'points')
        self.assertTrue(rc.customRenderingFlags()['myexport'])
        self.assertEqual(rc.customRenderingFlags()['omitgeometries'], 'points')

        # test that custom flags are correctly copied from settings
        settings = QgsMapSettings()
        settings.setCustomRenderingFlag('myexport', True)
        settings.setCustomRenderingFlag('omitgeometries', 'points')
        rc = QgsRenderContext.fromMapSettings(settings)
        self.assertTrue(rc.customRenderingFlags()['myexport'])
        self.assertEqual(rc.customRenderingFlags()['omitgeometries'], 'points')

    def testTemporalState(self):
        rc = QgsRenderContext()
        self.assertEqual(rc.isTemporal(), False)
        self.assertIsNotNone(rc.temporalRange())

    def testClippingRegion(self):
        ms = QgsMapSettings()
        rc = QgsRenderContext.fromMapSettings(ms)
        self.assertFalse(rc.clippingRegions())
        ms.addClippingRegion(QgsMapClippingRegion(QgsGeometry.fromWkt('Polygon(( 0 0, 1 0 , 1 1 , 0 1, 0 0 ))')))
        ms.addClippingRegion(QgsMapClippingRegion(QgsGeometry.fromWkt('Polygon(( 10 0, 11 0 , 11 1 , 10 1, 10 0 ))')))
        rc = QgsRenderContext.fromMapSettings(ms)
        self.assertEqual(len(rc.clippingRegions()), 2)
        self.assertEqual(rc.clippingRegions()[0].geometry().asWkt(), 'Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))')
        self.assertEqual(rc.clippingRegions()[1].geometry().asWkt(), 'Polygon ((10 0, 11 0, 11 1, 10 1, 10 0))')

    def testFeatureClipGeometry(self):
        rc = QgsRenderContext()
        self.assertTrue(rc.featureClipGeometry().isNull())
        rc.setFeatureClipGeometry(QgsGeometry.fromWkt('Polygon(( 0 0, 1 0 , 1 1 , 0 1, 0 0 ))'))
        self.assertEqual(rc.featureClipGeometry().asWkt(), 'Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))')
        rc2 = QgsRenderContext(rc)
        self.assertEqual(rc2.featureClipGeometry().asWkt(), 'Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))')

    def testSetPainterFlags(self):
        rc = QgsRenderContext()
        p = QPainter()
        im = QImage(1000, 600, QImage.Format_RGB32)
        p.begin(im)
        rc.setPainterFlagsUsingContext(p)
        self.assertFalse(p.testRenderHint(QPainter.Antialiasing))
        try:
            self.assertFalse(p.testRenderHint(QPainter.LosslessImageRendering))
        except AttributeError:
            pass

        rc.setPainter(p)
        rc.setFlag(QgsRenderContext.Antialiasing, True)
        rc.setFlag(QgsRenderContext.LosslessImageRendering, True)
        rc.setPainterFlagsUsingContext(p)
        self.assertTrue(p.testRenderHint(QPainter.Antialiasing))
        try:
            self.assertTrue(p.testRenderHint(QPainter.LosslessImageRendering))
        except AttributeError:
            pass

        p.end()


if __name__ == '__main__':
    unittest.main()
