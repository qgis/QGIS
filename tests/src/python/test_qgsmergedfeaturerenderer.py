# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsMergedFeatureRenderer

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '30/12/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

import os

from qgis.PyQt.QtCore import QSize, QDir, Qt
from qgis.PyQt.QtGui import QColor

from qgis.core import (QgsRenderChecker,
                       QgsMapSettings,
                       QgsVectorLayer,
                       QgsMergedFeatureRenderer,
                       QgsSingleSymbolRenderer,
                       QgsFillSymbol,
                       QgsSimpleFillSymbolLayer,
                       QgsCategorizedSymbolRenderer,
                       QgsRendererCategory,
                       QgsSimpleLineSymbolLayer,
                       QgsMarkerLineSymbolLayer,
                       QgsLineSymbol,
                       QgsTemplatedLineSymbolLayerBase,
                       QgsMarkerSymbol,
                       QgsMarkerSymbolLayer
                       )
from qgis.testing import unittest
from utilities import unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()


class TestQgsMergedFeatureRenderer(unittest.TestCase):

    def setUp(self):
        self.report = "<h1>Python QgsMergedFeatureRenderer Tests</h1>\n"

    def tearDown(self):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(self.report)

    def testSinglePolys(self):
        source = QgsVectorLayer(os.path.join(TEST_DATA_DIR, 'polys_overlapping.shp'))
        self.assertTrue(source.isValid())
        map_settings = QgsMapSettings()
        map_settings.setExtent(source.extent())
        map_settings.setDestinationCrs(source.crs())
        map_settings.setLayers([source])

        layer = QgsSimpleFillSymbolLayer()
        layer.setStrokeColor(QColor(0, 0, 0))
        layer.setStrokeWidth(1)
        layer.setColor(QColor(200, 250, 50))
        symbol = QgsFillSymbol([layer])

        sub_renderer = QgsSingleSymbolRenderer(symbol)
        source.setRenderer(QgsMergedFeatureRenderer(sub_renderer))

        self.assertTrue(self.imageCheck('single_subrenderer', 'single_subrenderer', map_settings))

    def testCategorizedPolys(self):
        source = QgsVectorLayer(os.path.join(TEST_DATA_DIR, 'polys_overlapping_with_cat.shp'))
        self.assertTrue(source.isValid())
        map_settings = QgsMapSettings()
        map_settings.setExtent(source.extent())
        map_settings.setDestinationCrs(source.crs())
        map_settings.setLayers([source])

        layer = QgsSimpleFillSymbolLayer()
        layer.setStrokeColor(QColor(0, 0, 0))
        layer.setStrokeWidth(1)
        layer.setColor(QColor(200, 250, 50, 150))
        symbol1 = QgsFillSymbol()
        symbol1.changeSymbolLayer(0, layer)

        layer = QgsSimpleFillSymbolLayer()
        layer.setStrokeColor(QColor(0, 0, 0))
        layer.setStrokeWidth(1)
        layer.setColor(QColor(50, 250, 200, 150))
        symbol2 = QgsFillSymbol()
        symbol2.changeSymbolLayer(0, layer)

        sub_renderer = QgsCategorizedSymbolRenderer('cat', [QgsRendererCategory('cat1', symbol1, 'cat1'),
                                                            QgsRendererCategory('cat2', symbol2, 'cat2')
                                                            ])
        source.setRenderer(QgsMergedFeatureRenderer(sub_renderer))

        self.assertTrue(self.imageCheck('polys_categorizedrenderer', 'polys_categorizedrenderer', map_settings))

    def testSingleLines(self):
        source = QgsVectorLayer(os.path.join(TEST_DATA_DIR, 'lines_touching.shp'))
        self.assertTrue(source.isValid())
        map_settings = QgsMapSettings()
        map_settings.setExtent(source.extent().buffered(2))
        map_settings.setDestinationCrs(source.crs())
        map_settings.setLayers([source])

        layer = QgsSimpleLineSymbolLayer()
        layer.setColor(QColor(0, 0, 0))
        layer.setWidth(1)
        symbol = QgsLineSymbol([layer])

        layer2 = QgsMarkerLineSymbolLayer()
        layer2.setPlacement(QgsTemplatedLineSymbolLayerBase.FirstVertex)
        marker = QgsMarkerSymbol.createSimple({'size': '4', 'color': '255,0,0', 'outline_style': 'no'})
        layer2.setSubSymbol(marker)
        symbol.appendSymbolLayer(layer2)

        sub_renderer = QgsSingleSymbolRenderer(symbol)
        source.setRenderer(QgsMergedFeatureRenderer(sub_renderer))

        self.assertTrue(self.imageCheck('lines_single_subrenderer', 'lines_single_subrenderer', map_settings))

    def testLinesCategorized(self):
        source = QgsVectorLayer(os.path.join(TEST_DATA_DIR, 'lines_touching.shp'))
        self.assertTrue(source.isValid())
        map_settings = QgsMapSettings()
        map_settings.setExtent(source.extent().buffered(2))
        map_settings.setDestinationCrs(source.crs())
        map_settings.setLayers([source])

        layer = QgsSimpleLineSymbolLayer()
        layer.setColor(QColor(0, 0, 0))
        layer.setWidth(1)
        symbol1 = QgsLineSymbol()
        symbol1.changeSymbolLayer(0, layer.clone())

        layer2 = QgsMarkerLineSymbolLayer()
        layer2.setPlacement(QgsTemplatedLineSymbolLayerBase.FirstVertex)
        marker = QgsMarkerSymbol.createSimple({'size': '4', 'color': '255,0,0', 'outline_style': 'no'})
        layer2.setSubSymbol(marker)
        symbol1.appendSymbolLayer(layer2)

        symbol2 = QgsLineSymbol()
        symbol2.changeSymbolLayer(0, layer.clone())
        layer2 = QgsMarkerLineSymbolLayer()
        layer2.setPlacement(QgsTemplatedLineSymbolLayerBase.FirstVertex)
        marker = QgsMarkerSymbol.createSimple({'size': '4', 'color': '0,255,0', 'outline_style': 'no'})
        layer2.setSubSymbol(marker)
        symbol2.appendSymbolLayer(layer2)

        sub_renderer = QgsCategorizedSymbolRenderer('cat', [QgsRendererCategory('cat1', symbol1, 'cat1'),
                                                            QgsRendererCategory('cat2', symbol2, 'cat2')
                                                            ])

        source.setRenderer(QgsMergedFeatureRenderer(sub_renderer))

        self.assertTrue(self.imageCheck('lines_categorized_subrenderer', 'lines_categorized_subrenderer', map_settings))

    def test_legend_key_to_expression(self):
        sym1 = QgsFillSymbol.createSimple({'color': '#fdbf6f', 'outline_color': 'black'})
        sub_renderer = QgsSingleSymbolRenderer(sym1)

        renderer = QgsMergedFeatureRenderer(sub_renderer)

        exp, ok = renderer.legendKeyToExpression('0', None)
        self.assertTrue(ok)
        self.assertEqual(exp, 'TRUE')

        exp, ok = renderer.legendKeyToExpression('xxxx', None)
        self.assertFalse(ok)

    def imageCheck(self, name, reference_image, map_settings):
        map_settings.setOutputDpi(96)
        self.report += "<h2>Render {}</h2>\n".format(name)

        checker = QgsRenderChecker()
        checker.setControlPathPrefix("mergedfeaturerenderer")
        checker.setControlName("expected_" + reference_image)
        checker.setMapSettings(map_settings)
        checker.setColorTolerance(2)
        result = checker.runTest(name, 20)
        self.report += checker.report()
        print(self.report)
        return result


if __name__ == '__main__':
    unittest.main()
