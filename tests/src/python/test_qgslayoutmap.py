"""QGIS Unit tests for QgsLayoutItemMap.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2017 Nyall Dawson"
__date__ = "20/10/2017"
__copyright__ = "Copyright 2017, The QGIS Project"

import os

from qgis.PyQt.QtCore import (
    Qt,
    QFileInfo,
    QRectF,
    QSizeF,
)
from qgis.PyQt.QtGui import QColor, QPainter, QImage
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    QgsAnnotationPolygonItem,
    QgsCoordinateReferenceSystem,
    QgsFeature,
    QgsFillSymbol,
    QgsFontUtils,
    QgsGeometry,
    QgsLabelingEngineSettings,
    QgsLayout,
    QgsLayoutItemMap,
    QgsLayoutItemMapOverview,
    QgsLayoutItemShape,
    QgsLayoutMeasurement,
    QgsLayoutObject,
    QgsMapClippingRegion,
    QgsMapSettings,
    QgsMultiBandColorRenderer,
    QgsNullSymbolRenderer,
    QgsPalLayerSettings,
    QgsPoint,
    QgsProject,
    QgsProperty,
    QgsRasterLayer,
    QgsReadWriteContext,
    QgsRectangle,
    QgsSingleSymbolRenderer,
    QgsTextFormat,
    QgsUnitTypes,
    QgsVectorLayer,
    QgsVectorLayerSimpleLabeling,
    QgsSimpleFillSymbolLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from test_qgslayoutitem import LayoutItemTestCase
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayoutMap(QgisTestCase, LayoutItemTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "composer_map"

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.item_class = QgsLayoutItemMap

    def __init__(self, methodName):
        """Run once on class initialization."""
        QgisTestCase.__init__(self, methodName)
        myPath = os.path.join(TEST_DATA_DIR, "rgb256x256.png")
        rasterFileInfo = QFileInfo(myPath)
        self.raster_layer = QgsRasterLayer(
            rasterFileInfo.filePath(), rasterFileInfo.completeBaseName()
        )
        rasterRenderer = QgsMultiBandColorRenderer(
            self.raster_layer.dataProvider(), 1, 2, 3
        )
        self.raster_layer.setRenderer(rasterRenderer)

        myPath = os.path.join(TEST_DATA_DIR, "points.shp")
        vector_file_info = QFileInfo(myPath)
        self.vector_layer = QgsVectorLayer(
            vector_file_info.filePath(), vector_file_info.completeBaseName(), "ogr"
        )
        assert self.vector_layer.isValid()

        # pipe = mRasterLayer.pipe()
        # assert pipe.set(rasterRenderer), 'Cannot set pipe renderer'
        QgsProject.instance().addMapLayers([self.raster_layer, self.vector_layer])

        # create layout with layout map
        self.layout = QgsLayout(QgsProject.instance())
        self.layout.initializeDefaults()
        self.map = QgsLayoutItemMap(self.layout)
        self.map.attemptSetSceneRect(QRectF(20, 20, 200, 100))
        self.map.setFrameEnabled(True)
        self.map.setLayers([self.raster_layer])
        self.layout.addLayoutItem(self.map)

    def test_opacity(self):
        """
        Test rendering the map with opacity
        """
        map_settings = QgsMapSettings()
        map_settings.setLayers([self.vector_layer])
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()

        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(20, 20, 200, 100))
        map.setFrameEnabled(True)
        map.zoomToExtent(self.vector_layer.extent())
        map.setLayers([self.vector_layer])
        layout.addLayoutItem(map)

        map.setItemOpacity(0.3)

        self.assertFalse(map.requiresRasterization())
        self.assertTrue(map.containsAdvancedEffects())

        self.assertTrue(self.render_layout_check("composermap_opacity", layout))

    def test_opacity_rendering_designer_preview(self):
        """
        Test rendering of map opacity while in designer dialogs
        """
        p = QgsProject()
        l = QgsLayout(p)
        self.assertTrue(l.renderContext().isPreviewRender())

        l.initializeDefaults()

        map = QgsLayoutItemMap(l)
        map.attemptSetSceneRect(QRectF(20, 20, 200, 100))
        map.setFrameEnabled(True)
        map.zoomToExtent(self.vector_layer.extent())
        map.setLayers([self.vector_layer])
        l.addLayoutItem(map)

        map.setItemOpacity(0.3)

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

        spy = QSignalSpy(map.previewRefreshed)

        l.render(
            painter,
            QRectF(0, 0, painter.device().width(), painter.device().height()),
            paper_rect,
        )
        painter.end()

        # we have to wait for the preview image to refresh, then redraw
        # the map to get the actual content which was generated in the
        # background thread
        spy.wait()

        im.fill(Qt.GlobalColor.transparent)
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
                "composermap_opacity", "composermap_opacity", im, allowed_mismatch=0
            )
        )

    def test_blend_mode(self):
        """
        Test rendering the map with a blend mode
        """
        map_settings = QgsMapSettings()
        map_settings.setLayers([self.vector_layer])
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

        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(20, 20, 200, 100))
        map.setFrameEnabled(True)
        map.zoomToExtent(self.vector_layer.extent())
        map.setLayers([self.vector_layer])
        layout.addLayoutItem(map)

        map.setBlendMode(QPainter.CompositionMode.CompositionMode_Darken)

        self.assertTrue(map.requiresRasterization())

        self.assertTrue(self.render_layout_check("composermap_blend_mode", layout))

    def test_blend_mode_designer_preview(self):
        """
        Test rendering the map with a blend mode
        """
        map_settings = QgsMapSettings()
        map_settings.setLayers([self.vector_layer])
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

        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(20, 20, 200, 100))
        map.setFrameEnabled(True)
        map.zoomToExtent(self.vector_layer.extent())
        map.setLayers([self.vector_layer])
        layout.addLayoutItem(map)

        map.setBlendMode(QPainter.CompositionMode.CompositionMode_Darken)

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

        spy = QSignalSpy(map.previewRefreshed)

        layout.render(
            painter,
            QRectF(0, 0, painter.device().width(), painter.device().height()),
            paper_rect,
        )
        painter.end()

        # we have to wait for the preview image to refresh, then redraw
        # the map to get the actual content which was generated in the
        # background thread
        spy.wait()

        im.fill(Qt.GlobalColor.transparent)
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
                "composermap_blend_mode",
                "composermap_blend_mode",
                im,
                allowed_mismatch=0,
            )
        )

    def testMapCrs(self):
        # create layout with layout map
        map_settings = QgsMapSettings()
        map_settings.setLayers([self.vector_layer])
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()

        # check that new maps inherit project CRS
        QgsProject.instance().setCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(20, 20, 200, 100))
        map.setFrameEnabled(True)
        rectangle = QgsRectangle(-13838977, 2369660, -8672298, 6250909)
        map.setExtent(rectangle)
        map.setLayers([self.vector_layer])
        layout.addLayoutItem(map)

        self.assertEqual(map.crs().authid(), "EPSG:4326")
        self.assertFalse(map.presetCrs().isValid())

        # overwrite CRS
        map.setCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        self.assertEqual(map.crs().authid(), "EPSG:3857")
        self.assertEqual(map.presetCrs().authid(), "EPSG:3857")

        self.assertTrue(self.render_layout_check("composermap_crs3857", layout))

        # overwrite CRS
        map.setCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        self.assertEqual(map.presetCrs().authid(), "EPSG:4326")
        self.assertEqual(map.crs().authid(), "EPSG:4326")
        rectangle = QgsRectangle(-124, 17, -78, 52)
        map.zoomToExtent(rectangle)
        self.assertTrue(self.render_layout_check("composermap_crs4326", layout))

        # change back to project CRS
        map.setCrs(QgsCoordinateReferenceSystem())
        self.assertEqual(map.crs().authid(), "EPSG:4326")
        self.assertFalse(map.presetCrs().isValid())

    def testContainsAdvancedEffects(self):
        map_settings = QgsMapSettings()
        map_settings.setLayers([self.vector_layer])
        layout = QgsLayout(QgsProject.instance())
        map = QgsLayoutItemMap(layout)

        self.assertFalse(map.containsAdvancedEffects())
        self.vector_layer.setBlendMode(QPainter.CompositionMode.CompositionMode_Darken)
        result = map.containsAdvancedEffects()
        self.vector_layer.setBlendMode(
            QPainter.CompositionMode.CompositionMode_SourceOver
        )
        self.assertTrue(result)

    def testRasterization(self):
        map_settings = QgsMapSettings()
        map_settings.setLayers([self.vector_layer])
        layout = QgsLayout(QgsProject.instance())
        map = QgsLayoutItemMap(layout)

        self.assertFalse(map.requiresRasterization())
        self.vector_layer.setBlendMode(QPainter.CompositionMode.CompositionMode_Darken)
        self.assertFalse(map.requiresRasterization())
        self.assertTrue(map.containsAdvancedEffects())

        map.setBackgroundEnabled(False)
        self.assertTrue(map.requiresRasterization())
        map.setBackgroundEnabled(True)
        map.setBackgroundColor(QColor(1, 1, 1, 1))
        self.assertTrue(map.requiresRasterization())

        self.vector_layer.setBlendMode(
            QPainter.CompositionMode.CompositionMode_SourceOver
        )

    def testLabelMargin(self):
        """
        Test rendering map item with a label margin set
        """
        format = QgsTextFormat()
        format.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        format.setSize(20)
        format.setNamedStyle("Bold")
        format.setColor(QColor(0, 0, 0))
        settings = QgsPalLayerSettings()
        settings.setFormat(format)
        settings.fieldName = "'X'"
        settings.isExpression = True
        settings.placement = QgsPalLayerSettings.Placement.OverPoint

        vl = QgsVectorLayer("Point?crs=epsg:4326&field=id:integer", "vl", "memory")
        vl.setRenderer(QgsNullSymbolRenderer())
        f = QgsFeature(vl.fields(), 1)
        for x in range(15):
            for y in range(15):
                f.setGeometry(QgsPoint(x, y))
                vl.dataProvider().addFeature(f)

        vl.setLabeling(QgsVectorLayerSimpleLabeling(settings))
        vl.setLabelsEnabled(True)

        p = QgsProject()

        engine_settings = QgsLabelingEngineSettings()
        engine_settings.setFlag(
            QgsLabelingEngineSettings.Flag.UsePartialCandidates, False
        )
        engine_settings.setFlag(QgsLabelingEngineSettings.Flag.DrawLabelRectOnly, True)
        p.setLabelingEngineSettings(engine_settings)

        p.addMapLayer(vl)
        layout = QgsLayout(p)
        layout.initializeDefaults()
        p.setCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(10, 10, 180, 180))
        map.setFrameEnabled(True)
        map.zoomToExtent(vl.extent())
        map.setLayers([vl])
        layout.addLayoutItem(map)

        self.assertTrue(self.render_layout_check("composermap_label_nomargin", layout))

        map.setLabelMargin(
            QgsLayoutMeasurement(15, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        self.assertTrue(self.render_layout_check("composermap_label_margin", layout))

        map.setLabelMargin(
            QgsLayoutMeasurement(3, QgsUnitTypes.LayoutUnit.LayoutCentimeters)
        )
        self.assertTrue(self.render_layout_check("composermap_label_cm_margin", layout))

        map.setMapRotation(45)
        map.zoomToExtent(vl.extent())
        map.setScale(map.scale() * 1.2)
        map.setLabelMargin(
            QgsLayoutMeasurement(3, QgsUnitTypes.LayoutUnit.LayoutCentimeters)
        )
        self.assertTrue(
            self.render_layout_check("composermap_rotated_label_margin", layout)
        )

        # data defined
        map.setMapRotation(0)
        map.zoomToExtent(vl.extent())
        map.dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.MapLabelMargin,
            QgsProperty.fromExpression("1+3"),
        )
        map.refresh()
        self.assertTrue(self.render_layout_check("composermap_dd_label_margin", layout))

    def testPartialLabels(self):
        """
        Test rendering map item with a show partial labels flag
        """
        format = QgsTextFormat()
        format.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        format.setSize(20)
        format.setNamedStyle("Bold")
        format.setColor(QColor(0, 0, 0))
        settings = QgsPalLayerSettings()
        settings.setFormat(format)
        settings.fieldName = "'X'"
        settings.isExpression = True
        settings.placement = QgsPalLayerSettings.Placement.OverPoint

        vl = QgsVectorLayer("Point?crs=epsg:4326&field=id:integer", "vl", "memory")
        vl.setRenderer(QgsNullSymbolRenderer())
        f = QgsFeature(vl.fields(), 1)
        for x in range(15):
            for y in range(15):
                f.setGeometry(QgsPoint(x, y))
                vl.dataProvider().addFeature(f)

        vl.setLabeling(QgsVectorLayerSimpleLabeling(settings))
        vl.setLabelsEnabled(True)

        p = QgsProject()

        engine_settings = QgsLabelingEngineSettings()
        engine_settings.setFlag(
            QgsLabelingEngineSettings.Flag.UsePartialCandidates, False
        )
        engine_settings.setFlag(QgsLabelingEngineSettings.Flag.DrawLabelRectOnly, True)
        p.setLabelingEngineSettings(engine_settings)

        p.addMapLayer(vl)
        layout = QgsLayout(p)
        layout.initializeDefaults()
        p.setCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(10, 10, 180, 180))
        map.setFrameEnabled(True)
        map.zoomToExtent(vl.extent())
        map.setLayers([vl])
        layout.addLayoutItem(map)

        # default should always be to hide partial labels
        self.assertFalse(
            map.mapFlags() & QgsLayoutItemMap.MapItemFlag.ShowPartialLabels
        )

        # hiding partial labels (the default)
        map.setMapFlags(QgsLayoutItemMap.MapItemFlags())
        self.assertTrue(self.render_layout_check("composermap_label_nomargin", layout))

        # showing partial labels
        map.setMapFlags(QgsLayoutItemMap.MapItemFlag.ShowPartialLabels)
        self.assertTrue(
            self.render_layout_check("composermap_show_partial_labels", layout)
        )

    def testBlockingItems(self):
        """
        Test rendering map item with blocking items
        """
        format = QgsTextFormat()
        format.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        format.setSize(20)
        format.setNamedStyle("Bold")
        format.setColor(QColor(0, 0, 0))
        settings = QgsPalLayerSettings()
        settings.setFormat(format)
        settings.fieldName = "'X'"
        settings.isExpression = True
        settings.placement = QgsPalLayerSettings.Placement.OverPoint

        vl = QgsVectorLayer("Point?crs=epsg:4326&field=id:integer", "vl", "memory")
        vl.setRenderer(QgsNullSymbolRenderer())
        f = QgsFeature(vl.fields(), 1)
        for x in range(15):
            for y in range(15):
                f.setGeometry(QgsPoint(x, y))
                vl.dataProvider().addFeature(f)

        vl.setLabeling(QgsVectorLayerSimpleLabeling(settings))
        vl.setLabelsEnabled(True)

        p = QgsProject()

        engine_settings = QgsLabelingEngineSettings()
        engine_settings.setFlag(QgsLabelingEngineSettings.Flag.DrawLabelRectOnly, True)
        p.setLabelingEngineSettings(engine_settings)

        p.addMapLayer(vl)
        layout = QgsLayout(p)
        layout.initializeDefaults()
        p.setCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(10, 10, 180, 180))
        map.setFrameEnabled(True)
        map.zoomToExtent(vl.extent())
        map.setLayers([vl])
        map.setId("map")
        layout.addLayoutItem(map)

        map2 = QgsLayoutItemMap(layout)
        map2.attemptSetSceneRect(QRectF(0, 5, 50, 80))
        map2.setFrameEnabled(True)
        map2.setBackgroundEnabled(False)
        map2.setId("map2")
        layout.addLayoutItem(map2)

        map3 = QgsLayoutItemMap(layout)
        map3.attemptSetSceneRect(QRectF(150, 160, 50, 50))
        map3.setFrameEnabled(True)
        map3.setBackgroundEnabled(False)
        map3.setId("map3")
        layout.addLayoutItem(map3)

        map.addLabelBlockingItem(map2)
        map.addLabelBlockingItem(map3)
        map.setMapFlags(QgsLayoutItemMap.MapItemFlags())
        self.assertTrue(self.render_layout_check("composermap_label_blockers", layout))

        doc = QDomDocument("testdoc")
        elem = layout.writeXml(doc, QgsReadWriteContext())

        l2 = QgsLayout(p)
        self.assertTrue(l2.readXml(elem, doc, QgsReadWriteContext()))
        map_restore = [
            i for i in l2.items() if isinstance(i, QgsLayoutItemMap) and i.id() == "map"
        ][0]
        map2_restore = [
            i
            for i in l2.items()
            if isinstance(i, QgsLayoutItemMap) and i.id() == "map2"
        ][0]
        map3_restore = [
            i
            for i in l2.items()
            if isinstance(i, QgsLayoutItemMap) and i.id() == "map3"
        ][0]
        self.assertTrue(map_restore.isLabelBlockingItem(map2_restore))
        self.assertTrue(map_restore.isLabelBlockingItem(map3_restore))

    def testTheme(self):
        layout = QgsLayout(QgsProject.instance())
        map = QgsLayoutItemMap(layout)
        self.assertFalse(map.followVisibilityPreset())
        self.assertFalse(map.followVisibilityPresetName())

        spy = QSignalSpy(map.themeChanged)

        map.setFollowVisibilityPresetName("theme")
        self.assertFalse(map.followVisibilityPreset())
        self.assertEqual(map.followVisibilityPresetName(), "theme")
        # should not be emitted - followVisibilityPreset is False
        self.assertEqual(len(spy), 0)
        map.setFollowVisibilityPresetName("theme2")
        self.assertEqual(map.followVisibilityPresetName(), "theme2")
        self.assertEqual(len(spy), 0)

        map.setFollowVisibilityPresetName("")
        map.setFollowVisibilityPreset(True)
        # should not be emitted - followVisibilityPresetName is empty
        self.assertEqual(len(spy), 0)
        self.assertFalse(map.followVisibilityPresetName())
        self.assertTrue(map.followVisibilityPreset())

        map.setFollowVisibilityPresetName("theme")
        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[-1][0], "theme")
        map.setFollowVisibilityPresetName("theme")
        self.assertEqual(len(spy), 1)
        map.setFollowVisibilityPresetName("theme2")
        self.assertEqual(len(spy), 2)
        self.assertEqual(spy[-1][0], "theme2")
        map.setFollowVisibilityPreset(False)
        self.assertEqual(len(spy), 3)
        self.assertFalse(spy[-1][0])
        map.setFollowVisibilityPreset(False)
        self.assertEqual(len(spy), 3)
        map.setFollowVisibilityPresetName("theme3")
        self.assertEqual(len(spy), 3)
        map.setFollowVisibilityPreset(True)
        self.assertEqual(len(spy), 4)
        self.assertEqual(spy[-1][0], "theme3")
        map.setFollowVisibilityPreset(True)
        self.assertEqual(len(spy), 4)

        # data defined theme
        map.dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.MapStylePreset,
            QgsProperty.fromValue("theme4"),
        )
        map.refresh()
        self.assertEqual(len(spy), 5)
        self.assertEqual(spy[-1][0], "theme4")
        map.refresh()
        self.assertEqual(len(spy), 5)
        map.dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.MapStylePreset,
            QgsProperty.fromValue("theme6"),
        )
        map.refresh()
        self.assertEqual(len(spy), 6)
        self.assertEqual(spy[-1][0], "theme6")

    def testClipping(self):
        format = QgsTextFormat()
        format.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        format.setSize(30)
        format.setNamedStyle("Bold")
        format.setColor(QColor(0, 0, 0))
        settings = QgsPalLayerSettings()
        settings.setFormat(format)
        settings.fieldName = "'XXXX'"
        settings.isExpression = True
        settings.placement = QgsPalLayerSettings.Placement.OverPoint

        vl = QgsVectorLayer("Polygon?crs=epsg:4326&field=id:integer", "vl", "memory")

        props = {
            "color": "127,255,127",
            "outline_style": "solid",
            "outline_width": "1",
            "outline_color": "0,0,255",
        }
        fillSymbol = QgsFillSymbol.createSimple(props)
        renderer = QgsSingleSymbolRenderer(fillSymbol)
        vl.setRenderer(renderer)

        f = QgsFeature(vl.fields(), 1)
        for x in range(0, 15, 3):
            for y in range(0, 15, 3):
                f.setGeometry(QgsGeometry(QgsPoint(x, y)).buffer(1, 3))
                vl.dataProvider().addFeature(f)

        vl.setLabeling(QgsVectorLayerSimpleLabeling(settings))
        vl.setLabelsEnabled(True)

        p = QgsProject()

        p.addMapLayer(vl)
        layout = QgsLayout(p)
        layout.initializeDefaults()
        p.setCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(10, 10, 180, 180))
        map.setFrameEnabled(False)
        map.zoomToExtent(vl.extent())
        map.setLayers([vl])
        layout.addLayoutItem(map)

        shape = QgsLayoutItemShape(layout)
        layout.addLayoutItem(shape)
        shape.setShapeType(QgsLayoutItemShape.Shape.Ellipse)
        shape.attemptSetSceneRect(QRectF(10, 10, 180, 180))
        props = {"color": "0,0,0,0", "outline_style": "no"}
        fillSymbol = QgsFillSymbol.createSimple(props)
        shape.setSymbol(fillSymbol)

        map.itemClippingSettings().setEnabled(True)
        map.itemClippingSettings().setSourceItem(shape)
        map.itemClippingSettings().setForceLabelsInsideClipPath(False)
        map.itemClippingSettings().setFeatureClippingType(
            QgsMapClippingRegion.FeatureClippingType.ClipToIntersection
        )

        self.assertTrue(self.render_layout_check("composermap_itemclip", layout))

    def testClippingForceLabelsInside(self):
        format = QgsTextFormat()
        format.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        format.setSize(30)
        format.setNamedStyle("Bold")
        format.setColor(QColor(0, 0, 0))
        settings = QgsPalLayerSettings()
        settings.setFormat(format)
        settings.fieldName = "'XXXX'"
        settings.isExpression = True
        settings.placement = QgsPalLayerSettings.Placement.OverPoint

        vl = QgsVectorLayer("Polygon?crs=epsg:4326&field=id:integer", "vl", "memory")

        props = {
            "color": "127,255,127",
            "outline_style": "solid",
            "outline_width": "1",
            "outline_color": "0,0,255",
        }
        fillSymbol = QgsFillSymbol.createSimple(props)
        renderer = QgsSingleSymbolRenderer(fillSymbol)
        vl.setRenderer(renderer)

        f = QgsFeature(vl.fields(), 1)
        for x in range(0, 15, 3):
            for y in range(0, 15, 3):
                f.setGeometry(QgsGeometry(QgsPoint(x, y)).buffer(1, 3))
                vl.dataProvider().addFeature(f)

        vl.setLabeling(QgsVectorLayerSimpleLabeling(settings))
        vl.setLabelsEnabled(True)

        p = QgsProject()

        p.addMapLayer(vl)
        layout = QgsLayout(p)
        layout.initializeDefaults()
        p.setCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(10, 10, 180, 180))
        map.setFrameEnabled(False)
        map.zoomToExtent(vl.extent())
        map.setLayers([vl])
        layout.addLayoutItem(map)

        shape = QgsLayoutItemShape(layout)
        layout.addLayoutItem(shape)
        shape.setShapeType(QgsLayoutItemShape.Shape.Ellipse)
        shape.attemptSetSceneRect(QRectF(10, 10, 180, 180))
        props = {"color": "0,0,0,0", "outline_style": "no"}
        fillSymbol = QgsFillSymbol.createSimple(props)
        shape.setSymbol(fillSymbol)

        map.itemClippingSettings().setEnabled(True)
        map.itemClippingSettings().setSourceItem(shape)
        map.itemClippingSettings().setForceLabelsInsideClipPath(True)
        map.itemClippingSettings().setFeatureClippingType(
            QgsMapClippingRegion.FeatureClippingType.ClipPainterOnly
        )

        self.assertTrue(
            self.render_layout_check("composermap_itemclip_force_labels_inside", layout)
        )

    def testClippingOverview(self):
        format = QgsTextFormat()
        format.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        format.setSize(30)
        format.setNamedStyle("Bold")
        format.setColor(QColor(0, 0, 0))
        settings = QgsPalLayerSettings()
        settings.setFormat(format)
        settings.fieldName = "'XXXX'"
        settings.isExpression = True
        settings.placement = QgsPalLayerSettings.Placement.OverPoint

        vl = QgsVectorLayer("Polygon?crs=epsg:4326&field=id:integer", "vl", "memory")

        props = {
            "color": "127,255,127",
            "outline_style": "solid",
            "outline_width": "1",
            "outline_color": "0,0,255",
        }
        fillSymbol = QgsFillSymbol.createSimple(props)
        renderer = QgsSingleSymbolRenderer(fillSymbol)
        vl.setRenderer(renderer)

        f = QgsFeature(vl.fields(), 1)
        for x in range(0, 15, 3):
            for y in range(0, 15, 3):
                f.setGeometry(QgsGeometry(QgsPoint(x, y)).buffer(1, 3))
                vl.dataProvider().addFeature(f)

        vl.setLabeling(QgsVectorLayerSimpleLabeling(settings))
        vl.setLabelsEnabled(True)

        vl2 = vl.clone()
        vl2.setLabelsEnabled(False)

        p = QgsProject()

        p.addMapLayer(vl)
        p.addMapLayer(vl2)
        layout = QgsLayout(p)
        layout.initializeDefaults()
        p.setCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(10, 10, 180, 180))
        map.setFrameEnabled(False)
        map.zoomToExtent(vl.extent())
        map.setLayers([vl])
        layout.addLayoutItem(map)

        map2 = QgsLayoutItemMap(layout)
        map2.attemptSetSceneRect(QRectF(160, 150, 70, 70))
        map2.setFrameEnabled(True)
        map2extent = vl.extent()
        map2extent.grow(3)
        map2.zoomToExtent(map2extent)
        map2.setLayers([vl2])
        layout.addLayoutItem(map2)
        overview = QgsLayoutItemMapOverview("t", map2)
        overview.setLinkedMap(map)
        props = {
            "color": "0,0,0,0",
            "outline_style": "solid",
            "outline_width": "1",
            "outline_color": "0,0,255",
        }
        fillSymbol = QgsFillSymbol.createSimple(props)
        overview.setFrameSymbol(fillSymbol)

        map2.overviews().addOverview(overview)

        shape = QgsLayoutItemShape(layout)
        layout.addLayoutItem(shape)
        shape.setShapeType(QgsLayoutItemShape.Shape.Ellipse)
        shape.attemptSetSceneRect(QRectF(10, 10, 180, 180))
        props = {"color": "0,0,0,0", "outline_style": "no"}
        fillSymbol = QgsFillSymbol.createSimple(props)
        shape.setSymbol(fillSymbol)

        map.itemClippingSettings().setEnabled(True)
        map.itemClippingSettings().setSourceItem(shape)
        map.itemClippingSettings().setForceLabelsInsideClipPath(False)
        map.itemClippingSettings().setFeatureClippingType(
            QgsMapClippingRegion.FeatureClippingType.ClipToIntersection
        )

        self.assertTrue(
            self.render_layout_check("composermap_itemclip_overview", layout)
        )

    def testClippingHideClipSource(self):
        """
        When an item is set to be the clip source, it shouldn't render anything itself
        """
        format = QgsTextFormat()
        format.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        format.setSize(30)
        format.setNamedStyle("Bold")
        format.setColor(QColor(0, 0, 0))
        settings = QgsPalLayerSettings()
        settings.setFormat(format)
        settings.fieldName = "'XXXX'"
        settings.isExpression = True
        settings.placement = QgsPalLayerSettings.Placement.OverPoint

        vl = QgsVectorLayer("Polygon?crs=epsg:4326&field=id:integer", "vl", "memory")

        props = {
            "color": "127,255,127",
            "outline_style": "solid",
            "outline_width": "1",
            "outline_color": "0,0,255",
        }
        fillSymbol = QgsFillSymbol.createSimple(props)
        renderer = QgsSingleSymbolRenderer(fillSymbol)
        vl.setRenderer(renderer)

        f = QgsFeature(vl.fields(), 1)
        for x in range(0, 15, 3):
            for y in range(0, 15, 3):
                f.setGeometry(QgsGeometry(QgsPoint(x, y)).buffer(1, 3))
                vl.dataProvider().addFeature(f)

        vl.setLabeling(QgsVectorLayerSimpleLabeling(settings))
        vl.setLabelsEnabled(True)

        p = QgsProject()

        p.addMapLayer(vl)
        layout = QgsLayout(p)
        layout.initializeDefaults()
        p.setCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(10, 10, 180, 180))
        map.setFrameEnabled(False)
        map.zoomToExtent(vl.extent())
        map.setLayers([vl])
        layout.addLayoutItem(map)

        shape = QgsLayoutItemShape(layout)
        layout.addLayoutItem(shape)
        shape.setShapeType(QgsLayoutItemShape.Shape.Ellipse)
        shape.attemptSetSceneRect(QRectF(10, 10, 180, 180))

        map.itemClippingSettings().setEnabled(True)
        map.itemClippingSettings().setSourceItem(shape)
        map.itemClippingSettings().setForceLabelsInsideClipPath(False)
        map.itemClippingSettings().setFeatureClippingType(
            QgsMapClippingRegion.FeatureClippingType.ClipToIntersection
        )

        self.assertTrue(
            self.render_layout_check("composermap_itemclip_nodrawsource", layout)
        )

    def testClippingBackgroundFrame(self):
        """
        Make sure frame/background are also clipped
        """
        vl = QgsVectorLayer("Polygon?crs=epsg:4326&field=id:integer", "vl", "memory")

        props = {
            "color": "127,255,127",
            "outline_style": "solid",
            "outline_width": "1",
            "outline_color": "0,0,255",
        }
        fillSymbol = QgsFillSymbol.createSimple(props)
        renderer = QgsSingleSymbolRenderer(fillSymbol)
        vl.setRenderer(renderer)

        f = QgsFeature(vl.fields(), 1)
        for x in range(0, 15, 3):
            for y in range(0, 15, 3):
                f.setGeometry(QgsGeometry(QgsPoint(x, y)).buffer(1, 3))
                vl.dataProvider().addFeature(f)

        p = QgsProject()

        p.addMapLayer(vl)
        layout = QgsLayout(p)
        layout.initializeDefaults()
        p.setCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(10, 10, 180, 180))
        map.setFrameEnabled(True)
        map.setFrameStrokeWidth(
            QgsLayoutMeasurement(2, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        map.setBackgroundEnabled(True)
        map.setBackgroundColor(QColor(200, 255, 200))
        map.zoomToExtent(vl.extent())
        map.setLayers([vl])
        layout.addLayoutItem(map)

        shape = QgsLayoutItemShape(layout)
        layout.addLayoutItem(shape)
        shape.setShapeType(QgsLayoutItemShape.Shape.Ellipse)
        shape.attemptSetSceneRect(QRectF(10, 10, 180, 180))

        map.itemClippingSettings().setEnabled(True)
        map.itemClippingSettings().setSourceItem(shape)
        map.itemClippingSettings().setFeatureClippingType(
            QgsMapClippingRegion.FeatureClippingType.ClipPainterOnly
        )

        self.assertTrue(
            self.render_layout_check("composermap_itemclip_background", layout)
        )

    def testMainAnnotationLayer(self):
        """
        Make sure main annotation layer is rendered in maps above all other layers
        """
        p = QgsProject()

        vl = QgsVectorLayer(
            "Polygon?crs=epsg:4326&field=fldtxt:string", "layer", "memory"
        )
        sym3 = QgsFillSymbol.createSimple({"color": "#b200b2"})
        vl.renderer().setSymbol(sym3)

        p.addMapLayer(vl)
        layout = QgsLayout(p)
        layout.initializeDefaults()
        p.setCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(10, 10, 180, 180))
        map.setFrameEnabled(True)
        map.setFrameStrokeWidth(
            QgsLayoutMeasurement(2, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        map.setBackgroundEnabled(True)
        map.setBackgroundColor(QColor(200, 255, 200))
        map.zoomToExtent(QgsRectangle(10, 30, 20, 35))
        map.setLayers([vl])
        layout.addLayoutItem(map)

        # add polygon to layer
        f = QgsFeature()
        f.setGeometry(QgsGeometry.fromRect(QgsRectangle(5, 25, 25, 45)))
        self.assertTrue(vl.dataProvider().addFeatures([f]))

        # no annotation yet...
        self.assertTrue(
            self.render_layout_check("composermap_annotation_empty", layout)
        )

        annotation_layer = p.mainAnnotationLayer()
        annotation_layer.setCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        annotation_geom = QgsGeometry.fromRect(QgsRectangle(12, 30, 18, 33))
        annotation = QgsAnnotationPolygonItem(annotation_geom.constGet().clone())
        sym3 = QgsFillSymbol.createSimple({"color": "#ff0000", "outline_style": "no"})
        annotation.setSymbol(sym3)
        annotation_layer.addItem(annotation)

        # annotation must be drawn above map layers
        self.assertTrue(self.render_layout_check("composermap_annotation_item", layout))

    def testCrsChanged(self):
        """
        Test that the CRS changed signal is emitted in the right circumstances
        """
        p = QgsProject()
        layout = QgsLayout(p)
        p.setCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        map = QgsLayoutItemMap(layout)

        spy = QSignalSpy(map.crsChanged)
        # map has no explicit crs set, so follows project crs => signal should be emitted
        # when project crs is changed
        p.setCrs(QgsCoordinateReferenceSystem("EPSG:3111"))
        self.assertEqual(len(spy), 1)
        p.setCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        self.assertEqual(len(spy), 2)
        # set explicit crs on map item
        map.setCrs(QgsCoordinateReferenceSystem("EPSG:28356"))
        self.assertEqual(len(spy), 3)
        map.setCrs(QgsCoordinateReferenceSystem("EPSG:28356"))
        self.assertEqual(len(spy), 3)
        map.setCrs(QgsCoordinateReferenceSystem("EPSG:28355"))
        self.assertEqual(len(spy), 4)
        # should not care about project crs changes anymore..
        p.setCrs(QgsCoordinateReferenceSystem("EPSG:3111"))
        self.assertEqual(len(spy), 4)
        # set back to project crs
        map.setCrs(QgsCoordinateReferenceSystem())
        self.assertEqual(len(spy), 5)

        map.setCrs(QgsCoordinateReferenceSystem("EPSG:28355"))
        self.assertEqual(len(spy), 6)
        # data defined crs
        map.dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.MapCrs,
            QgsProperty.fromValue("EPSG:4283"),
        )
        self.assertEqual(len(spy), 6)
        map.refresh()
        self.assertEqual(len(spy), 7)

    def testMapSettingsDpiTarget(self):
        """
        Test that the CRS changed signal is emitted in the right circumstances
        """
        p = QgsProject()
        layout = QgsLayout(p)
        layout.renderContext().setDpi(111.1)
        map = QgsLayoutItemMap(layout)
        ms = map.mapSettings(QgsRectangle(0, 0, 1, 1), QSizeF(10, 10), 96, False)
        self.assertEqual(ms.dpiTarget(), 111.1)


if __name__ == "__main__":
    unittest.main()
