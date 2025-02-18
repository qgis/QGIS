""""Test QgsLegendStyle

.. note:: This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

Run with ctest -V -R PyQgsLegendStyle

"""

from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtGui import QColor
from qgis.PyQt.QtXml import QDomDocument

from qgis.core import (
    QgsLegendStyle,
    QgsTextFormat,
    QgsReadWriteContext,
    QgsPalLayerSettings,
    QgsProperty,
    QgsRenderContext,
    QgsExpressionContext,
    QgsExpressionContextScope,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

QGISAPP = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsLegendStyle(QgisTestCase):

    def test_getters_setters(self):
        style = QgsLegendStyle()
        style.setMargin(QgsLegendStyle.Side.Top, 1.5)
        style.setMargin(QgsLegendStyle.Side.Right, 3.6)
        style.setMargin(QgsLegendStyle.Side.Bottom, 4.2)
        style.setMargin(QgsLegendStyle.Side.Left, 5.2)
        self.assertEqual(style.margin(QgsLegendStyle.Side.Top), 1.5)
        self.assertEqual(style.margin(QgsLegendStyle.Side.Right), 3.6)
        self.assertEqual(style.margin(QgsLegendStyle.Side.Bottom), 4.2)
        self.assertEqual(style.margin(QgsLegendStyle.Side.Left), 5.2)

        style.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.assertEqual(style.alignment(), Qt.AlignCenter)

        style.setIndent(33)
        self.assertEqual(style.indent(), 33)

        text_format = QgsTextFormat()
        text_format.setColor(QColor(255, 0, 0, 0))
        style.setTextFormat(text_format)

        self.assertEqual(style.textFormat().color(), QColor(255, 0, 0, 0))

    def test_write_read(self):
        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")

        style = QgsLegendStyle()
        style.setMargin(QgsLegendStyle.Side.Top, 1.5)
        style.setMargin(QgsLegendStyle.Side.Right, 3.6)
        style.setMargin(QgsLegendStyle.Side.Bottom, 4.2)
        style.setMargin(QgsLegendStyle.Side.Left, 5.2)
        style.setAlignment(Qt.AlignmentFlag.AlignCenter)
        style.setIndent(33)
        text_format = QgsTextFormat()
        text_format.setColor(QColor(255, 0, 0, 0))
        style.setTextFormat(text_format)

        style.writeXml("style", elem, doc, QgsReadWriteContext())

        style2 = QgsLegendStyle()
        style2.readXml(elem.firstChildElement("style"), doc, QgsReadWriteContext())

        self.assertEqual(style2.margin(QgsLegendStyle.Side.Top), 1.5)
        self.assertEqual(style2.margin(QgsLegendStyle.Side.Right), 3.6)
        self.assertEqual(style2.margin(QgsLegendStyle.Side.Bottom), 4.2)
        self.assertEqual(style2.margin(QgsLegendStyle.Side.Left), 5.2)
        self.assertEqual(style2.alignment(), Qt.AlignCenter)
        self.assertEqual(style2.indent(), 33)
        self.assertEqual(style2.textFormat().color(), QColor(255, 0, 0, 0))

    def test_update_data_defined_properties(self):
        style = QgsLegendStyle()
        text_format = QgsTextFormat()
        text_format.setColor(QColor(255, 0, 0, 255))
        text_format.dataDefinedProperties().setProperty(
            QgsPalLayerSettings.Property.Color,
            QgsProperty.fromExpression("@text_color"),
        )
        style.setTextFormat(text_format)
        self.assertEqual(style.textFormat().color(), QColor(255, 0, 0, 255))

        # apply data defined properties
        rc = QgsRenderContext()
        expression_context_scope = QgsExpressionContextScope()
        expression_context_scope.setVariable("text_color", "0,255,0")
        expression_context = QgsExpressionContext()
        expression_context.appendScope(expression_context_scope)
        rc.setExpressionContext(expression_context)
        style.updateDataDefinedProperties(rc)

        self.assertEqual(style.textFormat().color(), QColor(0, 255, 0, 255))

    def test_style_name(self):
        self.assertEqual(
            QgsLegendStyle.styleName(QgsLegendStyle.Style.Hidden), "hidden"
        )
        self.assertEqual(
            QgsLegendStyle.styleFromName("hidden"), QgsLegendStyle.Style.Hidden
        )
        self.assertEqual(QgsLegendStyle.styleName(QgsLegendStyle.Style.Title), "title")
        self.assertEqual(
            QgsLegendStyle.styleFromName("title"), QgsLegendStyle.Style.Title
        )
        self.assertEqual(QgsLegendStyle.styleName(QgsLegendStyle.Style.Group), "group")
        self.assertEqual(
            QgsLegendStyle.styleFromName("group"), QgsLegendStyle.Style.Group
        )
        self.assertEqual(
            QgsLegendStyle.styleName(QgsLegendStyle.Style.Subgroup), "subgroup"
        )
        self.assertEqual(
            QgsLegendStyle.styleFromName("subgroup"), QgsLegendStyle.Style.Subgroup
        )
        self.assertEqual(
            QgsLegendStyle.styleName(QgsLegendStyle.Style.Symbol), "symbol"
        )
        self.assertEqual(
            QgsLegendStyle.styleFromName("symbol"), QgsLegendStyle.Style.Symbol
        )
        self.assertEqual(
            QgsLegendStyle.styleName(QgsLegendStyle.Style.SymbolLabel), "symbolLabel"
        )
        self.assertEqual(
            QgsLegendStyle.styleFromName("symbolLabel"),
            QgsLegendStyle.Style.SymbolLabel,
        )


if __name__ == "__main__":
    unittest.main()
