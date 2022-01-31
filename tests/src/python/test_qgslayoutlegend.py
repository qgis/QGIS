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

from qgis.PyQt.QtCore import QRectF, QDir
from qgis.PyQt.QtGui import QColor
from qgis.PyQt.QtXml import QDomDocument

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
                       QgsFontUtils,
                       QgsLineSymbol,
                       QgsMapThemeCollection,
                       QgsCategorizedSymbolRenderer,
                       QgsRendererCategory,
                       QgsFillSymbol,
                       QgsUnitTypes,
                       QgsApplication,
                       QgsReadWriteContext)
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
        cls.report = "<h1>Python QgsLayoutItemLegend Tests</h1>\n"

    @classmethod
    def tearDownClass(cls):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(cls.report)

    def testInitialSizeSymbolMapUnits(self):
        """Test initial size of legend with a symbol size in map units"""
        QgsProject.instance().removeAllMapLayers()

        point_path = os.path.join(TEST_DATA_DIR, 'points.shp')
        point_layer = QgsVectorLayer(point_path, 'points', 'ogr')
        QgsProject.instance().clear()
        QgsProject.instance().addMapLayers([point_layer])

        marker_symbol = QgsMarkerSymbol.createSimple(
            {'color': '#ff0000', 'outline_style': 'no', 'size': '5', 'size_unit': 'MapUnit'})

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
        TestQgsLayoutItemLegend.report += checker.report()
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
        TestQgsLayoutItemLegend.report += checker.report()
        self.assertTrue(result, message)

        QgsProject.instance().removeMapLayers([point_layer.id()])

    def testResizeWithMapContentNoDoublePaint(self):
        """Test test legend resizes to match map content"""
        poly_path = os.path.join(TEST_DATA_DIR, 'polys.shp')
        poly_layer = QgsVectorLayer(poly_path, 'polys', 'ogr')
        p = QgsProject()
        p.addMapLayers([poly_layer])

        fill_symbol = QgsFillSymbol.createSimple({'color': '255,0,0,125', 'outline_style': 'no'})
        poly_layer.setRenderer(QgsSingleSymbolRenderer(fill_symbol))

        s = QgsMapSettings()
        s.setLayers([poly_layer])
        layout = QgsLayout(p)
        layout.initializeDefaults()

        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(20, 20, 80, 80))
        map.setFrameEnabled(True)
        map.setLayers([poly_layer])
        layout.addLayoutItem(map)
        map.setExtent(poly_layer.extent())

        legend = QgsLayoutItemLegend(layout)
        legend.setTitle("Legend")
        legend.attemptSetSceneRect(QRectF(120, 20, 80, 80))
        legend.setFrameEnabled(True)
        legend.setFrameStrokeWidth(QgsLayoutMeasurement(2))
        legend.setBackgroundEnabled(False)
        legend.setTitle('')
        layout.addLayoutItem(legend)
        legend.setLinkedMap(map)

        map.setExtent(QgsRectangle(-102.51, 41.16, -102.36, 41.30))

        checker = QgsLayoutChecker(
            'composer_legend_size_content_no_double_paint', layout)
        checker.setControlPathPrefix("composer_legend")
        result, message = checker.testLayout()
        TestQgsLayoutItemLegend.report += checker.report()
        self.assertTrue(result, message)

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
        TestQgsLayoutItemLegend.report += checker.report()
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
        TestQgsLayoutItemLegend.report += checker.report()
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
        layer_tree_layer.setCustomProperty("legend/title-label",
                                           'bbbb [% 1+2 %] xx [% @layout_name %] [% @layer_name %]')
        QgsMapLayerLegendUtils.setLegendNodeUserLabel(layer_tree_layer, 0, 'xxxx')
        legend.model().refreshLayerLegend(layer_tree_layer)
        legend.model().layerLegendNodes(layer_tree_layer)[0].setUserLabel(
            'bbbb [% 1+2 %] xx [% @layout_name %] [% @layer_name %]')

        layout.addLayoutItem(legend)
        legend.setLinkedMap(map)

        map.setExtent(QgsRectangle(-102.51, 41.16, -102.36, 41.30))

        checker = QgsLayoutChecker(
            'composer_legend_expressions', layout)
        checker.setControlPathPrefix("composer_legend")
        result, message = checker.testLayout()
        TestQgsLayoutItemLegend.report += checker.report()
        self.assertTrue(result, message)

        QgsProject.instance().removeMapLayers([point_layer.id()])

    def testSymbolExpressions(self):
        "Test expressions embedded in legend node text"
        QgsProject.instance().clear()
        point_path = os.path.join(TEST_DATA_DIR, 'points.shp')
        point_layer = QgsVectorLayer(point_path, 'points', 'ogr')

        layout = QgsPrintLayout(QgsProject.instance())
        layout.initializeDefaults()

        map = QgsLayoutItemMap(layout)
        map.setLayers([point_layer])
        layout.addLayoutItem(map)
        map.setExtent(point_layer.extent())

        legend = QgsLayoutItemLegend(layout)

        layer = QgsProject.instance().addMapLayer(point_layer)
        legendlayer = legend.model().rootGroup().addLayer(point_layer)

        counterTask = point_layer.countSymbolFeatures()
        counterTask.waitForFinished()
        legend.model().refreshLayerLegend(legendlayer)
        legendnodes = legend.model().layerLegendNodes(legendlayer)
        legendnodes[0].setUserLabel('[% @symbol_id %]')
        legendnodes[1].setUserLabel('[% @symbol_count %]')
        legendnodes[2].setUserLabel('[% sum("Pilots") %]')
        label1 = legendnodes[0].evaluateLabel()
        label2 = legendnodes[1].evaluateLabel()
        label3 = legendnodes[2].evaluateLabel()
        self.assertEqual(label1, '0')
        # self.assertEqual(label2, '5')
        # self.assertEqual(label3, '12')

        legendlayer.setLabelExpression("Concat(@symbol_label, @symbol_id)")

        label1 = legendnodes[0].evaluateLabel()
        label2 = legendnodes[1].evaluateLabel()
        label3 = legendnodes[2].evaluateLabel()

        self.assertEqual(label1, ' @symbol_id 0')
        # self.assertEqual(label2, '@symbol_count 1')
        # self.assertEqual(label3, 'sum("Pilots") 2')

        QgsProject.instance().clear()

    def testSymbolExpressionRender(self):
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
        counterTask = point_layer.countSymbolFeatures()
        counterTask.waitForFinished()
        layer_tree_layer.setCustomProperty("legend/title-label",
                                           'bbbb [% 1+2 %] xx [% @layout_name %] [% @layer_name %]')
        QgsMapLayerLegendUtils.setLegendNodeUserLabel(layer_tree_layer, 0, 'xxxx')
        legend.model().refreshLayerLegend(layer_tree_layer)
        layer_tree_layer.setLabelExpression('Concat(@symbol_id, @symbol_label, count("Class"))')
        legend.model().layerLegendNodes(layer_tree_layer)[0].setUserLabel(' sym 1')
        legend.model().layerLegendNodes(layer_tree_layer)[1].setUserLabel('[%@symbol_count %]')
        legend.model().layerLegendNodes(layer_tree_layer)[2].setUserLabel('[% count("Class") %]')
        layout.addLayoutItem(legend)
        legend.setLinkedMap(map)
        legend.updateLegend()
        print(layer_tree_layer.labelExpression())
        map.setExtent(QgsRectangle(-102.51, 41.16, -102.36, 41.30))
        checker = QgsLayoutChecker(
            'composer_legend_symbol_expression', layout)
        checker.setControlPathPrefix("composer_legend")
        sleep(4)
        result, message = checker.testLayout()
        self.assertTrue(result, message)

        QgsProject.instance().removeMapLayers([point_layer.id()])

    def testThemes(self):
        layout = QgsPrintLayout(QgsProject.instance())
        layout.setName('LAYOUT')

        map = QgsLayoutItemMap(layout)
        layout.addLayoutItem(map)
        legend = QgsLayoutItemLegend(layout)

        self.assertFalse(legend.themeName())
        legend.setLinkedMap(map)
        self.assertFalse(legend.themeName())

        map.setFollowVisibilityPresetName('theme1')
        map.setFollowVisibilityPreset(True)
        self.assertEqual(legend.themeName(), 'theme1')
        map.setFollowVisibilityPresetName('theme2')
        self.assertEqual(legend.themeName(), 'theme2')
        map.setFollowVisibilityPreset(False)
        self.assertFalse(legend.themeName())

        # with theme set before linking map
        map2 = QgsLayoutItemMap(layout)
        map2.setFollowVisibilityPresetName('theme3')
        map2.setFollowVisibilityPreset(True)
        legend.setLinkedMap(map2)
        self.assertEqual(legend.themeName(), 'theme3')
        map2.setFollowVisibilityPresetName('theme2')
        self.assertEqual(legend.themeName(), 'theme2')

        # replace with map with no theme
        map3 = QgsLayoutItemMap(layout)
        legend.setLinkedMap(map3)
        self.assertFalse(legend.themeName())

    def testLegendRenderWithMapTheme(self):
        """Test rendering legends linked to map themes"""
        QgsProject.instance().removeAllMapLayers()

        point_path = os.path.join(TEST_DATA_DIR, 'points.shp')
        point_layer = QgsVectorLayer(point_path, 'points', 'ogr')
        line_path = os.path.join(TEST_DATA_DIR, 'lines.shp')
        line_layer = QgsVectorLayer(line_path, 'lines', 'ogr')
        QgsProject.instance().clear()
        QgsProject.instance().addMapLayers([point_layer, line_layer])

        marker_symbol = QgsMarkerSymbol.createSimple({'color': '#ff0000', 'outline_style': 'no', 'size': '5'})
        point_layer.setRenderer(QgsSingleSymbolRenderer(marker_symbol))
        point_layer.styleManager().addStyleFromLayer("red")

        line_symbol = QgsLineSymbol.createSimple({'color': '#ff0000', 'line_width': '2'})
        line_layer.setRenderer(QgsSingleSymbolRenderer(line_symbol))
        line_layer.styleManager().addStyleFromLayer("red")

        red_record = QgsMapThemeCollection.MapThemeRecord()
        point_red_record = QgsMapThemeCollection.MapThemeLayerRecord(point_layer)
        point_red_record.usingCurrentStyle = True
        point_red_record.currentStyle = 'red'
        red_record.addLayerRecord(point_red_record)
        line_red_record = QgsMapThemeCollection.MapThemeLayerRecord(line_layer)
        line_red_record.usingCurrentStyle = True
        line_red_record.currentStyle = 'red'
        red_record.addLayerRecord(line_red_record)
        QgsProject.instance().mapThemeCollection().insert('red', red_record)

        marker_symbol1 = QgsMarkerSymbol.createSimple({'color': '#0000ff', 'outline_style': 'no', 'size': '5'})
        marker_symbol2 = QgsMarkerSymbol.createSimple(
            {'color': '#0000ff', 'name': 'diamond', 'outline_style': 'no', 'size': '5'})
        marker_symbol3 = QgsMarkerSymbol.createSimple(
            {'color': '#0000ff', 'name': 'rectangle', 'outline_style': 'no', 'size': '5'})

        point_layer.setRenderer(QgsCategorizedSymbolRenderer('Class', [QgsRendererCategory('B52', marker_symbol1, ''),
                                                                       QgsRendererCategory('Biplane', marker_symbol2,
                                                                                           ''),
                                                                       QgsRendererCategory('Jet', marker_symbol3, ''),
                                                                       ]))
        point_layer.styleManager().addStyleFromLayer("blue")

        line_symbol = QgsLineSymbol.createSimple({'color': '#0000ff', 'line_width': '2'})
        line_layer.setRenderer(QgsSingleSymbolRenderer(line_symbol))
        line_layer.styleManager().addStyleFromLayer("blue")

        blue_record = QgsMapThemeCollection.MapThemeRecord()
        point_blue_record = QgsMapThemeCollection.MapThemeLayerRecord(point_layer)
        point_blue_record.usingCurrentStyle = True
        point_blue_record.currentStyle = 'blue'
        blue_record.addLayerRecord(point_blue_record)
        line_blue_record = QgsMapThemeCollection.MapThemeLayerRecord(line_layer)
        line_blue_record.usingCurrentStyle = True
        line_blue_record.currentStyle = 'blue'
        blue_record.addLayerRecord(line_blue_record)
        QgsProject.instance().mapThemeCollection().insert('blue', blue_record)

        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()

        map1 = QgsLayoutItemMap(layout)
        map1.attemptSetSceneRect(QRectF(20, 20, 80, 80))
        map1.setFrameEnabled(True)
        map1.setLayers([point_layer, line_layer])
        layout.addLayoutItem(map1)
        map1.setExtent(point_layer.extent())
        map1.setFollowVisibilityPreset(True)
        map1.setFollowVisibilityPresetName('red')

        map2 = QgsLayoutItemMap(layout)
        map2.attemptSetSceneRect(QRectF(20, 120, 80, 80))
        map2.setFrameEnabled(True)
        map2.setLayers([point_layer, line_layer])
        layout.addLayoutItem(map2)
        map2.setExtent(point_layer.extent())
        map2.setFollowVisibilityPreset(True)
        map2.setFollowVisibilityPresetName('blue')

        legend = QgsLayoutItemLegend(layout)
        legend.setTitle("Legend")
        legend.attemptSetSceneRect(QRectF(120, 20, 80, 80))
        legend.setFrameEnabled(True)
        legend.setFrameStrokeWidth(QgsLayoutMeasurement(2))
        legend.setBackgroundColor(QColor(200, 200, 200))
        legend.setTitle('')
        layout.addLayoutItem(legend)
        legend.setLinkedMap(map1)

        legend2 = QgsLayoutItemLegend(layout)
        legend2.setTitle("Legend")
        legend2.attemptSetSceneRect(QRectF(120, 120, 80, 80))
        legend2.setFrameEnabled(True)
        legend2.setFrameStrokeWidth(QgsLayoutMeasurement(2))
        legend2.setBackgroundColor(QColor(200, 200, 200))
        legend2.setTitle('')
        layout.addLayoutItem(legend2)
        legend2.setLinkedMap(map2)

        checker = QgsLayoutChecker(
            'composer_legend_theme', layout)
        checker.setControlPathPrefix("composer_legend")
        result, message = checker.testLayout()
        TestQgsLayoutItemLegend.report += checker.report()
        self.assertTrue(result, message)

        QgsProject.instance().clear()

    def testLegendRenderLinkedMapScale(self):
        """Test rendering legends linked to maps follow scale correctly"""
        QgsProject.instance().removeAllMapLayers()

        line_path = os.path.join(TEST_DATA_DIR, 'lines.shp')
        line_layer = QgsVectorLayer(line_path, 'lines', 'ogr')
        QgsProject.instance().clear()
        QgsProject.instance().addMapLayers([line_layer])

        line_symbol = QgsLineSymbol.createSimple({'color': '#ff0000', 'width_unit': 'mapunits', 'width': '0.0001'})
        line_layer.setRenderer(QgsSingleSymbolRenderer(line_symbol))

        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()

        map1 = QgsLayoutItemMap(layout)
        map1.attemptSetSceneRect(QRectF(20, 20, 80, 80))
        map1.setFrameEnabled(True)
        map1.setLayers([line_layer])
        layout.addLayoutItem(map1)
        map1.setExtent(line_layer.extent())
        map1.setScale(2000)

        map2 = QgsLayoutItemMap(layout)
        map2.attemptSetSceneRect(QRectF(20, 120, 80, 80))
        map2.setFrameEnabled(True)
        map2.setLayers([line_layer])
        layout.addLayoutItem(map2)
        map2.setExtent(line_layer.extent())
        map2.setScale(5000)

        legend = QgsLayoutItemLegend(layout)
        legend.setTitle("Legend")
        legend.attemptSetSceneRect(QRectF(120, 20, 80, 80))
        legend.setFrameEnabled(True)
        legend.setFrameStrokeWidth(QgsLayoutMeasurement(2))
        legend.setBackgroundColor(QColor(200, 200, 200))
        legend.setTitle('')
        layout.addLayoutItem(legend)
        legend.setLinkedMap(map1)

        legend2 = QgsLayoutItemLegend(layout)
        legend2.setTitle("Legend")
        legend2.attemptSetSceneRect(QRectF(120, 120, 80, 80))
        legend2.setFrameEnabled(True)
        legend2.setFrameStrokeWidth(QgsLayoutMeasurement(2))
        legend2.setBackgroundColor(QColor(200, 200, 200))
        legend2.setTitle('')
        layout.addLayoutItem(legend2)
        legend2.setLinkedMap(map2)

        checker = QgsLayoutChecker(
            'composer_legend_scale_map', layout)
        checker.setControlPathPrefix("composer_legend")
        result, message = checker.testLayout()
        TestQgsLayoutItemLegend.report += checker.report()
        self.assertTrue(result, message)

        QgsProject.instance().clear()

    def test_rotated_map_hit(self):
        """Test filter by map handling of rotated map."""
        point_path = os.path.join(TEST_DATA_DIR, 'points.shp')
        point_layer = QgsVectorLayer(point_path, 'points', 'ogr')
        layouttemplate = os.path.join(os.path.join(TEST_DATA_DIR, "layouts"), 'map_filter_test_layout.qpt')

        QgsProject.instance().clear()
        QgsProject.instance().addMapLayers([point_layer])

        s = QgsMapSettings()
        s.setLayers([point_layer])
        layout = QgsLayout(QgsProject.instance())
        with open(layouttemplate) as f:
            template_content = f.read()
        doc = QDomDocument()
        doc.setContent(template_content)
        layout.loadFromTemplate(doc, QgsReadWriteContext(), True)

        checker = QgsLayoutChecker(
            'composer_legend_rotated_map', layout)
        checker.setControlPathPrefix("composer_legend")
        result, message = checker.testLayout()
        TestQgsLayoutItemLegend.report += checker.report()
        self.assertTrue(result, message)
        QgsProject.instance().clear()


if __name__ == '__main__':
    unittest.main()
