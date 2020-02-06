# -*- coding: utf-8 -*-

"""
***************************************************************************
    test_qgsrandommarkersymbollayer.py
    ---------------------
    Date                 : October 2019
    Copyright            : (C) 2019 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Nyall Dawson'
__date__ = 'October 2019'
__copyright__ = '(C) 2019, Nyall Dawson'

import qgis  # NOQA

from utilities import unitTestDataPath

from qgis.PyQt.QtCore import QDir, Qt, QSize
from qgis.PyQt.QtGui import QImage, QColor, QPainter
from qgis.PyQt.QtXml import QDomDocument

from qgis.core import (QgsGeometry,
                       QgsFillSymbol,
                       QgsRenderContext,
                       QgsFeature,
                       QgsMapSettings,
                       QgsRenderChecker,
                       QgsReadWriteContext,
                       QgsSymbolLayerUtils,
                       QgsSimpleMarkerSymbolLayer,
                       QgsSimpleFillSymbolLayer,
                       QgsMarkerSymbol,
                       QgsRandomMarkerFillSymbolLayer
                       )

from qgis.testing import unittest, start_app

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsRandomMarkerSymbolLayer(unittest.TestCase):

    def setUp(self):
        self.report = "<h1>Python QgsRandomMarkerFillSymbolLayer Tests</h1>\n"

    def tearDown(self):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(self.report)

    def testSimple(self):
        s = QgsFillSymbol()
        s.deleteSymbolLayer(0)

        random_fill = QgsRandomMarkerFillSymbolLayer(10, seed=481523)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Triangle, 4)
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        random_fill.setSubSymbol(marker_symbol)

        s.appendSymbolLayer(random_fill.clone())
        self.assertEqual(s.symbolLayer(0).pointCount(), 10)
        self.assertEqual(s.symbolLayer(0).seed(), 481523)
        s.symbolLayer(0).setPointCount(5)
        s.symbolLayer(0).setSeed(42)
        self.assertEqual(s.symbolLayer(0).pointCount(), 5)
        self.assertEqual(s.symbolLayer(0).seed(), 42)

        s2 = s.clone()
        self.assertEqual(s2.symbolLayer(0).pointCount(), 5)
        self.assertEqual(s2.symbolLayer(0).seed(), 42)

        doc = QDomDocument()
        context = QgsReadWriteContext()
        element = QgsSymbolLayerUtils.saveSymbol('test', s, doc, context)

        s2 = QgsSymbolLayerUtils.loadSymbol(element, context)
        self.assertEqual(s2.symbolLayer(0).pointCount(), 5)
        self.assertEqual(s2.symbolLayer(0).seed(), 42)

        # rendering test
        s3 = QgsFillSymbol()
        s3.deleteSymbolLayer(0)
        s3.appendSymbolLayer(random_fill.clone())

        g = QgsGeometry.fromWkt(
            'Polygon((0 0, 10 0, 10 10, 0 10, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1),(8 8, 9 8, 9 9, 8 9, 8 8))')
        rendered_image = self.renderGeometry(s3, g)
        self.assertTrue(self.imageCheck('randommarkerfill', 'randommarkerfill', rendered_image))

        s3.symbolLayer(0).setPointCount(3)
        g = QgsGeometry.fromWkt(
            'Polygon((0 0, 10 0, 10 10, 0 10, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1),(8 8, 9 8, 9 9, 8 9, 8 8))')
        rendered_image = self.renderGeometry(s3, g)
        self.assertTrue(self.imageCheck('randommarkerfill_3', 'randommarkerfill_3', rendered_image))

        s3.symbolLayer(0).setSeed(12783)
        g = QgsGeometry.fromWkt(
            'Polygon((0 0, 10 0, 10 10, 0 10, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1),(8 8, 9 8, 9 9, 8 9, 8 8))')
        rendered_image = self.renderGeometry(s3, g)
        self.assertTrue(self.imageCheck('randommarkerfill_seed', 'randommarkerfill_seed', rendered_image))

        # random seed
        s3.symbolLayer(0).setSeed(0)
        g = QgsGeometry.fromWkt(
            'Polygon((0 0, 10 0, 10 10, 0 10, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1),(8 8, 9 8, 9 9, 8 9, 8 8))')
        rendered_image = self.renderGeometry(s3, g)
        self.assertFalse(self.imageCheck('randommarkerfill_seed', 'randommarkerfill_seed', rendered_image))

        # density-based count
        s3.symbolLayer(0).setSeed(1)
        s3.symbolLayer(0).setCountMethod(QgsRandomMarkerFillSymbolLayer.DensityBasedCount)
        s3.symbolLayer(0).setPointCount(5)
        s3.symbolLayer(0).setDensityArea(250) # 250 square millimeter
        g = QgsGeometry.fromWkt(
            'Polygon((0 0, 10 0, 10 10, 0 10, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1),(8 8, 9 8, 9 9, 8 9, 8 8))')
        rendered_image = self.renderGeometry(s3, g)
        self.assertTrue(self.imageCheck('randommarkerfill_densitybasedcount', 'randommarkerfill_densitybasedcount', rendered_image))

    def testCreate(self):
        random_fill = QgsRandomMarkerFillSymbolLayer(10, seed=5)
        self.assertEqual(random_fill.seed(), 5)
        fill2 = QgsRandomMarkerFillSymbolLayer.create(random_fill.properties())
        self.assertEqual(fill2.seed(), 5)

        random_fill = QgsRandomMarkerFillSymbolLayer(10)
        self.assertEqual(random_fill.seed(), 0)
        fill2 = QgsRandomMarkerFillSymbolLayer.create(random_fill.properties())
        self.assertEqual(fill2.seed(), 0)

        # a newly created random fill should default to a random seed, not 0
        random_fill = QgsRandomMarkerFillSymbolLayer.create({})
        self.assertNotEqual(random_fill.seed(), 0)

    def testMultipart(self):
        """
        Test rendering a multi-part random marker fill symbol
        """
        s = QgsFillSymbol()
        s.deleteSymbolLayer(0)

        simple_fill = QgsSimpleFillSymbolLayer(color=QColor(255, 255, 255))
        s.appendSymbolLayer(simple_fill.clone())

        random_fill = QgsRandomMarkerFillSymbolLayer(12, seed=481523)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Triangle, 4)
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        random_fill.setSubSymbol(marker_symbol)

        s.appendSymbolLayer(random_fill.clone())

        g = QgsGeometry.fromWkt(
            'MultiPolygon(((0 0, 5 0, 5 10, 0 10, 0 0),(1 1, 1 9, 4 9, 4 1, 1 1)), ((6 0, 10 0, 10 5, 6 5, 6 0)), ((8 6, 10 6, 10 10, 8 10, 8 6)))')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.imageCheck('randommarkerfill_multipoly', 'randommarkerfill_multipoly', rendered_image))

    def renderGeometry(self, symbol, geom, buffer=20):
        f = QgsFeature()
        f.setGeometry(geom)

        image = QImage(200, 200, QImage.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        extent = geom.get().boundingBox()
        # buffer extent by 10%
        if extent.width() > 0:
            extent = extent.buffered((extent.height() + extent.width()) / buffer)
        else:
            extent = extent.buffered(buffer / 2)

        ms.setExtent(extent)
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.expressionContext().setFeature(f)

        painter.begin(image)
        try:
            image.fill(QColor(0, 0, 0))
            symbol.startRender(context)
            symbol.renderFeature(f, context)
            symbol.stopRender(context)
        finally:
            painter.end()

        return image

    def imageCheck(self, name, reference_image, image):
        self.report += "<h2>Render {}</h2>\n".format(name)
        temp_dir = QDir.tempPath() + '/'
        file_name = temp_dir + 'symbol_' + name + ".png"
        image.save(file_name, "PNG")
        checker = QgsRenderChecker()
        checker.setControlPathPrefix("symbol_randommarkerfill")
        checker.setControlName("expected_" + reference_image)
        checker.setRenderedImage(file_name)
        checker.setColorTolerance(2)
        result = checker.compareImages(name, 20)
        self.report += checker.report()
        print((self.report))
        return result


if __name__ == '__main__':
    unittest.main()
