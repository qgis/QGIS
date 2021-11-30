# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsInterpolatedLineSymbolLayer.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Vincent Cloares'
__date__ = '2021-04'
__copyright__ = 'Copyright 2020, The QGIS Project'


import qgis  # NOQA

from qgis.testing import unittest
from qgis.PyQt.QtCore import (QDir,
                              QPointF)
from qgis.PyQt.QtGui import (QImage,
                             QPainter,
                             QColor,
                             QPolygonF)
from qgis.core import (QgsRenderChecker,
                       QgsInterpolatedLineSymbolLayer,
                       QgsInterpolatedLineWidth,
                       QgsInterpolatedLineColor,
                       QgsColorRampShader,
                       QgsStyle,
                       QgsMapSettings,
                       QgsLineSymbol,
                       QgsGeometry,
                       QgsFeature,
                       QgsRenderContext,
                       QgsSymbolLayer,
                       QgsProperty)


class TestQgsLineSymbolLayers(unittest.TestCase):

    def setUp(self):
        self.report = "<h1>Python QgsInterpolatedLineSymbolLayer Tests</h1>\n"

    def tearDown(self):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(self.report)

    def imageCheck(self, name, reference_image, image):
        self.report += "<h2>Render {}</h2>\n".format(name)
        temp_dir = QDir.tempPath() + '/'
        file_name = temp_dir + 'symbollayer_' + name + ".png"
        image.save(file_name, "PNG")
        checker = QgsRenderChecker()
        checker.setControlPathPrefix("symbol_interpolatedline")
        checker.setControlName("expected_" + reference_image)
        checker.setRenderedImage(file_name)
        checker.setColorTolerance(2)
        result = checker.compareImages(name, 0)
        self.report += checker.report()
        print((self.report))
        return result

    def renderImage(self, interpolated_width, interpolated_color, image_name):
        layer = QgsInterpolatedLineSymbolLayer()
        layer.setDataDefinedProperty(QgsSymbolLayer.PropertyLineStartWidthValue, QgsProperty.fromExpression('5'))
        layer.setDataDefinedProperty(QgsSymbolLayer.PropertyLineEndWidthValue, QgsProperty.fromExpression('1'))
        layer.setDataDefinedProperty(QgsSymbolLayer.PropertyLineStartColorValue, QgsProperty.fromExpression('2'))
        layer.setDataDefinedProperty(QgsSymbolLayer.PropertyLineEndColorValue, QgsProperty.fromExpression('6'))
        layer.setInterpolatedWidth(interpolated_width)
        layer.setInterpolatedColor(interpolated_color)

        symbol = QgsLineSymbol()
        symbol.changeSymbolLayer(0, layer)

        image = QImage(200, 200, QImage.Format_RGB32)
        painter = QPainter()
        ms = QgsMapSettings()

        geom = QgsGeometry.fromWkt('LineString (0 0, 10 0, 10 10, 0 10, 0 5)')
        f = QgsFeature()
        f.setGeometry(geom)

        extent = geom.constGet().boundingBox()
        # buffer extent by 10%
        extent = extent.buffered((extent.height() + extent.width()) / 20.0)

        ms.setExtent(extent)
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI

        painter.begin(image)
        image.fill(QColor(255, 255, 255))

        symbol.startRender(context)
        symbol.renderFeature(f, context)
        symbol.stopRender(context)
        painter.end()

        self.assertTrue(self.imageCheck(image_name, image_name, image))

    def testFixedColorFixedWidth(self):
        """ test that rendering a interpolated line with fixed width and fixed color"""

        interpolated_width = QgsInterpolatedLineWidth()
        interpolated_color = QgsInterpolatedLineColor()

        interpolated_width.setIsVariableWidth(False)
        interpolated_width.setFixedStrokeWidth(5)
        interpolated_color.setColor(QColor(255, 0, 0))
        interpolated_color.setColoringMethod(QgsInterpolatedLineColor.SingleColor)

        self.renderImage(interpolated_width, interpolated_color, 'interpolatedlinesymbollayer_1')

    def testRenderNoFeature(self):
        """ test that rendering a interpolated line outside of a map render works"""

        interpolated_width = QgsInterpolatedLineWidth()
        interpolated_color = QgsInterpolatedLineColor()

        interpolated_width.setIsVariableWidth(False)
        interpolated_width.setFixedStrokeWidth(5)
        interpolated_color.setColor(QColor(255, 0, 0))
        interpolated_color.setColoringMethod(QgsInterpolatedLineColor.SingleColor)

        layer = QgsInterpolatedLineSymbolLayer()
        layer.setDataDefinedProperty(QgsSymbolLayer.PropertyLineStartWidthValue, QgsProperty.fromExpression('5'))
        layer.setDataDefinedProperty(QgsSymbolLayer.PropertyLineEndWidthValue, QgsProperty.fromExpression('1'))
        layer.setDataDefinedProperty(QgsSymbolLayer.PropertyLineStartColorValue, QgsProperty.fromExpression('2'))
        layer.setDataDefinedProperty(QgsSymbolLayer.PropertyLineEndColorValue, QgsProperty.fromExpression('6'))
        layer.setInterpolatedWidth(interpolated_width)
        layer.setInterpolatedColor(interpolated_color)

        symbol = QgsLineSymbol()
        symbol.changeSymbolLayer(0, layer)

        image = QImage(200, 200, QImage.Format_RGB32)
        image.fill(QColor(255, 255, 255))
        painter = QPainter(image)

        context = QgsRenderContext.fromQPainter(painter)

        symbol.startRender(context)

        symbol.renderPolyline(QPolygonF([QPointF(30, 50), QPointF(100, 70), QPointF(150, 30)]), None, context)

        symbol.stopRender(context)
        painter.end()

        self.assertTrue(self.imageCheck('interpolatedlinesymbollayer_no_feature', 'interpolatedlinesymbollayer_no_feature', image))

    def testVaryingColorFixedWidth(self):
        """ test that rendering a interpolated line with fixed width and varying color"""

        interpolated_width = QgsInterpolatedLineWidth()
        interpolated_color = QgsInterpolatedLineColor()

        interpolated_width.setIsVariableWidth(False)
        interpolated_width.setFixedStrokeWidth(5)
        color_ramp = QgsColorRampShader(0, 7, QgsStyle.defaultStyle().colorRamp('Viridis'),
                                        QgsColorRampShader.Interpolated)
        color_ramp.classifyColorRamp(10)
        interpolated_color.setColor(color_ramp)
        interpolated_color.setColoringMethod(QgsInterpolatedLineColor.ColorRamp)

        self.renderImage(interpolated_width, interpolated_color, 'interpolatedlinesymbollayer_2')

    def testFixedColorVaryingWidth(self):
        """ test that rendering a interpolated line with varying width and fixed color"""

        interpolated_width = QgsInterpolatedLineWidth()
        interpolated_color = QgsInterpolatedLineColor()

        interpolated_width.setIsVariableWidth(True)
        interpolated_width.setMinimumValue(1)
        interpolated_width.setMaximumValue(8)
        interpolated_width.setMinimumWidth(1)
        interpolated_width.setMaximumWidth(10)
        interpolated_color.setColor(QColor(0, 255, 0))
        interpolated_color.setColoringMethod(QgsInterpolatedLineColor.SingleColor)

        self.renderImage(interpolated_width, interpolated_color, 'interpolatedlinesymbollayer_3')

    def testVaryingColorVaryingWidth(self):
        """ test that rendering a interpolated line with varying width and varying color"""

        interpolated_width = QgsInterpolatedLineWidth()
        interpolated_color = QgsInterpolatedLineColor()

        interpolated_width.setIsVariableWidth(True)
        interpolated_width.setMinimumValue(1)
        interpolated_width.setMaximumValue(8)
        interpolated_width.setMinimumWidth(1)
        interpolated_width.setMaximumWidth(10)
        color_ramp = QgsColorRampShader(0, 7, QgsStyle.defaultStyle().colorRamp('Viridis'),
                                        QgsColorRampShader.Interpolated)
        color_ramp.classifyColorRamp(10)
        interpolated_color.setColor(color_ramp)
        interpolated_color.setColoringMethod(QgsInterpolatedLineColor.ColorRamp)

        self.renderImage(interpolated_width, interpolated_color, 'interpolatedlinesymbollayer_4')

    def testVaryingColorVaryingWidthDiscrete(self):
        """ test that rendering a interpolated line with varying width and varying color with discrete color ramp """

        interpolated_width = QgsInterpolatedLineWidth()
        interpolated_color = QgsInterpolatedLineColor()

        interpolated_width.setIsVariableWidth(True)
        interpolated_width.setMinimumValue(1)
        interpolated_width.setMaximumValue(8)
        interpolated_width.setMinimumWidth(1)
        interpolated_width.setMaximumWidth(10)
        color_ramp = QgsColorRampShader(2, 7, QgsStyle.defaultStyle().colorRamp('RdGy'),
                                        QgsColorRampShader.Discrete)
        color_ramp.classifyColorRamp(5)
        interpolated_color.setColor(color_ramp)
        interpolated_color.setColoringMethod(QgsInterpolatedLineColor.ColorRamp)

        self.renderImage(interpolated_width, interpolated_color, 'interpolatedlinesymbollayer_5')

    def testVaryingColorVaryingWidthExact(self):
        """ test that rendering a interpolated line with varying width and varying color with exact color ramp """

        interpolated_width = QgsInterpolatedLineWidth()
        interpolated_color = QgsInterpolatedLineColor()

        interpolated_width.setIsVariableWidth(True)
        interpolated_width.setMinimumValue(1)
        interpolated_width.setMaximumValue(8)
        interpolated_width.setMinimumWidth(1)
        interpolated_width.setMaximumWidth(10)
        color_ramp = QgsColorRampShader(0, 10, QgsStyle.defaultStyle().colorRamp('Viridis'),
                                        QgsColorRampShader.Exact)
        color_ramp.classifyColorRamp(10)
        interpolated_color.setColor(color_ramp)
        interpolated_color.setColoringMethod(QgsInterpolatedLineColor.ColorRamp)

        self.renderImage(interpolated_width, interpolated_color, 'interpolatedlinesymbollayer_6')


if __name__ == '__main__':
    unittest.main()
