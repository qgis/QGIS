# -*- coding: utf-8 -*-

"""
***************************************************************************
    test_qgslineburstsymbollayer.py
    ---------------------
    Date                 : October 2021
    Copyright            : (C) 2021 by Nyall Dawson
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
__date__ = 'October 2021'
__copyright__ = '(C) 2021, Nyall Dawson'

import qgis  # NOQA
from qgis.PyQt.QtCore import QDir, Qt
from qgis.PyQt.QtGui import QImage, QColor, QPainter
from qgis.core import (QgsGeometry,
                       Qgis,
                       QgsRenderContext,
                       QgsFeature,
                       QgsMapSettings,
                       QgsRenderChecker,
                       QgsGradientColorRamp,
                       QgsGradientStop,
                       QgsLineSymbol,
                       QgsLineburstSymbolLayer,
                       QgsSymbolLayer,
                       QgsProperty
                       )
from qgis.testing import unittest, start_app

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLineburstSymbolLayer(unittest.TestCase):

    def setUp(self):
        self.report = "<h1>Python QgsLineburstSymbolLayer Tests</h1>\n"

    def tearDown(self):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(self.report)

    def testTwoColor(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        line = QgsLineburstSymbolLayer()
        line.setColor(QColor(255, 0, 0))
        line.setColor2(QColor(0, 255, 0))
        line.setWidth(8)

        s.appendSymbolLayer(line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.imageCheck('lineburst_two_color', 'lineburst_two_color', rendered_image))

    def testDataDefinedColors(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        line = QgsLineburstSymbolLayer()
        line.setColor(QColor(255, 0, 0))
        line.setColor2(QColor(0, 255, 0))
        line.setWidth(8)
        line.setDataDefinedProperty(QgsSymbolLayer.PropertyStrokeColor, QgsProperty.fromExpression("'orange'"))
        line.setDataDefinedProperty(QgsSymbolLayer.PropertySecondaryColor, QgsProperty.fromExpression("'purple'"))

        s.appendSymbolLayer(line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.imageCheck('lineburst_datadefined_color', 'lineburst_datadefined_color', rendered_image))

    def testColorRamp(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        line = QgsLineburstSymbolLayer()
        line.setGradientColorType(Qgis.GradientColorSource.ColorRamp)
        line.setColorRamp(QgsGradientColorRamp(QColor(200, 0, 0), QColor(0, 200, 0), False,
                                               [QgsGradientStop(0.5, QColor(0, 0, 200))]))
        line.setWidth(8)

        s.appendSymbolLayer(line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.imageCheck('lineburst_colorramp', 'lineburst_colorramp', rendered_image))

    def testRenderClosedRing(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        line = QgsLineburstSymbolLayer()
        line.setColor(QColor(255, 0, 0))
        line.setColor2(QColor(0, 255, 0))
        line.setWidth(8)

        s.appendSymbolLayer(line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 10 0, 10 10, 0 10, 0 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.imageCheck('lineburst_closed', 'lineburst_closed', rendered_image))

    def testRenderFlatCap(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        line = QgsLineburstSymbolLayer()
        line.setColor(QColor(255, 0, 0))
        line.setColor2(QColor(0, 255, 0))
        line.setWidth(8)
        line.setPenCapStyle(Qt.FlatCap)

        s.appendSymbolLayer(line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.imageCheck('lineburst_flatcap', 'lineburst_flatcap', rendered_image))

    def testRenderSquareCap(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        line = QgsLineburstSymbolLayer()
        line.setColor(QColor(255, 0, 0))
        line.setColor2(QColor(0, 255, 0))
        line.setWidth(8)
        line.setPenCapStyle(Qt.SquareCap)

        s.appendSymbolLayer(line.clone())

        g = QgsGeometry.fromWkt('LineString(2 2, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.imageCheck('lineburst_squarecap', 'lineburst_squarecap', rendered_image))

    def testRenderMiterJoin(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        line = QgsLineburstSymbolLayer()
        line.setColor(QColor(255, 0, 0))
        line.setColor2(QColor(0, 255, 0))
        line.setWidth(8)
        line.setPenJoinStyle(Qt.MiterJoin)

        s.appendSymbolLayer(line.clone())

        g = QgsGeometry.fromWkt('LineString(0 15, 0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.imageCheck('lineburst_miterjoin', 'lineburst_miterjoin', rendered_image))

    def testRenderBevelJoin(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        line = QgsLineburstSymbolLayer()
        line.setColor(QColor(255, 0, 0))
        line.setColor2(QColor(0, 255, 0))
        line.setWidth(8)
        line.setPenJoinStyle(Qt.BevelJoin)

        s.appendSymbolLayer(line.clone())

        g = QgsGeometry.fromWkt('LineString(2 2, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.imageCheck('lineburst_beveljoin', 'lineburst_beveljoin', rendered_image))

    def testLineOffset(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        line = QgsLineburstSymbolLayer()
        line.setColor(QColor(255, 0, 0))
        line.setColor2(QColor(0, 255, 0))
        line.setWidth(5)
        line.setOffset(5)

        s.appendSymbolLayer(line.clone())

        g = QgsGeometry.fromWkt('LineString(2 2, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.imageCheck('lineburst_offset', 'lineburst_offset', rendered_image))

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
        checker.setControlPathPrefix("symbol_lineburst")
        checker.setControlName("expected_" + reference_image)
        checker.setRenderedImage(file_name)
        checker.setColorTolerance(2)
        result = checker.compareImages(name, 20)
        self.report += checker.report()
        print((self.report))
        return result


if __name__ == '__main__':
    unittest.main()
