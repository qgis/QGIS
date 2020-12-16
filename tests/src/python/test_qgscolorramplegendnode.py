# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsColorRampLegendNode.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2015-08'
__copyright__ = 'Copyright 2015, The QGIS Project'

import qgis  # NOQA
from qgis.PyQt.QtCore import QSize, QDir, Qt, QSizeF
from qgis.PyQt.QtGui import QColor, QImage, QPainter
from qgis.core import (QgsGradientColorRamp,
                       QgsRectangle,
                       QgsLayerTreeModelLegendNode,
                       QgsColorRampLegendNode,
                       QgsLayerTreeLayer,
                       QgsVectorLayer,
                       QgsMultiRenderChecker,
                       QgsFontUtils,
                       QgsLegendSettings,
                       QgsLegendStyle,
                       QgsLayerTreeModelLegendNode,
                       QgsRenderContext,
                       QgsMapSettings)
from qgis.testing import start_app, unittest

start_app()


class TestColorRampLegend(QgsColorRampLegendNode):

    """
    Override font role to use standard qgis test font
    """

    def data(self, role):
        if role == Qt.FontRole:
            return QgsFontUtils.getStandardTestFont('Bold', 18)

        else:
            return super().data(role)


class TestQgsColorRampLegendNode(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.report = "<h1>Python QgsColorRampLegendNode Tests</h1>\n"

    @classmethod
    def tearDownClass(cls):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(cls.report)

    def test_basic(self):
        r = QgsGradientColorRamp(QColor(200, 0, 0, 100), QColor(0, 200, 0, 200))

        # need a layer in order to make legend nodes
        layer = QgsVectorLayer('dummy', 'test', 'memory')
        layer_tree_layer = QgsLayerTreeLayer(layer)

        node = QgsColorRampLegendNode(layer_tree_layer, r, 'min_label', 'max_label')

        self.assertEqual(node.ramp().color1().name(), '#c80000')
        self.assertEqual(node.ramp().color2().name(), '#00c800')

        node.setIconSize(QSize(11, 12))
        self.assertEqual(node.iconSize(), QSize(11, 12))

        self.assertEqual(node.data(QgsLayerTreeModelLegendNode.NodeTypeRole),
                         QgsLayerTreeModelLegendNode.ColorRampLegend)

    def test_icon(self):
        r = QgsGradientColorRamp(QColor(200, 0, 0, 100), QColor(0, 200, 0, 200))

        # need a layer in order to make legend nodes
        layer = QgsVectorLayer('dummy', 'test', 'memory')
        layer_tree_layer = QgsLayerTreeLayer(layer)

        node = TestColorRampLegend(layer_tree_layer, r, 'min_label', 'max_label')

        pixmap = node.data(Qt.DecorationRole)

        im = QImage(pixmap.size(), QImage.Format_ARGB32)
        im.fill(QColor(255, 255, 255))
        p = QPainter(im)
        p.drawPixmap(0, 0, pixmap)
        p.end()

        self.assertTrue(self.imageCheck('color_ramp_legend_node_icon', 'color_ramp_legend_node_icon', im, 10))

    def test_draw(self):
        r = QgsGradientColorRamp(QColor(200, 0, 0, 100), QColor(0, 200, 0, 200))

        # need a layer in order to make legend nodes
        layer = QgsVectorLayer('dummy', 'test', 'memory')
        layer_tree_layer = QgsLayerTreeLayer(layer)

        node = QgsColorRampLegendNode(layer_tree_layer, r, 'min_label', 'max_label')

        ls = QgsLegendSettings()
        item_style = ls.style(QgsLegendStyle.SymbolLabel)
        item_style.setFont(QgsFontUtils.getStandardTestFont('Bold', 18))
        ls.setStyle(QgsLegendStyle.SymbolLabel, item_style)

        item_context = QgsLayerTreeModelLegendNode.ItemContext()

        image = QImage(400, 250, QImage.Format_ARGB32)
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

        node.drawSymbol(ls, item_context, 0)
        p.end()

        self.assertTrue(self.imageCheck('color_ramp_legend_node_draw', 'color_ramp_legend_node_draw', image))

    def imageCheck(self, name, reference_image, image, size_tolerance=0):
        TestQgsColorRampLegendNode.report += "<h2>Render {}</h2>\n".format(name)
        temp_dir = QDir.tempPath() + '/'
        file_name = temp_dir + name + ".png"
        image.save(file_name, "PNG")
        checker = QgsMultiRenderChecker()
        checker.setControlPathPrefix("color_ramp_legend_node")
        checker.setControlName("expected_" + reference_image)
        checker.setRenderedImage(file_name)
        checker.setColorTolerance(2)
        checker.setSizeTolerance(size_tolerance, size_tolerance)
        result = checker.runTest(name, 20)
        TestQgsColorRampLegendNode.report += checker.report()
        return result


if __name__ == '__main__':
    unittest.main()
