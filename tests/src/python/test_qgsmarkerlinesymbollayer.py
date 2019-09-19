# -*- coding: utf-8 -*-

"""
***************************************************************************
    test_qgsmarkerlinesymbollayer.py
    ---------------------
    Date                 : November 2018
    Copyright            : (C) 2018 by Nyall Dawson
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
__date__ = 'November 2018'
__copyright__ = '(C) 2018, Nyall Dawson'

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
                       QgsTemplatedLineSymbolLayerBase,
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
                       QgsUnitTypes
                       )

from qgis.testing import unittest, start_app

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsMarkerLineSymbolLayer(unittest.TestCase):

    def setUp(self):
        self.report = "<h1>Python QgsMarkerLineSymbolLayer Tests</h1>\n"

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

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacement(QgsMarkerLineSymbolLayer.FirstVertex)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Triangle, 10)
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)

        self.assertEqual(marker_line.width(), 10)
        self.assertAlmostEqual(marker_line.width(context), 37.795275590551185, 3)
        self.assertAlmostEqual(marker_line.width(context2), 118.11023622047244, 3)

        marker_line.subSymbol().setSizeUnit(QgsUnitTypes.RenderPixels)
        self.assertAlmostEqual(marker_line.width(context), 10.0, 3)
        self.assertAlmostEqual(marker_line.width(context2), 10.0, 3)

    def testRingFilter(self):
        # test filtering rings during rendering
        s = QgsFillSymbol()
        s.deleteSymbolLayer(0)

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacement(QgsMarkerLineSymbolLayer.FirstVertex)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Triangle, 4)
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)

        s.appendSymbolLayer(marker_line.clone())
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
            QgsMarkerLineSymbolLayer())
        s3.symbolLayer(0).setRingFilter(QgsLineSymbolLayer.ExteriorRingOnly)
        s3.symbolLayer(0).setAverageAngleLength(0)

        g = QgsGeometry.fromWkt('Polygon((0 0, 10 0, 10 10, 0 10, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1),(8 8, 9 8, 9 9, 8 9, 8 8))')
        rendered_image = self.renderGeometry(s3, g)
        assert self.imageCheck('markerline_exterioronly', 'markerline_exterioronly', rendered_image)

        s3.symbolLayer(0).setRingFilter(QgsLineSymbolLayer.InteriorRingsOnly)
        g = QgsGeometry.fromWkt('Polygon((0 0, 10 0, 10 10, 0 10, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1),(8 8, 9 8, 9 9, 8 9, 8 8))')
        rendered_image = self.renderGeometry(s3, g)
        assert self.imageCheck('markerline_interioronly', 'markerline_interioronly', rendered_image)

    def testPartNum(self):
        # test geometry_part_num variable
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        sym_layer = QgsGeometryGeneratorSymbolLayer.create({'geometryModifier': 'segments_to_lines($geometry)'})
        sym_layer.setSymbolType(QgsSymbol.Line)
        s.appendSymbolLayer(sym_layer)

        marker_line = QgsMarkerLineSymbolLayer(False)
        marker_line.setPlacement(QgsMarkerLineSymbolLayer.FirstVertex)
        f = QgsFontUtils.getStandardTestFont('Bold', 24)
        marker = QgsFontMarkerSymbolLayer(f.family(), 'x', 24, QColor(255, 255, 0))
        marker.setDataDefinedProperty(QgsSymbolLayer.PropertyCharacter, QgsProperty.fromExpression('@geometry_part_num'))
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)
        marker_line.setAverageAngleLength(0)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, marker_line)
        sym_layer.setSubSymbol(line_symbol)

        # rendering test
        g = QgsGeometry.fromWkt('LineString(0 0, 10 0, 10 10, 0 10)')
        rendered_image = self.renderGeometry(s, g, buffer=4)
        assert self.imageCheck('part_num_variable', 'part_num_variable', rendered_image)

        marker.setDataDefinedProperty(QgsSymbolLayer.PropertyCharacter,
                                      QgsProperty.fromExpression('@geometry_part_count'))

        # rendering test
        g = QgsGeometry.fromWkt('LineString(0 0, 10 0, 10 10, 0 10)')
        rendered_image = self.renderGeometry(s, g, buffer=4)
        assert self.imageCheck('part_count_variable', 'part_count_variable', rendered_image)

    def testMarkerAverageAngle(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacement(QgsTemplatedLineSymbolLayerBase.Interval)
        marker_line.setInterval(6)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Triangle, 4)
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)
        marker_line.setAverageAngleLength(60)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, marker_line)

        s.appendSymbolLayer(marker_line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        assert self.imageCheck('markerline_average_angle', 'markerline_average_angle', rendered_image)

    def testMarkerAverageAngleRing(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacement(QgsTemplatedLineSymbolLayerBase.Interval)
        marker_line.setInterval(6)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Triangle, 4)
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)
        marker_line.setAverageAngleLength(60)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, marker_line)

        s.appendSymbolLayer(marker_line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 0 10, 10 10, 10 0, 0 0)')
        rendered_image = self.renderGeometry(s, g)
        assert self.imageCheck('markerline_ring_average_angle', 'markerline_ring_average_angle', rendered_image)

    def testMarkerAverageAngleCenter(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacement(QgsTemplatedLineSymbolLayerBase.CentralPoint)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Triangle, 4)
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)
        marker_line.setAverageAngleLength(60)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, marker_line)

        s.appendSymbolLayer(marker_line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        assert self.imageCheck('markerline_center_average_angle', 'markerline_center_average_angle', rendered_image)

    def testRingNoDupe(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacement(QgsTemplatedLineSymbolLayerBase.Interval)
        marker_line.setInterval(10)
        marker_line.setIntervalUnit(QgsUnitTypes.RenderMapUnits)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Circle, 4)
        marker.setColor(QColor(255, 0, 0, 100))
        marker.setStrokeStyle(Qt.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, marker_line)

        s.appendSymbolLayer(marker_line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 0 10, 10 10, 10 0, 0 0)')
        rendered_image = self.renderGeometry(s, g)
        assert self.imageCheck('markerline_ring_no_dupes', 'markerline_ring_no_dupes', rendered_image)

    def testSinglePoint(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacement(QgsTemplatedLineSymbolLayerBase.Interval)
        marker_line.setInterval(1000)
        marker_line.setIntervalUnit(QgsUnitTypes.RenderMapUnits)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Circle, 4)
        marker.setColor(QColor(255, 0, 0, 100))
        marker.setStrokeStyle(Qt.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, marker_line)

        s.appendSymbolLayer(marker_line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 0 10, 10 10)')
        rendered_image = self.renderGeometry(s, g)
        assert self.imageCheck('markerline_single', 'markerline_single', rendered_image)

    def testNoPoint(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacement(QgsTemplatedLineSymbolLayerBase.Interval)
        marker_line.setOffsetAlongLine(1000)
        marker_line.setIntervalUnit(QgsUnitTypes.RenderMapUnits)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Circle, 4)
        marker.setColor(QColor(255, 0, 0, 100))
        marker.setStrokeStyle(Qt.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, marker_line)

        s.appendSymbolLayer(marker_line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 0 10, 10 10)')
        rendered_image = self.renderGeometry(s, g)
        assert self.imageCheck('markerline_none', 'markerline_none', rendered_image)

    def testCenterSegment(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacement(QgsTemplatedLineSymbolLayerBase.SegmentCenter)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Triangle, 4)
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, marker_line)

        s.appendSymbolLayer(marker_line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 10 0, 0 10)')
        rendered_image = self.renderGeometry(s, g)
        assert self.imageCheck('markerline_segmentcenter', 'markerline_segmentcenter', rendered_image)

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
        checker.setControlPathPrefix("symbol_markerline")
        checker.setControlName("expected_" + reference_image)
        checker.setRenderedImage(file_name)
        checker.setColorTolerance(2)
        result = checker.compareImages(name, 20)
        self.report += checker.report()
        print((self.report))
        return result


if __name__ == '__main__':
    unittest.main()
