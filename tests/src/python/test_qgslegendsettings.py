""""Test QgsLegendSettings

.. note:: This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

Run with ctest -V -R PyQgsLegendSettings

"""

from qgis.PyQt.QtCore import Qt, QSizeF
from qgis.PyQt.QtGui import QColor

from qgis.core import (
    Qgis,
    QgsLegendStyle,
    QgsLegendSettings,
    QgsExpressionContext,
    QgsExpressionContextScope,
    QgsTextFormat,
    QgsRenderContext,
    QgsPalLayerSettings,
    QgsProperty,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

QGISAPP = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsLegendSettings(QgisTestCase):

    def test_getters_setters(self):
        settings = QgsLegendSettings()

        title = "Test Legend"
        settings.setTitle(title)
        self.assertEqual(settings.title(), title)

        settings.setTitleAlignment(Qt.AlignmentFlag.AlignRight)
        self.assertEqual(settings.titleAlignment(), Qt.AlignmentFlag.AlignRight)

        test_style = QgsLegendStyle()
        test_style.setIndent(33)
        settings.setStyle(QgsLegendStyle.Style.Symbol, test_style)
        self.assertEqual(settings.style(QgsLegendStyle.Style.Symbol).indent(), 33)

        test_style2 = QgsLegendStyle()
        test_style2.setIndent(35)
        settings.setStyle(QgsLegendStyle.Style.SymbolLabel, test_style2)
        self.assertEqual(settings.style(QgsLegendStyle.Style.Symbol).indent(), 33)
        self.assertEqual(settings.style(QgsLegendStyle.Style.SymbolLabel).indent(), 35)

        settings.setBoxSpace(5.0)
        self.assertEqual(settings.boxSpace(), 5.0)

        settings.setWrapChar("\n")
        self.assertEqual(settings.wrapChar(), "\n")

        settings.setColumnSpace(5.0)
        self.assertEqual(settings.columnSpace(), 5.0)

        settings.setColumnCount(3)
        self.assertEqual(settings.columnCount(), 3)

        settings.setSplitLayer(True)
        self.assertTrue(settings.splitLayer())
        settings.setSplitLayer(False)
        self.assertFalse(settings.splitLayer())

        settings.setEqualColumnWidth(True)
        self.assertTrue(settings.equalColumnWidth())
        settings.setEqualColumnWidth(False)
        self.assertFalse(settings.equalColumnWidth())

        settings.setSymbolSize(QSizeF(10.0, 10.0))
        self.assertEqual(settings.symbolSize(), QSizeF(10.0, 10.0))
        settings.setMaximumSymbolSize(20.0)
        settings.setMinimumSymbolSize(5.0)
        self.assertEqual(settings.maximumSymbolSize(), 20.0)
        self.assertEqual(settings.minimumSymbolSize(), 5.0)

        settings.setSymbolAlignment(Qt.AlignRight)
        self.assertEqual(settings.symbolAlignment(), Qt.AlignRight)

        settings.setDrawRasterStroke(True)
        self.assertTrue(settings.drawRasterStroke())

        settings.setRasterStrokeColor(QColor(255, 0, 0))
        self.assertEqual(settings.rasterStrokeColor(), QColor(255, 0, 0))

        settings.setRasterStrokeWidth(2.0)
        self.assertEqual(settings.rasterStrokeWidth(), 2.0)

        settings.setWmsLegendSize(QSizeF(50.0, 150.0))
        self.assertEqual(settings.wmsLegendSize(), QSizeF(50.0, 150.0))

        settings.setSynchronousLegendRequests(True)
        self.assertTrue(settings.synchronousLegendRequests())
        settings.setSynchronousLegendRequests(False)
        self.assertFalse(settings.synchronousLegendRequests())

        settings.setJsonRenderFlags(Qgis.LegendJsonRenderFlag.ShowRuleDetails)
        self.assertEqual(
            settings.jsonRenderFlags(), Qgis.LegendJsonRenderFlag.ShowRuleDetails
        )

    def test_split_string_for_wrapping(self):
        settings = QgsLegendSettings()
        settings.setWrapChar("|")
        self.assertEqual(
            settings.splitStringForWrapping("Test|String|Wrapping"),
            ["Test", "String", "Wrapping"],
        )

    def test_evaluate_item_text(self):
        settings = QgsLegendSettings()
        settings.setWrapChar("|")

        expression_context_scope = QgsExpressionContextScope()
        expression_context_scope.setVariable("test_string", "Test|String")
        expression_context = QgsExpressionContext()
        expression_context.appendScope(expression_context_scope)

        self.assertEqual(
            settings.evaluateItemText(
                "[% @test_string %]|Wrapping", expression_context
            ),
            ["Test", "String", "Wrapping"],
        )

    def test_update_data_defined_properties(self):
        style = QgsLegendStyle()
        text_format = QgsTextFormat()
        text_format.setColor(QColor(255, 0, 0, 255))
        text_format.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.Color,
            QgsProperty.fromExpression("@text_color"),
        )
        style.setTextFormat(text_format)
        settings = QgsLegendSettings()
        settings.setStyle(QgsLegendStyle.Style.Group, style)

        style2 = QgsLegendStyle()
        text_format = QgsTextFormat()
        text_format.setColor(QColor(255, 0, 255, 255))
        text_format.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.Color,
            QgsProperty.fromExpression("@text_color2"),
        )
        style2.setTextFormat(text_format)
        settings.setStyle(QgsLegendStyle.Style.Subgroup, style2)

        self.assertEqual(
            settings.style(QgsLegendStyle.Style.Group).textFormat().color(),
            QColor(255, 0, 0, 255),
        )
        self.assertEqual(
            settings.style(QgsLegendStyle.Style.Subgroup).textFormat().color(),
            QColor(255, 0, 255, 255),
        )

        # apply data defined properties
        rc = QgsRenderContext()
        expression_context_scope = QgsExpressionContextScope()
        expression_context_scope.setVariable("text_color", "0,255,0")
        expression_context_scope.setVariable("text_color2", "255,255,255")
        expression_context = QgsExpressionContext()
        expression_context.appendScope(expression_context_scope)
        rc.setExpressionContext(expression_context)
        settings.updateDataDefinedProperties(rc)

        self.assertEqual(
            settings.style(QgsLegendStyle.Style.Group).textFormat().color(),
            QColor(0, 255, 0, 255),
        )
        self.assertEqual(
            settings.style(QgsLegendStyle.Style.Subgroup).textFormat().color(),
            QColor(255, 255, 255, 255),
        )


if __name__ == "__main__":
    unittest.main()
