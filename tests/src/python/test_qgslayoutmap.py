# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayoutItemMap.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2017 Nyall Dawson'
__date__ = '20/10/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import qgis  # NOQA

import os

from qgis.PyQt.QtCore import QFileInfo, QRectF, QDir, QCoreApplication, QEvent, QSizeF
from qgis.PyQt.QtXml import QDomDocument
from qgis.PyQt.QtGui import QPainter, QColor
from qgis.PyQt.QtTest import QSignalSpy

from qgis.core import (QgsLayoutItemMap,
                       QgsRectangle,
                       QgsRasterLayer,
                       QgsVectorLayer,
                       QgsLayout,
                       QgsMapSettings,
                       QgsProject,
                       QgsMultiBandColorRenderer,
                       QgsCoordinateReferenceSystem,
                       QgsTextFormat,
                       QgsFontUtils,
                       QgsPalLayerSettings,
                       QgsNullSymbolRenderer,
                       QgsPoint,
                       QgsFeature,
                       QgsVectorLayerSimpleLabeling,
                       QgsLabelingEngineSettings,
                       QgsLayoutMeasurement,
                       QgsUnitTypes,
                       QgsLayoutObject,
                       QgsProperty,
                       QgsReadWriteContext,
                       QgsFillSymbol,
                       QgsSingleSymbolRenderer,
                       QgsGeometry,
                       QgsLayoutItemShape,
                       QgsMapClippingRegion,
                       QgsLayoutItemMapOverview,
                       QgsAnnotationPolygonItem)

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath
from qgslayoutchecker import QgsLayoutChecker
from test_qgslayoutitem import LayoutItemTestCase

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayoutMap(unittest.TestCase, LayoutItemTestCase):

    @classmethod
    def setUpClass(cls):
        cls.item_class = QgsLayoutItemMap
        cls.report = "<h1>Python QgsLayoutItemMap Tests</h1>\n"

    @classmethod
    def tearDownClass(cls):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(cls.report)

    def __init__(self, methodName):
        """Run once on class initialization."""
        unittest.TestCase.__init__(self, methodName)
        myPath = os.path.join(TEST_DATA_DIR, 'rgb256x256.png')
        rasterFileInfo = QFileInfo(myPath)
        self.raster_layer = QgsRasterLayer(rasterFileInfo.filePath(),
                                           rasterFileInfo.completeBaseName())
        rasterRenderer = QgsMultiBandColorRenderer(
            self.raster_layer.dataProvider(), 1, 2, 3)
        self.raster_layer.setRenderer(rasterRenderer)

        myPath = os.path.join(TEST_DATA_DIR, 'points.shp')
        vector_file_info = QFileInfo(myPath)
        self.vector_layer = QgsVectorLayer(vector_file_info.filePath(),
                                           vector_file_info.completeBaseName(), 'ogr')
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

    def testMapCrs(self):
        # create layout with layout map
        map_settings = QgsMapSettings()
        map_settings.setLayers([self.vector_layer])
        layout = QgsLayout(QgsProject.instance())
        layout.initializeDefaults()

        # check that new maps inherit project CRS
        QgsProject.instance().setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(20, 20, 200, 100))
        map.setFrameEnabled(True)
        rectangle = QgsRectangle(-13838977, 2369660, -8672298, 6250909)
        map.setExtent(rectangle)
        map.setLayers([self.vector_layer])
        layout.addLayoutItem(map)

        self.assertEqual(map.crs().authid(), 'EPSG:4326')
        self.assertFalse(map.presetCrs().isValid())

        # overwrite CRS
        map.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        self.assertEqual(map.crs().authid(), 'EPSG:3857')
        self.assertEqual(map.presetCrs().authid(), 'EPSG:3857')

        checker = QgsLayoutChecker('composermap_crs3857', layout)
        checker.setControlPathPrefix("composer_map")
        result, message = checker.testLayout()
        TestQgsLayoutMap.report += checker.report()
        self.assertTrue(result, message)

        # overwrite CRS
        map.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        self.assertEqual(map.presetCrs().authid(), 'EPSG:4326')
        self.assertEqual(map.crs().authid(), 'EPSG:4326')
        rectangle = QgsRectangle(-124, 17, -78, 52)
        map.zoomToExtent(rectangle)
        checker = QgsLayoutChecker('composermap_crs4326', layout)
        checker.setControlPathPrefix("composer_map")
        result, message = checker.testLayout()
        TestQgsLayoutMap.report += checker.report()
        self.assertTrue(result, message)

        # change back to project CRS
        map.setCrs(QgsCoordinateReferenceSystem())
        self.assertEqual(map.crs().authid(), 'EPSG:4326')
        self.assertFalse(map.presetCrs().isValid())

    def testContainsAdvancedEffects(self):
        map_settings = QgsMapSettings()
        map_settings.setLayers([self.vector_layer])
        layout = QgsLayout(QgsProject.instance())
        map = QgsLayoutItemMap(layout)

        self.assertFalse(map.containsAdvancedEffects())
        self.vector_layer.setBlendMode(QPainter.CompositionMode_Darken)
        result = map.containsAdvancedEffects()
        self.vector_layer.setBlendMode(QPainter.CompositionMode_SourceOver)
        self.assertTrue(result)

    def testRasterization(self):
        map_settings = QgsMapSettings()
        map_settings.setLayers([self.vector_layer])
        layout = QgsLayout(QgsProject.instance())
        map = QgsLayoutItemMap(layout)

        self.assertFalse(map.requiresRasterization())
        self.vector_layer.setBlendMode(QPainter.CompositionMode_Darken)
        self.assertFalse(map.requiresRasterization())
        self.assertTrue(map.containsAdvancedEffects())

        map.setBackgroundEnabled(False)
        self.assertTrue(map.requiresRasterization())
        map.setBackgroundEnabled(True)
        map.setBackgroundColor(QColor(1, 1, 1, 1))
        self.assertTrue(map.requiresRasterization())

        self.vector_layer.setBlendMode(QPainter.CompositionMode_SourceOver)

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
        settings.placement = QgsPalLayerSettings.OverPoint

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
        engine_settings.setFlag(QgsLabelingEngineSettings.UsePartialCandidates, False)
        engine_settings.setFlag(QgsLabelingEngineSettings.DrawLabelRectOnly, True)
        p.setLabelingEngineSettings(engine_settings)

        p.addMapLayer(vl)
        layout = QgsLayout(p)
        layout.initializeDefaults()
        p.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(10, 10, 180, 180))
        map.setFrameEnabled(True)
        map.zoomToExtent(vl.extent())
        map.setLayers([vl])
        layout.addLayoutItem(map)

        checker = QgsLayoutChecker('composermap_label_nomargin', layout)
        checker.setControlPathPrefix("composer_map")
        result, message = checker.testLayout()
        TestQgsLayoutMap.report += checker.report()
        self.assertTrue(result, message)

        map.setLabelMargin(QgsLayoutMeasurement(15, QgsUnitTypes.LayoutMillimeters))
        checker = QgsLayoutChecker('composermap_label_margin', layout)
        checker.setControlPathPrefix("composer_map")
        result, message = checker.testLayout()
        TestQgsLayoutMap.report += checker.report()
        self.assertTrue(result, message)

        map.setLabelMargin(QgsLayoutMeasurement(3, QgsUnitTypes.LayoutCentimeters))
        checker = QgsLayoutChecker('composermap_label_cm_margin', layout)
        checker.setControlPathPrefix("composer_map")
        result, message = checker.testLayout()
        TestQgsLayoutMap.report += checker.report()
        self.assertTrue(result, message)

        map.setMapRotation(45)
        map.zoomToExtent(vl.extent())
        map.setScale(map.scale() * 1.2)
        checker = QgsLayoutChecker('composermap_rotated_label_margin', layout)
        checker.setControlPathPrefix("composer_map")
        result, message = checker.testLayout()
        TestQgsLayoutMap.report += checker.report()
        self.assertTrue(result, message)

        # data defined
        map.setMapRotation(0)
        map.zoomToExtent(vl.extent())
        map.dataDefinedProperties().setProperty(QgsLayoutObject.MapLabelMargin, QgsProperty.fromExpression('1+3'))
        map.refresh()
        checker = QgsLayoutChecker('composermap_dd_label_margin', layout)
        checker.setControlPathPrefix("composer_map")
        result, message = checker.testLayout()
        TestQgsLayoutMap.report += checker.report()
        self.assertTrue(result, message)

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
        settings.placement = QgsPalLayerSettings.OverPoint

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
        engine_settings.setFlag(QgsLabelingEngineSettings.UsePartialCandidates, False)
        engine_settings.setFlag(QgsLabelingEngineSettings.DrawLabelRectOnly, True)
        p.setLabelingEngineSettings(engine_settings)

        p.addMapLayer(vl)
        layout = QgsLayout(p)
        layout.initializeDefaults()
        p.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(10, 10, 180, 180))
        map.setFrameEnabled(True)
        map.zoomToExtent(vl.extent())
        map.setLayers([vl])
        layout.addLayoutItem(map)

        # default should always be to hide partial labels
        self.assertFalse(map.mapFlags() & QgsLayoutItemMap.ShowPartialLabels)

        # hiding partial labels (the default)
        map.setMapFlags(QgsLayoutItemMap.MapItemFlags())
        checker = QgsLayoutChecker('composermap_label_nomargin', layout)
        checker.setControlPathPrefix("composer_map")
        result, message = checker.testLayout()
        TestQgsLayoutMap.report += checker.report()
        self.assertTrue(result, message)

        # showing partial labels
        map.setMapFlags(QgsLayoutItemMap.ShowPartialLabels)
        checker = QgsLayoutChecker('composermap_show_partial_labels', layout)
        checker.setControlPathPrefix("composer_map")
        result, message = checker.testLayout()
        TestQgsLayoutMap.report += checker.report()
        self.assertTrue(result, message)

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
        settings.placement = QgsPalLayerSettings.OverPoint

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
        engine_settings.setFlag(QgsLabelingEngineSettings.DrawLabelRectOnly, True)
        p.setLabelingEngineSettings(engine_settings)

        p.addMapLayer(vl)
        layout = QgsLayout(p)
        layout.initializeDefaults()
        p.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(10, 10, 180, 180))
        map.setFrameEnabled(True)
        map.zoomToExtent(vl.extent())
        map.setLayers([vl])
        map.setId('map')
        layout.addLayoutItem(map)

        map2 = QgsLayoutItemMap(layout)
        map2.attemptSetSceneRect(QRectF(0, 5, 50, 80))
        map2.setFrameEnabled(True)
        map2.setBackgroundEnabled(False)
        map2.setId('map2')
        layout.addLayoutItem(map2)

        map3 = QgsLayoutItemMap(layout)
        map3.attemptSetSceneRect(QRectF(150, 160, 50, 50))
        map3.setFrameEnabled(True)
        map3.setBackgroundEnabled(False)
        map3.setId('map3')
        layout.addLayoutItem(map3)

        map.addLabelBlockingItem(map2)
        map.addLabelBlockingItem(map3)
        map.setMapFlags(QgsLayoutItemMap.MapItemFlags())
        checker = QgsLayoutChecker('composermap_label_blockers', layout)
        checker.setControlPathPrefix("composer_map")
        result, message = checker.testLayout()
        TestQgsLayoutMap.report += checker.report()
        self.assertTrue(result, message)

        doc = QDomDocument("testdoc")
        elem = layout.writeXml(doc, QgsReadWriteContext())

        l2 = QgsLayout(p)
        self.assertTrue(l2.readXml(elem, doc, QgsReadWriteContext()))
        map_restore = [i for i in l2.items() if isinstance(i, QgsLayoutItemMap) and i.id() == 'map'][0]
        map2_restore = [i for i in l2.items() if isinstance(i, QgsLayoutItemMap) and i.id() == 'map2'][0]
        map3_restore = [i for i in l2.items() if isinstance(i, QgsLayoutItemMap) and i.id() == 'map3'][0]
        self.assertTrue(map_restore.isLabelBlockingItem(map2_restore))
        self.assertTrue(map_restore.isLabelBlockingItem(map3_restore))

    def testTheme(self):
        layout = QgsLayout(QgsProject.instance())
        map = QgsLayoutItemMap(layout)
        self.assertFalse(map.followVisibilityPreset())
        self.assertFalse(map.followVisibilityPresetName())

        spy = QSignalSpy(map.themeChanged)

        map.setFollowVisibilityPresetName('theme')
        self.assertFalse(map.followVisibilityPreset())
        self.assertEqual(map.followVisibilityPresetName(), 'theme')
        # should not be emitted - followVisibilityPreset is False
        self.assertEqual(len(spy), 0)
        map.setFollowVisibilityPresetName('theme2')
        self.assertEqual(map.followVisibilityPresetName(), 'theme2')
        self.assertEqual(len(spy), 0)

        map.setFollowVisibilityPresetName('')
        map.setFollowVisibilityPreset(True)
        # should not be emitted - followVisibilityPresetName is empty
        self.assertEqual(len(spy), 0)
        self.assertFalse(map.followVisibilityPresetName())
        self.assertTrue(map.followVisibilityPreset())

        map.setFollowVisibilityPresetName('theme')
        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[-1][0], 'theme')
        map.setFollowVisibilityPresetName('theme')
        self.assertEqual(len(spy), 1)
        map.setFollowVisibilityPresetName('theme2')
        self.assertEqual(len(spy), 2)
        self.assertEqual(spy[-1][0], 'theme2')
        map.setFollowVisibilityPreset(False)
        self.assertEqual(len(spy), 3)
        self.assertFalse(spy[-1][0])
        map.setFollowVisibilityPreset(False)
        self.assertEqual(len(spy), 3)
        map.setFollowVisibilityPresetName('theme3')
        self.assertEqual(len(spy), 3)
        map.setFollowVisibilityPreset(True)
        self.assertEqual(len(spy), 4)
        self.assertEqual(spy[-1][0], 'theme3')
        map.setFollowVisibilityPreset(True)
        self.assertEqual(len(spy), 4)

        # data defined theme
        map.dataDefinedProperties().setProperty(QgsLayoutObject.MapStylePreset, QgsProperty.fromValue('theme4'))
        map.refresh()
        self.assertEqual(len(spy), 5)
        self.assertEqual(spy[-1][0], 'theme4')
        map.refresh()
        self.assertEqual(len(spy), 5)
        map.dataDefinedProperties().setProperty(QgsLayoutObject.MapStylePreset, QgsProperty.fromValue('theme6'))
        map.refresh()
        self.assertEqual(len(spy), 6)
        self.assertEqual(spy[-1][0], 'theme6')

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
        settings.placement = QgsPalLayerSettings.OverPoint

        vl = QgsVectorLayer("Polygon?crs=epsg:4326&field=id:integer", "vl", "memory")

        props = {"color": "127,255,127", 'outline_style': 'solid', 'outline_width': '1', 'outline_color': '0,0,255'}
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
        p.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(10, 10, 180, 180))
        map.setFrameEnabled(False)
        map.zoomToExtent(vl.extent())
        map.setLayers([vl])
        layout.addLayoutItem(map)

        shape = QgsLayoutItemShape(layout)
        layout.addLayoutItem(shape)
        shape.setShapeType(QgsLayoutItemShape.Ellipse)
        shape.attemptSetSceneRect(QRectF(10, 10, 180, 180))
        props = {"color": "0,0,0,0", 'outline_style': 'no'}
        fillSymbol = QgsFillSymbol.createSimple(props)
        shape.setSymbol(fillSymbol)

        map.itemClippingSettings().setEnabled(True)
        map.itemClippingSettings().setSourceItem(shape)
        map.itemClippingSettings().setForceLabelsInsideClipPath(False)
        map.itemClippingSettings().setFeatureClippingType(QgsMapClippingRegion.FeatureClippingType.ClipToIntersection)

        checker = QgsLayoutChecker('composermap_itemclip', layout)
        checker.setControlPathPrefix("composer_map")
        result, message = checker.testLayout()
        TestQgsLayoutMap.report += checker.report()
        self.assertTrue(result, message)

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
        settings.placement = QgsPalLayerSettings.OverPoint

        vl = QgsVectorLayer("Polygon?crs=epsg:4326&field=id:integer", "vl", "memory")

        props = {"color": "127,255,127", 'outline_style': 'solid', 'outline_width': '1', 'outline_color': '0,0,255'}
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
        p.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(10, 10, 180, 180))
        map.setFrameEnabled(False)
        map.zoomToExtent(vl.extent())
        map.setLayers([vl])
        layout.addLayoutItem(map)

        shape = QgsLayoutItemShape(layout)
        layout.addLayoutItem(shape)
        shape.setShapeType(QgsLayoutItemShape.Ellipse)
        shape.attemptSetSceneRect(QRectF(10, 10, 180, 180))
        props = {"color": "0,0,0,0", 'outline_style': 'no'}
        fillSymbol = QgsFillSymbol.createSimple(props)
        shape.setSymbol(fillSymbol)

        map.itemClippingSettings().setEnabled(True)
        map.itemClippingSettings().setSourceItem(shape)
        map.itemClippingSettings().setForceLabelsInsideClipPath(True)
        map.itemClippingSettings().setFeatureClippingType(QgsMapClippingRegion.FeatureClippingType.ClipPainterOnly)

        checker = QgsLayoutChecker('composermap_itemclip_force_labels_inside', layout)
        checker.setControlPathPrefix("composer_map")
        result, message = checker.testLayout()
        TestQgsLayoutMap.report += checker.report()
        self.assertTrue(result, message)

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
        settings.placement = QgsPalLayerSettings.OverPoint

        vl = QgsVectorLayer("Polygon?crs=epsg:4326&field=id:integer", "vl", "memory")

        props = {"color": "127,255,127", 'outline_style': 'solid', 'outline_width': '1', 'outline_color': '0,0,255'}
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
        p.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
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
        props = {"color": "0,0,0,0", 'outline_style': 'solid', 'outline_width': '1', 'outline_color': '0,0,255'}
        fillSymbol = QgsFillSymbol.createSimple(props)
        overview.setFrameSymbol(fillSymbol)

        map2.overviews().addOverview(overview)

        shape = QgsLayoutItemShape(layout)
        layout.addLayoutItem(shape)
        shape.setShapeType(QgsLayoutItemShape.Ellipse)
        shape.attemptSetSceneRect(QRectF(10, 10, 180, 180))
        props = {"color": "0,0,0,0", 'outline_style': 'no'}
        fillSymbol = QgsFillSymbol.createSimple(props)
        shape.setSymbol(fillSymbol)

        map.itemClippingSettings().setEnabled(True)
        map.itemClippingSettings().setSourceItem(shape)
        map.itemClippingSettings().setForceLabelsInsideClipPath(False)
        map.itemClippingSettings().setFeatureClippingType(QgsMapClippingRegion.FeatureClippingType.ClipToIntersection)

        checker = QgsLayoutChecker('composermap_itemclip_overview', layout)
        checker.setControlPathPrefix("composer_map")
        result, message = checker.testLayout()
        TestQgsLayoutMap.report += checker.report()
        self.assertTrue(result, message)

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
        settings.placement = QgsPalLayerSettings.OverPoint

        vl = QgsVectorLayer("Polygon?crs=epsg:4326&field=id:integer", "vl", "memory")

        props = {"color": "127,255,127", 'outline_style': 'solid', 'outline_width': '1', 'outline_color': '0,0,255'}
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
        p.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(10, 10, 180, 180))
        map.setFrameEnabled(False)
        map.zoomToExtent(vl.extent())
        map.setLayers([vl])
        layout.addLayoutItem(map)

        shape = QgsLayoutItemShape(layout)
        layout.addLayoutItem(shape)
        shape.setShapeType(QgsLayoutItemShape.Ellipse)
        shape.attemptSetSceneRect(QRectF(10, 10, 180, 180))

        map.itemClippingSettings().setEnabled(True)
        map.itemClippingSettings().setSourceItem(shape)
        map.itemClippingSettings().setForceLabelsInsideClipPath(False)
        map.itemClippingSettings().setFeatureClippingType(QgsMapClippingRegion.FeatureClippingType.ClipToIntersection)

        checker = QgsLayoutChecker('composermap_itemclip_nodrawsource', layout)
        checker.setControlPathPrefix("composer_map")
        result, message = checker.testLayout()
        TestQgsLayoutMap.report += checker.report()
        self.assertTrue(result, message)

    def testClippingBackgroundFrame(self):
        """
        Make sure frame/background are also clipped
        """
        vl = QgsVectorLayer("Polygon?crs=epsg:4326&field=id:integer", "vl", "memory")

        props = {"color": "127,255,127", 'outline_style': 'solid', 'outline_width': '1', 'outline_color': '0,0,255'}
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
        p.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(10, 10, 180, 180))
        map.setFrameEnabled(True)
        map.setFrameStrokeWidth(QgsLayoutMeasurement(2, QgsUnitTypes.LayoutMillimeters))
        map.setBackgroundEnabled(True)
        map.setBackgroundColor(QColor(200, 255, 200))
        map.zoomToExtent(vl.extent())
        map.setLayers([vl])
        layout.addLayoutItem(map)

        shape = QgsLayoutItemShape(layout)
        layout.addLayoutItem(shape)
        shape.setShapeType(QgsLayoutItemShape.Ellipse)
        shape.attemptSetSceneRect(QRectF(10, 10, 180, 180))

        map.itemClippingSettings().setEnabled(True)
        map.itemClippingSettings().setSourceItem(shape)
        map.itemClippingSettings().setFeatureClippingType(QgsMapClippingRegion.FeatureClippingType.ClipPainterOnly)

        checker = QgsLayoutChecker('composermap_itemclip_background', layout)
        checker.setControlPathPrefix("composer_map")
        result, message = checker.testLayout()
        TestQgsLayoutMap.report += checker.report()
        self.assertTrue(result, message)

    def testMainAnnotationLayer(self):
        """
        Make sure main annotation layer is rendered in maps above all other layers
        """
        p = QgsProject()

        vl = QgsVectorLayer("Polygon?crs=epsg:4326&field=fldtxt:string",
                            "layer", "memory")
        sym3 = QgsFillSymbol.createSimple({'color': '#b200b2'})
        vl.renderer().setSymbol(sym3)

        p.addMapLayer(vl)
        layout = QgsLayout(p)
        layout.initializeDefaults()
        p.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(10, 10, 180, 180))
        map.setFrameEnabled(True)
        map.setFrameStrokeWidth(QgsLayoutMeasurement(2, QgsUnitTypes.LayoutMillimeters))
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
        checker = QgsLayoutChecker('composermap_annotation_empty', layout)
        checker.setControlPathPrefix("composer_map")
        result, message = checker.testLayout()
        TestQgsLayoutMap.report += checker.report()
        self.assertTrue(result, message)

        annotation_layer = p.mainAnnotationLayer()
        annotation_layer.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        annotation_geom = QgsGeometry.fromRect(QgsRectangle(12, 30, 18, 33))
        annotation = QgsAnnotationPolygonItem(annotation_geom.constGet().clone())
        sym3 = QgsFillSymbol.createSimple({'color': '#ff0000', 'outline_style': 'no'})
        annotation.setSymbol(sym3)
        annotation_layer.addItem(annotation)

        # annotation must be drawn above map layers
        checker = QgsLayoutChecker('composermap_annotation_item', layout)
        checker.setControlPathPrefix("composer_map")
        result, message = checker.testLayout()
        TestQgsLayoutMap.report += checker.report()
        self.assertTrue(result, message)

    def testCrsChanged(self):
        """
        Test that the CRS changed signal is emitted in the right circumstances
        """
        p = QgsProject()
        layout = QgsLayout(p)
        p.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        map = QgsLayoutItemMap(layout)

        spy = QSignalSpy(map.crsChanged)
        # map has no explicit crs set, so follows project crs => signal should be emitted
        # when project crs is changed
        p.setCrs(QgsCoordinateReferenceSystem('EPSG:3111'))
        self.assertEqual(len(spy), 1)
        p.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        self.assertEqual(len(spy), 2)
        # set explicit crs on map item
        map.setCrs(QgsCoordinateReferenceSystem('EPSG:28356'))
        self.assertEqual(len(spy), 3)
        map.setCrs(QgsCoordinateReferenceSystem('EPSG:28356'))
        self.assertEqual(len(spy), 3)
        map.setCrs(QgsCoordinateReferenceSystem('EPSG:28355'))
        self.assertEqual(len(spy), 4)
        # should not care about project crs changes anymore..
        p.setCrs(QgsCoordinateReferenceSystem('EPSG:3111'))
        self.assertEqual(len(spy), 4)
        # set back to project crs
        map.setCrs(QgsCoordinateReferenceSystem())
        self.assertEqual(len(spy), 5)

        map.setCrs(QgsCoordinateReferenceSystem('EPSG:28355'))
        self.assertEqual(len(spy), 6)
        # data defined crs
        map.dataDefinedProperties().setProperty(QgsLayoutObject.MapCrs, QgsProperty.fromValue('EPSG:4283'))
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


if __name__ == '__main__':
    unittest.main()
