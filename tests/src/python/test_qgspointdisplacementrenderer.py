# -*- coding: utf-8 -*-

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
"""

__author__ = 'Nyall Dawson'
__date__ = 'September 2016'
__copyright__ = '(C) 2016, Nyall Dawson'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

import os

from qgis.PyQt.QtGui import QColor
from qgis.PyQt.QtCore import QSize, QThreadPool, QDir
from qgis.PyQt.QtXml import QDomDocument

from qgis.core import (QgsVectorLayer,
                       QgsProject,
                       QgsRectangle,
                       QgsMultiRenderChecker,
                       QgsPointDisplacementRenderer,
                       QgsFontUtils,
                       QgsUnitTypes,
                       QgsMapUnitScale,
                       QgsMarkerSymbol,
                       QgsCategorizedSymbolRenderer,
                       QgsRendererCategory,
                       QgsSingleSymbolRenderer,
                       QgsPointClusterRenderer,
                       QgsMapSettings,
                       QgsProperty,
                       QgsReadWriteContext,
                       QgsSymbolLayer
                       )
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

# Convenience instances in case you may need them
# not used in this test
start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsPointDisplacementRenderer(unittest.TestCase):

    def setUp(self):
        self.report = "<h1>Python QgsPointDisplacementRenderer Tests</h1>\n"

    def tearDown(self):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(self.report)

    def _setUp(self):
        myShpFile = os.path.join(TEST_DATA_DIR, 'points.shp')
        layer = QgsVectorLayer(myShpFile, 'Points', 'ogr')
        QgsProject.instance().addMapLayer(layer)

        renderer = QgsPointDisplacementRenderer()
        sym1 = QgsMarkerSymbol.createSimple({'color': '#ff00ff', 'size': '3', 'outline_style': 'no'})
        sym_renderer = QgsSingleSymbolRenderer(sym1)
        renderer.setEmbeddedRenderer(sym_renderer)
        renderer.setCircleRadiusAddition(2)
        renderer.setCircleWidth(1)
        renderer.setCircleColor(QColor(0, 0, 0))
        renderer.setCenterSymbol(QgsMarkerSymbol.createSimple({'color': '#ffff00', 'size': '3', 'outline_style': 'no'}))
        layer.setRenderer(renderer)

        rendered_layers = [layer]
        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setExtent(QgsRectangle(-123, 18, -70, 52))
        mapsettings.setLayers(rendered_layers)
        return layer, renderer, mapsettings

    def _tearDown(self, layer):
        #QgsProject.instance().removeAllMapLayers()
        QgsProject.instance().removeMapLayer(layer)

    @classmethod
    def tearDownClass(cls):
        # avoid crash on finish, probably related to https://bugreports.qt.io/browse/QTBUG-35760
        QThreadPool.globalInstance().waitForDone()

    def _setProperties(self, r):
        """ set properties for a renderer for testing with _checkProperties"""
        r.setLabelAttributeName('name')
        f = QgsFontUtils.getStandardTestFont('Bold Oblique', 14)
        r.setLabelFont(f)
        r.setMinimumLabelScale(50000)
        r.setLabelColor(QColor(255, 0, 0))
        r.setTolerance(5)
        r.setToleranceUnit(QgsUnitTypes.RenderMapUnits)
        r.setToleranceMapUnitScale(QgsMapUnitScale(5, 15))
        r.setCircleWidth(15)
        r.setCircleColor(QColor(0, 255, 0))
        r.setCircleRadiusAddition(2.5)
        r.setPlacement(QgsPointDisplacementRenderer.ConcentricRings)
        r.setLabelDistanceFactor(0.25)
        m = QgsMarkerSymbol()
        m.setColor(QColor(0, 255, 0))
        r.setCenterSymbol(m)
        sym1 = QgsMarkerSymbol.createSimple({'color': '#fdbf6f'})
        renderer = QgsSingleSymbolRenderer(sym1)
        r.setEmbeddedRenderer(renderer)

    def _checkProperties(self, r):
        """ test properties of renderer against expected"""
        self.assertEqual(r.labelAttributeName(), 'name')
        f = QgsFontUtils.getStandardTestFont('Bold Oblique', 14)
        self.assertEqual(r.labelFont().styleName(), f.styleName())
        self.assertEqual(r.minimumLabelScale(), 50000)
        self.assertEqual(r.labelColor(), QColor(255, 0, 0))
        self.assertEqual(r.tolerance(), 5)
        self.assertEqual(r.toleranceUnit(), QgsUnitTypes.RenderMapUnits)
        self.assertEqual(r.toleranceMapUnitScale(), QgsMapUnitScale(5, 15))
        self.assertEqual(r.circleWidth(), 15)
        self.assertEqual(r.circleColor(), QColor(0, 255, 0))
        self.assertEqual(r.circleRadiusAddition(), 2.5)
        self.assertEqual(r.placement(), QgsPointDisplacementRenderer.ConcentricRings)
        self.assertEqual(r.centerSymbol().color(), QColor(0, 255, 0))
        self.assertEqual(r.embeddedRenderer().symbol().color().name(), '#fdbf6f')
        self.assertEqual(r.labelDistanceFactor(), 0.25)

    def _create_categorized_renderer(self):
        cat_renderer = QgsCategorizedSymbolRenderer(attrName='Class')
        sym1 = QgsMarkerSymbol.createSimple({'color': '#ff00ff', 'size': '6', 'outline_style': 'no'})
        cat1 = QgsRendererCategory('Biplane', sym1, 'Big')
        cat_renderer.addCategory(cat1)
        sym2 = QgsMarkerSymbol.createSimple({'color': '#ff00ff', 'size': '3', 'outline_style': 'no'})
        cat2 = QgsRendererCategory(['B52', 'Jet'], sym2, 'Smaller')
        cat_renderer.addCategory(cat2)
        return cat_renderer

    def testGettersSetters(self):
        """ test getters and setters """
        r = QgsPointDisplacementRenderer()
        self._setProperties(r)
        self._checkProperties(r)

    def testClone(self):
        """ test cloning renderer """
        r = QgsPointDisplacementRenderer()
        self._setProperties(r)
        c = r.clone()
        self._checkProperties(c)

    def testSaveCreate(self):
        """ test saving and recreating from XML """
        r = QgsPointDisplacementRenderer()
        self._setProperties(r)
        doc = QDomDocument("testdoc")
        elem = r.save(doc, QgsReadWriteContext())
        c = QgsPointDisplacementRenderer.create(elem, QgsReadWriteContext())
        self._checkProperties(c)

    def testConvert(self):
        """ test renderer conversion """

        # same type, should clone
        r = QgsPointDisplacementRenderer()
        self._setProperties(r)
        c = QgsPointDisplacementRenderer.convertFromRenderer(r)
        self._checkProperties(c)

        # test conversion from cluster renderer
        r = QgsPointClusterRenderer()
        r.setTolerance(5)
        r.setToleranceUnit(QgsUnitTypes.RenderMapUnits)
        r.setToleranceMapUnitScale(QgsMapUnitScale(5, 15))
        m = QgsMarkerSymbol()
        m.setColor(QColor(0, 255, 0))
        r.setClusterSymbol(m)
        sym1 = QgsMarkerSymbol.createSimple({'color': '#fdbf6f'})
        renderer = QgsSingleSymbolRenderer(sym1)
        r.setEmbeddedRenderer(renderer)

        # want to keep as many settings as possible when converting between cluster and displacement renderer
        d = QgsPointDisplacementRenderer.convertFromRenderer(r)
        self.assertEqual(d.tolerance(), 5)
        self.assertEqual(d.toleranceUnit(), QgsUnitTypes.RenderMapUnits)
        self.assertEqual(d.toleranceMapUnitScale(), QgsMapUnitScale(5, 15))
        self.assertEqual(d.centerSymbol().color(), QColor(0, 255, 0))
        self.assertEqual(d.embeddedRenderer().symbol().color().name(), '#fdbf6f')

    def testRenderNoCluster(self):
        layer, renderer, mapsettings = self._setUp()
        layer.renderer().setTolerance(1)
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('displacement_renderer')
        renderchecker.setControlName('expected_displacement_no_cluster')
        res = renderchecker.runTest('displacement_no_cluster')
        self.report += renderchecker.report()
        self.assertTrue(res)
        self._tearDown(layer)

    def testRenderWithin(self):
        layer, renderer, mapsettings = self._setUp()
        layer.renderer().setTolerance(10)
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('displacement_renderer')
        renderchecker.setControlName('expected_displacement_cluster')
        res = renderchecker.runTest('expected_displacement_cluster')
        self.report += renderchecker.report()
        self.assertTrue(res)
        self._tearDown(layer)

    def testRenderVariables(self):
        """ test rendering with expression variables in marker """
        layer, renderer, mapsettings = self._setUp()
        layer.renderer().setTolerance(10)

        old_marker = layer.renderer().centerSymbol().clone()

        new_marker = QgsMarkerSymbol.createSimple({'color': '#ffff00', 'size': '3', 'outline_style': 'no'})
        new_marker.symbolLayer(0).setDataDefinedProperty(QgsSymbolLayer.PropertyFillColor, QgsProperty.fromExpression('@cluster_color'))
        new_marker.symbolLayer(0).setDataDefinedProperty(QgsSymbolLayer.PropertySize, QgsProperty.fromExpression('@cluster_size*2'))
        layer.renderer().setCenterSymbol(new_marker)
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('displacement_renderer')
        renderchecker.setControlName('expected_displacement_variables')
        result = renderchecker.runTest('expected_displacement_variables')
        self.report += renderchecker.report()
        layer.renderer().setCenterSymbol(old_marker)
        self.assertTrue(result)
        self._tearDown(layer)

    def testRenderGrid(self):
        layer, renderer, mapsettings = self._setUp()
        layer.renderer().setTolerance(10)
        layer.renderer().setPlacement(QgsPointDisplacementRenderer.Grid)
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('displacement_renderer')
        renderchecker.setControlName('expected_displacement_grid')
        res = renderchecker.runTest('expected_displacement_grid')
        self.report += renderchecker.report()
        self.assertTrue(res)
        self._tearDown(layer)

    def testRenderGridAdjust(self):
        layer, renderer, mapsettings = self._setUp()
        layer.renderer().setTolerance(10)
        layer.renderer().setCircleRadiusAddition(5)
        layer.renderer().setPlacement(QgsPointDisplacementRenderer.Grid)
        layer.renderer().setCircleColor(QColor())
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('displacement_renderer')
        renderchecker.setControlName('expected_displacement_adjust_grid')
        res = renderchecker.runTest('expected_displacement_adjust_grid')
        self.report += renderchecker.report()
        self.assertTrue(res)
        self._tearDown(layer)

    def testClusterRingLabels(self):
        layer, renderer, mapsettings = self._setUp()
        layer.renderer().setTolerance(10)
        layer.renderer().setLabelAttributeName('Class')
        layer.renderer().setLabelDistanceFactor(0.35)
        f = QgsFontUtils.getStandardTestFont('Bold', 14)
        layer.renderer().setLabelFont(f)
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('displacement_renderer')
        renderchecker.setControlName('expected_displacement_cluster_ring_labels')
        res = renderchecker.runTest('expected_displacement_cluster_ring_labels')
        self.report += renderchecker.report()
        self.assertTrue(res)
        self._tearDown(layer)

    def testClusterGridLabels(self):
        layer, renderer, mapsettings = self._setUp()
        layer.renderer().setTolerance(10)
        layer.renderer().setLabelAttributeName('Class')
        layer.renderer().setLabelDistanceFactor(0.35)
        f = QgsFontUtils.getStandardTestFont('Bold', 14)
        layer.renderer().setLabelFont(f)
        layer.renderer().setPlacement(QgsPointDisplacementRenderer.Grid)
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('displacement_renderer')
        renderchecker.setControlName('expected_displacement_cluster_grid_labels')
        res = renderchecker.runTest('expected_displacement_cluster_grid_labels')
        self.report += renderchecker.report()
        self.assertTrue(res)
        self._tearDown(layer)

    def testClusterConcentricLabels(self):
        layer, renderer, mapsettings = self._setUp()
        layer.renderer().setTolerance(10)
        layer.renderer().setLabelAttributeName('Class')
        layer.renderer().setLabelDistanceFactor(0.35)
        f = QgsFontUtils.getStandardTestFont('Bold', 14)
        layer.renderer().setLabelFont(f)
        layer.renderer().setPlacement(QgsPointDisplacementRenderer.ConcentricRings)
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('displacement_renderer')
        renderchecker.setControlName('expected_displacement_cluster_concentric_labels')
        res = renderchecker.runTest('expected_displacement_cluster_concentric_labels')
        self.report += renderchecker.report()
        self.assertTrue(res)
        self._tearDown(layer)

    def testClusterRingLabelsDifferentSizes(self):
        layer, renderer, mapsettings = self._setUp()
        renderer.setEmbeddedRenderer(self._create_categorized_renderer())
        layer.renderer().setTolerance(10)
        layer.renderer().setLabelAttributeName('Class')
        layer.renderer().setLabelDistanceFactor(0.35)
        f = QgsFontUtils.getStandardTestFont('Bold', 14)
        layer.renderer().setLabelFont(f)
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('displacement_renderer')
        renderchecker.setControlName('expected_displacement_cluster_ring_labels_diff_size')
        res = renderchecker.runTest('expected_displacement_cluster_ring_labels_diff_size')
        self.report += renderchecker.report()
        self.assertTrue(res)
        self._tearDown(layer)

    def testClusterGridLabelsDifferentSizes(self):
        layer, renderer, mapsettings = self._setUp()
        renderer.setEmbeddedRenderer(self._create_categorized_renderer())
        layer.renderer().setTolerance(10)
        layer.renderer().setLabelAttributeName('Class')
        layer.renderer().setLabelDistanceFactor(0.35)
        f = QgsFontUtils.getStandardTestFont('Bold', 14)
        layer.renderer().setLabelFont(f)
        layer.renderer().setPlacement(QgsPointDisplacementRenderer.Grid)
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('displacement_renderer')
        renderchecker.setControlName('expected_displacement_cluster_grid_labels_diff_size')
        res = renderchecker.runTest('expected_displacement_cluster_grid_labels_diff_size')
        self.report += renderchecker.report()
        self.assertTrue(res)
        self._tearDown(layer)

    def testClusterConcentricLabelsDifferentSizes(self):
        layer, renderer, mapsettings = self._setUp()
        renderer.setEmbeddedRenderer(self._create_categorized_renderer())
        layer.renderer().setTolerance(10)
        layer.renderer().setLabelAttributeName('Class')
        layer.renderer().setLabelDistanceFactor(0.35)
        f = QgsFontUtils.getStandardTestFont('Bold', 14)
        layer.renderer().setLabelFont(f)
        layer.renderer().setPlacement(QgsPointDisplacementRenderer.ConcentricRings)
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('displacement_renderer')
        renderchecker.setControlName('expected_displacement_cluster_concentric_labels_diff_size')
        res = renderchecker.runTest('expected_displacement_cluster_concentric_labels_diff_size')
        self.report += renderchecker.report()
        self.assertTrue(res)
        self._tearDown(layer)

    def testClusterRingLabelsDifferentSizesFarther(self):
        layer, renderer, mapsettings = self._setUp()
        renderer.setEmbeddedRenderer(self._create_categorized_renderer())
        layer.renderer().setTolerance(10)
        layer.renderer().setLabelAttributeName('Class')
        layer.renderer().setLabelDistanceFactor(1)
        f = QgsFontUtils.getStandardTestFont('Bold', 14)
        layer.renderer().setLabelFont(f)
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('displacement_renderer')
        renderchecker.setControlName('expected_displacement_cluster_ring_labels_diff_size_farther')
        res = renderchecker.runTest('expected_displacement_cluster_ring_labels_diff_size_farther')
        self.report += renderchecker.report()
        self.assertTrue(res)
        self._tearDown(layer)

    def testClusterGridLabelsDifferentSizesFarther(self):
        layer, renderer, mapsettings = self._setUp()
        renderer.setEmbeddedRenderer(self._create_categorized_renderer())
        layer.renderer().setTolerance(10)
        layer.renderer().setLabelAttributeName('Class')
        layer.renderer().setLabelDistanceFactor(1)
        layer.renderer().setPlacement(QgsPointDisplacementRenderer.Grid)
        f = QgsFontUtils.getStandardTestFont('Bold', 14)
        layer.renderer().setLabelFont(f)
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('displacement_renderer')
        renderchecker.setControlName('expected_displacement_cluster_grid_labels_diff_size_farther')
        res = renderchecker.runTest('expected_displacement_cluster_grid_labels_diff_size_farther')
        self.report += renderchecker.report()
        self.assertTrue(res)
        self._tearDown(layer)

    def testClusterConcentricLabelsDifferentSizesFarther(self):
        layer, renderer, mapsettings = self._setUp()
        renderer.setEmbeddedRenderer(self._create_categorized_renderer())
        layer.renderer().setTolerance(10)
        layer.renderer().setLabelAttributeName('Class')
        layer.renderer().setLabelDistanceFactor(1)
        f = QgsFontUtils.getStandardTestFont('Bold', 14)
        layer.renderer().setLabelFont(f)
        layer.renderer().setPlacement(QgsPointDisplacementRenderer.ConcentricRings)
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('displacement_renderer')
        renderchecker.setControlName('expected_displacement_cluster_concentric_labels_diff_size_farther')
        res = renderchecker.runTest('expected_displacement_cluster_concentric_labels_diff_size_farther')
        self.report += renderchecker.report()
        self.assertTrue(res)
        self._tearDown(layer)


if __name__ == '__main__':
    unittest.main()
