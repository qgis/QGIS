# -*- coding: utf-8 -*-

"""
***************************************************************************
    test_qgspointclusterrenderer.py
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

From build dir, run: ctest -R PyQgsPointClusterRenderer -V

"""

__author__ = 'Nyall Dawson'
__date__ = 'September 2016'
__copyright__ = '(C) 2016, Nyall Dawson'

import os

from qgis.PyQt.QtCore import QSize
from qgis.PyQt.QtGui import QColor
from qgis.PyQt.QtXml import QDomDocument

from qgis.core import (QgsVectorLayer,
                       QgsProject,
                       QgsRectangle,
                       QgsMultiRenderChecker,
                       QgsPointClusterRenderer,
                       QgsUnitTypes,
                       QgsMapUnitScale,
                       QgsMarkerSymbol,
                       QgsSingleSymbolRenderer,
                       QgsReadWriteContext,
                       QgsPointDisplacementRenderer,
                       QgsMapSettings,
                       QgsProperty,
                       QgsSymbolLayer,
                       QgsRenderContext,
                       QgsFeature,
                       QgsGeometry
                       )
from qgis.testing import start_app, unittest
from utilities import (unitTestDataPath)

# Convenience instances in case you may need them
# not used in this test
start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsPointClusterRenderer(unittest.TestCase):

    def setUp(self):
        myShpFile = os.path.join(TEST_DATA_DIR, 'points.shp')
        self.layer = QgsVectorLayer(myShpFile, 'Points', 'ogr')
        QgsProject.instance().addMapLayer(self.layer)

        self.renderer = QgsPointClusterRenderer()
        sym1 = QgsMarkerSymbol.createSimple({'color': '#ff00ff', 'size': '3', 'outline_style': 'no'})
        renderer = QgsSingleSymbolRenderer(sym1)
        self.renderer.setEmbeddedRenderer(renderer)
        self.renderer.setClusterSymbol(QgsMarkerSymbol.createSimple({'color': '#ffff00', 'size': '3', 'outline_style': 'no'}))
        self.layer.setRenderer(self.renderer)

        rendered_layers = [self.layer]
        self.mapsettings = QgsMapSettings()
        self.mapsettings.setOutputSize(QSize(400, 400))
        self.mapsettings.setOutputDpi(96)
        self.mapsettings.setExtent(QgsRectangle(-123, 18, -70, 52))
        self.mapsettings.setLayers(rendered_layers)

    def tearDown(self):
        QgsProject.instance().removeAllMapLayers()

    def _setProperties(self, r):
        """ set properties for a renderer for testing with _checkProperties"""
        r.setTolerance(5)
        r.setToleranceUnit(QgsUnitTypes.RenderMapUnits)
        r.setToleranceMapUnitScale(QgsMapUnitScale(5, 15))
        m = QgsMarkerSymbol()
        m.setColor(QColor(0, 255, 0))
        r.setClusterSymbol(m)
        sym1 = QgsMarkerSymbol.createSimple({'color': '#fdbf6f'})
        renderer = QgsSingleSymbolRenderer(sym1)
        r.setEmbeddedRenderer(renderer)

    def _checkProperties(self, r):
        """ test properties of renderer against expected"""
        self.assertEqual(r.tolerance(), 5)
        self.assertEqual(r.toleranceUnit(), QgsUnitTypes.RenderMapUnits)
        self.assertEqual(r.toleranceMapUnitScale(), QgsMapUnitScale(5, 15))
        self.assertEqual(r.clusterSymbol().color(), QColor(0, 255, 0))
        self.assertEqual(r.embeddedRenderer().symbol().color().name(), '#fdbf6f')

    def testGettersSetters(self):
        """ test getters and setters """
        r = QgsPointClusterRenderer()
        self._setProperties(r)
        self._checkProperties(r)

    def testClone(self):
        """ test cloning renderer """
        r = QgsPointClusterRenderer()
        self._setProperties(r)
        c = r.clone()
        self._checkProperties(c)

    def testSaveCreate(self):
        """ test saving and recreating from XML """
        r = QgsPointClusterRenderer()
        self._setProperties(r)
        doc = QDomDocument("testdoc")
        elem = r.save(doc, QgsReadWriteContext())
        c = QgsPointClusterRenderer.create(elem, QgsReadWriteContext())
        self._checkProperties(c)

    def testConvert(self):
        """ test renderer conversion """

        # same type, should clone
        r = QgsPointClusterRenderer()
        self._setProperties(r)
        c = QgsPointClusterRenderer.convertFromRenderer(r)
        self._checkProperties(c)

        # test conversion from displacement renderer
        r = QgsPointDisplacementRenderer()
        r.setTolerance(5)
        r.setToleranceUnit(QgsUnitTypes.RenderMapUnits)
        r.setToleranceMapUnitScale(QgsMapUnitScale(5, 15))
        m = QgsMarkerSymbol()
        m.setColor(QColor(0, 255, 0))
        r.setCenterSymbol(m)
        sym1 = QgsMarkerSymbol.createSimple({'color': '#fdbf6f'})
        renderer = QgsSingleSymbolRenderer(sym1)
        r.setEmbeddedRenderer(renderer)

        # want to keep as many settings as possible when converting between cluster and displacement renderer
        d = QgsPointClusterRenderer.convertFromRenderer(r)
        self.assertEqual(d.tolerance(), 5)
        self.assertEqual(d.toleranceUnit(), QgsUnitTypes.RenderMapUnits)
        self.assertEqual(d.toleranceMapUnitScale(), QgsMapUnitScale(5, 15))
        self.assertEqual(d.clusterSymbol().color(), QColor(0, 255, 0))
        self.assertEqual(d.embeddedRenderer().symbol().color().name(), '#fdbf6f')

    def testRenderNoCluster(self):
        self.layer.renderer().setTolerance(1)
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(self.mapsettings)
        renderchecker.setControlPathPrefix('cluster_renderer')
        renderchecker.setControlName('expected_cluster_no_cluster')
        self.assertTrue(renderchecker.runTest('cluster_no_cluster'))

    def testRenderWithin(self):
        self.layer.renderer().setTolerance(10)
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(self.mapsettings)
        renderchecker.setControlPathPrefix('cluster_renderer')
        renderchecker.setControlName('expected_cluster_cluster')
        self.assertTrue(renderchecker.runTest('expected_cluster_cluster'))

    def testRenderVariables(self):
        """ test rendering with expression variables in marker """
        self.layer.renderer().setTolerance(10)

        old_marker = self.layer.renderer().clusterSymbol().clone()

        new_marker = QgsMarkerSymbol.createSimple({'color': '#ffff00', 'size': '3', 'outline_style': 'no'})
        new_marker.symbolLayer(0).setDataDefinedProperty(QgsSymbolLayer.PropertyFillColor, QgsProperty.fromExpression('@cluster_color'))
        new_marker.symbolLayer(0).setDataDefinedProperty(QgsSymbolLayer.PropertySize, QgsProperty.fromExpression('@cluster_size*2'))
        self.layer.renderer().setClusterSymbol(new_marker)
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(self.mapsettings)
        renderchecker.setControlPathPrefix('cluster_renderer')
        renderchecker.setControlName('expected_cluster_variables')
        result = renderchecker.runTest('expected_cluster_variables')
        self.layer.renderer().setClusterSymbol(old_marker)
        self.assertTrue(result)

    def testMultiPoint(self):
        """
        Test multipoint handling
        """
        layer = QgsVectorLayer('Multipoint?field=cat:string', '', 'memory')
        self.assertTrue(layer.isValid())

        f = QgsFeature(layer.fields())
        f.setAttributes(['a'])
        f.setGeometry(QgsGeometry.fromWkt('MultiPoint(5 5, 5 6, 9 9)'))
        layer.dataProvider().addFeature(f)
        f.setAttributes(['b'])
        f.setGeometry(QgsGeometry.fromWkt('MultiPoint(2 1, 2 2, 5 5)'))
        layer.dataProvider().addFeature(f)
        f.setAttributes(['c'])
        f.setGeometry(QgsGeometry.fromWkt('MultiPoint(9 1)'))
        layer.dataProvider().addFeature(f)

        renderer = QgsPointClusterRenderer()
        sym1 = QgsMarkerSymbol.createSimple({'color': '#ff00ff', 'size': '3', 'outline_style': 'no'})
        sub_renderer = QgsSingleSymbolRenderer(sym1)
        renderer.setEmbeddedRenderer(sub_renderer)
        renderer.setClusterSymbol(QgsMarkerSymbol.createSimple({'color': '#ffff00', 'size': '3', 'outline_style': 'no'}))
        renderer.setToleranceUnit(QgsUnitTypes.RenderMapUnits)
        renderer.setTolerance(2)
        layer.setRenderer(renderer)

        rendered_layers = [layer]
        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setExtent(QgsRectangle(0, 0, 10, 10))
        mapsettings.setLayers(rendered_layers)

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('cluster_renderer')
        renderchecker.setControlName('expected_cluster_multipoint')
        self.assertTrue(renderchecker.runTest('expected_cluster_multipoint'))

    def testUsedAttributes(self):
        ctx = QgsRenderContext.fromMapSettings(self.mapsettings)

        self.assertCountEqual(self.renderer.usedAttributes(ctx), {})


if __name__ == '__main__':
    unittest.main()
