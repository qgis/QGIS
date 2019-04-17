# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayoutItemLegend.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2017 by Nyall Dawson'
__date__ = '24/10/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.PyQt.QtCore import QRectF
from qgis.PyQt.QtGui import QColor

from qgis.core import (QgsPrintLayout,
                       QgsLayoutItemLegend,
                       QgsLayoutItemMap,
                       QgsLayout,
                       QgsMapSettings,
                       QgsVectorLayer,
                       QgsMarkerSymbol,
                       QgsSingleSymbolRenderer,
                       QgsRectangle,
                       QgsProject,
                       QgsLayoutObject,
                       QgsProperty,
                       QgsLayoutMeasurement,
                       QgsLayoutItem,
                       QgsLayoutPoint,
                       QgsLayoutSize,
                       QgsExpression,
                       QgsMapLayerLegendUtils,
                       QgsLegendStyle,
                       QgsFontUtils)
from qgis.testing import (start_app,
                          unittest
                          )
from utilities import unitTestDataPath
from qgslayoutchecker import QgsLayoutChecker
import os
from time import sleep
from test_qgslayoutitem import LayoutItemTestCase

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayoutItemLegend(unittest.TestCase, LayoutItemTestCase):

    @classmethod
    def setUpClass(cls):
        cls.item_class = QgsLayoutItemLegend

    def testInitialSizeSymbolMapUnits(self):
        """Test initial size of legend with a symbol size in map units"""

        QgsProject.instance().clear()
        point_path = os.path.join(TEST_DATA_DIR, 'points.shp')
        point_layer = QgsVectorLayer(point_path, 'points', 'ogr')
        QgsProject.instance().addMapLayers([point_layer])

        marker_symbol = QgsMarkerSymbol.createSimple({'color': '#ff0000', 'outline_style': 'no', 'size': '5', 'size_unit': 'MapUnit'})

        point_layer.setRenderer(QgsSingleSymbolRenderer(marker_symbol))

        s = QgsMapSettings()
        s.setLayers([point_layer])
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()

        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(20, 20, 80, 80))
        map.setFrameEnabled(True)
        map.setLayers([point_layer])
        layout.addLayoutItem(map)
        map.setExtent(point_layer.extent())

        legend = QgsLayoutItemLegend(layout)
        legend.setTitle("Legend")
        legend.attemptSetSceneRect(QRectF(120, 20, 80, 80))
        legend.setFrameEnabled(True)
        legend.setFrameStrokeWidth(QgsLayoutMeasurement(2))
        legend.setBackgroundColor(QColor(200, 200, 200))
        legend.setTitle('')
        layout.addLayoutItem(legend)
        legend.setLinkedMap(map)

        checker = QgsLayoutChecker(
            'composer_legend_mapunits', layout)
        checker.setControlPathPrefix("composer_legend")
        result, message = checker.testLayout()
        self.assertTrue(result, message)

        # resize with non-top-left reference point
        legend.setResizeToContents(False)
        legend.setReferencePoint(QgsLayoutItem.LowerRight)
        legend.attemptMove(QgsLayoutPoint(120, 90))
        legend.attemptResize(QgsLayoutSize(50, 60))

        self.assertEqual(legend.positionWithUnits().x(), 120.0)
        self.assertEqual(legend.positionWithUnits().y(), 90.0)
        self.assertAlmostEqual(legend.pos().x(), 70, -1)
        self.assertAlmostEqual(legend.pos().y(), 30, -1)

        legend.setResizeToContents(True)
        legend.updateLegend()
        self.assertEqual(legend.positionWithUnits().x(), 120.0)
        self.assertEqual(legend.positionWithUnits().y(), 90.0)
        self.assertAlmostEqual(legend.pos().x(), 91, -1)
        self.assertAlmostEqual(legend.pos().y(), 71, -1)

        QgsProject.instance().removeMapLayers([point_layer.id()])

    def testResizeWithMapContent(self):
        """Test test legend resizes to match map content"""

        point_path = os.path.join(TEST_DATA_DIR, 'points.shp')
        point_layer = QgsVectorLayer(point_path, 'points', 'ogr')
        QgsProject.instance().addMapLayers([point_layer])

        s = QgsMapSettings()
        s.setLayers([point_layer])
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()

        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(20, 20, 80, 80))
        map.setFrameEnabled(True)
        map.setLayers([point_layer])
        layout.addLayoutItem(map)
        map.setExtent(point_layer.extent())

        legend = QgsLayoutItemLegend(layout)
        legend.setTitle("Legend")
        legend.attemptSetSceneRect(QRectF(120, 20, 80, 80))
        legend.setFrameEnabled(True)
        legend.setFrameStrokeWidth(QgsLayoutMeasurement(2))
        legend.setBackgroundColor(QColor(200, 200, 200))
        legend.setTitle('')
        legend.setLegendFilterByMapEnabled(True)
        layout.addLayoutItem(legend)
        legend.setLinkedMap(map)

        map.setExtent(QgsRectangle(-102.51, 41.16, -102.36, 41.30))

        checker = QgsLayoutChecker(
            'composer_legend_size_content', layout)
        checker.setControlPathPrefix("composer_legend")
        result, message = checker.testLayout()
        self.assertTrue(result, message)

        QgsProject.instance().removeMapLayers([point_layer.id()])

    def testResizeDisabled(self):
        """Test that test legend does not resize if auto size is disabled"""

        point_path = os.path.join(TEST_DATA_DIR, 'points.shp')
        point_layer = QgsVectorLayer(point_path, 'points', 'ogr')
        QgsProject.instance().addMapLayers([point_layer])

        s = QgsMapSettings()
        s.setLayers([point_layer])
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()

        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(20, 20, 80, 80))
        map.setFrameEnabled(True)
        map.setLayers([point_layer])
        layout.addLayoutItem(map)
        map.setExtent(point_layer.extent())

        legend = QgsLayoutItemLegend(layout)
        legend.setTitle("Legend")
        legend.attemptSetSceneRect(QRectF(120, 20, 80, 80))
        legend.setFrameEnabled(True)
        legend.setFrameStrokeWidth(QgsLayoutMeasurement(2))
        legend.setBackgroundColor(QColor(200, 200, 200))
        legend.setTitle('')
        legend.setLegendFilterByMapEnabled(True)

        # disable auto resizing
        legend.setResizeToContents(False)

        layout.addLayoutItem(legend)
        legend.setLinkedMap(map)

        map.setExtent(QgsRectangle(-102.51, 41.16, -102.36, 41.30))

        checker = QgsLayoutChecker(
            'composer_legend_noresize', layout)
        checker.setControlPathPrefix("composer_legend")
        result, message = checker.testLayout()
        self.assertTrue(result, message)

        QgsProject.instance().removeMapLayers([point_layer.id()])

    def testResizeDisabledCrop(self):
        """Test that if legend resizing is disabled, and legend is too small, then content is cropped"""

        point_path = os.path.join(TEST_DATA_DIR, 'points.shp')
        point_layer = QgsVectorLayer(point_path, 'points', 'ogr')
        QgsProject.instance().addMapLayers([point_layer])

        s = QgsMapSettings()
        s.setLayers([point_layer])
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()

        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(20, 20, 80, 80))
        map.setFrameEnabled(True)
        map.setLayers([point_layer])
        layout.addLayoutItem(map)
        map.setExtent(point_layer.extent())

        legend = QgsLayoutItemLegend(layout)
        legend.setTitle("Legend")
        legend.attemptSetSceneRect(QRectF(120, 20, 20, 20))
        legend.setFrameEnabled(True)
        legend.setFrameStrokeWidth(QgsLayoutMeasurement(2))
        legend.setBackgroundColor(QColor(200, 200, 200))
        legend.setTitle('')
        legend.setLegendFilterByMapEnabled(True)

        # disable auto resizing
        legend.setResizeToContents(False)

        layout.addLayoutItem(legend)
        legend.setLinkedMap(map)

        map.setExtent(QgsRectangle(-102.51, 41.16, -102.36, 41.30))

        checker = QgsLayoutChecker(
            'composer_legend_noresize_crop', layout)
        checker.setControlPathPrefix("composer_legend")
        result, message = checker.testLayout()
        self.assertTrue(result, message)

        QgsProject.instance().removeMapLayers([point_layer.id()])

    def testDataDefinedTitle(self):
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()

        legend = QgsLayoutItemLegend(layout)
        layout.addLayoutItem(legend)

        legend.setTitle('original')
        self.assertEqual(legend.title(), 'original')
        self.assertEqual(legend.legendSettings().title(), 'original')

        legend.dataDefinedProperties().setProperty(QgsLayoutObject.LegendTitle, QgsProperty.fromExpression("'new'"))
        legend.refreshDataDefinedProperty()
        self.assertEqual(legend.title(), 'original')
        self.assertEqual(legend.legendSettings().title(), 'new')

    def testDataDefinedColumnCount(self):
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()

        legend = QgsLayoutItemLegend(layout)
        legend.setTitle("Legend")
        layout.addLayoutItem(legend)

        legend.setColumnCount(2)
        self.assertEqual(legend.columnCount(), 2)
        self.assertEqual(legend.legendSettings().columnCount(), 2)

        legend.dataDefinedProperties().setProperty(QgsLayoutObject.LegendColumnCount, QgsProperty.fromExpression("5"))
        legend.refreshDataDefinedProperty()
        self.assertEqual(legend.columnCount(), 2)
        self.assertEqual(legend.legendSettings().columnCount(), 5)

    def testLegendScopeVariables(self):
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()

        legend = QgsLayoutItemLegend(layout)
        legend.setTitle("Legend")
        layout.addLayoutItem(legend)

        legend.setColumnCount(2)
        legend.setWrapString('d')
        legend.setLegendFilterOutAtlas(True)

        expc = legend.createExpressionContext()
        exp1 = QgsExpression("@legend_title")
        self.assertEqual(exp1.evaluate(expc), "Legend")
        exp2 = QgsExpression("@legend_column_count")
        self.assertEqual(exp2.evaluate(expc), 2)
        exp3 = QgsExpression("@legend_wrap_string")
        self.assertEqual(exp3.evaluate(expc), 'd')
        exp4 = QgsExpression("@legend_split_layers")
        self.assertEqual(exp4.evaluate(expc), False)
        exp5 = QgsExpression("@legend_filter_out_atlas")
        self.assertEqual(exp5.evaluate(expc), True)

        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(20, 20, 80, 80))
        map.setFrameEnabled(True)
        map.setExtent(QgsRectangle(781662.375, 3339523.125, 793062.375, 3345223.125))
        layout.addLayoutItem(map)
        map.setScale(15000)
        legend.setLinkedMap(map)
        expc2 = legend.createExpressionContext()
        exp6 = QgsExpression("@map_scale")
        self.assertAlmostEqual(exp6.evaluate(expc2), 15000, 2)

    def testExpressionInText(self):
        """Test expressions embedded in legend node text"""

        point_path = os.path.join(TEST_DATA_DIR, 'points.shp')
        point_layer = QgsVectorLayer(point_path, 'points', 'ogr')

        layout = QgsPrintLayout(QgsProject.instance())
        layout.setName('LAYOUT')
        layout.initializeDefaults()

        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(20, 20, 80, 80))
        map.setFrameEnabled(True)
        map.setLayers([point_layer])
        layout.addLayoutItem(map)
        map.setExtent(point_layer.extent())

        legend = QgsLayoutItemLegend(layout)
        legend.setTitle("Legend")
        legend.attemptSetSceneRect(QRectF(120, 20, 100, 100))
        legend.setFrameEnabled(True)
        legend.setFrameStrokeWidth(QgsLayoutMeasurement(2))
        legend.setBackgroundColor(QColor(200, 200, 200))
        legend.setTitle('')
        legend.setLegendFilterByMapEnabled(False)
        legend.setStyleFont(QgsLegendStyle.Title, QgsFontUtils.getStandardTestFont('Bold', 16))
        legend.setStyleFont(QgsLegendStyle.Group, QgsFontUtils.getStandardTestFont('Bold', 16))
        legend.setStyleFont(QgsLegendStyle.Subgroup, QgsFontUtils.getStandardTestFont('Bold', 16))
        legend.setStyleFont(QgsLegendStyle.Symbol, QgsFontUtils.getStandardTestFont('Bold', 16))
        legend.setStyleFont(QgsLegendStyle.SymbolLabel, QgsFontUtils.getStandardTestFont('Bold', 16))

        legend.setAutoUpdateModel(False)

        QgsProject.instance().addMapLayers([point_layer])
        s = QgsMapSettings()
        s.setLayers([point_layer])

        group = legend.model().rootGroup().addGroup("Group [% 1 + 5 %] [% @layout_name %]")
        layer_tree_layer = group.addLayer(point_layer)
        layer_tree_layer.setCustomProperty("legend/title-label", 'bbbb [% 1+2 %] xx [% @layout_name %] [% @layer_name %]')
        QgsMapLayerLegendUtils.setLegendNodeUserLabel(layer_tree_layer, 0, 'xxxx')
        legend.model().refreshLayerLegend(layer_tree_layer)
        legend.model().layerLegendNodes(layer_tree_layer)[0].setUserLabel('bbbb [% 1+2 %] xx [% @layout_name %] [% @layer_name %]')

        layout.addLayoutItem(legend)
        legend.setLinkedMap(map)

        map.setExtent(QgsRectangle(-102.51, 41.16, -102.36, 41.30))

        checker = QgsLayoutChecker(
            'composer_legend_expressions', layout)
        checker.setControlPathPrefix("composer_legend")
        result, message = checker.testLayout()
        self.assertTrue(result, message)

        QgsProject.instance().removeMapLayers([point_layer.id()])

    def testSymbolExpressions(self):
        """Test expressions embedded in legend node text"""

        point_path = os.path.join(TEST_DATA_DIR, 'points.shp')
        point_layer = QgsVectorLayer(point_path, 'points', 'ogr')
        point_layer.countSymbolFeatures()
        QgsProject.instance().clear()

        layout = QgsPrintLayout(QgsProject.instance())
        layout.initializeDefaults()

        map = QgsLayoutItemMap(layout)
        map.setLayers([point_layer])
        layout.addLayoutItem(map)
        map.setExtent(point_layer.extent())

        legend = QgsLayoutItemLegend(layout)

        QgsProject.instance().addMapLayers([point_layer])
        legendlayer = legend.model().rootGroup().addLayer(point_layer)
        sleep(1) # try to ensure that the counting is done before evaluation

        legend.model().refreshLayerLegend(legendlayer)
        legendnodes = legend.model().layerLegendNodes(legendlayer)
        legendnodes[0].setUserLabel('[%@symbol_id %]')
        legendnodes[1].setUserLabel('[%@symbol_count %]')
        legendnodes[2].setUserLabel('[%sum("Pilots") %]')

        label1 = legendnodes[0].evaluateLabel()
        label2 = legendnodes[1].evaluateLabel()
        label3 = legendnodes[2].evaluateLabel()

        self.assertEqual(label1, '0')
        self.assertEqual(label2, '5')
        self.assertEqual(label3, '12')

        legendlayer.setExpression("Concat(@symbol_label, @symbol_id)")

        label1 = legendnodes[0].evaluateLabel()
        label2 = legendnodes[1].evaluateLabel()
        label3 = legendnodes[2].evaluateLabel()

        self.assertEqual(label1, '@symbol_id 0')
        self.assertEqual(label2, '@symbol_count 1')
        self.assertEqual(label3, 'sum("Pilots") 2')

        QgsProject.instance().clear()


if __name__ == '__main__':
    unittest.main()
