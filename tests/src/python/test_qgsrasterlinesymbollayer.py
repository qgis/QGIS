# -*- coding: utf-8 -*-

"""
***************************************************************************
    test_qgshashlinesymbollayer.py
    ---------------------
    Date                 : March 2019
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
__date__ = 'March 2019'
__copyright__ = '(C) 2019, Nyall Dawson'

import qgis  # NOQA

import os
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
                       QgsLineSymbolLayer,
                       QgsMarkerLineSymbolLayer,
                       QgsMarkerSymbol,
                       QgsGeometryGeneratorSymbolLayer,
                       QgsSymbol,
                       QgsFontMarkerSymbolLayer,
                       QgsMultiRenderChecker,
                       QgsLineSymbol,
                       QgsSymbolLayer,
                       QgsProperty,
                       QgsRectangle,
                       QgsUnitTypes,
                       QgsSimpleLineSymbolLayer,
                       QgsTemplatedLineSymbolLayerBase,
                       QgsHashedLineSymbolLayer,
                       QgsVectorLayer,
                       QgsSingleSymbolRenderer,
                       QgsRasterLineSymbolLayer
                       )

from qgis.testing import unittest, start_app

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsRasterLineSymbolLayer(unittest.TestCase):

    def setUp(self):
        self.report = "<h1>Python QgsRasterLineSymbolLayer Tests</h1>\n"

    def tearDown(self):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(self.report)

    def testRender(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + '/raster_brush.png')
        raster_line.setWidth(8)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.imageCheck('rasterline_render', 'rasterline_render', rendered_image))

    def testRenderHairline(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + '/raster_brush.png')
        raster_line.setWidth(0)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.imageCheck('rasterline_hairline', 'rasterline_hairline', rendered_image))

    def testRenderClosedRing(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + '/raster_brush.png')
        raster_line.setWidth(8)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 10 0, 10 10, 0 10, 0 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.imageCheck('rasterline_closed', 'rasterline_closed', rendered_image))

    def testRenderOpacity(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + '/raster_brush.png')
        raster_line.setWidth(8)
        raster_line.setOpacity(0.5)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.imageCheck('rasterline_opacity', 'rasterline_opacity', rendered_image))

    def testRenderFlatCap(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + '/raster_brush.png')
        raster_line.setWidth(15)
        raster_line.setPenCapStyle(Qt.FlatCap)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.imageCheck('rasterline_flatcap', 'rasterline_flatcap', rendered_image))

    def testRenderSquareCap(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + '/raster_brush.png')
        raster_line.setWidth(15)
        raster_line.setPenCapStyle(Qt.SquareCap)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt('LineString(2 2, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.imageCheck('rasterline_squarecap', 'rasterline_squarecap', rendered_image))

    def testRenderMiterJoin(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + '/raster_brush.png')
        raster_line.setWidth(15)
        raster_line.setPenJoinStyle(Qt.MiterJoin)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt('LineString(0 15, 0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.imageCheck('rasterline_miterjoin', 'rasterline_miterjoin', rendered_image))

    def testRenderBevelJoin(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + '/raster_brush.png')
        raster_line.setWidth(15)
        raster_line.setPenJoinStyle(Qt.BevelJoin)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt('LineString(2 2, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.imageCheck('rasterline_beveljoin', 'rasterline_beveljoin', rendered_image))

    def testLineOffset(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + '/raster_brush.png')
        raster_line.setWidth(5)
        raster_line.setOffset(5)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt('LineString(2 2, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.imageCheck('rasterline_offset', 'rasterline_offset', rendered_image))

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
        checker.setControlPathPrefix("symbol_rasterline")
        checker.setControlName("expected_" + reference_image)
        checker.setRenderedImage(file_name)
        checker.setColorTolerance(2)
        result = checker.compareImages(name, 20)
        self.report += checker.report()
        print((self.report))
        return result


if __name__ == '__main__':
    unittest.main()
