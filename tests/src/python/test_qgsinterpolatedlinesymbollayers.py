"""QGIS Unit tests for QgsInterpolatedLineSymbolLayer.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Vincent Cloares"
__date__ = "2021-04"
__copyright__ = "Copyright 2020, The QGIS Project"

import unittest

from qgis.PyQt.QtCore import QPointF
from qgis.PyQt.QtGui import QColor, QImage, QPainter, QPolygonF
from qgis.core import (
    QgsColorRampShader,
    QgsFeature,
    QgsGeometry,
    QgsInterpolatedLineColor,
    QgsInterpolatedLineSymbolLayer,
    QgsInterpolatedLineWidth,
    QgsLineSymbol,
    QgsMapSettings,
    QgsProperty,
    QgsRenderContext,
    QgsStyle,
    QgsSymbolLayer,
)
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsInterpolatedLineSymbolLayers(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "symbol_interpolatedline"

    def render_image(
        self,
        interpolated_width: QgsInterpolatedLineWidth,
        interpolated_color: QgsInterpolatedLineColor,
    ) -> QImage:
        layer = QgsInterpolatedLineSymbolLayer()
        layer.setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyLineStartWidthValue,
            QgsProperty.fromExpression("5"),
        )
        layer.setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyLineEndWidthValue,
            QgsProperty.fromExpression("1"),
        )
        layer.setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyLineStartColorValue,
            QgsProperty.fromExpression("2"),
        )
        layer.setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyLineEndColorValue,
            QgsProperty.fromExpression("6"),
        )
        layer.setInterpolatedWidth(interpolated_width)
        layer.setInterpolatedColor(interpolated_color)

        symbol = QgsLineSymbol()
        symbol.changeSymbolLayer(0, layer)

        image = QImage(200, 200, QImage.Format.Format_RGB32)
        painter = QPainter()
        ms = QgsMapSettings()

        geom = QgsGeometry.fromWkt("LineString (0 0, 10 0, 10 10, 0 10, 0 5)")
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

        return image

    def testFixedColorFixedWidth(self):
        """
        Test that rendering an interpolated line with fixed width and fixed
        color
        """

        interpolated_width = QgsInterpolatedLineWidth()
        interpolated_color = QgsInterpolatedLineColor()

        interpolated_width.setIsVariableWidth(False)
        interpolated_width.setFixedStrokeWidth(5)
        interpolated_color.setColor(QColor(255, 0, 0))
        interpolated_color.setColoringMethod(
            QgsInterpolatedLineColor.ColoringMethod.SingleColor
        )

        rendered_image = self.render_image(interpolated_width, interpolated_color)

        self.assertTrue(
            self.image_check(
                "interpolatedlinesymbollayer_1",
                "interpolatedlinesymbollayer_1",
                rendered_image,
                "expected_interpolatedlinesymbollayer_1",
                color_tolerance=2,
                allowed_mismatch=0,
            )
        )

    def testRenderNoFeature(self):
        """
        Test that rendering an interpolated line outside of a map render works
        """

        interpolated_width = QgsInterpolatedLineWidth()
        interpolated_color = QgsInterpolatedLineColor()

        interpolated_width.setIsVariableWidth(False)
        interpolated_width.setFixedStrokeWidth(5)
        interpolated_color.setColor(QColor(255, 0, 0))
        interpolated_color.setColoringMethod(
            QgsInterpolatedLineColor.ColoringMethod.SingleColor
        )

        layer = QgsInterpolatedLineSymbolLayer()
        layer.setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyLineStartWidthValue,
            QgsProperty.fromExpression("5"),
        )
        layer.setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyLineEndWidthValue,
            QgsProperty.fromExpression("1"),
        )
        layer.setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyLineStartColorValue,
            QgsProperty.fromExpression("2"),
        )
        layer.setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyLineEndColorValue,
            QgsProperty.fromExpression("6"),
        )
        layer.setInterpolatedWidth(interpolated_width)
        layer.setInterpolatedColor(interpolated_color)

        symbol = QgsLineSymbol()
        symbol.changeSymbolLayer(0, layer)

        image = QImage(200, 200, QImage.Format.Format_RGB32)
        image.fill(QColor(255, 255, 255))
        painter = QPainter(image)

        context = QgsRenderContext.fromQPainter(painter)

        symbol.startRender(context)

        symbol.renderPolyline(
            QPolygonF([QPointF(30, 50), QPointF(100, 70), QPointF(150, 30)]),
            None,
            context,
        )

        symbol.stopRender(context)
        painter.end()

        self.assertTrue(
            self.image_check(
                "interpolatedlinesymbollayer_no_feature",
                "interpolatedlinesymbollayer_no_feature",
                image,
                "expected_interpolatedlinesymbollayer_no_feature",
                color_tolerance=2,
                allowed_mismatch=0,
            )
        )

    def testVaryingColorFixedWidth(self):
        """
        Test rendering an interpolated line with fixed width
        and varying color
        """

        interpolated_width = QgsInterpolatedLineWidth()
        interpolated_color = QgsInterpolatedLineColor()

        interpolated_width.setIsVariableWidth(False)
        interpolated_width.setFixedStrokeWidth(5)
        color_ramp = QgsColorRampShader(
            0,
            7,
            QgsStyle.defaultStyle().colorRamp("Viridis"),
            QgsColorRampShader.Type.Interpolated,
        )
        color_ramp.classifyColorRamp(10)
        interpolated_color.setColor(color_ramp)
        interpolated_color.setColoringMethod(
            QgsInterpolatedLineColor.ColoringMethod.ColorRamp
        )

        rendered_image = self.render_image(interpolated_width, interpolated_color)

        self.assertTrue(
            self.image_check(
                "interpolatedlinesymbollayer_2",
                "interpolatedlinesymbollayer_2",
                rendered_image,
                "expected_interpolatedlinesymbollayer_2",
                color_tolerance=2,
                allowed_mismatch=0,
            )
        )

    def testFixedColorVaryingWidth(self):
        """
        Test rendering an interpolated line with varying width and
        fixed color
        """

        interpolated_width = QgsInterpolatedLineWidth()
        interpolated_color = QgsInterpolatedLineColor()

        interpolated_width.setIsVariableWidth(True)
        interpolated_width.setMinimumValue(1)
        interpolated_width.setMaximumValue(8)
        interpolated_width.setMinimumWidth(1)
        interpolated_width.setMaximumWidth(10)
        interpolated_color.setColor(QColor(0, 255, 0))
        interpolated_color.setColoringMethod(
            QgsInterpolatedLineColor.ColoringMethod.SingleColor
        )

        rendered_image = self.render_image(interpolated_width, interpolated_color)

        self.assertTrue(
            self.image_check(
                "interpolatedlinesymbollayer_3",
                "interpolatedlinesymbollayer_3",
                rendered_image,
                "expected_interpolatedlinesymbollayer_3",
                color_tolerance=2,
                allowed_mismatch=0,
            )
        )

    def testVaryingColorVaryingWidth(self):
        """
        Test rendering an interpolated line with varying width and
        varying color
        """

        interpolated_width = QgsInterpolatedLineWidth()
        interpolated_color = QgsInterpolatedLineColor()

        interpolated_width.setIsVariableWidth(True)
        interpolated_width.setMinimumValue(1)
        interpolated_width.setMaximumValue(8)
        interpolated_width.setMinimumWidth(1)
        interpolated_width.setMaximumWidth(10)
        color_ramp = QgsColorRampShader(
            0,
            7,
            QgsStyle.defaultStyle().colorRamp("Viridis"),
            QgsColorRampShader.Type.Interpolated,
        )
        color_ramp.classifyColorRamp(10)
        interpolated_color.setColor(color_ramp)
        interpolated_color.setColoringMethod(
            QgsInterpolatedLineColor.ColoringMethod.ColorRamp
        )

        rendered_image = self.render_image(interpolated_width, interpolated_color)

        self.assertTrue(
            self.image_check(
                "interpolatedlinesymbollayer_4",
                "interpolatedlinesymbollayer_4",
                rendered_image,
                "expected_interpolatedlinesymbollayer_4",
                color_tolerance=2,
                allowed_mismatch=0,
            )
        )

    def testVaryingColorVaryingWidthDiscrete(self):
        """
        Test rendering an interpolated line with varying width and
        varying color with discrete color ramp
        """

        interpolated_width = QgsInterpolatedLineWidth()
        interpolated_color = QgsInterpolatedLineColor()

        interpolated_width.setIsVariableWidth(True)
        interpolated_width.setMinimumValue(1)
        interpolated_width.setMaximumValue(8)
        interpolated_width.setMinimumWidth(1)
        interpolated_width.setMaximumWidth(10)
        color_ramp = QgsColorRampShader(
            2,
            7,
            QgsStyle.defaultStyle().colorRamp("RdGy"),
            QgsColorRampShader.Type.Discrete,
        )
        color_ramp.classifyColorRamp(5)
        interpolated_color.setColor(color_ramp)
        interpolated_color.setColoringMethod(
            QgsInterpolatedLineColor.ColoringMethod.ColorRamp
        )

        rendered_image = self.render_image(interpolated_width, interpolated_color)

        self.assertTrue(
            self.image_check(
                "interpolatedlinesymbollayer_5",
                "interpolatedlinesymbollayer_5",
                rendered_image,
                "expected_interpolatedlinesymbollayer_5",
                color_tolerance=2,
                allowed_mismatch=0,
            )
        )

    def testVaryingColorVaryingWidthExact(self):
        """
        Test rendering an interpolated line with varying width
        and varying color with exact color ramp
        """

        interpolated_width = QgsInterpolatedLineWidth()
        interpolated_color = QgsInterpolatedLineColor()

        interpolated_width.setIsVariableWidth(True)
        interpolated_width.setMinimumValue(1)
        interpolated_width.setMaximumValue(8)
        interpolated_width.setMinimumWidth(1)
        interpolated_width.setMaximumWidth(10)
        color_ramp = QgsColorRampShader(
            0,
            10,
            QgsStyle.defaultStyle().colorRamp("Viridis"),
            QgsColorRampShader.Type.Exact,
        )
        color_ramp.classifyColorRamp(10)
        interpolated_color.setColor(color_ramp)
        interpolated_color.setColoringMethod(
            QgsInterpolatedLineColor.ColoringMethod.ColorRamp
        )

        rendered_image = self.render_image(interpolated_width, interpolated_color)

        self.assertTrue(
            self.image_check(
                "interpolatedlinesymbollayer_6",
                "interpolatedlinesymbollayer_6",
                rendered_image,
                "expected_interpolatedlinesymbollayer_6",
                color_tolerance=2,
                allowed_mismatch=0,
            )
        )


if __name__ == "__main__":
    unittest.main()
