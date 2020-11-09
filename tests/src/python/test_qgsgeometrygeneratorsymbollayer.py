# -*- coding: utf-8 -*-

"""
***************************************************************************
    test_qgsgeometrygeneratorsymbollayer.py
    ---------------------
    Date                 : December 2015
    Copyright            : (C) 2015 by Matthias Kuhn
    Email                : matthias at opengis dot ch
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Matthias Kuhn'
__date__ = 'December 2015'
__copyright__ = '(C) 2015, Matthias Kuhn'

import qgis  # NOQA

import os

from qgis.PyQt.QtCore import QSize, QDir
from qgis.PyQt.QtGui import QColor
from qgis.core import (
    QgsVectorLayer,
    QgsSingleSymbolRenderer,
    QgsFillSymbol,
    QgsLineSymbol,
    QgsMarkerSymbol,
    QgsProject,
    QgsRectangle,
    QgsGeometryGeneratorSymbolLayer,
    QgsSymbol,
    QgsMultiRenderChecker,
    QgsMapSettings
)

from qgis.testing import start_app, unittest
from qgis.testing.mocked import get_iface
from utilities import unitTestDataPath

# Convenience instances in case you may need them
# not used in this test
start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsGeometryGeneratorSymbolLayerV2(unittest.TestCase):

    def setUp(self):
        self.iface = get_iface()

        polys_shp = os.path.join(TEST_DATA_DIR, 'polys.shp')
        points_shp = os.path.join(TEST_DATA_DIR, 'points.shp')
        lines_shp = os.path.join(TEST_DATA_DIR, 'lines.shp')
        self.polys_layer = QgsVectorLayer(polys_shp, 'Polygons', 'ogr')
        self.points_layer = QgsVectorLayer(points_shp, 'Points', 'ogr')
        self.lines_layer = QgsVectorLayer(lines_shp, 'Lines', 'ogr')
        QgsProject.instance().addMapLayer(self.polys_layer)
        QgsProject.instance().addMapLayer(self.lines_layer)
        QgsProject.instance().addMapLayer(self.points_layer)

        # Create style
        sym1 = QgsFillSymbol.createSimple({'color': '#fdbf6f', 'outline_color': 'black'})
        sym2 = QgsLineSymbol.createSimple({'color': '#fdbf6f'})
        sym3 = QgsMarkerSymbol.createSimple({'color': '#fdbf6f', 'outline_color': 'black'})

        self.polys_layer.setRenderer(QgsSingleSymbolRenderer(sym1))
        self.lines_layer.setRenderer(QgsSingleSymbolRenderer(sym2))
        self.points_layer.setRenderer(QgsSingleSymbolRenderer(sym3))

        self.mapsettings = self.iface.mapCanvas().mapSettings()
        self.mapsettings.setOutputSize(QSize(400, 400))
        self.mapsettings.setOutputDpi(96)
        self.mapsettings.setExtent(QgsRectangle(-133, 22, -70, 52))

        self.report = "<h1>Python QgsGeometryGeneratorSymbolLayer Tests</h1>\n"

    def tearDown(self):
        QgsProject.instance().removeAllMapLayers()
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(self.report)

    def test_marker(self):
        sym = self.polys_layer.renderer().symbol()
        sym_layer = QgsGeometryGeneratorSymbolLayer.create({'geometryModifier': 'centroid($geometry)'})
        sym_layer.setSymbolType(QgsSymbol.Marker)
        sym_layer.subSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        sym.changeSymbolLayer(0, sym_layer)

        rendered_layers = [self.polys_layer]
        self.mapsettings.setLayers(rendered_layers)

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(self.mapsettings)
        renderchecker.setControlName('expected_geometrygenerator_marker')
        res = renderchecker.runTest('geometrygenerator_marker')
        self.report += renderchecker.report()
        self.assertTrue(res)

    def test_mixed(self):
        sym = self.polys_layer.renderer().symbol()

        buffer_layer = QgsGeometryGeneratorSymbolLayer.create({'geometryModifier': 'buffer($geometry, "value"/15)', 'outline_color': 'black'})
        buffer_layer.setSymbolType(QgsSymbol.Fill)
        buffer_layer.subSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        self.assertIsNotNone(buffer_layer.subSymbol())
        sym.appendSymbolLayer(buffer_layer)
        marker_layer = QgsGeometryGeneratorSymbolLayer.create({'geometryModifier': 'centroid($geometry)', 'outline_color': 'black'})
        marker_layer.setSymbolType(QgsSymbol.Marker)
        marker_layer.subSymbol().symbolLayer(0).setStrokeColor(QColor(0, 0, 0))
        sym.appendSymbolLayer(marker_layer)

        rendered_layers = [self.polys_layer]
        self.mapsettings.setLayers(rendered_layers)

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(self.mapsettings)
        renderchecker.setControlName('expected_geometrygenerator_mixed')
        res = renderchecker.runTest('geometrygenerator_mixed')
        self.report += renderchecker.report()
        self.assertTrue(res)

    def test_buffer_lines(self):
        sym = self.lines_layer.renderer().symbol()

        buffer_layer = QgsGeometryGeneratorSymbolLayer.create({'geometryModifier': 'buffer($geometry, "value"/15)', 'outline_color': 'black'})
        buffer_layer.setSymbolType(QgsSymbol.Fill)
        self.assertIsNotNone(buffer_layer.subSymbol())
        sym.appendSymbolLayer(buffer_layer)

        rendered_layers = [self.lines_layer]
        self.mapsettings.setLayers(rendered_layers)

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(self.mapsettings)
        renderchecker.setControlName('expected_geometrygenerator_buffer_lines')
        res = renderchecker.runTest('geometrygenerator_buffer_lines')
        self.report += renderchecker.report()
        self.assertTrue(res)

    def test_buffer_points(self):
        sym = self.points_layer.renderer().symbol()

        buffer_layer = QgsGeometryGeneratorSymbolLayer.create({'geometryModifier': 'buffer($geometry, "staff"/15)', 'outline_color': 'black'})
        buffer_layer.setSymbolType(QgsSymbol.Fill)
        self.assertIsNotNone(buffer_layer.subSymbol())
        sym.appendSymbolLayer(buffer_layer)

        rendered_layers = [self.points_layer]
        self.mapsettings.setLayers(rendered_layers)

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(self.mapsettings)
        renderchecker.setControlName('expected_geometrygenerator_buffer_points')
        res = renderchecker.runTest('geometrygenerator_buffer_points')
        self.report += renderchecker.report()
        self.assertTrue(res)

    def test_multi_poly_opacity(self):
        # test that multi-type features are only rendered once

        multipoly = QgsVectorLayer(os.path.join(TEST_DATA_DIR, 'multipatch.shp'), 'Polygons', 'ogr')

        sym = QgsFillSymbol.createSimple({'color': '#77fdbf6f', 'outline_color': 'black'})

        buffer_layer = QgsGeometryGeneratorSymbolLayer.create({'geometryModifier': 'buffer($geometry, -0.01)', 'outline_color': 'black'})
        buffer_layer.setSymbolType(QgsSymbol.Fill)
        buffer_layer.setSubSymbol(sym)
        geom_symbol = QgsFillSymbol()
        geom_symbol.changeSymbolLayer(0, buffer_layer)
        multipoly.renderer().setSymbol(geom_symbol)

        mapsettings = QgsMapSettings(self.mapsettings)
        mapsettings.setExtent(multipoly.extent())
        mapsettings.setLayers([multipoly])

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlName('expected_geometrygenerator_opacity')
        res = renderchecker.runTest('geometrygenerator_opacity')
        self.report += renderchecker.report()
        self.assertTrue(res)


if __name__ == '__main__':
    unittest.main()
