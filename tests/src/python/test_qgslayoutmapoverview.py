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
from typing import Optional

from qgis.PyQt.QtCore import QDir, QFileInfo, QRectF
from qgis.PyQt.QtGui import QPainter
from qgis.core import (
    QgsCoordinateReferenceSystem,
    QgsFeature,
    QgsFillSymbol,
    QgsGeometry,
    QgsLayout,
    QgsLayoutItemMap,
    QgsLayoutItemMapItem,
    QgsLayoutItemMapOverview,
    QgsMultiBandColorRenderer,
    QgsPointXY,
    QgsProject,
    QgsProperty,
    QgsRasterLayer,
    QgsRectangle,
    QgsSingleSymbolRenderer,
    QgsSymbolLayer,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from test_qgslayoutitem import LayoutItemTestCase
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayoutMap(QgisTestCase, LayoutItemTestCase):

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.item_class = QgsLayoutItemMap

    @classmethod
    def control_path_prefix(cls):
        return "composer_mapoverview"

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
        QgsProject.instance().addMapLayers([self.raster_layer, self.vector_layer])

        # create layout with layout map
        self.layout = QgsLayout(QgsProject.instance())
        self.layout.initializeDefaults()
        self.map = QgsLayoutItemMap(self.layout)
        self.map.attemptSetSceneRect(QRectF(20, 20, 200, 100))
        self.map.setFrameEnabled(True)
        self.map.setLayers([self.raster_layer])
        self.layout.addLayoutItem(self.map)

    def testOverviewMap(self):
        overviewMap = QgsLayoutItemMap(self.layout)
        overviewMap.attemptSetSceneRect(QRectF(20, 130, 70, 70))
        overviewMap.setFrameEnabled(True)
        overviewMap.setLayers([self.raster_layer])
        self.assertFalse(overviewMap.overviews().hasEnabledItems())
        self.layout.addLayoutItem(overviewMap)
        # zoom in
        myRectangle = QgsRectangle(96, -152, 160, -120)
        self.map.setExtent(myRectangle)
        myRectangle2 = QgsRectangle(0, -256, 256, 0)
        overviewMap.setExtent(myRectangle2)
        overviewMap.overview().setLinkedMap(self.map)
        self.assertTrue(overviewMap.overviews().hasEnabledItems())

        result = self.render_layout_check(
            "composermap_overview", self.layout, color_tolerance=6
        )

        self.layout.removeLayoutItem(overviewMap)
        self.assertTrue(result)

    def testOverviewMapBlend(self):
        overviewMap = QgsLayoutItemMap(self.layout)
        overviewMap.attemptSetSceneRect(QRectF(20, 130, 70, 70))
        overviewMap.setFrameEnabled(True)
        overviewMap.setLayers([self.raster_layer])
        self.layout.addLayoutItem(overviewMap)
        # zoom in
        myRectangle = QgsRectangle(96, -152, 160, -120)
        self.map.setExtent(myRectangle)
        myRectangle2 = QgsRectangle(0, -256, 256, 0)
        overviewMap.setExtent(myRectangle2)
        overviewMap.overview().setLinkedMap(self.map)
        overviewMap.overview().setBlendMode(
            QPainter.CompositionMode.CompositionMode_Multiply
        )

        result = self.render_layout_check("composermap_overview_blending", self.layout)

        self.layout.removeLayoutItem(overviewMap)
        self.assertTrue(result)

    def testOverviewMapInvert(self):
        overviewMap = QgsLayoutItemMap(self.layout)
        overviewMap.attemptSetSceneRect(QRectF(20, 130, 70, 70))
        overviewMap.setFrameEnabled(True)
        overviewMap.setLayers([self.raster_layer])
        self.layout.addLayoutItem(overviewMap)
        # zoom in
        myRectangle = QgsRectangle(96, -152, 160, -120)
        self.map.setExtent(myRectangle)
        myRectangle2 = QgsRectangle(0, -256, 256, 0)
        overviewMap.setExtent(myRectangle2)
        overviewMap.overview().setLinkedMap(self.map)
        overviewMap.overview().setInverted(True)

        result = self.render_layout_check("composermap_overview_invert", self.layout)

        self.layout.removeLayoutItem(overviewMap)
        self.assertTrue(result)

    def testOverviewMapCenter(self):
        overviewMap = QgsLayoutItemMap(self.layout)
        overviewMap.attemptSetSceneRect(QRectF(20, 130, 70, 70))
        overviewMap.setFrameEnabled(True)
        overviewMap.setLayers([self.raster_layer])
        self.layout.addLayoutItem(overviewMap)
        # zoom in
        myRectangle = QgsRectangle(192, -288, 320, -224)
        self.map.setExtent(myRectangle)
        myRectangle2 = QgsRectangle(0, -256, 256, 0)
        overviewMap.setExtent(myRectangle2)
        overviewMap.overview().setLinkedMap(self.map)
        overviewMap.overview().setInverted(False)
        overviewMap.overview().setCentered(True)

        result = self.render_layout_check("composermap_overview_center", self.layout)

        self.layout.removeLayoutItem(overviewMap)
        self.assertTrue(result)

    def testAsMapLayer(self):
        l = QgsLayout(QgsProject.instance())
        l.initializeDefaults()
        map = QgsLayoutItemMap(l)
        map.attemptSetSceneRect(QRectF(20, 20, 200, 100))
        l.addLayoutItem(map)

        overviewMap = QgsLayoutItemMap(l)
        overviewMap.attemptSetSceneRect(QRectF(20, 130, 70, 70))
        l.addLayoutItem(overviewMap)
        # zoom in
        myRectangle = QgsRectangle(96, -152, 160, -120)
        map.setExtent(myRectangle)
        myRectangle2 = QgsRectangle(0, -256, 256, 0)
        overviewMap.setExtent(myRectangle2)
        overviewMap.overview().setLinkedMap(map)

        layer = overviewMap.overview().asMapLayer()
        self.assertIsNotNone(layer)
        self.assertTrue(layer.isValid())
        geoms = [f.geometry() for f in layer.getFeatures()]
        for g in geoms:
            g.normalize()

        self.assertEqual(
            [g.asWkt() for g in geoms],
            ["Polygon ((96 -152, 96 -120, 160 -120, 160 -152, 96 -152))"],
        )

        # check that layer has correct renderer
        fill_symbol = QgsFillSymbol.createSimple(
            {"color": "#00ff00", "outline_color": "#ff0000", "outline_width": "10"}
        )
        overviewMap.overview().setFrameSymbol(fill_symbol)
        layer = overviewMap.overview().asMapLayer()
        self.assertIsInstance(layer.renderer(), QgsSingleSymbolRenderer)
        self.assertEqual(
            layer.renderer().symbol().symbolLayer(0).properties()["color"],
            "0,255,0,255,rgb:0,1,0,1",
        )
        self.assertEqual(
            layer.renderer().symbol().symbolLayer(0).properties()["outline_color"],
            "255,0,0,255,rgb:1,0,0,1",
        )

        # test layer blend mode
        self.assertEqual(
            layer.blendMode(), QPainter.CompositionMode.CompositionMode_SourceOver
        )
        overviewMap.overview().setBlendMode(
            QPainter.CompositionMode.CompositionMode_Clear
        )
        layer = overviewMap.overview().asMapLayer()
        self.assertEqual(
            layer.blendMode(), QPainter.CompositionMode.CompositionMode_Clear
        )

        # should have no effect
        overviewMap.setMapRotation(45)
        layer = overviewMap.overview().asMapLayer()
        geoms = [f.geometry() for f in layer.getFeatures()]
        for g in geoms:
            g.normalize()
        self.assertEqual(
            [g.asWkt() for g in geoms],
            ["Polygon ((96 -152, 96 -120, 160 -120, 160 -152, 96 -152))"],
        )

        map.setMapRotation(15)
        layer = overviewMap.overview().asMapLayer()
        geoms = [f.geometry() for f in layer.getFeatures()]
        for g in geoms:
            g.normalize()
        self.assertEqual(
            [g.asWkt(0) for g in geoms],
            ["Polygon ((93 -129, 155 -112, 163 -143, 101 -160, 93 -129))"],
        )

        # with reprojection
        map.setCrs(QgsCoordinateReferenceSystem("EPSG:3875"))
        layer = overviewMap.overview().asMapLayer()
        geoms = [f.geometry() for f in layer.getFeatures()]
        for g in geoms:
            g.normalize()
        self.assertEqual(
            [g.asWkt(0) for g in geoms],
            [
                "Polygon ((93 -129, 96 -128, 99 -127, 102 -126, 105 -126, 108 -125, 111 -124, 114 -123, 116 -123, 119 -122, 122 -121, 125 -120, 128 -119, 131 -119, 134 -118, 137 -117, 140 -116, 143 -115, 146 -115, 149 -114, 152 -113, 155 -112, 155 -114, 156 -115, 156 -117, 156 -118, 157 -120, 157 -121, 158 -123, 158 -124, 158 -126, 159 -127, 159 -128, 160 -130, 160 -131, 160 -133, 161 -134, 161 -136, 161 -137, 162 -139, 162 -140, 163 -142, 163 -143, 160 -144, 157 -145, 154 -146, 151 -146, 148 -147, 145 -148, 142 -149, 140 -149, 137 -150, 134 -151, 131 -152, 128 -153, 125 -153, 122 -154, 119 -155, 116 -156, 113 -157, 110 -157, 107 -158, 104 -159, 101 -160, 101 -158, 100 -157, 100 -155, 100 -154, 99 -152, 99 -151, 98 -149, 98 -148, 98 -146, 97 -145, 97 -144, 96 -142, 96 -141, 96 -139, 95 -138, 95 -136, 95 -135, 94 -133, 94 -132, 93 -130, 93 -129))"
            ],
        )

        map.setCrs(overviewMap.crs())
        # with invert
        overviewMap.overview().setInverted(True)
        layer = overviewMap.overview().asMapLayer()
        geoms = [f.geometry() for f in layer.getFeatures()]
        for g in geoms:
            g.normalize()
        self.assertEqual(
            [g.asWkt(0) for g in geoms],
            [
                "Polygon ((-53 -128, 128 53, 309 -128, 128 -309, -53 -128),(93 -129, 101 -160, 163 -143, 155 -112, 93 -129))"
            ],
        )

    def test_StackingPosition(self):
        l = QgsLayout(QgsProject.instance())
        l.initializeDefaults()

        overviewMap = QgsLayoutItemMap(l)
        overviewMap.attemptSetSceneRect(QRectF(20, 130, 70, 70))
        l.addLayoutItem(overviewMap)
        overviewMap.overview().setStackingPosition(
            QgsLayoutItemMapItem.StackingPosition.StackBelowMap
        )
        self.assertEqual(
            overviewMap.overview().stackingPosition(),
            QgsLayoutItemMapItem.StackingPosition.StackBelowMap,
        )
        overviewMap.overview().setStackingPosition(
            QgsLayoutItemMapItem.StackingPosition.StackBelowMapLayer
        )
        self.assertEqual(
            overviewMap.overview().stackingPosition(),
            QgsLayoutItemMapItem.StackingPosition.StackBelowMapLayer,
        )

        overviewMap.overview().setStackingLayer(self.raster_layer)
        self.assertEqual(overviewMap.overview().stackingLayer(), self.raster_layer)
        overviewMap.overview().setStackingLayer(self.vector_layer)
        self.assertEqual(overviewMap.overview().stackingLayer(), self.vector_layer)
        overviewMap.overview().setStackingLayer(None)
        self.assertIsNone(overviewMap.overview().stackingLayer())

    def test_ModifyMapLayerList(self):
        l = QgsLayout(QgsProject.instance())
        l.initializeDefaults()

        overviewMap = QgsLayoutItemMap(l)
        overviewMap.attemptSetSceneRect(QRectF(20, 130, 70, 70))
        l.addLayoutItem(overviewMap)
        map = QgsLayoutItemMap(l)
        map.attemptSetSceneRect(QRectF(20, 20, 200, 100))
        l.addLayoutItem(map)

        self.assertFalse(overviewMap.overviews().modifyMapLayerList([]))
        self.assertEqual(
            overviewMap.overviews().modifyMapLayerList(
                [self.raster_layer, self.vector_layer]
            ),
            [self.raster_layer, self.vector_layer],
        )
        overviewMap.overview().setLinkedMap(map)
        overviewMap.overview().setStackingPosition(
            QgsLayoutItemMapItem.StackingPosition.StackBelowMap
        )
        self.assertEqual(
            overviewMap.overviews().modifyMapLayerList(
                [self.raster_layer, self.vector_layer]
            ),
            [self.raster_layer, self.vector_layer, overviewMap.overview().asMapLayer()],
        )
        overviewMap.overview().setStackingPosition(
            QgsLayoutItemMapItem.StackingPosition.StackBelowMapLayer
        )
        self.assertEqual(
            overviewMap.overviews().modifyMapLayerList(
                [self.raster_layer, self.vector_layer]
            ),
            [self.raster_layer, self.vector_layer],
        )
        overviewMap.overview().setStackingLayer(self.raster_layer)
        self.assertEqual(
            overviewMap.overviews().modifyMapLayerList(
                [self.raster_layer, self.vector_layer]
            ),
            [self.raster_layer, overviewMap.overview().asMapLayer(), self.vector_layer],
        )
        overviewMap.overview().setStackingLayer(self.vector_layer)
        self.assertEqual(
            overviewMap.overviews().modifyMapLayerList(
                [self.raster_layer, self.vector_layer]
            ),
            [self.raster_layer, self.vector_layer, overviewMap.overview().asMapLayer()],
        )
        overviewMap.overview().setStackingPosition(
            QgsLayoutItemMapItem.StackingPosition.StackAboveMapLayer
        )
        overviewMap.overview().setStackingLayer(None)
        self.assertEqual(
            overviewMap.overviews().modifyMapLayerList(
                [self.raster_layer, self.vector_layer]
            ),
            [self.raster_layer, self.vector_layer],
        )
        overviewMap.overview().setStackingLayer(self.raster_layer)
        self.assertEqual(
            overviewMap.overviews().modifyMapLayerList(
                [self.raster_layer, self.vector_layer]
            ),
            [overviewMap.overview().asMapLayer(), self.raster_layer, self.vector_layer],
        )
        overviewMap.overview().setStackingLayer(self.vector_layer)
        self.assertEqual(
            overviewMap.overviews().modifyMapLayerList(
                [self.raster_layer, self.vector_layer]
            ),
            [self.raster_layer, overviewMap.overview().asMapLayer(), self.vector_layer],
        )
        overviewMap.overview().setStackingPosition(
            QgsLayoutItemMapItem.StackingPosition.StackBelowMapLabels
        )
        self.assertEqual(
            overviewMap.overviews().modifyMapLayerList(
                [self.raster_layer, self.vector_layer]
            ),
            [overviewMap.overview().asMapLayer(), self.raster_layer, self.vector_layer],
        )
        overviewMap.overview().setStackingPosition(
            QgsLayoutItemMapItem.StackingPosition.StackAboveMapLabels
        )
        self.assertEqual(
            overviewMap.overviews().modifyMapLayerList(
                [self.raster_layer, self.vector_layer]
            ),
            [self.raster_layer, self.vector_layer],
        )

        # two overviews
        overviewMap.overview().setStackingPosition(
            QgsLayoutItemMapItem.StackingPosition.StackBelowMap
        )
        overviewMap.overviews().addOverview(QgsLayoutItemMapOverview("x", overviewMap))
        overviewMap.overviews().overview(1).setLinkedMap(map)
        overviewMap.overviews().overview(1).setStackingPosition(
            QgsLayoutItemMapItem.StackingPosition.StackBelowMapLabels
        )
        self.assertEqual(
            overviewMap.overviews().modifyMapLayerList(
                [self.raster_layer, self.vector_layer]
            ),
            [
                overviewMap.overviews().overview(1).asMapLayer(),
                self.raster_layer,
                self.vector_layer,
                overviewMap.overview().asMapLayer(),
            ],
        )

    def testOverviewStacking(self):
        l = QgsLayout(QgsProject.instance())
        l.initializeDefaults()
        map = QgsLayoutItemMap(l)
        map.attemptSetSceneRect(QRectF(20, 20, 200, 100))
        map.setFrameEnabled(True)
        map.setLayers([self.raster_layer])
        l.addLayoutItem(map)

        overviewMap = QgsLayoutItemMap(l)
        overviewMap.attemptSetSceneRect(QRectF(20, 130, 70, 70))
        l.addLayoutItem(overviewMap)
        overviewMap.setFrameEnabled(True)
        overviewMap.setLayers([self.raster_layer])
        # zoom in
        myRectangle = QgsRectangle(96, -152, 160, -120)
        map.setExtent(myRectangle)
        myRectangle2 = QgsRectangle(-20, -276, 276, 20)
        overviewMap.setExtent(myRectangle2)
        overviewMap.overview().setLinkedMap(map)
        overviewMap.overview().setInverted(True)
        overviewMap.overview().setStackingPosition(
            QgsLayoutItemMapItem.StackingPosition.StackBelowMapLayer
        )
        overviewMap.overview().setStackingLayer(self.raster_layer)

        self.assertTrue(
            self.render_layout_check(
                "composermap_overview_belowmap", l, color_tolerance=6
            )
        )

        overviewMap.overview().setStackingPosition(
            QgsLayoutItemMapItem.StackingPosition.StackAboveMapLayer
        )
        overviewMap.overview().setStackingLayer(self.raster_layer)

        self.assertTrue(
            self.render_layout_check(
                "composermap_overview_abovemap", l, color_tolerance=6
            )
        )

    def testOverviewExpressionContextStacking(self):
        atlas_layer = QgsVectorLayer(
            "Point?crs=epsg:4326&field=attr:int(1)&field=label:string(20)",
            "points",
            "memory",
        )

        atlas_feature1 = QgsFeature(atlas_layer.fields())
        atlas_feature1.setAttributes([5, "a"])
        atlas_feature1.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(55, 55)))
        atlas_layer.dataProvider().addFeature(atlas_feature1)
        atlas_feature2 = QgsFeature(atlas_layer.fields())
        atlas_feature2.setAttributes([15, "b"])
        atlas_feature2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(55, 55)))
        atlas_layer.dataProvider().addFeature(atlas_feature2)

        l = QgsLayout(QgsProject.instance())
        l.initializeDefaults()
        map = QgsLayoutItemMap(l)
        map.attemptSetSceneRect(QRectF(20, 20, 200, 100))
        map.setFrameEnabled(True)
        map.setLayers([atlas_layer])
        l.addLayoutItem(map)

        overviewMap = QgsLayoutItemMap(l)
        overviewMap.attemptSetSceneRect(QRectF(20, 130, 70, 70))
        l.addLayoutItem(overviewMap)
        overviewMap.setFrameEnabled(True)
        overviewMap.setLayers([atlas_layer])
        # zoom in
        myRectangle = QgsRectangle(96, -152, 160, -120)
        map.setExtent(myRectangle)
        myRectangle2 = QgsRectangle(-20, -276, 276, 20)
        overviewMap.setExtent(myRectangle2)
        overviewMap.overview().setLinkedMap(map)
        overviewMap.overview().setStackingPosition(
            QgsLayoutItemMapItem.StackingPosition.StackAboveMapLayer
        )
        overviewMap.overview().setStackingLayer(atlas_layer)

        fill_symbol = QgsFillSymbol.createSimple(
            {"color": "#0000ff", "outline_style": "no"}
        )
        fill_symbol[0].setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyFillColor,
            QgsProperty.fromExpression(
                "case when label='a' then 'red' else 'green' end"
            ),
        )

        overviewMap.overview().setFrameSymbol(fill_symbol)

        l.reportContext().setLayer(atlas_layer)
        l.reportContext().setFeature(atlas_feature1)

        self.assertTrue(
            self.render_layout_check(
                "composermap_overview_atlas_1", l, color_tolerance=6
            )
        )

        l.reportContext().setFeature(atlas_feature2)

        self.assertTrue(
            self.render_layout_check(
                "composermap_overview_atlas_2", l, color_tolerance=6
            )
        )


if __name__ == "__main__":
    unittest.main()
