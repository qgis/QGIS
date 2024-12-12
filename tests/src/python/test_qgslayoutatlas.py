"""QGIS Unit tests for QgsLayoutAtlas

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "19/12/2017"
__copyright__ = "Copyright 2017, The QGIS Project"

import glob
import os
import shutil
import tempfile

from qgis.PyQt.QtCore import QFileInfo, QRectF
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    QgsCategorizedSymbolRenderer,
    QgsCoordinateReferenceSystem,
    QgsFeature,
    QgsFillSymbol,
    QgsFontUtils,
    QgsGeometry,
    QgsLayoutItemLabel,
    QgsLayoutItemLegend,
    QgsLayoutItemMap,
    QgsLayoutObject,
    QgsLayoutPoint,
    QgsLegendStyle,
    QgsMarkerSymbol,
    QgsPointXY,
    QgsPrintLayout,
    QgsProject,
    QgsProperty,
    QgsReadWriteContext,
    QgsRectangle,
    QgsRendererCategory,
    QgsSingleSymbolRenderer,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase


from utilities import unitTestDataPath

start_app()


class TestQgsLayoutAtlas(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "atlas"

    def testCase(self):
        self.TEST_DATA_DIR = unitTestDataPath()
        tmppath = tempfile.mkdtemp()
        for file in glob.glob(os.path.join(self.TEST_DATA_DIR, "france_parts.*")):
            shutil.copy(os.path.join(self.TEST_DATA_DIR, file), tmppath)
        vectorFileInfo = QFileInfo(tmppath + "/france_parts.shp")
        mVectorLayer = QgsVectorLayer(
            vectorFileInfo.filePath(), vectorFileInfo.completeBaseName(), "ogr"
        )

        QgsProject.instance().addMapLayers([mVectorLayer])
        self.layers = [mVectorLayer]

        # create layout with layout map

        # select epsg:2154
        crs = QgsCoordinateReferenceSystem("epsg:2154")
        QgsProject.instance().setCrs(crs)

        self.layout = QgsPrintLayout(QgsProject.instance())
        self.layout.initializeDefaults()

        # fix the renderer, fill with green
        props = {"color": "0,127,0", "outline_color": "black"}
        fillSymbol = QgsFillSymbol.createSimple(props)
        renderer = QgsSingleSymbolRenderer(fillSymbol)
        mVectorLayer.setRenderer(renderer)

        # the atlas map
        self.atlas_map = QgsLayoutItemMap(self.layout)
        self.atlas_map.attemptSetSceneRect(QRectF(20, 20, 130, 130))
        self.atlas_map.setFrameEnabled(True)
        self.atlas_map.setLayers([mVectorLayer])
        self.layout.addLayoutItem(self.atlas_map)

        # the atlas
        self.atlas = self.layout.atlas()
        self.atlas.setCoverageLayer(mVectorLayer)
        self.atlas.setEnabled(True)

        # an overview
        self.overview = QgsLayoutItemMap(self.layout)
        self.overview.attemptSetSceneRect(QRectF(180, 20, 50, 50))
        self.overview.setFrameEnabled(True)
        self.overview.overview().setLinkedMap(self.atlas_map)
        self.overview.setLayers([mVectorLayer])
        self.layout.addLayoutItem(self.overview)
        nextent = QgsRectangle(49670.718, 6415139.086, 699672.519, 7065140.887)
        self.overview.setExtent(nextent)

        # set the fill symbol of the overview map
        props2 = {"color": "127,0,0,127", "outline_color": "black"}
        fillSymbol2 = QgsFillSymbol.createSimple(props2)
        self.overview.overview().setFrameSymbol(fillSymbol2)

        # header label
        self.mLabel1 = QgsLayoutItemLabel(self.layout)
        self.layout.addLayoutItem(self.mLabel1)
        self.mLabel1.setText('[% "NAME_1" %] area')
        self.mLabel1.setFont(QgsFontUtils.getStandardTestFont())
        self.mLabel1.adjustSizeToText()
        self.mLabel1.attemptSetSceneRect(QRectF(150, 5, 60, 15))
        self.mLabel1.setMarginX(1)
        self.mLabel1.setMarginY(1)

        # feature number label
        self.mLabel2 = QgsLayoutItemLabel(self.layout)
        self.layout.addLayoutItem(self.mLabel2)
        self.mLabel2.setText(
            "# [%@atlas_featurenumber || ' / ' || @atlas_totalfeatures%]"
        )
        self.mLabel2.setFont(QgsFontUtils.getStandardTestFont())
        self.mLabel2.adjustSizeToText()
        self.mLabel2.attemptSetSceneRect(QRectF(150, 200, 60, 15))
        self.mLabel2.setMarginX(1)
        self.mLabel2.setMarginY(1)

        self.filename_test()
        self.autoscale_render_test()
        self.fixedscale_render_test()
        self.predefinedscales_render_test()
        self.hidden_render_test()
        self.legend_test()
        self.rotation_test()

        shutil.rmtree(tmppath, True)

    def testReadWriteXml(self):
        p = QgsProject()
        vectorFileInfo = QFileInfo(unitTestDataPath() + "/france_parts.shp")
        vector_layer = QgsVectorLayer(
            vectorFileInfo.filePath(), vectorFileInfo.completeBaseName(), "ogr"
        )
        self.assertTrue(vector_layer.isValid())
        p.addMapLayer(vector_layer)

        l = QgsPrintLayout(p)
        atlas = l.atlas()
        atlas.setEnabled(True)
        atlas.setHideCoverage(True)
        atlas.setFilenameExpression("filename exp")
        atlas.setCoverageLayer(vector_layer)
        atlas.setPageNameExpression("page name")
        atlas.setSortFeatures(True)
        atlas.setSortAscending(False)
        atlas.setSortExpression("sort exp")
        atlas.setFilterFeatures(True)
        atlas.setFilterExpression("filter exp")

        doc = QDomDocument("testdoc")
        elem = l.writeXml(doc, QgsReadWriteContext())

        l2 = QgsPrintLayout(p)
        self.assertTrue(l2.readXml(elem, doc, QgsReadWriteContext()))
        atlas2 = l2.atlas()
        self.assertTrue(atlas2.enabled())
        self.assertTrue(atlas2.hideCoverage())
        self.assertEqual(atlas2.filenameExpression(), "filename exp")
        self.assertEqual(atlas2.coverageLayer(), vector_layer)
        self.assertEqual(atlas2.pageNameExpression(), "page name")
        self.assertTrue(atlas2.sortFeatures())
        self.assertFalse(atlas2.sortAscending())
        self.assertEqual(atlas2.sortExpression(), "sort exp")
        self.assertTrue(atlas2.filterFeatures())
        self.assertEqual(atlas2.filterExpression(), "filter exp")

    def testIteration(self):
        p = QgsProject()
        vectorFileInfo = QFileInfo(unitTestDataPath() + "/france_parts.shp")
        vector_layer = QgsVectorLayer(
            vectorFileInfo.filePath(), vectorFileInfo.completeBaseName(), "ogr"
        )
        self.assertTrue(vector_layer.isValid())
        p.addMapLayer(vector_layer)

        l = QgsPrintLayout(p)
        atlas = l.atlas()
        atlas.setEnabled(True)
        atlas.setCoverageLayer(vector_layer)

        atlas_feature_changed_spy = QSignalSpy(atlas.featureChanged)
        context_changed_spy = QSignalSpy(l.reportContext().changed)

        self.assertTrue(atlas.beginRender())
        self.assertTrue(atlas.first())
        self.assertEqual(len(atlas_feature_changed_spy), 1)
        self.assertEqual(len(context_changed_spy), 1)
        self.assertEqual(atlas.currentFeatureNumber(), 0)
        self.assertEqual(l.reportContext().feature()[4], "Basse-Normandie")
        self.assertEqual(l.reportContext().layer(), vector_layer)
        f1 = l.reportContext().feature()

        self.assertTrue(atlas.next())
        self.assertEqual(len(atlas_feature_changed_spy), 2)
        self.assertEqual(len(context_changed_spy), 2)
        self.assertEqual(atlas.currentFeatureNumber(), 1)
        self.assertEqual(l.reportContext().feature()[4], "Bretagne")
        f2 = l.reportContext().feature()

        self.assertTrue(atlas.next())
        self.assertEqual(len(atlas_feature_changed_spy), 3)
        self.assertEqual(len(context_changed_spy), 3)
        self.assertEqual(atlas.currentFeatureNumber(), 2)
        self.assertEqual(l.reportContext().feature()[4], "Pays de la Loire")
        f3 = l.reportContext().feature()

        self.assertTrue(atlas.next())
        self.assertEqual(len(atlas_feature_changed_spy), 4)
        self.assertEqual(len(context_changed_spy), 4)
        self.assertEqual(atlas.currentFeatureNumber(), 3)
        self.assertEqual(l.reportContext().feature()[4], "Centre")
        f4 = l.reportContext().feature()

        self.assertFalse(atlas.next())
        self.assertTrue(atlas.seekTo(2))
        self.assertEqual(len(atlas_feature_changed_spy), 5)
        self.assertEqual(len(context_changed_spy), 5)
        self.assertEqual(atlas.currentFeatureNumber(), 2)
        self.assertEqual(l.reportContext().feature()[4], "Pays de la Loire")

        self.assertTrue(atlas.last())
        self.assertEqual(len(atlas_feature_changed_spy), 6)
        self.assertEqual(len(context_changed_spy), 6)
        self.assertEqual(atlas.currentFeatureNumber(), 3)
        self.assertEqual(l.reportContext().feature()[4], "Centre")

        self.assertTrue(atlas.previous())
        self.assertEqual(len(atlas_feature_changed_spy), 7)
        self.assertEqual(len(context_changed_spy), 7)
        self.assertEqual(atlas.currentFeatureNumber(), 2)
        self.assertEqual(l.reportContext().feature()[4], "Pays de la Loire")

        self.assertTrue(atlas.previous())
        self.assertTrue(atlas.previous())
        self.assertEqual(len(atlas_feature_changed_spy), 9)
        self.assertFalse(atlas.previous())
        self.assertEqual(len(atlas_feature_changed_spy), 9)

        self.assertTrue(atlas.endRender())
        self.assertEqual(len(atlas_feature_changed_spy), 10)

        self.assertTrue(atlas.seekTo(f1))
        self.assertEqual(l.reportContext().feature()[4], "Basse-Normandie")
        self.assertTrue(atlas.seekTo(f4))
        self.assertEqual(l.reportContext().feature()[4], "Centre")
        self.assertTrue(atlas.seekTo(f3))
        self.assertEqual(l.reportContext().feature()[4], "Pays de la Loire")
        self.assertTrue(atlas.seekTo(f2))
        self.assertEqual(l.reportContext().feature()[4], "Bretagne")
        self.assertFalse(atlas.seekTo(QgsFeature(5)))

    def testUpdateFeature(self):
        p = QgsProject()
        vectorFileInfo = QFileInfo(unitTestDataPath() + "/france_parts.shp")
        vector_layer = QgsVectorLayer(
            vectorFileInfo.filePath(), vectorFileInfo.completeBaseName(), "ogr"
        )
        self.assertTrue(vector_layer.isValid())
        p.addMapLayer(vector_layer)

        l = QgsPrintLayout(p)
        atlas = l.atlas()
        atlas.setEnabled(True)
        atlas.setCoverageLayer(vector_layer)

        self.assertTrue(atlas.beginRender())
        self.assertTrue(atlas.first())
        self.assertEqual(atlas.currentFeatureNumber(), 0)
        self.assertEqual(l.reportContext().feature()[4], "Basse-Normandie")
        self.assertEqual(l.reportContext().layer(), vector_layer)

        vector_layer.startEditing()
        self.assertTrue(
            vector_layer.changeAttributeValue(
                l.reportContext().feature().id(), 4, "Nah, Canberra mate!"
            )
        )
        self.assertEqual(l.reportContext().feature()[4], "Basse-Normandie")
        l.atlas().refreshCurrentFeature()
        self.assertEqual(l.reportContext().feature()[4], "Nah, Canberra mate!")
        vector_layer.rollBack()

    def testFileName(self):
        p = QgsProject()
        vectorFileInfo = QFileInfo(unitTestDataPath() + "/france_parts.shp")
        vector_layer = QgsVectorLayer(
            vectorFileInfo.filePath(), vectorFileInfo.completeBaseName(), "ogr"
        )
        self.assertTrue(vector_layer.isValid())
        p.addMapLayer(vector_layer)

        l = QgsPrintLayout(p)
        atlas = l.atlas()
        atlas.setEnabled(True)
        atlas.setCoverageLayer(vector_layer)
        atlas.setFilenameExpression("'output_' || \"NAME_1\"")

        self.assertTrue(atlas.beginRender())
        self.assertEqual(atlas.count(), 4)
        atlas.first()
        self.assertEqual(atlas.currentFilename(), "output_Basse-Normandie")
        self.assertEqual(
            atlas.filePath("/tmp/output/", "png"),
            "/tmp/output/output_Basse-Normandie.png",
        )
        self.assertEqual(
            atlas.filePath("/tmp/output/", ".png"),
            "/tmp/output/output_Basse-Normandie.png",
        )
        self.assertEqual(
            atlas.filePath("/tmp/output/", "svg"),
            "/tmp/output/output_Basse-Normandie.svg",
        )

        atlas.next()
        self.assertEqual(atlas.currentFilename(), "output_Bretagne")
        self.assertEqual(
            atlas.filePath("/tmp/output/", "png"), "/tmp/output/output_Bretagne.png"
        )
        atlas.next()
        self.assertEqual(atlas.currentFilename(), "output_Pays de la Loire")
        self.assertEqual(
            atlas.filePath("/tmp/output/", "png"),
            "/tmp/output/output_Pays de la Loire.png",
        )
        atlas.next()
        self.assertEqual(atlas.currentFilename(), "output_Centre")
        self.assertEqual(
            atlas.filePath("/tmp/output/", "png"), "/tmp/output/output_Centre.png"
        )

        # try changing expression, filename should be updated instantly
        atlas.setFilenameExpression("'export_' || \"NAME_1\"")
        self.assertEqual(atlas.currentFilename(), "export_Centre")

        atlas.endRender()

    def testNameForPage(self):
        p = QgsProject()
        vectorFileInfo = QFileInfo(unitTestDataPath() + "/france_parts.shp")
        vector_layer = QgsVectorLayer(
            vectorFileInfo.filePath(), vectorFileInfo.completeBaseName(), "ogr"
        )
        self.assertTrue(vector_layer.isValid())
        p.addMapLayer(vector_layer)

        l = QgsPrintLayout(p)
        atlas = l.atlas()
        atlas.setEnabled(True)
        atlas.setCoverageLayer(vector_layer)
        atlas.setPageNameExpression('"NAME_1"')

        self.assertTrue(atlas.beginRender())
        self.assertEqual(atlas.nameForPage(0), "Basse-Normandie")
        self.assertEqual(atlas.nameForPage(1), "Bretagne")
        self.assertEqual(atlas.nameForPage(2), "Pays de la Loire")
        self.assertEqual(atlas.nameForPage(3), "Centre")

    def filename_test(self):
        self.atlas.setFilenameExpression("'output_' || @atlas_featurenumber")
        self.atlas.beginRender()
        for i in range(0, self.atlas.count()):
            self.atlas.seekTo(i)
            expected = "output_%d" % (i + 1)
            self.assertEqual(self.atlas.currentFilename(), expected)
        self.atlas.endRender()

        # using feature attribute (refs https://github.com/qgis/QGIS/issues/27379)

        self.atlas.setFilenameExpression(
            "'output_' || attribute(@atlas_feature,'NAME_1')"
        )
        expected = [
            "output_Basse-Normandie",
            "output_Bretagne",
            "output_Pays de la Loire",
            "output_Centre",
        ]
        self.atlas.beginRender()
        for i in range(0, self.atlas.count()):
            self.atlas.seekTo(i)
            self.assertEqual(self.atlas.currentFilename(), expected[i])
        self.atlas.endRender()

    def autoscale_render_test(self):
        self.atlas_map.setExtent(
            QgsRectangle(
                332719.06221504929,
                6765214.5887386119,
                560957.85090677091,
                6993453.3774303338,
            )
        )

        self.atlas_map.setAtlasDriven(True)
        self.atlas_map.setAtlasScalingMode(QgsLayoutItemMap.AtlasScalingMode.Auto)
        self.atlas_map.setAtlasMargin(0.10)

        self.atlas.beginRender()

        for i in range(0, 2):
            self.atlas.seekTo(i)
            self.mLabel1.adjustSizeToText()

            self.assertTrue(
                self.render_layout_check(
                    "atlas_autoscale%d" % (i + 1), self.layout, allowed_mismatch=200
                )
            )
        self.atlas.endRender()

        self.atlas_map.setAtlasDriven(False)
        self.atlas_map.setAtlasScalingMode(QgsLayoutItemMap.AtlasScalingMode.Fixed)
        self.atlas_map.setAtlasMargin(0)

    def fixedscale_render_test(self):
        self.atlas_map.setExtent(
            QgsRectangle(209838.166, 6528781.020, 610491.166, 6920530.620)
        )
        self.atlas_map.setAtlasDriven(True)
        self.atlas_map.setAtlasScalingMode(QgsLayoutItemMap.AtlasScalingMode.Fixed)

        self.atlas.beginRender()

        for i in range(0, 2):
            self.atlas.seekTo(i)
            self.mLabel1.adjustSizeToText()

            self.assertTrue(
                self.render_layout_check(
                    "atlas_fixedscale%d" % (i + 1), self.layout, allowed_mismatch=200
                )
            )

        self.atlas.endRender()

    def predefinedscales_render_test(self):
        self.atlas_map.setExtent(
            QgsRectangle(209838.166, 6528781.020, 610491.166, 6920530.620)
        )
        self.atlas_map.setAtlasDriven(True)
        self.atlas_map.setAtlasScalingMode(QgsLayoutItemMap.AtlasScalingMode.Predefined)

        scales = [1800000, 5000000]
        self.layout.renderContext().setPredefinedScales(scales)
        for i, s in enumerate(self.layout.renderContext().predefinedScales()):
            self.assertEqual(s, scales[i])

        self.atlas.beginRender()

        for i in range(0, 2):
            self.atlas.seekTo(i)
            self.mLabel1.adjustSizeToText()

            self.assertTrue(
                self.render_layout_check(
                    "atlas_predefinedscales%d" % (i + 1),
                    self.layout,
                    allowed_mismatch=200,
                )
            )
        self.atlas.endRender()

    def hidden_render_test(self):
        self.atlas_map.setExtent(
            QgsRectangle(209838.166, 6528781.020, 610491.166, 6920530.620)
        )
        self.atlas_map.setAtlasScalingMode(QgsLayoutItemMap.AtlasScalingMode.Fixed)
        self.atlas.setHideCoverage(True)

        self.atlas.beginRender()

        for i in range(0, 2):
            self.atlas.seekTo(i)
            self.mLabel1.adjustSizeToText()

            self.assertTrue(
                self.render_layout_check(
                    "atlas_hiding%d" % (i + 1), self.layout, allowed_mismatch=200
                )
            )
        self.atlas.endRender()

        self.atlas.setHideCoverage(False)

    def sorting_render_test(self):
        self.atlas_map.setExtent(
            QgsRectangle(209838.166, 6528781.020, 610491.166, 6920530.620)
        )
        self.atlas_map.setAtlasScalingMode(QgsLayoutItemMap.AtlasScalingMode.Fixed)
        self.atlas.setHideCoverage(False)

        self.atlas.setSortFeatures(True)
        self.atlas.setSortKeyAttributeIndex(4)  # departement name
        self.atlas.setSortAscending(False)

        self.atlas.beginRender()

        for i in range(0, 2):
            self.atlas.seekTo(i)
            self.mLabel1.adjustSizeToText()

            self.assertTrue(
                self.render_layout_check(
                    "atlas_sorting%d" % (i + 1), self.layout, allowed_mismatch=200
                )
            )

        self.atlas.endRender()

    def filtering_render_test(self):
        self.atlas_map.setExtent(
            QgsRectangle(209838.166, 6528781.020, 610491.166, 6920530.620)
        )
        self.atlas_map.setAtlasScalingMode(QgsLayoutItemMap.AtlasScalingMode.Fixed)
        self.atlas.setHideCoverage(False)

        self.atlas.setSortFeatures(False)

        self.atlas.setFilterFeatures(True)
        self.atlas.setFeatureFilter(
            "substr(NAME_1,1,1)='P'"
        )  # select only 'Pays de la loire'

        self.atlas.beginRender()

        for i in range(0, 1):
            self.atlas.seekTo(i)
            self.mLabel1.adjustSizeToText()

            self.assertTrue(
                self.render_layout_check(
                    "atlas_filtering%d" % (i + 1), self.layout, allowed_mismatch=200
                )
            )
        self.atlas.endRender()

    def test_clipping(self):
        vectorFileInfo = QFileInfo(unitTestDataPath() + "/france_parts.shp")
        vectorLayer = QgsVectorLayer(
            vectorFileInfo.filePath(), vectorFileInfo.completeBaseName(), "ogr"
        )

        p = QgsProject()
        p.addMapLayers([vectorLayer])

        # create layout with layout map

        # select epsg:2154
        crs = QgsCoordinateReferenceSystem("epsg:2154")
        p.setCrs(crs)

        layout = QgsPrintLayout(p)
        layout.initializeDefaults()

        # fix the renderer, fill with green
        props = {"color": "0,127,0", "outline_style": "no"}
        fillSymbol = QgsFillSymbol.createSimple(props)
        renderer = QgsSingleSymbolRenderer(fillSymbol)
        vectorLayer.setRenderer(renderer)

        # the atlas map
        atlas_map = QgsLayoutItemMap(layout)
        atlas_map.attemptSetSceneRect(QRectF(20, 20, 130, 130))
        atlas_map.setFrameEnabled(False)
        atlas_map.setLayers([vectorLayer])
        layout.addLayoutItem(atlas_map)

        # the atlas
        atlas = layout.atlas()
        atlas.setCoverageLayer(vectorLayer)
        atlas.setEnabled(True)

        atlas_map.setExtent(
            QgsRectangle(
                332719.06221504929,
                6765214.5887386119,
                560957.85090677091,
                6993453.3774303338,
            )
        )

        atlas_map.setAtlasDriven(True)
        atlas_map.setAtlasScalingMode(QgsLayoutItemMap.AtlasScalingMode.Auto)
        atlas_map.setAtlasMargin(0.10)

        atlas_map.atlasClippingSettings().setEnabled(True)

        atlas.beginRender()

        for i in range(0, 2):
            atlas.seekTo(i)

            self.assertTrue(
                self.render_layout_check(
                    "atlas_clipping%d" % (i + 1), layout, allowed_mismatch=200
                )
            )

        atlas.endRender()

    def legend_test(self):
        self.atlas_map.setAtlasDriven(True)
        self.atlas_map.setAtlasScalingMode(QgsLayoutItemMap.AtlasScalingMode.Auto)
        self.atlas_map.setAtlasMargin(0.10)

        # add a point layer
        ptLayer = QgsVectorLayer(
            "Point?crs=epsg:4326&field=attr:int(1)&field=label:string(20)",
            "points",
            "memory",
        )

        pr = ptLayer.dataProvider()
        f1 = QgsFeature(1)
        f1.initAttributes(2)
        f1.setAttribute(0, 1)
        f1.setAttribute(1, "Test label 1")
        f1.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(-0.638, 48.954)))
        f2 = QgsFeature(2)
        f2.initAttributes(2)
        f2.setAttribute(0, 2)
        f2.setAttribute(1, "Test label 2")
        f2.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(-1.682, 48.550)))
        pr.addFeatures([f1, f2])

        # categorized symbology
        r = QgsCategorizedSymbolRenderer(
            "attr",
            [
                QgsRendererCategory(
                    1,
                    QgsMarkerSymbol.createSimple(
                        {"color": "255,0,0", "outline_color": "black"}
                    ),
                    "red",
                ),
                QgsRendererCategory(
                    2,
                    QgsMarkerSymbol.createSimple(
                        {"color": "0,0,255", "outline_color": "black"}
                    ),
                    "blue",
                ),
            ],
        )
        ptLayer.setRenderer(r)

        QgsProject.instance().addMapLayer(ptLayer)

        # add the point layer to the map settings
        layers = self.layers
        layers = [ptLayer] + layers
        self.atlas_map.setLayers(layers)
        self.overview.setLayers(layers)

        # add a legend
        legend = QgsLayoutItemLegend(self.layout)
        legend.rstyle(QgsLegendStyle.Style.Title).setFont(
            QgsFontUtils.getStandardTestFont("Bold", 20)
        )
        legend.rstyle(QgsLegendStyle.Style.Group).setFont(
            QgsFontUtils.getStandardTestFont("Bold", 18)
        )
        legend.rstyle(QgsLegendStyle.Style.Subgroup).setFont(
            QgsFontUtils.getStandardTestFont("Bold", 18)
        )
        legend.rstyle(QgsLegendStyle.Style.SymbolLabel).setFont(
            QgsFontUtils.getStandardTestFont("Bold", 14)
        )

        legend.setTitle("Legend")
        legend.attemptMove(QgsLayoutPoint(200, 100))
        # sets the legend filter parameter
        legend.setLinkedMap(self.atlas_map)
        legend.setLegendFilterOutAtlas(True)
        self.layout.addLayoutItem(legend)

        self.atlas.beginRender()

        self.atlas.seekTo(0)
        self.mLabel1.adjustSizeToText()

        self.assertTrue(self.render_layout_check("atlas_legend", self.layout))

        self.atlas.endRender()

        # restore state
        self.atlas_map.setLayers([layers[1]])
        self.layout.removeLayoutItem(legend)
        QgsProject.instance().removeMapLayer(ptLayer.id())

    def rotation_test(self):
        # We will create a polygon layer with a rotated rectangle.
        # Then we will make it the object layer for the atlas,
        # rotate the map and test that the bounding rectangle
        # is smaller than the bounds without rotation.
        polygonLayer = QgsVectorLayer("Polygon", "test_polygon", "memory")
        poly = QgsFeature(polygonLayer.fields())
        points = [(10, 15), (15, 10), (45, 40), (40, 45)]
        poly.setGeometry(
            QgsGeometry.fromPolygonXY([[QgsPointXY(x[0], x[1]) for x in points]])
        )
        polygonLayer.dataProvider().addFeatures([poly])
        QgsProject.instance().addMapLayer(polygonLayer)

        # Recreating the layout locally
        composition = QgsPrintLayout(QgsProject.instance())
        composition.initializeDefaults()

        # the atlas map
        atlasMap = QgsLayoutItemMap(composition)
        atlasMap.attemptSetSceneRect(QRectF(20, 20, 130, 130))
        atlasMap.setFrameEnabled(True)
        atlasMap.setLayers([polygonLayer])
        atlasMap.setExtent(QgsRectangle(0, 0, 100, 50))
        composition.addLayoutItem(atlasMap)

        # the atlas
        atlas = composition.atlas()
        atlas.setCoverageLayer(polygonLayer)
        atlas.setEnabled(True)

        atlasMap.setAtlasDriven(True)
        atlasMap.setAtlasScalingMode(QgsLayoutItemMap.AtlasScalingMode.Auto)
        atlasMap.setAtlasMargin(0.0)

        # Testing
        atlasMap.setMapRotation(0.0)
        atlas.beginRender()
        atlas.first()
        nonRotatedExtent = QgsRectangle(atlasMap.extent())

        atlasMap.setMapRotation(45.0)
        atlas.first()
        rotatedExtent = QgsRectangle(atlasMap.extent())

        self.assertLess(rotatedExtent.width(), nonRotatedExtent.width() * 0.9)
        self.assertLess(rotatedExtent.height(), nonRotatedExtent.height() * 0.9)

        QgsProject.instance().removeMapLayer(polygonLayer)

    def test_datadefined_margin(self):
        polygonLayer = QgsVectorLayer(
            "Polygon?field=margin:int", "test_polygon", "memory"
        )
        poly = QgsFeature(polygonLayer.fields())
        poly.setAttributes([0])
        poly.setGeometry(
            QgsGeometry.fromWkt("Polygon((30 30, 40 30, 40 40, 30 40, 30 30))")
        )
        polygonLayer.dataProvider().addFeatures([poly])
        poly = QgsFeature(polygonLayer.fields())
        poly.setAttributes([10])
        poly.setGeometry(
            QgsGeometry.fromWkt("Polygon((10 10, 20 10, 20 20, 10 20, 10 10))")
        )
        polygonLayer.dataProvider().addFeatures([poly])
        poly = QgsFeature(polygonLayer.fields())
        poly.setAttributes([20])
        poly.setGeometry(
            QgsGeometry.fromWkt("Polygon((50 50, 60 50, 60 60, 50 60, 50 50))")
        )
        polygonLayer.dataProvider().addFeatures([poly])
        QgsProject.instance().addMapLayer(polygonLayer)

        layout = QgsPrintLayout(QgsProject.instance())
        map = QgsLayoutItemMap(layout)
        map.setCrs(polygonLayer.crs())
        map.attemptSetSceneRect(QRectF(20, 20, 130, 130))
        map.setFrameEnabled(True)
        map.setLayers([polygonLayer])
        map.setExtent(QgsRectangle(0, 0, 100, 50))
        layout.addLayoutItem(map)

        atlas = layout.atlas()
        atlas.setCoverageLayer(polygonLayer)
        atlas.setEnabled(True)

        map.setAtlasDriven(True)
        map.setAtlasScalingMode(QgsLayoutItemMap.AtlasScalingMode.Auto)
        map.setAtlasMargin(77.0)
        map.dataDefinedProperties().setProperty(
            QgsLayoutObject.DataDefinedProperty.MapAtlasMargin,
            QgsProperty.fromExpression("margin/2"),
        )

        atlas.beginRender()
        atlas.first()

        self.assertEqual(map.extent(), QgsRectangle(25, 30, 45, 40))
        self.assertTrue(atlas.next())
        self.assertEqual(map.extent(), QgsRectangle(4.5, 9.75, 25.5, 20.25))
        self.assertTrue(atlas.next())
        self.assertEqual(map.extent(), QgsRectangle(44, 49.5, 66, 60.5))

        QgsProject.instance().removeMapLayer(polygonLayer)

    def testChangedSignal(self):
        layout = QgsPrintLayout(QgsProject.instance())
        atlas = layout.atlas()
        s = QSignalSpy(atlas.changed)

        atlas.setPageNameExpression("1+2")
        self.assertEqual(len(s), 1)
        atlas.setPageNameExpression("1+2")
        self.assertEqual(len(s), 1)

        atlas.setSortFeatures(True)
        self.assertEqual(len(s), 2)
        atlas.setSortFeatures(True)
        self.assertEqual(len(s), 2)

        atlas.setSortAscending(False)
        self.assertEqual(len(s), 3)
        atlas.setSortAscending(False)
        self.assertEqual(len(s), 3)

        atlas.setSortExpression("1+2")
        self.assertEqual(len(s), 4)
        atlas.setSortExpression("1+2")
        self.assertEqual(len(s), 4)

        atlas.setFilterFeatures(True)
        self.assertEqual(len(s), 5)
        atlas.setFilterFeatures(True)
        self.assertEqual(len(s), 5)

        atlas.setFilterExpression("1+2")
        self.assertEqual(len(s), 6)
        atlas.setFilterExpression("1+2")
        self.assertEqual(len(s), 6)

        atlas.setHideCoverage(True)
        self.assertEqual(len(s), 7)
        atlas.setHideCoverage(True)
        self.assertEqual(len(s), 7)

        atlas.setFilenameExpression("1+2")
        self.assertEqual(len(s), 8)
        atlas.setFilenameExpression("1+2")
        self.assertEqual(len(s), 8)


if __name__ == "__main__":
    unittest.main()
