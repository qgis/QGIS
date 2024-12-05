"""QGIS Unit tests for QgsColorRampLegendNode.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "2015-08"
__copyright__ = "Copyright 2015, The QGIS Project"

from qgis.PyQt.QtCore import QSize, QSizeF, Qt
from qgis.PyQt.QtGui import QColor, QImage, QPainter
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    QgsBasicNumericFormat,
    QgsBearingNumericFormat,
    QgsColorRampLegendNode,
    QgsColorRampLegendNodeSettings,
    QgsFontUtils,
    QgsGradientColorRamp,
    QgsLayerTreeLayer,
    QgsLayerTreeModelLegendNode,
    QgsLegendSettings,
    QgsLegendStyle,
    QgsMapSettings,
    QgsReadWriteContext,
    QgsRectangle,
    QgsRenderContext,
    QgsTextFormat,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestColorRampLegend(QgsColorRampLegendNode):
    """
    Override font role to use standard qgis test font
    """

    def data(self, role):
        if role == Qt.ItemDataRole.FontRole:
            return QgsFontUtils.getStandardTestFont("Bold", 18)

        else:
            return super().data(role)


class TestQgsColorRampLegendNode(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "color_ramp_legend_node"

    def test_settings(self):
        settings = QgsColorRampLegendNodeSettings()
        settings.setDirection(QgsColorRampLegendNodeSettings.Direction.MaximumToMinimum)
        self.assertEqual(
            settings.direction(),
            QgsColorRampLegendNodeSettings.Direction.MaximumToMinimum,
        )
        settings.setMinimumLabel("min")
        self.assertEqual(settings.minimumLabel(), "min")
        settings.setMaximumLabel("max")
        self.assertEqual(settings.maximumLabel(), "max")
        settings.setPrefix("pref")
        self.assertEqual(settings.prefix(), "pref")
        settings.setSuffix("suff")
        self.assertEqual(settings.suffix(), "suff")
        self.assertEqual(settings.orientation(), Qt.Orientation.Vertical)
        settings.setOrientation(Qt.Orientation.Horizontal)
        self.assertEqual(settings.orientation(), Qt.Orientation.Horizontal)
        # Test default
        self.assertTrue(settings.useContinuousLegend())
        settings.setUseContinuousLegend(False)
        self.assertFalse(settings.useContinuousLegend())

        self.assertFalse(settings.textFormat().isValid())
        tf = QgsTextFormat()
        tf.setSize(13)
        settings.setTextFormat(tf)
        self.assertEqual(settings.textFormat().size(), 13)

        self.assertIsNotNone(settings.numericFormat())
        settings.setNumericFormat(QgsBearingNumericFormat())
        self.assertIsInstance(settings.numericFormat(), QgsBearingNumericFormat)

        settings2 = QgsColorRampLegendNodeSettings(settings)
        self.assertEqual(
            settings2.direction(),
            QgsColorRampLegendNodeSettings.Direction.MaximumToMinimum,
        )
        self.assertEqual(settings2.minimumLabel(), "min")
        self.assertEqual(settings2.maximumLabel(), "max")
        self.assertIsInstance(settings2.numericFormat(), QgsBearingNumericFormat)
        self.assertEqual(settings2.prefix(), "pref")
        self.assertEqual(settings2.suffix(), "suff")
        self.assertEqual(settings2.textFormat().size(), 13)
        self.assertEqual(settings2.orientation(), Qt.Orientation.Horizontal)

        settings2.setTextFormat(QgsTextFormat())
        settings2a = QgsColorRampLegendNodeSettings(settings2)
        self.assertFalse(settings2a.textFormat().isValid())

        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        settings.writeXml(doc, elem, QgsReadWriteContext())

        settings3 = QgsColorRampLegendNodeSettings()
        settings3.readXml(elem, QgsReadWriteContext())
        self.assertEqual(
            settings3.direction(),
            QgsColorRampLegendNodeSettings.Direction.MaximumToMinimum,
        )
        self.assertEqual(settings3.minimumLabel(), "min")
        self.assertEqual(settings3.maximumLabel(), "max")
        self.assertIsInstance(settings3.numericFormat(), QgsBearingNumericFormat)
        self.assertEqual(settings3.prefix(), "pref")
        self.assertEqual(settings3.suffix(), "suff")
        self.assertEqual(settings3.textFormat().size(), 13)
        self.assertEqual(settings3.orientation(), Qt.Orientation.Horizontal)
        self.assertFalse(settings3.useContinuousLegend())

        # no text format
        elem = doc.createElement("test2")
        settings2.writeXml(doc, elem, QgsReadWriteContext())
        settings3a = QgsColorRampLegendNodeSettings()
        settings3a.readXml(elem, QgsReadWriteContext())
        self.assertFalse(settings3a.textFormat().isValid())

    def test_basic(self):
        r = QgsGradientColorRamp(QColor(200, 0, 0, 100), QColor(0, 200, 0, 200))

        # need a layer in order to make legend nodes
        layer = QgsVectorLayer("dummy", "test", "memory")
        layer_tree_layer = QgsLayerTreeLayer(layer)

        node = QgsColorRampLegendNode(
            layer_tree_layer, r, "min_label", "max_label", None, "key", "parentKey"
        )

        self.assertEqual(node.ramp().color1().name(), "#c80000")
        self.assertEqual(node.ramp().color2().name(), "#00c800")
        self.assertEqual(
            node.data(QgsLayerTreeModelLegendNode.LegendNodeRoles.RuleKeyRole), "key"
        )
        self.assertEqual(
            node.data(QgsLayerTreeModelLegendNode.LegendNodeRoles.ParentRuleKeyRole),
            "parentKey",
        )

        node.setIconSize(QSize(11, 12))
        self.assertEqual(node.iconSize(), QSize(11, 12))

        self.assertEqual(
            node.data(QgsLayerTreeModelLegendNode.LegendNodeRoles.NodeTypeRole),
            QgsLayerTreeModelLegendNode.NodeTypes.ColorRampLegend,
        )

    def test_icon(self):
        r = QgsGradientColorRamp(QColor(200, 0, 0, 100), QColor(0, 200, 0, 200))

        # need a layer in order to make legend nodes
        layer = QgsVectorLayer("dummy", "test", "memory")
        layer_tree_layer = QgsLayerTreeLayer(layer)

        node = TestColorRampLegend(layer_tree_layer, r, "min_label", "max_label")

        pixmap = node.data(Qt.ItemDataRole.DecorationRole)

        im = QImage(pixmap.size(), QImage.Format.Format_ARGB32)
        im.fill(QColor(255, 255, 255))
        p = QPainter(im)
        p.drawPixmap(0, 0, pixmap)
        p.end()

        self.assertTrue(
            self.image_check(
                "color_ramp_legend_node_icon",
                "color_ramp_legend_node_icon",
                im,
                size_tolerance=10,
            )
        )

    def test_icon_with_settings(self):
        r = QgsGradientColorRamp(QColor(200, 0, 0, 100), QColor(0, 200, 0, 200))

        # need a layer in order to make legend nodes
        layer = QgsVectorLayer("dummy", "test", "memory")
        layer_tree_layer = QgsLayerTreeLayer(layer)

        settings = QgsColorRampLegendNodeSettings()
        format = QgsBasicNumericFormat()
        format.setShowTrailingZeros(True)
        format.setNumberDecimalPlaces(3)
        settings.setNumericFormat(format)
        settings.setDirection(QgsColorRampLegendNodeSettings.Direction.MaximumToMinimum)

        node = TestColorRampLegend(layer_tree_layer, r, settings, 5, 10)

        pixmap = node.data(Qt.ItemDataRole.DecorationRole)

        im = QImage(pixmap.size(), QImage.Format.Format_ARGB32)
        im.fill(QColor(255, 255, 255))
        p = QPainter(im)
        p.drawPixmap(0, 0, pixmap)
        p.end()

        self.assertTrue(
            self.image_check(
                "color_ramp_legend_node_settings_icon",
                "color_ramp_legend_node_settings_icon",
                im,
                size_tolerance=10,
            )
        )

    def test_icon_prefix_suffix(self):
        r = QgsGradientColorRamp(QColor(200, 0, 0, 100), QColor(0, 200, 0, 200))

        # need a layer in order to make legend nodes
        layer = QgsVectorLayer("dummy", "test", "memory")
        layer_tree_layer = QgsLayerTreeLayer(layer)

        settings = QgsColorRampLegendNodeSettings()
        settings.setPrefix("pref ")
        settings.setSuffix(" suff")

        node = TestColorRampLegend(layer_tree_layer, r, settings, 5, 10)

        pixmap = node.data(Qt.ItemDataRole.DecorationRole)

        im = QImage(pixmap.size(), QImage.Format.Format_ARGB32)
        im.fill(QColor(255, 255, 255))
        p = QPainter(im)
        p.drawPixmap(0, 0, pixmap)
        p.end()

        self.assertTrue(
            self.image_check(
                "color_ramp_legend_node_prefix_suffix_icon",
                "color_ramp_legend_node_prefix_suffix_icon",
                im,
                size_tolerance=10,
            )
        )

    def test_icon_horizontal(self):
        r = QgsGradientColorRamp(QColor(200, 0, 0, 100), QColor(0, 200, 0, 200))

        # need a layer in order to make legend nodes
        layer = QgsVectorLayer("dummy", "test", "memory")
        layer_tree_layer = QgsLayerTreeLayer(layer)

        settings = QgsColorRampLegendNodeSettings()
        settings.setOrientation(Qt.Orientation.Horizontal)

        node = TestColorRampLegend(layer_tree_layer, r, settings, 5, 10)

        pixmap = node.data(Qt.ItemDataRole.DecorationRole)

        im = QImage(pixmap.size(), QImage.Format.Format_ARGB32)
        im.fill(QColor(255, 255, 255))
        p = QPainter(im)
        p.drawPixmap(0, 0, pixmap)
        p.end()

        self.assertTrue(
            self.image_check(
                "color_ramp_legend_node_horizontal_icon",
                "color_ramp_legend_node_horizontal_icon",
                im,
                size_tolerance=10,
            )
        )

    def test_icon_horizontal_flipped(self):
        r = QgsGradientColorRamp(QColor(200, 0, 0, 100), QColor(0, 200, 0, 200))

        # need a layer in order to make legend nodes
        layer = QgsVectorLayer("dummy", "test", "memory")
        layer_tree_layer = QgsLayerTreeLayer(layer)

        settings = QgsColorRampLegendNodeSettings()
        settings.setDirection(QgsColorRampLegendNodeSettings.Direction.MaximumToMinimum)
        settings.setOrientation(Qt.Orientation.Horizontal)

        node = TestColorRampLegend(layer_tree_layer, r, settings, 5, 10)

        pixmap = node.data(Qt.ItemDataRole.DecorationRole)

        im = QImage(pixmap.size(), QImage.Format.Format_ARGB32)
        im.fill(QColor(255, 255, 255))
        p = QPainter(im)
        p.drawPixmap(0, 0, pixmap)
        p.end()

        self.assertTrue(
            self.image_check(
                "color_ramp_legend_node_flipped_horizontal_icon",
                "color_ramp_legend_node_flipped_horizontal_icon",
                im,
                size_tolerance=10,
            )
        )

    def test_draw(self):
        r = QgsGradientColorRamp(QColor(200, 0, 0, 100), QColor(0, 200, 0, 200))

        # need a layer in order to make legend nodes
        layer = QgsVectorLayer("dummy", "test", "memory")
        layer_tree_layer = QgsLayerTreeLayer(layer)

        node = QgsColorRampLegendNode(layer_tree_layer, r, "min_label", "max_label")

        ls = QgsLegendSettings()
        item_style = ls.style(QgsLegendStyle.Style.SymbolLabel)
        item_style.setFont(QgsFontUtils.getStandardTestFont("Bold", 18))
        ls.setStyle(QgsLegendStyle.Style.SymbolLabel, item_style)

        item_context = QgsLayerTreeModelLegendNode.ItemContext()

        image = QImage(400, 250, QImage.Format.Format_ARGB32)
        image.fill(QColor(255, 255, 255))

        p = QPainter(image)

        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(1, 10, 1, 10))
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(p)
        context.setScaleFactor(150 / 25.4)  # 150 DPI

        p.scale(context.scaleFactor(), context.scaleFactor())

        item_context.context = context
        item_context.painter = p
        item_context.top = 1
        item_context.columnLeft = 3
        item_context.columnRight = 30
        item_context.patchSize = QSizeF(12, 40)

        symbol_size = node.drawSymbol(ls, item_context, 0)
        node.drawSymbolText(ls, item_context, symbol_size)
        p.end()

        self.assertTrue(
            self.image_check(
                "color_ramp_legend_node_draw", "color_ramp_legend_node_draw", image
            )
        )

    def test_draw_settings(self):
        r = QgsGradientColorRamp(QColor(200, 0, 0, 100), QColor(0, 200, 0, 200))

        # need a layer in order to make legend nodes
        layer = QgsVectorLayer("dummy", "test", "memory")
        layer_tree_layer = QgsLayerTreeLayer(layer)

        settings = QgsColorRampLegendNodeSettings()
        format = QgsBasicNumericFormat()
        format.setShowTrailingZeros(True)
        format.setNumberDecimalPlaces(3)
        settings.setNumericFormat(format)
        settings.setDirection(QgsColorRampLegendNodeSettings.Direction.MaximumToMinimum)

        node = QgsColorRampLegendNode(layer_tree_layer, r, settings, 5, 10)

        ls = QgsLegendSettings()
        item_style = ls.style(QgsLegendStyle.Style.SymbolLabel)
        item_style.setFont(QgsFontUtils.getStandardTestFont("Bold", 18))
        ls.setStyle(QgsLegendStyle.Style.SymbolLabel, item_style)

        item_context = QgsLayerTreeModelLegendNode.ItemContext()

        image = QImage(400, 250, QImage.Format.Format_ARGB32)
        image.fill(QColor(255, 255, 255))

        p = QPainter(image)

        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(1, 10, 1, 10))
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(p)
        context.setScaleFactor(150 / 25.4)  # 150 DPI

        p.scale(context.scaleFactor(), context.scaleFactor())

        item_context.context = context
        item_context.painter = p
        item_context.top = 1
        item_context.columnLeft = 3
        item_context.columnRight = 30
        item_context.patchSize = QSizeF(12, 40)

        symbol_size = node.drawSymbol(ls, item_context, 0)
        node.drawSymbolText(ls, item_context, symbol_size)
        p.end()

        self.assertTrue(
            self.image_check(
                "color_ramp_legend_node_settings_draw",
                "color_ramp_legend_node_settings_draw",
                image,
            )
        )

    def test_draw_prefix_suffix(self):
        r = QgsGradientColorRamp(QColor(200, 0, 0, 100), QColor(0, 200, 0, 200))

        # need a layer in order to make legend nodes
        layer = QgsVectorLayer("dummy", "test", "memory")
        layer_tree_layer = QgsLayerTreeLayer(layer)

        settings = QgsColorRampLegendNodeSettings()
        settings.setPrefix("pref ")
        settings.setSuffix(" suff")
        node = QgsColorRampLegendNode(layer_tree_layer, r, settings, 5, 10)

        ls = QgsLegendSettings()
        item_style = ls.style(QgsLegendStyle.Style.SymbolLabel)
        item_style.setFont(QgsFontUtils.getStandardTestFont("Bold", 18))
        ls.setStyle(QgsLegendStyle.Style.SymbolLabel, item_style)

        item_context = QgsLayerTreeModelLegendNode.ItemContext()

        image = QImage(400, 250, QImage.Format.Format_ARGB32)
        image.fill(QColor(255, 255, 255))

        p = QPainter(image)

        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(1, 10, 1, 10))
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(p)
        context.setScaleFactor(150 / 25.4)  # 150 DPI

        p.scale(context.scaleFactor(), context.scaleFactor())

        item_context.context = context
        item_context.painter = p
        item_context.top = 1
        item_context.columnLeft = 3
        item_context.columnRight = 30
        item_context.patchSize = QSizeF(12, 40)

        symbol_size = node.drawSymbol(ls, item_context, 0)
        node.drawSymbolText(ls, item_context, symbol_size)
        p.end()

        self.assertTrue(
            self.image_check(
                "color_ramp_legend_node_prefix_suffix_draw",
                "color_ramp_legend_node_prefix_suffix_draw",
                image,
            )
        )

    def test_draw_text_format(self):
        r = QgsGradientColorRamp(QColor(200, 0, 0, 100), QColor(0, 200, 0, 200))

        # need a layer in order to make legend nodes
        layer = QgsVectorLayer("dummy", "test", "memory")
        layer_tree_layer = QgsLayerTreeLayer(layer)

        settings = QgsColorRampLegendNodeSettings()
        tf = QgsTextFormat()
        tf.setFont(QgsFontUtils.getStandardTestFont("Bold", 18))
        tf.setSize(30)
        tf.setColor(QColor(200, 100, 50))
        settings.setTextFormat(tf)
        node = QgsColorRampLegendNode(layer_tree_layer, r, settings, 5, 10)

        ls = QgsLegendSettings()
        item_style = ls.style(QgsLegendStyle.Style.SymbolLabel)
        item_style.setFont(QgsFontUtils.getStandardTestFont("Bold", 18))
        ls.setStyle(QgsLegendStyle.Style.SymbolLabel, item_style)

        item_context = QgsLayerTreeModelLegendNode.ItemContext()

        image = QImage(400, 250, QImage.Format.Format_ARGB32)
        image.fill(QColor(255, 255, 255))

        p = QPainter(image)

        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(1, 10, 1, 10))
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(p)
        context.setScaleFactor(150 / 25.4)  # 150 DPI

        p.scale(context.scaleFactor(), context.scaleFactor())

        item_context.context = context
        item_context.painter = p
        item_context.top = 1
        item_context.columnLeft = 3
        item_context.columnRight = 30
        item_context.patchSize = QSizeF(12, 40)

        symbol_size = node.drawSymbol(ls, item_context, 0)
        node.drawSymbolText(ls, item_context, symbol_size)
        p.end()

        self.assertTrue(
            self.image_check(
                "color_ramp_legend_node_text_format_draw",
                "color_ramp_legend_node_text_format_draw",
                image,
            )
        )

    def test_draw_horizontal(self):
        r = QgsGradientColorRamp(QColor(200, 0, 0, 100), QColor(0, 200, 0, 200))

        # need a layer in order to make legend nodes
        layer = QgsVectorLayer("dummy", "test", "memory")
        layer_tree_layer = QgsLayerTreeLayer(layer)

        settings = QgsColorRampLegendNodeSettings()
        settings.setOrientation(Qt.Orientation.Horizontal)
        node = QgsColorRampLegendNode(layer_tree_layer, r, settings, 5, 10)

        ls = QgsLegendSettings()
        item_style = ls.style(QgsLegendStyle.Style.SymbolLabel)
        item_style.setFont(QgsFontUtils.getStandardTestFont("Bold", 18))
        ls.setStyle(QgsLegendStyle.Style.SymbolLabel, item_style)

        item_context = QgsLayerTreeModelLegendNode.ItemContext()

        image = QImage(400, 250, QImage.Format.Format_ARGB32)
        image.fill(QColor(255, 255, 255))

        p = QPainter(image)

        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(1, 10, 1, 10))
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(p)
        context.setScaleFactor(150 / 25.4)  # 150 DPI

        p.scale(context.scaleFactor(), context.scaleFactor())

        item_context.context = context
        item_context.painter = p
        item_context.top = 1
        item_context.columnLeft = 3
        item_context.columnRight = 50
        item_context.patchSize = QSizeF(40, 12)

        symbol_size = node.drawSymbol(ls, item_context, 0)
        node.drawSymbolText(ls, item_context, symbol_size)
        p.end()

        self.assertTrue(
            self.image_check(
                "color_ramp_legend_node_horizontal_draw",
                "color_ramp_legend_node_horizontal_draw",
                image,
            )
        )

    def test_draw_horizontal_reversed(self):
        r = QgsGradientColorRamp(QColor(200, 0, 0, 100), QColor(0, 200, 0, 200))

        # need a layer in order to make legend nodes
        layer = QgsVectorLayer("dummy", "test", "memory")
        layer_tree_layer = QgsLayerTreeLayer(layer)

        settings = QgsColorRampLegendNodeSettings()
        settings.setOrientation(Qt.Orientation.Horizontal)
        settings.setDirection(QgsColorRampLegendNodeSettings.Direction.MaximumToMinimum)
        node = QgsColorRampLegendNode(layer_tree_layer, r, settings, 5, 10)

        ls = QgsLegendSettings()
        item_style = ls.style(QgsLegendStyle.Style.SymbolLabel)
        item_style.setFont(QgsFontUtils.getStandardTestFont("Bold", 18))
        ls.setStyle(QgsLegendStyle.Style.SymbolLabel, item_style)

        item_context = QgsLayerTreeModelLegendNode.ItemContext()

        image = QImage(400, 250, QImage.Format.Format_ARGB32)
        image.fill(QColor(255, 255, 255))

        p = QPainter(image)

        ms = QgsMapSettings()
        ms.setExtent(QgsRectangle(1, 10, 1, 10))
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(p)
        context.setScaleFactor(150 / 25.4)  # 150 DPI

        p.scale(context.scaleFactor(), context.scaleFactor())

        item_context.context = context
        item_context.painter = p
        item_context.top = 1
        item_context.columnLeft = 3
        item_context.columnRight = 50
        item_context.patchSize = QSizeF(40, 12)

        symbol_size = node.drawSymbol(ls, item_context, 0)
        node.drawSymbolText(ls, item_context, symbol_size)
        p.end()

        self.assertTrue(
            self.image_check(
                "color_ramp_legend_node_flipped_horizontal_draw",
                "color_ramp_legend_node_flipped_horizontal_draw",
                image,
            )
        )


if __name__ == "__main__":
    unittest.main()
