"""QGIS Unit tests for QgsLayoutItemLegend.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2017 by Nyall Dawson"
__date__ = "24/10/2017"
__copyright__ = "Copyright 2017, The QGIS Project"

import os
from time import sleep

from qgis.PyQt.QtCore import Qt, QRectF
from qgis.PyQt.QtGui import QColor, QImage, QPainter
from qgis.PyQt.QtXml import QDomDocument

from qgis.core import (
    QgsMapLayer,
    QgsCategorizedSymbolRenderer,
    QgsCoordinateReferenceSystem,
    QgsExpression,
    QgsFeature,
    QgsFillSymbol,
    QgsFontUtils,
    QgsGeometry,
    QgsGeometryGeneratorSymbolLayer,
    QgsLayout,
    QgsLayoutItem,
    QgsLayoutItemLegend,
    QgsLayoutItemMap,
    QgsLayoutMeasurement,
    QgsLayoutObject,
    QgsLayoutPoint,
    QgsLayoutSize,
    QgsLegendStyle,
    QgsLineSymbol,
    QgsMapLayerLegendUtils,
    QgsMapSettings,
    QgsMapThemeCollection,
    QgsMarkerSymbol,
    QgsPrintLayout,
    QgsProject,
    QgsProperty,
    QgsRectangle,
    QgsRendererCategory,
    QgsRuleBasedRenderer,
    QgsSingleSymbolRenderer,
    QgsSymbol,
    QgsVectorLayer,
    QgsReadWriteContext,
    QgsTextFormat,
    QgsFeatureRequest,
    QgsLayoutItemShape,
    QgsSimpleFillSymbolLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from test_qgslayoutitem import LayoutItemTestCase
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayoutItemLegend(QgisTestCase, LayoutItemTestCase):

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.item_class = QgsLayoutItemLegend

    @classmethod
    def control_path_prefix(cls):
        return "composer_legend"

    def test_opacity(self):
        """
        Test rendering the legend with opacity
        """
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()

        legend = QgsLayoutItemLegend(layout)
        legend.setTitle("Legend")
        legend.attemptSetSceneRect(QRectF(120, 20, 80, 80))
        legend.setFrameEnabled(True)
        legend.setFrameStrokeWidth(QgsLayoutMeasurement(2))
        legend.setBackgroundColor(QColor(200, 200, 200))
        layout.addLayoutItem(legend)

        text_format = QgsTextFormat()
        text_format.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        text_format.setSize(16)

        for legend_item in [
            QgsLegendStyle.Style.Title,
            QgsLegendStyle.Style.Group,
            QgsLegendStyle.Style.Subgroup,
            QgsLegendStyle.Style.Symbol,
            QgsLegendStyle.Style.SymbolLabel,
        ]:
            style = legend.style(legend_item)
            style.setTextFormat(text_format)
            legend.setStyle(legend_item, style)

        legend.setItemOpacity(0.3)

        self.assertFalse(legend.requiresRasterization())
        self.assertTrue(legend.containsAdvancedEffects())

        self.assertTrue(self.render_layout_check("composerlegend_opacity", layout))

    def test_opacity_rendering_designer_preview(self):
        """
        Test rendering of legend opacity while in designer dialogs
        """
        p = QgsProject()
        l = QgsLayout(p)
        self.assertTrue(l.renderContext().isPreviewRender())

        l.initializeDefaults()
        legend = QgsLayoutItemLegend(l)
        legend.setTitle("Legend")
        legend.attemptSetSceneRect(QRectF(120, 20, 80, 80))
        legend.setFrameEnabled(True)
        legend.setFrameStrokeWidth(QgsLayoutMeasurement(2))
        legend.setBackgroundColor(QColor(200, 200, 200))
        l.addLayoutItem(legend)

        text_format = QgsTextFormat()
        text_format.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        text_format.setSize(16)

        for legend_item in [
            QgsLegendStyle.Style.Title,
            QgsLegendStyle.Style.Group,
            QgsLegendStyle.Style.Subgroup,
            QgsLegendStyle.Style.Symbol,
            QgsLegendStyle.Style.SymbolLabel,
        ]:
            style = legend.style(legend_item)
            style.setTextFormat(text_format)
            legend.setStyle(legend_item, style)

        legend.setItemOpacity(0.3)

        page_item = l.pageCollection().page(0)
        paper_rect = QRectF(
            page_item.pos().x(),
            page_item.pos().y(),
            page_item.rect().width(),
            page_item.rect().height(),
        )

        im = QImage(1122, 794, QImage.Format.Format_ARGB32)
        im.fill(Qt.GlobalColor.transparent)
        im.setDotsPerMeterX(int(300 / 25.4 * 1000))
        im.setDotsPerMeterY(int(300 / 25.4 * 1000))
        painter = QPainter(im)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)

        l.render(
            painter,
            QRectF(0, 0, painter.device().width(), painter.device().height()),
            paper_rect,
        )
        painter.end()

        self.assertTrue(
            self.image_check(
                "composerlegend_opacity",
                "composerlegend_opacity",
                im,
                allowed_mismatch=0,
            )
        )

    def test_blend_mode(self):
        """
        Test rendering the legend with a blend mode
        """
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()

        item1 = QgsLayoutItemShape(layout)
        item1.attemptSetSceneRect(QRectF(20, 20, 150, 100))
        item1.setShapeType(QgsLayoutItemShape.Shape.Rectangle)
        simple_fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, simple_fill)
        simple_fill.setColor(QColor(0, 100, 50))
        simple_fill.setStrokeColor(Qt.GlobalColor.black)
        item1.setSymbol(fill_symbol)
        layout.addLayoutItem(item1)

        legend = QgsLayoutItemLegend(layout)
        legend.setTitle("Legend")
        legend.attemptSetSceneRect(QRectF(120, 20, 80, 80))
        legend.setFrameEnabled(True)
        legend.setFrameStrokeWidth(QgsLayoutMeasurement(2))
        legend.setBackgroundColor(QColor(200, 200, 200))
        layout.addLayoutItem(legend)

        text_format = QgsTextFormat()
        text_format.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        text_format.setSize(16)

        for legend_item in [
            QgsLegendStyle.Style.Title,
            QgsLegendStyle.Style.Group,
            QgsLegendStyle.Style.Subgroup,
            QgsLegendStyle.Style.Symbol,
            QgsLegendStyle.Style.SymbolLabel,
        ]:
            style = legend.style(legend_item)
            style.setTextFormat(text_format)
            legend.setStyle(legend_item, style)

        legend.setBlendMode(QPainter.CompositionMode.CompositionMode_Darken)

        self.assertTrue(legend.requiresRasterization())

        self.assertTrue(self.render_layout_check("composerlegend_blendmode", layout))

    def test_blend_mode_designer_preview(self):
        """
        Test rendering the legend with a blend mode
        """
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()
        self.assertTrue(layout.renderContext().isPreviewRender())

        item1 = QgsLayoutItemShape(layout)
        item1.attemptSetSceneRect(QRectF(20, 20, 150, 100))
        item1.setShapeType(QgsLayoutItemShape.Shape.Rectangle)
        simple_fill = QgsSimpleFillSymbolLayer()
        fill_symbol = QgsFillSymbol()
        fill_symbol.changeSymbolLayer(0, simple_fill)
        simple_fill.setColor(QColor(0, 100, 50))
        simple_fill.setStrokeColor(Qt.GlobalColor.black)
        item1.setSymbol(fill_symbol)
        layout.addLayoutItem(item1)

        legend = QgsLayoutItemLegend(layout)
        legend.setTitle("Legend")
        legend.attemptSetSceneRect(QRectF(120, 20, 80, 80))
        legend.setFrameEnabled(True)
        legend.setFrameStrokeWidth(QgsLayoutMeasurement(2))
        legend.setBackgroundColor(QColor(200, 200, 200))
        layout.addLayoutItem(legend)

        text_format = QgsTextFormat()
        text_format.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        text_format.setSize(16)

        for legend_item in [
            QgsLegendStyle.Style.Title,
            QgsLegendStyle.Style.Group,
            QgsLegendStyle.Style.Subgroup,
            QgsLegendStyle.Style.Symbol,
            QgsLegendStyle.Style.SymbolLabel,
        ]:
            style = legend.style(legend_item)
            style.setTextFormat(text_format)
            legend.setStyle(legend_item, style)

        legend.setBlendMode(QPainter.CompositionMode.CompositionMode_Darken)

        page_item = layout.pageCollection().page(0)
        paper_rect = QRectF(
            page_item.pos().x(),
            page_item.pos().y(),
            page_item.rect().width(),
            page_item.rect().height(),
        )

        im = QImage(1122, 794, QImage.Format.Format_ARGB32)
        im.fill(Qt.GlobalColor.transparent)
        im.setDotsPerMeterX(int(300 / 25.4 * 1000))
        im.setDotsPerMeterY(int(300 / 25.4 * 1000))
        painter = QPainter(im)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)

        layout.render(
            painter,
            QRectF(0, 0, painter.device().width(), painter.device().height()),
            paper_rect,
        )
        painter.end()

        self.assertTrue(
            self.image_check(
                "composerlegend_blendmode",
                "composerlegend_blendmode",
                im,
                allowed_mismatch=0,
            )
        )

    def testInitialSizeSymbolMapUnits(self):
        """
        Test initial size of legend with a symbol size in map units
        """
        QgsProject.instance().removeAllMapLayers()

        point_path = os.path.join(TEST_DATA_DIR, "points.shp")
        point_layer = QgsVectorLayer(point_path, "points", "ogr")
        QgsProject.instance().clear()
        QgsProject.instance().addMapLayers([point_layer])

        marker_symbol = QgsMarkerSymbol.createSimple(
            {
                "color": "#ff0000",
                "outline_style": "no",
                "size": "5",
                "size_unit": "MapUnit",
            }
        )

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
        legend.setTitle("")
        layout.addLayoutItem(legend)
        legend.setLinkedMap(map)

        self.assertTrue(self.render_layout_check("composer_legend_mapunits", layout))

        # resize with non-top-left reference point
        legend.setResizeToContents(False)
        legend.setReferencePoint(QgsLayoutItem.ReferencePoint.LowerRight)
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
        """
        Test legend resizes to match map content
        """
        point_path = os.path.join(TEST_DATA_DIR, "points.shp")
        point_layer = QgsVectorLayer(point_path, "points", "ogr")
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
        legend.setTitle("")
        legend.setLegendFilterByMapEnabled(True)
        layout.addLayoutItem(legend)
        legend.setLinkedMap(map)

        legend.setStyleFont(
            QgsLegendStyle.Style.Title, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Group, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Subgroup, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Symbol, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.SymbolLabel,
            QgsFontUtils.getStandardTestFont("Bold", 16),
        )

        map.setExtent(QgsRectangle(-102.51, 41.16, -102.36, 41.30))

        self.assertTrue(
            self.render_layout_check("composer_legend_size_content", layout)
        )

        QgsProject.instance().removeMapLayers([point_layer.id()])

    def testResizeWithMapContentNoDoublePaint(self):
        """
        Test legend resizes to match map content
        """
        poly_path = os.path.join(TEST_DATA_DIR, "polys.shp")
        poly_layer = QgsVectorLayer(poly_path, "polys", "ogr")
        p = QgsProject()
        p.addMapLayers([poly_layer])

        fill_symbol = QgsFillSymbol.createSimple(
            {"color": "255,0,0,125", "outline_style": "no"}
        )
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
        legend.setTitle("")
        layout.addLayoutItem(legend)
        legend.setLinkedMap(map)

        map.setExtent(QgsRectangle(-102.51, 41.16, -102.36, 41.30))

        self.assertTrue(
            self.render_layout_check(
                "composer_legend_size_content_no_double_paint", layout
            )
        )

    def test_private_layers(self):
        """
        Test legend does not contain private layers by default
        """
        point_path = os.path.join(TEST_DATA_DIR, "points.shp")
        point_layer1 = QgsVectorLayer(point_path, "points 1", "ogr")
        point_layer2 = QgsVectorLayer(point_path, "points 2", "ogr")
        point_layer2.setFlags(QgsMapLayer.LayerFlag.Private)
        p = QgsProject()
        p.addMapLayers([point_layer1, point_layer2])

        marker_symbol = QgsMarkerSymbol.createSimple(
            {
                "color": "#ff0000",
                "outline_style": "no",
                "size": "5",
                "size_unit": "MapUnit",
            }
        )

        point_layer1.setRenderer(QgsSingleSymbolRenderer(marker_symbol.clone()))
        point_layer2.setRenderer(QgsSingleSymbolRenderer(marker_symbol.clone()))

        layout = QgsLayout(p)
        layout.initializeDefaults()

        legend = QgsLayoutItemLegend(layout)
        legend.setTitle("Legend")
        legend.attemptSetSceneRect(QRectF(120, 20, 80, 80))
        legend.setFrameEnabled(True)
        legend.setFrameStrokeWidth(QgsLayoutMeasurement(2))
        legend.setBackgroundColor(QColor(200, 200, 200))
        legend.setTitle("")
        layout.addLayoutItem(legend)

        legend.setStyleFont(
            QgsLegendStyle.Style.Title, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Group, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Subgroup, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Symbol, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.SymbolLabel,
            QgsFontUtils.getStandardTestFont("Bold", 16),
        )

        self.assertTrue(
            self.render_layout_check("composer_legend_private_layers", layout)
        )

    def testResizeDisabled(self):
        """
        Test that test legend does not resize if auto size is disabled
        """
        point_path = os.path.join(TEST_DATA_DIR, "points.shp")
        point_layer = QgsVectorLayer(point_path, "points", "ogr")
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
        legend.setTitle("")
        legend.setLegendFilterByMapEnabled(True)

        legend.setStyleFont(
            QgsLegendStyle.Style.Title, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Group, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Subgroup, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Symbol, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.SymbolLabel,
            QgsFontUtils.getStandardTestFont("Bold", 16),
        )

        # disable auto resizing
        legend.setResizeToContents(False)

        layout.addLayoutItem(legend)
        legend.setLinkedMap(map)

        map.setExtent(QgsRectangle(-102.51, 41.16, -102.36, 41.30))

        self.assertTrue(self.render_layout_check("composer_legend_noresize", layout))

        QgsProject.instance().removeMapLayers([point_layer.id()])

    def testResizeDisabledCrop(self):
        """
        Test that if legend resizing is disabled, and legend is too small,
        then content is cropped
        """
        point_path = os.path.join(TEST_DATA_DIR, "points.shp")
        point_layer = QgsVectorLayer(point_path, "points", "ogr")
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
        legend.setTitle("")
        legend.setLegendFilterByMapEnabled(True)

        legend.setStyleFont(
            QgsLegendStyle.Style.Title, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Group, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Subgroup, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Symbol, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.SymbolLabel,
            QgsFontUtils.getStandardTestFont("Bold", 16),
        )

        # disable auto resizing
        legend.setResizeToContents(False)

        layout.addLayoutItem(legend)
        legend.setLinkedMap(map)

        map.setExtent(QgsRectangle(-102.51, 41.16, -102.36, 41.30))

        self.assertTrue(
            self.render_layout_check("composer_legend_noresize_crop", layout)
        )

        QgsProject.instance().removeMapLayers([point_layer.id()])

    def testDataDefinedTitle(self):
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()

        legend = QgsLayoutItemLegend(layout)
        layout.addLayoutItem(legend)

        legend.setTitle("original")
        self.assertEqual(legend.title(), "original")
        self.assertEqual(legend.legendSettings().title(), "original")

        legend.dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.LegendTitle,
            QgsProperty.fromExpression("'new'"),
        )
        legend.refreshDataDefinedProperty()
        self.assertEqual(legend.title(), "original")
        self.assertEqual(legend.legendSettings().title(), "new")

    def testDataDefinedColumnCount(self):
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()

        legend = QgsLayoutItemLegend(layout)
        legend.setTitle("Legend")
        layout.addLayoutItem(legend)

        legend.setColumnCount(2)
        self.assertEqual(legend.columnCount(), 2)
        self.assertEqual(legend.legendSettings().columnCount(), 2)

        legend.dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.LegendColumnCount,
            QgsProperty.fromExpression("5"),
        )
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
        legend.setWrapString("d")
        legend.setLegendFilterOutAtlas(True)

        expc = legend.createExpressionContext()
        exp1 = QgsExpression("@legend_title")
        self.assertEqual(exp1.evaluate(expc), "Legend")
        exp2 = QgsExpression("@legend_column_count")
        self.assertEqual(exp2.evaluate(expc), 2)
        exp3 = QgsExpression("@legend_wrap_string")
        self.assertEqual(exp3.evaluate(expc), "d")
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
        """
        Test expressions embedded in legend node text
        """
        point_path = os.path.join(TEST_DATA_DIR, "points.shp")
        point_layer = QgsVectorLayer(point_path, "points", "ogr")
        layout = QgsPrintLayout(QgsProject.instance())
        layout.setName("LAYOUT")
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
        legend.setTitle("")
        legend.setLegendFilterByMapEnabled(False)
        legend.setStyleFont(
            QgsLegendStyle.Style.Title, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Group, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Subgroup, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Symbol, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.SymbolLabel,
            QgsFontUtils.getStandardTestFont("Bold", 16),
        )

        legend.setAutoUpdateModel(False)

        QgsProject.instance().addMapLayers([point_layer])
        s = QgsMapSettings()
        s.setLayers([point_layer])

        group = (
            legend.model().rootGroup().addGroup("Group [% 1 + 5 %] [% @layout_name %]")
        )
        layer_tree_layer = group.addLayer(point_layer)
        layer_tree_layer.setCustomProperty(
            "legend/title-label",
            "bbbb [% 1+2 %] xx [% @layout_name %] [% @layer_name %]",
        )
        QgsMapLayerLegendUtils.setLegendNodeUserLabel(layer_tree_layer, 0, "xxxx")
        legend.model().refreshLayerLegend(layer_tree_layer)
        legend.model().layerLegendNodes(layer_tree_layer)[0].setUserLabel(
            "bbbb [% 1+2 %] xx [% @layout_name %] [% @layer_name %]"
        )

        layout.addLayoutItem(legend)
        legend.setLinkedMap(map)

        map.setExtent(QgsRectangle(-102.51, 41.16, -102.36, 41.30))

        self.assertTrue(self.render_layout_check("composer_legend_expressions", layout))

        QgsProject.instance().removeMapLayers([point_layer.id()])

    def testSymbolExpressions(self):
        """
        Test expressions embedded in legend node text
        """
        QgsProject.instance().clear()
        point_path = os.path.join(TEST_DATA_DIR, "points.shp")
        point_layer = QgsVectorLayer(point_path, "points", "ogr")

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
        legendnodes[0].setUserLabel("[% @symbol_id %]")
        legendnodes[1].setUserLabel("[% @symbol_count %]")
        legendnodes[2].setUserLabel('[% sum("Pilots") %]')
        label1 = legendnodes[0].evaluateLabel()
        label2 = legendnodes[1].evaluateLabel()
        label3 = legendnodes[2].evaluateLabel()
        self.assertEqual(label1, "0")
        # self.assertEqual(label2, '5')
        # self.assertEqual(label3, '12')

        legendlayer.setLabelExpression("Concat(@symbol_label, @symbol_id)")

        label1 = legendnodes[0].evaluateLabel()
        label2 = legendnodes[1].evaluateLabel()
        label3 = legendnodes[2].evaluateLabel()

        self.assertEqual(label1, " @symbol_id 0")
        # self.assertEqual(label2, '@symbol_count 1')
        # self.assertEqual(label3, 'sum("Pilots") 2')

        QgsProject.instance().clear()

    def testSymbolExpressionRender(self):
        """
        Test expressions embedded in legend node text
        """
        point_path = os.path.join(TEST_DATA_DIR, "points.shp")
        point_layer = QgsVectorLayer(point_path, "points", "ogr")
        layout = QgsPrintLayout(QgsProject.instance())
        layout.setName("LAYOUT")
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
        legend.setTitle("")
        legend.setLegendFilterByMapEnabled(False)
        legend.setStyleFont(
            QgsLegendStyle.Style.Title, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Group, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Subgroup, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Symbol, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.SymbolLabel,
            QgsFontUtils.getStandardTestFont("Bold", 16),
        )

        legend.setAutoUpdateModel(False)

        QgsProject.instance().addMapLayers([point_layer])
        s = QgsMapSettings()
        s.setLayers([point_layer])

        group = (
            legend.model().rootGroup().addGroup("Group [% 1 + 5 %] [% @layout_name %]")
        )
        layer_tree_layer = group.addLayer(point_layer)
        counterTask = point_layer.countSymbolFeatures()
        counterTask.waitForFinished()
        layer_tree_layer.setCustomProperty(
            "legend/title-label",
            "bbbb [% 1+2 %] xx [% @layout_name %] [% @layer_name %]",
        )
        QgsMapLayerLegendUtils.setLegendNodeUserLabel(layer_tree_layer, 0, "xxxx")
        legend.model().refreshLayerLegend(layer_tree_layer)
        layer_tree_layer.setLabelExpression(
            'Concat(@symbol_id, @symbol_label, count("Class"))'
        )
        legend.model().layerLegendNodes(layer_tree_layer)[0].setUserLabel(" sym 1")
        legend.model().layerLegendNodes(layer_tree_layer)[1].setUserLabel(
            "[%@symbol_count %]"
        )
        legend.model().layerLegendNodes(layer_tree_layer)[2].setUserLabel(
            '[% count("Class") %]'
        )
        layout.addLayoutItem(legend)
        legend.setLinkedMap(map)
        legend.updateLegend()
        map.setExtent(QgsRectangle(-102.51, 41.16, -102.36, 41.30))

        self.assertTrue(
            self.render_layout_check("composer_legend_symbol_expression", layout)
        )

        QgsProject.instance().removeMapLayers([point_layer.id()])

    def testThemes(self):
        layout = QgsPrintLayout(QgsProject.instance())
        layout.setName("LAYOUT")

        map = QgsLayoutItemMap(layout)
        layout.addLayoutItem(map)
        legend = QgsLayoutItemLegend(layout)

        self.assertFalse(legend.themeName())
        legend.setLinkedMap(map)
        self.assertFalse(legend.themeName())

        map.setFollowVisibilityPresetName("theme1")
        map.setFollowVisibilityPreset(True)
        self.assertEqual(legend.themeName(), "theme1")
        map.setFollowVisibilityPresetName("theme2")
        self.assertEqual(legend.themeName(), "theme2")
        map.setFollowVisibilityPreset(False)
        self.assertFalse(legend.themeName())

        # with theme set before linking map
        map2 = QgsLayoutItemMap(layout)
        map2.setFollowVisibilityPresetName("theme3")
        map2.setFollowVisibilityPreset(True)
        legend.setLinkedMap(map2)
        self.assertEqual(legend.themeName(), "theme3")
        map2.setFollowVisibilityPresetName("theme2")
        self.assertEqual(legend.themeName(), "theme2")

        # replace with map with no theme
        map3 = QgsLayoutItemMap(layout)
        legend.setLinkedMap(map3)
        self.assertFalse(legend.themeName())

    def testLegendRenderWithMapTheme(self):
        """
        Test rendering legends linked to map themes
        """
        QgsProject.instance().removeAllMapLayers()

        point_path = os.path.join(TEST_DATA_DIR, "points.shp")
        point_layer = QgsVectorLayer(point_path, "points", "ogr")
        line_path = os.path.join(TEST_DATA_DIR, "lines.shp")
        line_layer = QgsVectorLayer(line_path, "lines", "ogr")
        QgsProject.instance().clear()
        QgsProject.instance().addMapLayers([point_layer, line_layer])

        marker_symbol = QgsMarkerSymbol.createSimple(
            {"color": "#ff0000", "outline_style": "no", "size": "5"}
        )
        point_layer.setRenderer(QgsSingleSymbolRenderer(marker_symbol))
        point_layer.styleManager().addStyleFromLayer("red")

        line_symbol = QgsLineSymbol.createSimple(
            {"color": "#ff0000", "line_width": "2"}
        )
        line_layer.setRenderer(QgsSingleSymbolRenderer(line_symbol))
        line_layer.styleManager().addStyleFromLayer("red")

        red_record = QgsMapThemeCollection.MapThemeRecord()
        point_red_record = QgsMapThemeCollection.MapThemeLayerRecord(point_layer)
        point_red_record.usingCurrentStyle = True
        point_red_record.currentStyle = "red"
        red_record.addLayerRecord(point_red_record)
        line_red_record = QgsMapThemeCollection.MapThemeLayerRecord(line_layer)
        line_red_record.usingCurrentStyle = True
        line_red_record.currentStyle = "red"
        red_record.addLayerRecord(line_red_record)
        QgsProject.instance().mapThemeCollection().insert("red", red_record)

        marker_symbol1 = QgsMarkerSymbol.createSimple(
            {"color": "#0000ff", "outline_style": "no", "size": "5"}
        )
        marker_symbol2 = QgsMarkerSymbol.createSimple(
            {"color": "#0000ff", "name": "diamond", "outline_style": "no", "size": "5"}
        )
        marker_symbol3 = QgsMarkerSymbol.createSimple(
            {
                "color": "#0000ff",
                "name": "rectangle",
                "outline_style": "no",
                "size": "5",
            }
        )

        point_layer.setRenderer(
            QgsCategorizedSymbolRenderer(
                "Class",
                [
                    QgsRendererCategory("B52", marker_symbol1, ""),
                    QgsRendererCategory("Biplane", marker_symbol2, ""),
                    QgsRendererCategory("Jet", marker_symbol3, ""),
                ],
            )
        )
        point_layer.styleManager().addStyleFromLayer("blue")

        line_symbol = QgsLineSymbol.createSimple(
            {"color": "#0000ff", "line_width": "2"}
        )
        line_layer.setRenderer(QgsSingleSymbolRenderer(line_symbol))
        line_layer.styleManager().addStyleFromLayer("blue")

        blue_record = QgsMapThemeCollection.MapThemeRecord()
        point_blue_record = QgsMapThemeCollection.MapThemeLayerRecord(point_layer)
        point_blue_record.usingCurrentStyle = True
        point_blue_record.currentStyle = "blue"
        blue_record.addLayerRecord(point_blue_record)
        line_blue_record = QgsMapThemeCollection.MapThemeLayerRecord(line_layer)
        line_blue_record.usingCurrentStyle = True
        line_blue_record.currentStyle = "blue"
        blue_record.addLayerRecord(line_blue_record)
        QgsProject.instance().mapThemeCollection().insert("blue", blue_record)

        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()

        map1 = QgsLayoutItemMap(layout)
        map1.attemptSetSceneRect(QRectF(20, 20, 80, 80))
        map1.setFrameEnabled(True)
        map1.setLayers([point_layer, line_layer])
        layout.addLayoutItem(map1)
        map1.setExtent(point_layer.extent())
        map1.setFollowVisibilityPreset(True)
        map1.setFollowVisibilityPresetName("red")

        map2 = QgsLayoutItemMap(layout)
        map2.attemptSetSceneRect(QRectF(20, 120, 80, 80))
        map2.setFrameEnabled(True)
        map2.setLayers([point_layer, line_layer])
        layout.addLayoutItem(map2)
        map2.setExtent(point_layer.extent())
        map2.setFollowVisibilityPreset(True)
        map2.setFollowVisibilityPresetName("blue")

        legend = QgsLayoutItemLegend(layout)
        legend.setTitle("Legend")
        legend.attemptSetSceneRect(QRectF(120, 20, 80, 80))
        legend.setFrameEnabled(True)
        legend.setFrameStrokeWidth(QgsLayoutMeasurement(2))
        legend.setBackgroundColor(QColor(200, 200, 200))
        legend.setTitle("")
        layout.addLayoutItem(legend)
        legend.setLinkedMap(map1)
        legend.setStyleFont(
            QgsLegendStyle.Style.Title, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Group, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Subgroup, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Symbol, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.SymbolLabel,
            QgsFontUtils.getStandardTestFont("Bold", 16),
        )

        legend2 = QgsLayoutItemLegend(layout)
        legend2.setTitle("Legend")
        legend2.attemptSetSceneRect(QRectF(120, 120, 80, 80))
        legend2.setFrameEnabled(True)
        legend2.setFrameStrokeWidth(QgsLayoutMeasurement(2))
        legend2.setBackgroundColor(QColor(200, 200, 200))
        legend2.setTitle("")
        layout.addLayoutItem(legend2)
        legend2.setLinkedMap(map2)
        legend2.setStyleFont(
            QgsLegendStyle.Style.Title, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend2.setStyleFont(
            QgsLegendStyle.Style.Group, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend2.setStyleFont(
            QgsLegendStyle.Style.Subgroup, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend2.setStyleFont(
            QgsLegendStyle.Style.Symbol, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend2.setStyleFont(
            QgsLegendStyle.Style.SymbolLabel,
            QgsFontUtils.getStandardTestFont("Bold", 16),
        )

        self.assertTrue(self.render_layout_check("composer_legend_theme", layout))

        QgsProject.instance().clear()

    def testLegendRenderLinkedMapScale(self):
        """
        Test rendering legends linked to maps follow scale correctly
        """
        QgsProject.instance().removeAllMapLayers()

        line_path = os.path.join(TEST_DATA_DIR, "lines.shp")
        line_layer = QgsVectorLayer(line_path, "lines", "ogr")
        QgsProject.instance().clear()
        QgsProject.instance().addMapLayers([line_layer])

        line_symbol = QgsLineSymbol.createSimple(
            {"color": "#ff0000", "width_unit": "mapunits", "width": "0.0001"}
        )
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
        legend.setTitle("")
        layout.addLayoutItem(legend)
        legend.setLinkedMap(map1)

        legend.setStyleFont(
            QgsLegendStyle.Style.Title, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Group, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Subgroup, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Symbol, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.SymbolLabel,
            QgsFontUtils.getStandardTestFont("Bold", 16),
        )

        legend2 = QgsLayoutItemLegend(layout)
        legend2.setTitle("Legend")
        legend2.attemptSetSceneRect(QRectF(120, 120, 80, 80))
        legend2.setFrameEnabled(True)
        legend2.setFrameStrokeWidth(QgsLayoutMeasurement(2))
        legend2.setBackgroundColor(QColor(200, 200, 200))
        legend2.setTitle("")
        layout.addLayoutItem(legend2)
        legend2.setLinkedMap(map2)

        legend2.setStyleFont(
            QgsLegendStyle.Style.Title, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend2.setStyleFont(
            QgsLegendStyle.Style.Group, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend2.setStyleFont(
            QgsLegendStyle.Style.Subgroup, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend2.setStyleFont(
            QgsLegendStyle.Style.Symbol, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend2.setStyleFont(
            QgsLegendStyle.Style.SymbolLabel,
            QgsFontUtils.getStandardTestFont("Bold", 16),
        )

        self.assertTrue(self.render_layout_check("composer_legend_scale_map", layout))

        QgsProject.instance().clear()

    def testReferencePoint(self):
        """
        Test reference point parameter when resizeToContent is enabled
        """
        QgsProject.instance().removeAllMapLayers()

        point_path = os.path.join(TEST_DATA_DIR, "points.shp")
        point_layer = QgsVectorLayer(point_path, "points", "ogr")
        QgsProject.instance().clear()
        QgsProject.instance().addMapLayers([point_layer])

        marker_symbol = QgsMarkerSymbol.createSimple(
            {
                "color": "#ff0000",
                "outline_style": "no",
                "size": "5",
                "size_unit": "MapUnit",
            }
        )

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
        map.zoomToExtent(point_layer.extent())

        legend = QgsLayoutItemLegend(layout)
        legend.setStyleFont(
            QgsLegendStyle.Style.SymbolLabel,
            QgsFontUtils.getStandardTestFont("Bold", 16),
        )
        legend.setReferencePoint(QgsLayoutItem.ReferencePoint.LowerLeft)
        legend.setResizeToContents(True)
        legend.setTitle("Legend")
        legend.attemptSetSceneRect(QRectF(20, 20, 300, 80))
        legend.setFrameEnabled(True)
        legend.setFrameStrokeWidth(QgsLayoutMeasurement(2))
        legend.setBackgroundEnabled(False)
        legend.setBackgroundColor(QColor(200, 200, 200))
        legend.setTitle("")
        layout.addLayoutItem(legend)
        legend.setLinkedMap(map)

        self.assertTrue(
            self.render_layout_check("composer_legend_reference_point", layout)
        )

        # re-render with filtering to trigger mapHitTest which ends up by calling adjustBoxSize().
        # These last change the box size and therefore position (because our reference point is lower left)
        # So check that the legend position is according to what we expect (change also marker size
        # so we are sure that it's size and position have correcly been updated)
        legend.setLegendFilterByMapEnabled(True)
        marker_symbol.setSize(10)

        self.assertTrue(
            self.render_layout_check("composer_legend_reference_point_newsize", layout)
        )

        QgsProject.instance().clear()

    def test_rulebased_child_filter(self):
        point_path = os.path.join(TEST_DATA_DIR, "points.shp")
        point_layer = QgsVectorLayer(point_path, "points", "ogr")

        root_rule = QgsRuleBasedRenderer.Rule(None)
        marker_symbol = QgsMarkerSymbol.createSimple(
            {"color": "#ff0000", "outline_style": "no", "size": "8"}
        )

        less_than_two_rule = QgsRuleBasedRenderer.Rule(
            marker_symbol, filterExp='"Importance" <=2', label="lessthantwo"
        )
        root_rule.appendChild(less_than_two_rule)

        else_rule = QgsRuleBasedRenderer.Rule(None, elseRule=True)

        marker_symbol = QgsMarkerSymbol.createSimple(
            {"color": "#00ffff", "outline_style": "no", "size": "4"}
        )
        one_rule = QgsRuleBasedRenderer.Rule(
            marker_symbol, filterExp='"Pilots" = 1', label="1"
        )
        else_rule.appendChild(one_rule)
        marker_symbol = QgsMarkerSymbol.createSimple(
            {"color": "#ff8888", "outline_style": "no", "size": "4"}
        )
        two_rule = QgsRuleBasedRenderer.Rule(
            marker_symbol, filterExp='"Pilots" = 2', label="2"
        )
        else_rule.appendChild(two_rule)
        marker_symbol = QgsMarkerSymbol.createSimple(
            {"color": "#8888ff", "outline_style": "no", "size": "4"}
        )
        three_rule = QgsRuleBasedRenderer.Rule(
            marker_symbol, filterExp='"Pilots" = 3', label="3"
        )
        else_rule.appendChild(three_rule)

        root_rule.appendChild(else_rule)

        renderer = QgsRuleBasedRenderer(root_rule)
        point_layer.setRenderer(renderer)
        p = QgsProject()
        p.addMapLayer(point_layer)

        s = QgsMapSettings()
        s.setLayers([point_layer])
        layout = QgsLayout(p)
        layout.initializeDefaults()

        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(19, 17, 185, 165))
        map.setFrameEnabled(True)
        map.setCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        map.setLayers([point_layer])
        layout.addLayoutItem(map)

        legend = QgsLayoutItemLegend(layout)
        legend.setTitle("Legend")
        legend.attemptSetSceneRect(QRectF(220, 20, 20, 20))
        legend.setFrameEnabled(True)
        legend.setFrameStrokeWidth(QgsLayoutMeasurement(2))
        legend.setBackgroundColor(QColor(200, 200, 200))
        legend.setTitle("")
        legend.setLegendFilterByMapEnabled(True)

        legend.setStyleFont(
            QgsLegendStyle.Style.Title, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Group, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Subgroup, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Symbol, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.SymbolLabel,
            QgsFontUtils.getStandardTestFont("Bold", 16),
        )

        layout.addLayoutItem(legend)
        legend.setLinkedMap(map)
        legend.setLegendFilterByMapEnabled(True)

        map.setExtent(QgsRectangle(-119.778, 18.158, -82.444, 51.514))

        self.assertTrue(self.render_layout_check("composer_legend_elseChild", layout))

    def test_filter_by_map_items(self):
        p = QgsProject()

        layout = QgsLayout(p)
        layout.initializeDefaults()

        map1 = QgsLayoutItemMap(layout)
        map1.setId("map 1")
        layout.addLayoutItem(map1)

        map2 = QgsLayoutItemMap(layout)
        map2.setId("map 2")
        layout.addLayoutItem(map2)

        map3 = QgsLayoutItemMap(layout)
        map3.setId("map 3")
        layout.addLayoutItem(map3)

        legend = QgsLayoutItemLegend(layout)
        layout.addLayoutItem(legend)
        self.assertFalse(legend.filterByMapItems())

        legend.setFilterByMapItems([map1, map3])
        self.assertEqual(legend.filterByMapItems(), [map1, map3])

        # test restoring from xml
        doc = QDomDocument("testdoc")
        elem = layout.writeXml(doc, QgsReadWriteContext())

        l2 = QgsLayout(p)
        self.assertTrue(l2.readXml(elem, doc, QgsReadWriteContext()))
        map1_restore = [
            i
            for i in l2.items()
            if isinstance(i, QgsLayoutItemMap) and i.id() == "map 1"
        ][0]
        map3_restore = [
            i
            for i in l2.items()
            if isinstance(i, QgsLayoutItemMap) and i.id() == "map 3"
        ][0]
        legend_restore = [i for i in l2.items() if isinstance(i, QgsLayoutItemLegend)][
            0
        ]

        self.assertEqual(
            legend_restore.filterByMapItems(), [map1_restore, map3_restore]
        )

    def test_filter_by_map_content_rendering(self):
        point_path = os.path.join(TEST_DATA_DIR, "points.shp")
        point_layer = QgsVectorLayer(point_path, "points", "ogr")

        root_rule = QgsRuleBasedRenderer.Rule(None)
        marker_symbol = QgsMarkerSymbol.createSimple(
            {"color": "#ff0000", "outline_style": "no", "size": "8"}
        )

        less_than_two_rule = QgsRuleBasedRenderer.Rule(
            marker_symbol, filterExp='"Importance" <=2', label="lessthantwo"
        )
        root_rule.appendChild(less_than_two_rule)

        else_rule = QgsRuleBasedRenderer.Rule(None, elseRule=True)

        marker_symbol = QgsMarkerSymbol.createSimple(
            {"color": "#00ffff", "outline_style": "no", "size": "4"}
        )
        one_rule = QgsRuleBasedRenderer.Rule(
            marker_symbol, filterExp='"Pilots" = 1', label="1"
        )
        else_rule.appendChild(one_rule)
        marker_symbol = QgsMarkerSymbol.createSimple(
            {"color": "#ff8888", "outline_style": "no", "size": "4"}
        )
        two_rule = QgsRuleBasedRenderer.Rule(
            marker_symbol, filterExp='"Pilots" = 2', label="2"
        )
        else_rule.appendChild(two_rule)
        marker_symbol = QgsMarkerSymbol.createSimple(
            {"color": "#8888ff", "outline_style": "no", "size": "4"}
        )
        three_rule = QgsRuleBasedRenderer.Rule(
            marker_symbol, filterExp='"Pilots" = 3', label="3"
        )
        else_rule.appendChild(three_rule)

        root_rule.appendChild(else_rule)

        renderer = QgsRuleBasedRenderer(root_rule)
        point_layer.setRenderer(renderer)
        p = QgsProject()
        p.addMapLayer(point_layer)

        layout = QgsLayout(p)
        layout.initializeDefaults()

        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(19, 17, 100, 165))
        map.setFrameEnabled(True)
        map.setCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        map.setLayers([point_layer])
        map.zoomToExtent(QgsRectangle(-120, 14, -100, 18))
        map.setMapRotation(45)

        layout.addLayoutItem(map)

        map2 = QgsLayoutItemMap(layout)
        map2.attemptSetSceneRect(QRectF(150, 117, 100, 165))
        map2.setFrameEnabled(True)
        map2.setCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        map2.setLayers([point_layer])
        map2.setExtent(QgsRectangle(-12309930, 3091263, -11329181, 3977074))

        layout.addLayoutItem(map2)

        legend = QgsLayoutItemLegend(layout)
        legend.setLegendFilterByMapEnabled(True)
        legend.setFilterByMapItems([map, map2])
        layout.addLayoutItem(legend)
        legend.setTitle("Legend")
        legend.attemptSetSceneRect(QRectF(220, 20, 20, 20))
        legend.setFrameEnabled(True)
        legend.setFrameStrokeWidth(QgsLayoutMeasurement(2))
        legend.setBackgroundColor(QColor(200, 200, 200))
        legend.setTitle("")

        legend.setStyleFont(
            QgsLegendStyle.Style.Title, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Group, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Subgroup, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Symbol, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.SymbolLabel,
            QgsFontUtils.getStandardTestFont("Bold", 16),
        )

        self.assertTrue(self.render_layout_check("legend_multiple_filter_maps", layout))

    def test_filter_by_map_content_rendering_different_layers(self):
        point_path = os.path.join(TEST_DATA_DIR, "points.shp")
        point_layer = QgsVectorLayer(point_path, "points", "ogr")

        point_layer2 = QgsVectorLayer(point_path, "points2", "ogr")

        root_rule = QgsRuleBasedRenderer.Rule(None)
        marker_symbol = QgsMarkerSymbol.createSimple(
            {"color": "#ff0000", "outline_style": "no", "size": "8"}
        )

        less_than_two_rule = QgsRuleBasedRenderer.Rule(
            marker_symbol, filterExp='"Importance" <=2', label="lessthantwo"
        )
        root_rule.appendChild(less_than_two_rule)

        else_rule = QgsRuleBasedRenderer.Rule(None, elseRule=True)

        marker_symbol = QgsMarkerSymbol.createSimple(
            {"color": "#00ffff", "outline_style": "no", "size": "4"}
        )
        one_rule = QgsRuleBasedRenderer.Rule(
            marker_symbol, filterExp='"Pilots" = 1', label="1"
        )
        else_rule.appendChild(one_rule)
        marker_symbol = QgsMarkerSymbol.createSimple(
            {"color": "#ff8888", "outline_style": "no", "size": "4"}
        )
        two_rule = QgsRuleBasedRenderer.Rule(
            marker_symbol, filterExp='"Pilots" = 2', label="2"
        )
        else_rule.appendChild(two_rule)
        marker_symbol = QgsMarkerSymbol.createSimple(
            {"color": "#8888ff", "outline_style": "no", "size": "4"}
        )
        three_rule = QgsRuleBasedRenderer.Rule(
            marker_symbol, filterExp='"Pilots" = 3', label="3"
        )
        else_rule.appendChild(three_rule)

        root_rule.appendChild(else_rule)

        renderer = QgsRuleBasedRenderer(root_rule)
        point_layer.setRenderer(renderer)

        marker_symbol = QgsMarkerSymbol.createSimple(
            {"color": "#003366", "outline_style": "no", "size": "8"}
        )
        point_layer2.setRenderer(QgsSingleSymbolRenderer(marker_symbol))

        p = QgsProject()
        p.addMapLayers([point_layer, point_layer2])

        layout = QgsLayout(p)
        layout.initializeDefaults()

        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(19, 17, 100, 165))
        map.setFrameEnabled(True)
        map.setCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        map.setLayers([point_layer])
        map.zoomToExtent(QgsRectangle(-120, 14, -100, 18))
        map.setMapRotation(45)

        layout.addLayoutItem(map)

        map2 = QgsLayoutItemMap(layout)
        map2.attemptSetSceneRect(QRectF(150, 117, 100, 165))
        map2.setFrameEnabled(True)
        map2.setCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        map2.setLayers([point_layer2])
        map2.setExtent(QgsRectangle(-12309930, 3091263, -11329181, 3977074))

        layout.addLayoutItem(map2)

        legend = QgsLayoutItemLegend(layout)
        legend.setLegendFilterByMapEnabled(True)
        legend.setFilterByMapItems([map, map2])
        layout.addLayoutItem(legend)
        legend.setTitle("Legend")
        legend.attemptSetSceneRect(QRectF(220, 20, 20, 20))
        legend.setFrameEnabled(True)
        legend.setFrameStrokeWidth(QgsLayoutMeasurement(2))
        legend.setBackgroundColor(QColor(200, 200, 200))
        legend.setTitle("")

        legend.setStyleFont(
            QgsLegendStyle.Style.Title, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Group, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Subgroup, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Symbol, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.SymbolLabel,
            QgsFontUtils.getStandardTestFont("Bold", 16),
        )

        self.assertTrue(
            self.render_layout_check(
                "legend_multiple_filter_maps_different_layers", layout
            )
        )

    def test_atlas_legend_clipping(self):
        """Test issue GH #54654"""

        p = QgsProject()
        self.assertTrue(
            p.read(os.path.join(TEST_DATA_DIR, "layouts", "atlas_legend_clipping.qgs"))
        )

        layout = p.layoutManager().layoutByName("layout1")

        layer = list(p.mapLayers().values())[0]
        feature = layer.getFeature(3)
        req = QgsFeatureRequest()
        req.setFilterExpression("value = 11")
        feature = next(layer.getFeatures(req))

        layout.reportContext().setFeature(feature)
        legend = layout.items()[0]
        legend.setStyleFont(
            QgsLegendStyle.Style.Title, QgsFontUtils.getStandardTestFont("Bold", 20)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Group, QgsFontUtils.getStandardTestFont("Bold", 20)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Subgroup, QgsFontUtils.getStandardTestFont("Bold", 20)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Symbol, QgsFontUtils.getStandardTestFont("Bold", 20)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.SymbolLabel,
            QgsFontUtils.getStandardTestFont("Bold", 20),
        )
        legend.refresh()

        self.assertTrue(self.render_layout_check("atlas_legend_clipping", layout))

    def test_filter_by_map_content_rendering_different_layers_in_atlas(self):
        point_path = os.path.join(TEST_DATA_DIR, "points.shp")
        point_layer = QgsVectorLayer(point_path, "points", "ogr")

        point_layer2 = QgsVectorLayer(point_path, "points2", "ogr")

        root_rule = QgsRuleBasedRenderer.Rule(None)
        marker_symbol = QgsMarkerSymbol.createSimple(
            {"color": "#ff0000", "outline_style": "no", "size": "8"}
        )

        marker_symbol = QgsMarkerSymbol.createSimple(
            {"color": "#00ffff", "outline_style": "no", "size": "4"}
        )
        one_rule = QgsRuleBasedRenderer.Rule(
            marker_symbol, filterExp='"Pilots" = 1', label="1"
        )
        root_rule.appendChild(one_rule)
        marker_symbol = QgsMarkerSymbol.createSimple(
            {"color": "#ff8888", "outline_style": "no", "size": "4"}
        )
        two_rule = QgsRuleBasedRenderer.Rule(
            marker_symbol, filterExp='"Pilots" = 2', label="2"
        )
        root_rule.appendChild(two_rule)
        marker_symbol = QgsMarkerSymbol.createSimple(
            {"color": "#8888ff", "outline_style": "no", "size": "4"}
        )
        three_rule = QgsRuleBasedRenderer.Rule(
            marker_symbol, filterExp='"Pilots" = 3', label="3"
        )
        root_rule.appendChild(three_rule)

        renderer = QgsRuleBasedRenderer(root_rule)
        point_layer.setRenderer(renderer)

        marker_symbol = QgsMarkerSymbol.createSimple(
            {"color": "#003366", "outline_style": "no", "size": "8"}
        )
        point_layer2.setRenderer(QgsSingleSymbolRenderer(marker_symbol))

        p = QgsProject()
        p.addMapLayers([point_layer, point_layer2])

        layout = QgsLayout(p)
        layout.initializeDefaults()

        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(19, 17, 100, 165))
        map.setFrameEnabled(True)
        map.setCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        map.setLayers([point_layer])
        map.zoomToExtent(
            QgsRectangle(
                -108.52403736600929562,
                22.4408089916287814,
                -97.776639147740255,
                29.00866345834875304,
            )
        )
        map.setMapRotation(45)

        layout.addLayoutItem(map)

        map2 = QgsLayoutItemMap(layout)
        map2.attemptSetSceneRect(QRectF(150, 117, 100, 165))
        map2.setFrameEnabled(True)
        map2.setCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        map2.setLayers([point_layer2])
        map2.setExtent(QgsRectangle(-12309930, 3091263, -11329181, 3977074))

        layout.addLayoutItem(map2)

        legend = QgsLayoutItemLegend(layout)
        legend.setLegendFilterByMapEnabled(True)
        legend.setLinkedMap(map2)
        legend.setFilterByMapItems([map])
        layout.addLayoutItem(legend)
        legend.setTitle("Legend")
        legend.attemptSetSceneRect(QRectF(220, 20, 20, 20))
        legend.setFrameEnabled(True)
        legend.setFrameStrokeWidth(QgsLayoutMeasurement(2))
        legend.setBackgroundColor(QColor(200, 200, 200))
        legend.setTitle("")

        legend.setStyleFont(
            QgsLegendStyle.Style.Title, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Group, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Subgroup, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.Symbol, QgsFontUtils.getStandardTestFont("Bold", 16)
        )
        legend.setStyleFont(
            QgsLegendStyle.Style.SymbolLabel,
            QgsFontUtils.getStandardTestFont("Bold", 16),
        )
        legend.setLegendFilterOutAtlas(True)

        atlas_layer = QgsVectorLayer(
            "Polygon?field=fldtxt:string&field=fldint:integer", "addfeat", "memory"
        )
        layout.reportContext().setLayer(atlas_layer)
        f = QgsFeature()
        f.setGeometry(
            QgsGeometry.fromWkt(
                "Polygon ((-115.422 42.22, -118.202 36.246, -103.351 22.06, -102.314 22.682, -116.542 37.159, -113.348 41.846, -98.747 39.98, -93.313 47.281, -94.225 47.861, -99.95 41.39, -114.51 43.34, -115.422 42.22))"
            )
        )
        layout.reportContext().setFeature(f)
        legend.refresh()

        self.assertTrue(
            self.render_layout_check(
                "legend_multiple_filter_maps_different_layers_atlas", layout
            )
        )

    def testGeomGeneratorPoints(self):
        """
        Test legend behavior when geometry generator on points is involved
        """
        point_path = os.path.join(TEST_DATA_DIR, "points.shp")
        point_layer = QgsVectorLayer(point_path, "points", "ogr")
        QgsProject.instance().addMapLayers([point_layer])

        sub_symbol = QgsFillSymbol.createSimple(
            {"color": "#8888ff", "outline_style": "no"}
        )
        sym = QgsMarkerSymbol()
        buffer_layer = QgsGeometryGeneratorSymbolLayer.create(
            {"geometryModifier": "buffer($geometry, 0.05)"}
        )
        buffer_layer.setSymbolType(QgsSymbol.SymbolType.Fill)
        buffer_layer.setSubSymbol(sub_symbol)
        sym.changeSymbolLayer(0, buffer_layer)
        point_layer.setRenderer(QgsSingleSymbolRenderer(sym))

        s = QgsMapSettings()
        s.setLayers([point_layer])
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()

        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(20, 20, 80, 80))
        map.setFrameEnabled(True)
        map.setLayers([point_layer])
        layout.addLayoutItem(map)

        legend = QgsLayoutItemLegend(layout)
        legend.setTitle("Legend")
        legend.attemptSetSceneRect(QRectF(120, 20, 80, 80))
        legend.setFrameEnabled(True)
        legend.setFrameStrokeWidth(QgsLayoutMeasurement(2))
        legend.setBackgroundColor(QColor(200, 200, 200))
        legend.setTitle("")
        legend.setLegendFilterByMapEnabled(True)
        legend.setSymbolWidth(20)
        legend.setSymbolHeight(10)

        text_format = QgsTextFormat()
        text_format.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        text_format.setSize(16)

        for legend_item in [
            QgsLegendStyle.Style.Title,
            QgsLegendStyle.Style.Group,
            QgsLegendStyle.Style.Subgroup,
            QgsLegendStyle.Style.Symbol,
            QgsLegendStyle.Style.SymbolLabel,
        ]:
            style = legend.style(legend_item)
            style.setTextFormat(text_format)
            legend.setStyle(legend_item, style)

        # disable auto resizing
        legend.setResizeToContents(False)

        layout.addLayoutItem(legend)
        legend.setLinkedMap(map)

        map.setExtent(QgsRectangle(-102.51, 41.16, -102.36, 41.30))

        self.assertTrue(
            self.render_layout_check("composer_legend_geomgenerator_point", layout)
        )
        QgsProject.instance().removeMapLayers([point_layer.id()])

    def testGeomGeneratorLines(self):
        """
        Test legend behavior when geometry generator on lines is involved
        """
        line_path = os.path.join(TEST_DATA_DIR, "lines.shp")
        line_layer = QgsVectorLayer(line_path, "lines", "ogr")
        QgsProject.instance().addMapLayers([line_layer])

        sub_symbol = QgsFillSymbol.createSimple(
            {"color": "#8888ff", "outline_style": "no"}
        )
        sym = QgsLineSymbol()
        buffer_layer = QgsGeometryGeneratorSymbolLayer.create(
            {"geometryModifier": "buffer($geometry, 0.2)"}
        )
        buffer_layer.setSymbolType(QgsSymbol.SymbolType.Fill)
        buffer_layer.setSubSymbol(sub_symbol)
        sym.changeSymbolLayer(0, buffer_layer)
        line_layer.setRenderer(QgsSingleSymbolRenderer(sym))

        s = QgsMapSettings()
        s.setLayers([line_layer])
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()

        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(20, 20, 80, 80))
        map.setFrameEnabled(True)
        map.setLayers([line_layer])
        layout.addLayoutItem(map)
        map.setExtent(line_layer.extent())

        legend = QgsLayoutItemLegend(layout)
        legend.setTitle("Legend")
        legend.attemptSetSceneRect(QRectF(120, 20, 80, 80))
        legend.setFrameEnabled(True)
        legend.setFrameStrokeWidth(QgsLayoutMeasurement(2))
        legend.setBackgroundColor(QColor(200, 200, 200))
        legend.setTitle("")
        legend.setLegendFilterByMapEnabled(True)
        legend.setSymbolWidth(20)
        legend.setSymbolHeight(10)

        text_format = QgsTextFormat()
        text_format.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        text_format.setSize(16)

        for legend_item in [
            QgsLegendStyle.Style.Title,
            QgsLegendStyle.Style.Group,
            QgsLegendStyle.Style.Subgroup,
            QgsLegendStyle.Style.Symbol,
            QgsLegendStyle.Style.SymbolLabel,
        ]:
            style = legend.style(legend_item)
            style.setTextFormat(text_format)
            legend.setStyle(legend_item, style)

        # disable auto resizing
        legend.setResizeToContents(False)

        layout.addLayoutItem(legend)
        legend.setLinkedMap(map)

        map.setExtent(QgsRectangle(-100.3127, 35.7607, -98.5259, 36.5145))

        self.assertTrue(
            self.render_layout_check("composer_legend_geomgenerator_line", layout)
        )

        QgsProject.instance().removeMapLayers([line_layer.id()])


if __name__ == "__main__":
    unittest.main()
