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
                       QgsFontUtils,
                       QgsLineSymbol,
                       QgsSymbolLayer,
                       QgsProperty,
                       QgsRectangle,
                       QgsUnitTypes,
                       QgsSimpleLineSymbolLayer,
                       QgsTemplatedLineSymbolLayerBase,
                       QgsHashedLineSymbolLayer
                       )

from qgis.testing import unittest, start_app

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsHashedLineSymbolLayer(unittest.TestCase):

    def setUp(self):
        self.report = "<h1>Python QgsHashedLineSymbolLayer Tests</h1>\n"

    def tearDown(self):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(self.report)

    def testWidth(self):
        ms = QgsMapSettings()
        extent = QgsRectangle(100, 200, 100, 200)
        ms.setExtent(extent)
        ms.setOutputSize(QSize(400, 400))
        context = QgsRenderContext.fromMapSettings(ms)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        ms.setExtent(QgsRectangle(100, 150, 100, 150))
        ms.setOutputDpi(ms.outputDpi() * 2)
        context2 = QgsRenderContext.fromMapSettings(ms)
        context2.setScaleFactor(300 / 25.4)

        s = QgsFillSymbol()
        s.deleteSymbolLayer(0)

        hash_line = QgsHashedLineSymbolLayer(True)
        hash_line.setPlacement(QgsTemplatedLineSymbolLayerBase.FirstVertex)
        simple_line = QgsSimpleLineSymbolLayer()
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, simple_line)
        hash_line.setSubSymbol(line_symbol)
        hash_line.setHashLength(10)

        self.assertEqual(hash_line.width(), 10)
        self.assertAlmostEqual(hash_line.width(context), 37.795275590551185, 3)
        self.assertAlmostEqual(hash_line.width(context2), 118.11023622047244, 3)

        hash_line.setHashLengthUnit(QgsUnitTypes.RenderPixels)
        self.assertAlmostEqual(hash_line.width(context), 10.0, 3)
        self.assertAlmostEqual(hash_line.width(context2), 10.0, 3)

    def testHashAngle(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        hash_line = QgsHashedLineSymbolLayer(True)
        hash_line.setPlacement(QgsTemplatedLineSymbolLayerBase.Interval)
        hash_line.setInterval(6)
        simple_line = QgsSimpleLineSymbolLayer()
        simple_line.setColor(QColor(0, 255, 0))
        simple_line.setWidth(1)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, simple_line)
        hash_line.setSubSymbol(line_symbol)
        hash_line.setHashLength(7)
        hash_line.setHashAngle(45)
        hash_line.setAverageAngleLength(0)

        s.appendSymbolLayer(hash_line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        assert self.imageCheck('line_hash_angle', 'line_hash_angle', rendered_image)

        s.symbolLayer(0).setRotateSymbols(False)

        g = QgsGeometry.fromWkt('LineString(0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        assert self.imageCheck('line_hash_no_rotate', 'line_hash_no_rotate', rendered_image)

    def testHashAverageAngle(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        hash_line = QgsHashedLineSymbolLayer(True)
        hash_line.setPlacement(QgsTemplatedLineSymbolLayerBase.Interval)
        hash_line.setInterval(6)
        simple_line = QgsSimpleLineSymbolLayer()
        simple_line.setColor(QColor(0, 255, 0))
        simple_line.setWidth(1)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, simple_line)
        hash_line.setSubSymbol(line_symbol)
        hash_line.setHashLength(7)
        hash_line.setHashAngle(45)
        hash_line.setAverageAngleLength(30)

        s.appendSymbolLayer(hash_line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        assert self.imageCheck('line_hash_average_angle', 'line_hash_average_angle', rendered_image)

    def testHashAverageAngle(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        hash_line = QgsHashedLineSymbolLayer(True)
        hash_line.setPlacement(QgsTemplatedLineSymbolLayerBase.CentralPoint)
        simple_line = QgsSimpleLineSymbolLayer()
        simple_line.setColor(QColor(0, 255, 0))
        simple_line.setWidth(1)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, simple_line)
        hash_line.setSubSymbol(line_symbol)
        hash_line.setHashLength(7)
        hash_line.setHashAngle(45)
        hash_line.setAverageAngleLength(30)

        s.appendSymbolLayer(hash_line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        assert self.imageCheck('line_hash_center_average_angle', 'line_hash_center_average_angle', rendered_image)

    def testHashAverageAngleClosedRing(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        hash_line = QgsHashedLineSymbolLayer(True)
        hash_line.setPlacement(QgsTemplatedLineSymbolLayerBase.Interval)
        hash_line.setInterval(6)
        simple_line = QgsSimpleLineSymbolLayer()
        simple_line.setColor(QColor(0, 255, 0))
        simple_line.setWidth(1)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, simple_line)
        hash_line.setSubSymbol(line_symbol)
        hash_line.setHashLength(7)
        hash_line.setHashAngle(0)
        hash_line.setAverageAngleLength(30)

        s.appendSymbolLayer(hash_line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 0 10, 10 10, 10 0, 0 0)')
        rendered_image = self.renderGeometry(s, g)
        assert self.imageCheck('line_hash_ring_average_angle', 'line_hash_ring_average_angle', rendered_image)

    def testHashPlacement(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        hash_line = QgsHashedLineSymbolLayer(True)
        hash_line.setPlacement(QgsTemplatedLineSymbolLayerBase.Vertex)
        hash_line.setInterval(6)
        simple_line = QgsSimpleLineSymbolLayer()
        simple_line.setColor(QColor(0, 255, 0))
        simple_line.setWidth(1)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, simple_line)
        hash_line.setSubSymbol(line_symbol)
        hash_line.setHashLength(7)
        hash_line.setAverageAngleLength(0)

        s.appendSymbolLayer(hash_line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        assert self.imageCheck('line_hash_vertex', 'line_hash_vertex', rendered_image)

        s.symbolLayer(0).setPlacement(QgsTemplatedLineSymbolLayerBase.FirstVertex)

        g = QgsGeometry.fromWkt('LineString(0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        assert self.imageCheck('line_hash_first', 'line_hash_first', rendered_image)

        s.symbolLayer(0).setPlacement(QgsTemplatedLineSymbolLayerBase.LastVertex)

        g = QgsGeometry.fromWkt('LineString(0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        assert self.imageCheck('line_hash_last', 'line_hash_last', rendered_image)

    def testRingFilter(self):
        # test filtering rings during rendering
        s = QgsFillSymbol()
        s.deleteSymbolLayer(0)

        hash_line = QgsHashedLineSymbolLayer(True)
        hash_line.setPlacement(QgsTemplatedLineSymbolLayerBase.Interval)
        hash_line.setInterval(6)
        simple_line = QgsSimpleLineSymbolLayer()
        simple_line.setColor(QColor(0, 255, 0))
        simple_line.setWidth(1)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, simple_line)
        hash_line.setSubSymbol(line_symbol)
        hash_line.setHashLength(10)
        hash_line.setAverageAngleLength(0)

        s.appendSymbolLayer(hash_line.clone())
        self.assertEqual(s.symbolLayer(0).ringFilter(), QgsLineSymbolLayer.AllRings)
        s.symbolLayer(0).setRingFilter(QgsLineSymbolLayer.ExteriorRingOnly)
        self.assertEqual(s.symbolLayer(0).ringFilter(), QgsLineSymbolLayer.ExteriorRingOnly)

        s2 = s.clone()
        self.assertEqual(s2.symbolLayer(0).ringFilter(), QgsLineSymbolLayer.ExteriorRingOnly)

        doc = QDomDocument()
        context = QgsReadWriteContext()
        element = QgsSymbolLayerUtils.saveSymbol('test', s, doc, context)

        s2 = QgsSymbolLayerUtils.loadSymbol(element, context)
        self.assertEqual(s2.symbolLayer(0).ringFilter(), QgsLineSymbolLayer.ExteriorRingOnly)

        # rendering test
        s3 = QgsFillSymbol()
        s3.deleteSymbolLayer(0)
        s3.appendSymbolLayer(
            hash_line.clone())
        s3.symbolLayer(0).setRingFilter(QgsLineSymbolLayer.ExteriorRingOnly)

        g = QgsGeometry.fromWkt('Polygon((0 0, 10 0, 10 10, 0 10, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1),(8 8, 9 8, 9 9, 8 9, 8 8))')
        rendered_image = self.renderGeometry(s3, g)
        assert self.imageCheck('hashline_exterioronly', 'hashline_exterioronly', rendered_image)

        s3.symbolLayer(0).setRingFilter(QgsLineSymbolLayer.InteriorRingsOnly)
        g = QgsGeometry.fromWkt('Polygon((0 0, 10 0, 10 10, 0 10, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1),(8 8, 9 8, 9 9, 8 9, 8 8))')
        rendered_image = self.renderGeometry(s3, g)
        assert self.imageCheck('hashline_interioronly', 'hashline_interioronly', rendered_image)

    def testLineOffset(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        hash_line = QgsHashedLineSymbolLayer(True)
        hash_line.setPlacement(QgsTemplatedLineSymbolLayerBase.Interval)
        hash_line.setInterval(6)
        simple_line = QgsSimpleLineSymbolLayer()
        simple_line.setColor(QColor(0, 255, 0))
        simple_line.setWidth(1)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, simple_line)
        hash_line.setSubSymbol(line_symbol)
        hash_line.setHashLength(10)
        hash_line.setAverageAngleLength(0)

        s.appendSymbolLayer(hash_line.clone())

        s.symbolLayer(0).setOffset(3)
        g = QgsGeometry.fromWkt('LineString(0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        assert self.imageCheck('line_offset_positive', 'line_offset_positive', rendered_image)

        s.symbolLayer(0).setOffset(-3)
        g = QgsGeometry.fromWkt('LineString(0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        assert self.imageCheck('line_offset_negative', 'line_offset_negative', rendered_image)

    def testPointNumInterval(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        hash_line = QgsHashedLineSymbolLayer(True)
        hash_line.setPlacement(QgsTemplatedLineSymbolLayerBase.Interval)
        hash_line.setInterval(6)
        simple_line = QgsSimpleLineSymbolLayer()
        simple_line.setColor(QColor(0, 255, 0))
        simple_line.setWidth(1)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, simple_line)
        hash_line.setSubSymbol(line_symbol)
        hash_line.setHashLength(10)
        hash_line.setAverageAngleLength(0)

        s.appendSymbolLayer(hash_line.clone())

        s.symbolLayer(0).setDataDefinedProperty(QgsSymbolLayer.PropertyLineDistance, QgsProperty.fromExpression(
            "@geometry_point_num * 2"))

        g = QgsGeometry.fromWkt('LineString(0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        assert self.imageCheck('line_dd_size', 'line_dd_size', rendered_image)

    def testSegmentCenter(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        hash_line = QgsHashedLineSymbolLayer(True)
        hash_line.setPlacement(QgsTemplatedLineSymbolLayerBase.SegmentCenter)
        simple_line = QgsSimpleLineSymbolLayer()
        simple_line.setColor(QColor(0, 255, 0))
        simple_line.setWidth(1)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, simple_line)
        hash_line.setSubSymbol(line_symbol)
        hash_line.setHashLength(10)
        hash_line.setAverageAngleLength(0)

        s.appendSymbolLayer(hash_line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 10 0, 0 10)')
        rendered_image = self.renderGeometry(s, g)
        assert self.imageCheck('line_hash_segmentcenter', 'line_hash_segmentcenter', rendered_image)

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
        checker.setControlPathPrefix("symbol_hashline")
        checker.setControlName("expected_" + reference_image)
        checker.setRenderedImage(file_name)
        checker.setColorTolerance(2)
        result = checker.compareImages(name, 20)
        self.report += checker.report()
        print((self.report))
        return result


if __name__ == '__main__':
    unittest.main()
