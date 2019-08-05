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
                       QgsRenderedFeatureHandlerInterface)
from qgis.PyQt.QtCore import QSize
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

    def testCopyConstructor(self):
        """
        Test the copy constructor
        """
        c1 = QgsRenderContext()

        c1.setTextRenderFormat(QgsRenderContext.TextFormatAlwaysText)
        c1.setMapExtent(QgsRectangle(1, 2, 3, 4))

        c2 = QgsRenderContext(c1)
        self.assertEqual(c2.textRenderFormat(), QgsRenderContext.TextFormatAlwaysText)
        self.assertEqual(c2.mapExtent(), QgsRectangle(1, 2, 3, 4))

        c1.setTextRenderFormat(QgsRenderContext.TextFormatAlwaysOutlines)
        c2 = QgsRenderContext(c1)
        self.assertEqual(c2.textRenderFormat(), QgsRenderContext.TextFormatAlwaysOutlines)

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
        self.assertAlmostEqual(c.scaleFactor(), 88 / 25.4, 3)

        im = QImage(1000, 600, QImage.Format_RGB32)
        dots_per_m = 300 / 25.4 * 1000  # 300 dpi to dots per m
        im.setDotsPerMeterX(dots_per_m)
        im.setDotsPerMeterY(dots_per_m)
        p = QPainter(im)
        p.setRenderHint(QPainter.Antialiasing)
        c = QgsRenderContext.fromQPainter(p)
        self.assertEqual(c.painter(), p)
        self.assertEqual(c.testFlag(QgsRenderContext.Antialiasing), True)
        self.assertAlmostEqual(c.scaleFactor(), dots_per_m / 1000, 3)  # scaleFactor should be pixels/mm

    def testFromMapSettings(self):
        """
        test QgsRenderContext.fromMapSettings()
        """
        ms = QgsMapSettings()
        ms.setOutputSize(QSize(1000, 1000))
        ms.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:3111'))
        ms.setExtent(QgsRectangle(10000, 20000, 30000, 40000))

        ms.setTextRenderFormat(QgsRenderContext.TextFormatAlwaysText)
        rc = QgsRenderContext.fromMapSettings(ms)
        self.assertEqual(rc.textRenderFormat(), QgsRenderContext.TextFormatAlwaysText)

        ms.setTextRenderFormat(QgsRenderContext.TextFormatAlwaysOutlines)
        rc = QgsRenderContext.fromMapSettings(ms)
        self.assertEqual(rc.textRenderFormat(), QgsRenderContext.TextFormatAlwaysOutlines)

        self.assertEqual(rc.mapExtent(), QgsRectangle(10000, 20000, 30000, 40000))

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
        length_meter_mapunits = da_wsg84.measureLineProjected(point_berlin_wsg84, 1.0, (math.pi / 2))
        meters_test_mapunits = meters_test * length_wsg84_mapunits
        meters_test_pixel = meters_test * length_wsg84_mapunits
        ms = QgsMapSettings()
        ms.setDestinationCrs(crs_wsg84)
        ms.setExtent(rt_extent)
        r = QgsRenderContext.fromMapSettings(ms)
        r.setExtent(rt_extent)
        self.assertEqual(r.extent().center().toString(7), point_berlin_wsg84.toString(7))
        c = QgsMapUnitScale()
        r.setDistanceArea(da_wsg84)
        result_test_painterunits = r.convertToPainterUnits(meters_test, QgsUnitTypes.RenderMetersInMapUnits, c)
        self.assertEqual(QgsDistanceArea.formatDistance(result_test_painterunits, 7, QgsUnitTypes.DistanceUnknownUnit, True), QgsDistanceArea.formatDistance(meters_test_mapunits, 7, QgsUnitTypes.DistanceUnknownUnit, True))
        result_test_mapunits = r.convertToMapUnits(meters_test, QgsUnitTypes.RenderMetersInMapUnits, c)
        self.assertEqual(QgsDistanceArea.formatDistance(result_test_mapunits, 7, QgsUnitTypes.DistanceDegrees, True), QgsDistanceArea.formatDistance(meters_test_mapunits, 7, QgsUnitTypes.DistanceDegrees, True))
        result_test_meters = r.convertFromMapUnits(meters_test_mapunits, QgsUnitTypes.RenderMetersInMapUnits)
        self.assertEqual(QgsDistanceArea.formatDistance(result_test_meters, 1, QgsUnitTypes.DistanceMeters, True), QgsDistanceArea.formatDistance(meters_test, 1, QgsUnitTypes.DistanceMeters, True))

    def testConvertSingleUnit(self):

        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(0, 0, 100, 100))
        ms.setOutputSize(QSize(100, 50))
        ms.setOutputDpi(300)
        r = QgsRenderContext.fromMapSettings(ms)

        # renderer scale should be about 1:291937841

        # start with no min/max scale
        c = QgsMapUnitScale()
        #self.assertEqual(r.scaleFactor(),666)

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
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderMillimeters, c)
        self.assertAlmostEqual(size, 47.244094, places=5)
        size = r.convertToMapUnits(5.66929, QgsUnitTypes.RenderPoints, c)
        self.assertAlmostEqual(size, 47.2440833, places=5)
        size = r.convertToMapUnits(5.66929, QgsUnitTypes.RenderInches, c)
        self.assertAlmostEqual(size, 3401.574, places=5)
        size = r.convertToMapUnits(2, QgsUnitTypes.RenderPixels, c)
        self.assertAlmostEqual(size, 4.0, places=5)

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


if __name__ == '__main__':
    unittest.main()
