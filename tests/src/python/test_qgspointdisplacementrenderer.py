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
from qgis.PyQt.QtCore import QSize
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
        myShpFile = os.path.join(TEST_DATA_DIR, 'points.shp')
        self.layer = QgsVectorLayer(myShpFile, 'Points', 'ogr')
        QgsProject.instance().addMapLayer(self.layer)

        self.renderer = QgsPointDisplacementRenderer()
        sym1 = QgsMarkerSymbol.createSimple({'color': '#ff00ff', 'size': '3', 'outline_style': 'no'})
        renderer = QgsSingleSymbolRenderer(sym1)
        self.renderer.setEmbeddedRenderer(renderer)
        self.renderer.setCircleRadiusAddition(2)
        self.renderer.setCircleWidth(1)
        self.renderer.setCircleColor(QColor(0, 0, 0))
        self.renderer.setCenterSymbol(QgsMarkerSymbol.createSimple({'color': '#ffff00', 'size': '3', 'outline_style': 'no'}))
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
        self.layer.renderer().setTolerance(1)
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(self.mapsettings)
        renderchecker.setControlPathPrefix('displacement_renderer')
        renderchecker.setControlName('expected_displacement_no_cluster')
        self.assertTrue(renderchecker.runTest('displacement_no_cluster'))

    def testRenderWithin(self):
        self.layer.renderer().setTolerance(10)
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(self.mapsettings)
        renderchecker.setControlPathPrefix('displacement_renderer')
        renderchecker.setControlName('expected_displacement_cluster')
        self.assertTrue(renderchecker.runTest('expected_displacement_cluster'))

    def testRenderVariables(self):
        """ test rendering with expression variables in marker """
        self.layer.renderer().setTolerance(10)

        old_marker = self.layer.renderer().centerSymbol().clone()

        new_marker = QgsMarkerSymbol.createSimple({'color': '#ffff00', 'size': '3', 'outline_style': 'no'})
        new_marker.symbolLayer(0).setDataDefinedProperty(QgsSymbolLayer.PropertyFillColor, QgsProperty.fromExpression('@cluster_color'))
        new_marker.symbolLayer(0).setDataDefinedProperty(QgsSymbolLayer.PropertySize, QgsProperty.fromExpression('@cluster_size*2'))
        self.layer.renderer().setCenterSymbol(new_marker)
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(self.mapsettings)
        renderchecker.setControlPathPrefix('displacement_renderer')
        renderchecker.setControlName('expected_displacement_variables')
        result = renderchecker.runTest('expected_displacement_variables')
        self.layer.renderer().setCenterSymbol(old_marker)
        self.assertTrue(result)

    def testRenderGrid(self):
        self.layer.renderer().setTolerance(10)
        self.layer.renderer().setPlacement(QgsPointDisplacementRenderer.Grid)
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(self.mapsettings)
        renderchecker.setControlPathPrefix('displacement_renderer')
        renderchecker.setControlName('expected_displacement_grid')
        self.assertTrue(renderchecker.runTest('expected_displacement_grid'))

    def testRenderGridAdjust(self):
        self.layer.renderer().setTolerance(10)
        self.layer.renderer().setCircleRadiusAddition(5)
        self.layer.renderer().setPlacement(QgsPointDisplacementRenderer.Grid)
        self.layer.renderer().setCircleColor(QColor())
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(self.mapsettings)
        renderchecker.setControlPathPrefix('displacement_renderer')
        renderchecker.setControlName('expected_displacement_adjust_grid')
        self.assertTrue(renderchecker.runTest('expected_displacement_adjust_grid'))


if __name__ == '__main__':
    unittest.main()
