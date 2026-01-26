"""
***************************************************************************
    test_qgspointdisplacementrenderer.py
    -----------------------------
    Date                 : September 2016
    Copyright            : (C) 2016 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************

From build dir, run: ctest -R PyQgsPointDisplacementRenderer -V

"""

__author__ = "Nyall Dawson"
__date__ = "September 2016"
__copyright__ = "(C) 2016, Nyall Dawson"

import os

from qgis.PyQt.QtCore import QSize
from qgis.PyQt.QtGui import QColor
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    QgsCategorizedSymbolRenderer,
    QgsFeature,
    QgsFontUtils,
    QgsGeometry,
    QgsGeometryGeneratorSymbolLayer,
    QgsMapRendererSequentialJob,
    QgsMapSettings,
    QgsMapUnitScale,
    QgsMarkerSymbol,
    QgsPointClusterRenderer,
    QgsPointDisplacementRenderer,
    QgsProject,
    QgsProperty,
    QgsReadWriteContext,
    QgsRectangle,
    QgsRenderContext,
    QgsRendererCategory,
    QgsSingleSymbolRenderer,
    QgsSymbol,
    QgsSymbolLayer,
    QgsUnitTypes,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

# Convenience instances in case you may need them
# not used in this test
start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsPointDisplacementRenderer(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "displacement_renderer"

    def _setUp(self):
        myShpFile = os.path.join(TEST_DATA_DIR, "points.shp")
        layer = QgsVectorLayer(myShpFile, "Points", "ogr")
        QgsProject.instance().addMapLayer(layer)

        renderer = QgsPointDisplacementRenderer()
        sym1 = QgsMarkerSymbol.createSimple(
            {"color": "#ff00ff", "size": "3", "outline_style": "no"}
        )
        sym_renderer = QgsSingleSymbolRenderer(sym1)
        renderer.setEmbeddedRenderer(sym_renderer)
        renderer.setCircleRadiusAddition(2)
        renderer.setCircleWidth(1)
        renderer.setCircleColor(QColor(0, 0, 0))
        renderer.setCenterSymbol(
            QgsMarkerSymbol.createSimple(
                {"color": "#ffff00", "size": "3", "outline_style": "no"}
            )
        )
        layer.setRenderer(renderer)

        rendered_layers = [layer]
        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setExtent(QgsRectangle(-123, 18, -70, 52))
        mapsettings.setLayers(rendered_layers)
        return layer, renderer, mapsettings

    def _tearDown(self, layer):
        # QgsProject.instance().removeAllMapLayers()
        QgsProject.instance().removeMapLayer(layer)

    def _setProperties(self, r):
        """set properties for a renderer for testing with _checkProperties"""
        r.setLabelAttributeName("name")
        f = QgsFontUtils.getStandardTestFont("Bold Oblique", 14)
        r.setLabelFont(f)
        r.setMinimumLabelScale(50000)
        r.setLabelColor(QColor(255, 0, 0))
        r.setTolerance(5)
        r.setToleranceUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        r.setToleranceMapUnitScale(QgsMapUnitScale(5, 15))
        r.setCircleWidth(15)
        r.setCircleColor(QColor(0, 255, 0))
        r.setCircleRadiusAddition(2.5)
        r.setPlacement(QgsPointDisplacementRenderer.Placement.ConcentricRings)
        r.setLabelDistanceFactor(0.25)
        m = QgsMarkerSymbol()
        m.setColor(QColor(0, 255, 0))
        r.setCenterSymbol(m)
        sym1 = QgsMarkerSymbol.createSimple({"color": "#fdbf6f"})
        renderer = QgsSingleSymbolRenderer(sym1)
        r.setEmbeddedRenderer(renderer)

    def _checkProperties(self, r):
        """test properties of renderer against expected"""
        self.assertEqual(r.labelAttributeName(), "name")
        f = QgsFontUtils.getStandardTestFont("Bold Oblique", 14)
        self.assertEqual(r.labelFont().styleName(), f.styleName())
        self.assertEqual(r.minimumLabelScale(), 50000)
        self.assertEqual(r.labelColor(), QColor(255, 0, 0))
        self.assertEqual(r.tolerance(), 5)
        self.assertEqual(r.toleranceUnit(), QgsUnitTypes.RenderUnit.RenderMapUnits)
        self.assertEqual(r.toleranceMapUnitScale(), QgsMapUnitScale(5, 15))
        self.assertEqual(r.circleWidth(), 15)
        self.assertEqual(r.circleColor(), QColor(0, 255, 0))
        self.assertEqual(r.circleRadiusAddition(), 2.5)
        self.assertEqual(
            r.placement(), QgsPointDisplacementRenderer.Placement.ConcentricRings
        )
        self.assertEqual(r.centerSymbol().color(), QColor(0, 255, 0))
        self.assertEqual(r.embeddedRenderer().symbol().color().name(), "#fdbf6f")
        self.assertEqual(r.labelDistanceFactor(), 0.25)

    def _create_categorized_renderer(self):
        cat_renderer = QgsCategorizedSymbolRenderer(attrName="Class")
        sym1 = QgsMarkerSymbol.createSimple(
            {"color": "#ff00ff", "size": "6", "outline_style": "no"}
        )
        cat1 = QgsRendererCategory("Biplane", sym1, "Big")
        cat_renderer.addCategory(cat1)
        sym2 = QgsMarkerSymbol.createSimple(
            {"color": "#ff00ff", "size": "3", "outline_style": "no"}
        )
        cat2 = QgsRendererCategory(["B52", "Jet"], sym2, "Smaller")
        cat_renderer.addCategory(cat2)
        return cat_renderer

    def testGettersSetters(self):
        """test getters and setters"""
        r = QgsPointDisplacementRenderer()
        self._setProperties(r)
        self._checkProperties(r)

    def testClone(self):
        """test cloning renderer"""
        r = QgsPointDisplacementRenderer()
        self._setProperties(r)
        c = r.clone()
        self._checkProperties(c)

    def testSaveCreate(self):
        """test saving and recreating from XML"""
        r = QgsPointDisplacementRenderer()
        self._setProperties(r)
        doc = QDomDocument("testdoc")
        elem = r.save(doc, QgsReadWriteContext())
        c = QgsPointDisplacementRenderer.create(elem, QgsReadWriteContext())
        self._checkProperties(c)

    def testConvert(self):
        """test renderer conversion"""

        # same type, should clone
        r = QgsPointDisplacementRenderer()
        self._setProperties(r)
        c = QgsPointDisplacementRenderer.convertFromRenderer(r)
        self._checkProperties(c)

        # test conversion from cluster renderer
        r = QgsPointClusterRenderer()
        r.setTolerance(5)
        r.setToleranceUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        r.setToleranceMapUnitScale(QgsMapUnitScale(5, 15))
        m = QgsMarkerSymbol()
        m.setColor(QColor(0, 255, 0))
        r.setClusterSymbol(m)
        sym1 = QgsMarkerSymbol.createSimple({"color": "#fdbf6f"})
        renderer = QgsSingleSymbolRenderer(sym1)
        r.setEmbeddedRenderer(renderer)

        # want to keep as many settings as possible when converting between cluster and displacement renderer
        d = QgsPointDisplacementRenderer.convertFromRenderer(r)
        self.assertEqual(d.tolerance(), 5)
        self.assertEqual(d.toleranceUnit(), QgsUnitTypes.RenderUnit.RenderMapUnits)
        self.assertEqual(d.toleranceMapUnitScale(), QgsMapUnitScale(5, 15))
        self.assertEqual(d.embeddedRenderer().symbol().color().name(), "#fdbf6f")

    def testRenderNoCluster(self):
        layer, renderer, mapsettings = self._setUp()
        layer.renderer().setTolerance(1)
        res = self.render_map_settings_check(
            "displacement_no_cluster", "displacement_no_cluster", mapsettings
        )

        self.assertTrue(res)
        self._tearDown(layer)

    def testRenderWithin(self):
        layer, renderer, mapsettings = self._setUp()
        layer.renderer().setTolerance(10)
        res = self.render_map_settings_check(
            "displacement_cluster", "displacement_cluster", mapsettings
        )
        self.assertTrue(res)
        self._tearDown(layer)

    def testMultiPoint(self):
        """
        Test multipoint handling
        """
        layer = QgsVectorLayer("Multipoint?field=cat:string", "", "memory")
        self.assertTrue(layer.isValid())

        f = QgsFeature(layer.fields())
        f.setAttributes(["a"])
        f.setGeometry(QgsGeometry.fromWkt("MultiPoint(5 5, 5 6, 9 9)"))
        layer.dataProvider().addFeature(f)
        f.setAttributes(["b"])
        f.setGeometry(QgsGeometry.fromWkt("MultiPoint(2 1, 2 2, 5 5)"))
        layer.dataProvider().addFeature(f)
        f.setAttributes(["c"])
        f.setGeometry(QgsGeometry.fromWkt("MultiPoint(9 1)"))
        layer.dataProvider().addFeature(f)

        renderer = QgsPointDisplacementRenderer()
        sym1 = QgsMarkerSymbol.createSimple(
            {"color": "#ff00ff", "size": "3", "outline_style": "no"}
        )
        sym_renderer = QgsCategorizedSymbolRenderer()
        sym_renderer.setClassAttribute("cat")
        sym1.setColor(QColor(255, 0, 0))
        sym_renderer.addCategory(QgsRendererCategory("a", sym1.clone(), "a"))
        sym1.setColor(QColor(0, 255, 0))
        sym_renderer.addCategory(QgsRendererCategory("b", sym1.clone(), "b"))
        sym1.setColor(QColor(0, 0, 255))
        sym_renderer.addCategory(QgsRendererCategory("c", sym1.clone(), "c"))
        renderer.setEmbeddedRenderer(sym_renderer)

        renderer.setCircleRadiusAddition(2)
        renderer.setCircleWidth(1)
        renderer.setCircleColor(QColor(0, 0, 0))
        renderer.setCenterSymbol(
            QgsMarkerSymbol.createSimple(
                {"color": "#ffff00", "size": "3", "outline_style": "no"}
            )
        )
        renderer.setToleranceUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        renderer.setTolerance(2)
        layer.setRenderer(renderer)

        rendered_layers = [layer]
        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setExtent(QgsRectangle(0, 0, 10, 10))
        mapsettings.setLayers(rendered_layers)

        result = self.render_map_settings_check(
            "displacement_multipoint", "displacement_multipoint", mapsettings
        )
        self.assertTrue(result)

    def testRenderVariables(self):
        """test rendering with expression variables in marker"""
        layer, renderer, mapsettings = self._setUp()
        layer.renderer().setTolerance(10)

        old_marker = layer.renderer().centerSymbol().clone()

        new_marker = QgsMarkerSymbol.createSimple(
            {"color": "#ffff00", "size": "3", "outline_style": "no"}
        )
        new_marker.symbolLayer(0).setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyFillColor,
            QgsProperty.fromExpression("@cluster_color"),
        )
        new_marker.symbolLayer(0).setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertySize,
            QgsProperty.fromExpression("@cluster_size*2"),
        )
        layer.renderer().setCenterSymbol(new_marker)
        result = self.render_map_settings_check(
            "displacement_variables", "displacement_variables", mapsettings
        )
        layer.renderer().setCenterSymbol(old_marker)
        self.assertTrue(result)
        self._tearDown(layer)

    def testRenderGrid(self):
        layer, renderer, mapsettings = self._setUp()
        layer.renderer().setTolerance(10)
        layer.renderer().setPlacement(QgsPointDisplacementRenderer.Placement.Grid)
        res = self.render_map_settings_check(
            "displacement_grid", "displacement_grid", mapsettings
        )
        self.assertTrue(res)
        self._tearDown(layer)

    def testRenderGridAdjust(self):
        layer, renderer, mapsettings = self._setUp()
        layer.renderer().setTolerance(10)
        layer.renderer().setCircleRadiusAddition(5)
        layer.renderer().setPlacement(QgsPointDisplacementRenderer.Placement.Grid)
        layer.renderer().setCircleColor(QColor())
        res = self.render_map_settings_check(
            "displacement_adjust_grid", "displacement_adjust_grid", mapsettings
        )
        self.assertTrue(res)
        self._tearDown(layer)

    def testClusterRingLabels(self):
        layer, renderer, mapsettings = self._setUp()
        layer.renderer().setTolerance(10)
        layer.renderer().setLabelAttributeName("Class")
        layer.renderer().setLabelDistanceFactor(0.35)
        f = QgsFontUtils.getStandardTestFont("Bold", 14)
        layer.renderer().setLabelFont(f)
        res = self.render_map_settings_check(
            "displacement_cluster_ring_labels",
            "displacement_cluster_ring_labels",
            mapsettings,
        )
        self.assertTrue(res)
        self._tearDown(layer)

    def testClusterGridLabels(self):
        layer, renderer, mapsettings = self._setUp()
        layer.renderer().setTolerance(10)
        layer.renderer().setLabelAttributeName("Class")
        layer.renderer().setLabelDistanceFactor(0.35)
        f = QgsFontUtils.getStandardTestFont("Bold", 14)
        layer.renderer().setLabelFont(f)
        layer.renderer().setPlacement(QgsPointDisplacementRenderer.Placement.Grid)
        res = self.render_map_settings_check(
            "displacement_cluster_grid_labels",
            "displacement_cluster_grid_labels",
            mapsettings,
        )
        self.assertTrue(res)
        self._tearDown(layer)

    def testClusterConcentricLabels(self):
        layer, renderer, mapsettings = self._setUp()
        layer.renderer().setTolerance(10)
        layer.renderer().setLabelAttributeName("Class")
        layer.renderer().setLabelDistanceFactor(0.35)
        f = QgsFontUtils.getStandardTestFont("Bold", 14)
        layer.renderer().setLabelFont(f)
        layer.renderer().setPlacement(
            QgsPointDisplacementRenderer.Placement.ConcentricRings
        )
        res = self.render_map_settings_check(
            "displacement_cluster_concentric_labels",
            "displacement_cluster_concentric_labels",
            mapsettings,
        )
        self.assertTrue(res)
        self._tearDown(layer)

    def testClusterRingLabelsDifferentSizes(self):
        layer, renderer, mapsettings = self._setUp()
        renderer.setEmbeddedRenderer(self._create_categorized_renderer())
        layer.renderer().setTolerance(10)
        layer.renderer().setLabelAttributeName("Class")
        layer.renderer().setLabelDistanceFactor(0.35)
        f = QgsFontUtils.getStandardTestFont("Bold", 14)
        layer.renderer().setLabelFont(f)
        res = self.render_map_settings_check(
            "displacement_cluster_ring_labels_diff_size",
            "displacement_cluster_ring_labels_diff_size",
            mapsettings,
        )
        self.assertTrue(res)
        self._tearDown(layer)

    def testClusterGridLabelsDifferentSizes(self):
        layer, renderer, mapsettings = self._setUp()
        renderer.setEmbeddedRenderer(self._create_categorized_renderer())
        layer.renderer().setTolerance(10)
        layer.renderer().setLabelAttributeName("Class")
        layer.renderer().setLabelDistanceFactor(0.35)
        f = QgsFontUtils.getStandardTestFont("Bold", 14)
        layer.renderer().setLabelFont(f)
        layer.renderer().setPlacement(QgsPointDisplacementRenderer.Placement.Grid)
        res = self.render_map_settings_check(
            "displacement_cluster_grid_labels_diff_size",
            "displacement_cluster_grid_labels_diff_size",
            mapsettings,
        )
        self.assertTrue(res)
        self._tearDown(layer)

    def testClusterConcentricLabelsDifferentSizes(self):
        layer, renderer, mapsettings = self._setUp()
        renderer.setEmbeddedRenderer(self._create_categorized_renderer())
        layer.renderer().setTolerance(10)
        layer.renderer().setLabelAttributeName("Class")
        layer.renderer().setLabelDistanceFactor(0.35)
        f = QgsFontUtils.getStandardTestFont("Bold", 14)
        layer.renderer().setLabelFont(f)
        layer.renderer().setPlacement(
            QgsPointDisplacementRenderer.Placement.ConcentricRings
        )
        res = self.render_map_settings_check(
            "displacement_cluster_concentric_labels_diff_size",
            "displacement_cluster_concentric_labels_diff_size",
            mapsettings,
        )
        self.assertTrue(res)
        self._tearDown(layer)

    def testClusterRingLabelsDifferentSizesFarther(self):
        layer, renderer, mapsettings = self._setUp()
        renderer.setEmbeddedRenderer(self._create_categorized_renderer())
        layer.renderer().setTolerance(10)
        layer.renderer().setLabelAttributeName("Class")
        layer.renderer().setLabelDistanceFactor(1)
        f = QgsFontUtils.getStandardTestFont("Bold", 14)
        layer.renderer().setLabelFont(f)
        res = self.render_map_settings_check(
            "displacement_cluster_ring_labels_diff_size_farther",
            "displacement_cluster_ring_labels_diff_size_farther",
            mapsettings,
        )
        self.assertTrue(res)
        self._tearDown(layer)

    def testClusterGridLabelsDifferentSizesFarther(self):
        layer, renderer, mapsettings = self._setUp()
        renderer.setEmbeddedRenderer(self._create_categorized_renderer())
        layer.renderer().setTolerance(10)
        layer.renderer().setLabelAttributeName("Class")
        layer.renderer().setLabelDistanceFactor(1)
        layer.renderer().setPlacement(QgsPointDisplacementRenderer.Placement.Grid)
        f = QgsFontUtils.getStandardTestFont("Bold", 14)
        layer.renderer().setLabelFont(f)
        res = self.render_map_settings_check(
            "displacement_cluster_grid_labels_diff_size_farther",
            "displacement_cluster_grid_labels_diff_size_farther",
            mapsettings,
        )
        self.assertTrue(res)
        self._tearDown(layer)

    def testClusterConcentricLabelsDifferentSizesFarther(self):
        layer, renderer, mapsettings = self._setUp()
        renderer.setEmbeddedRenderer(self._create_categorized_renderer())
        layer.renderer().setTolerance(10)
        layer.renderer().setLabelAttributeName("Class")
        layer.renderer().setLabelDistanceFactor(1)
        f = QgsFontUtils.getStandardTestFont("Bold", 14)
        layer.renderer().setLabelFont(f)
        layer.renderer().setPlacement(
            QgsPointDisplacementRenderer.Placement.ConcentricRings
        )
        res = self.render_map_settings_check(
            "displacement_cluster_concentric_labels_diff_size_farther",
            "displacement_cluster_concentric_labels_diff_size_farther",
            mapsettings,
        )
        self.assertTrue(res)
        self._tearDown(layer)

    def test_legend_keys(self):
        symbol1 = QgsMarkerSymbol()
        symbol2 = QgsMarkerSymbol()
        sub_renderer = QgsCategorizedSymbolRenderer(
            "cat",
            [
                QgsRendererCategory("cat1", symbol1, "cat1", True, "0"),
                QgsRendererCategory("cat2", symbol2, "cat2", True, "1"),
            ],
        )

        renderer = QgsPointDisplacementRenderer()
        renderer.setEmbeddedRenderer(sub_renderer)

        self.assertEqual(renderer.legendKeys(), {"0", "1"})

    def test_legend_key_to_expression(self):
        sym1 = QgsMarkerSymbol.createSimple(
            {"color": "#fdbf6f", "outline_color": "black"}
        )
        sub_renderer = QgsSingleSymbolRenderer(sym1)

        renderer = QgsPointDisplacementRenderer()
        renderer.setEmbeddedRenderer(sub_renderer)

        exp, ok = renderer.legendKeyToExpression("0", None)
        self.assertTrue(ok)
        self.assertEqual(exp, "TRUE")

        exp, ok = renderer.legendKeyToExpression("xxxx", None)
        self.assertFalse(ok)

    def testUsedAttributes(self):
        layer, renderer, mapsettings = self._setUp()
        ctx = QgsRenderContext.fromMapSettings(mapsettings)

        self.assertCountEqual(renderer.usedAttributes(ctx), {})

    def testGeometryGenerator(self):
        """
        Don't check result image here, there is no point in using geometry generators
        with point displacement renderer, we just want to avoid crash
        """

        layer, renderer, mapsettings = self._setUp()
        layer.renderer().setTolerance(10)
        layer.renderer().setPlacement(QgsPointDisplacementRenderer.Placement.Ring)
        layer.renderer().setCircleRadiusAddition(0)

        geomGeneratorSymbolLayer = QgsGeometryGeneratorSymbolLayer.create(
            {"geometryModifier": "$geometry"}
        )
        geomGeneratorSymbolLayer.setSymbolType(QgsSymbol.SymbolType.Marker)
        geomGeneratorSymbolLayer.subSymbol().setSize(2.5)

        markerSymbol = QgsMarkerSymbol()
        markerSymbol.deleteSymbolLayer(0)
        markerSymbol.appendSymbolLayer(geomGeneratorSymbolLayer)
        self.assertEqual(markerSymbol.size(), 2.5)

        layer.renderer().setEmbeddedRenderer(QgsSingleSymbolRenderer(markerSymbol))

        job = QgsMapRendererSequentialJob(mapsettings)
        job.start()
        job.waitForFinished()


if __name__ == "__main__":
    unittest.main()
