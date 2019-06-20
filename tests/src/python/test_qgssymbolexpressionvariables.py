# -*- coding: utf-8 -*-

"""
***************************************************************************
    test_qgssymbolexpressionvariables.py
    ---------------------
    Date                 : January 2016
    Copyright            : (C) 2016 by Matthias Kuhn
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
__date__ = 'January 2016'
__copyright__ = '(C) 2016, Matthiasd Kuhn'

import qgis  # NOQA

import os

from qgis.PyQt.QtCore import QSize

from qgis.core import (
    QgsVectorLayer,
    QgsProject,
    QgsRectangle,
    QgsMultiRenderChecker,
    QgsSingleSymbolRenderer,
    QgsFillSymbol,
    QgsRenderContext,
    QgsProperty,
    QgsSymbolLayer
)

from qgis.testing import unittest, start_app
from qgis.testing.mocked import get_iface
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsSymbolExpressionVariables(unittest.TestCase):

    def setUp(self):
        myShpFile = os.path.join(TEST_DATA_DIR, 'polys.shp')
        self.layer = QgsVectorLayer(myShpFile, 'Polys', 'ogr')
        QgsProject.instance().addMapLayer(self.layer)

        self.iface = get_iface()
        rendered_layers = [self.layer]
        self.mapsettings = self.iface.mapCanvas().mapSettings()
        self.mapsettings.setOutputSize(QSize(400, 400))
        self.mapsettings.setOutputDpi(96)
        self.mapsettings.setExtent(QgsRectangle(-163, 22, -70, 52))
        self.mapsettings.setLayers(rendered_layers)

    def tearDown(self):
        QgsProject.instance().removeAllMapLayers()

    def testPartNum(self):
        # Create rulebased style
        sym1 = QgsFillSymbol.createSimple({'color': '#fdbf6f', 'outline_color': 'black'})

        renderer = QgsSingleSymbolRenderer(sym1)
        renderer.symbols(QgsRenderContext())[0].symbolLayers()[0].setDataDefinedProperty(QgsSymbolLayer.PropertyFillColor, QgsProperty.fromExpression('color_rgb( (@geometry_part_num - 1) * 200, 0, 0 )'))
        self.layer.setRenderer(renderer)

        # Setup rendering check
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(self.mapsettings)
        renderchecker.setControlName('expected_geometry_part_num')
        result = renderchecker.runTest('part_geometry_part_num')

        self.assertTrue(result)

    def testPartCount(self):
        # Create rulebased style
        sym1 = QgsFillSymbol.createSimple({'color': '#fdbf6f', 'outline_color': 'black'})

        renderer = QgsSingleSymbolRenderer(sym1)
        renderer.symbols(QgsRenderContext())[0].symbolLayers()[0].setDataDefinedProperty(QgsSymbolLayer.PropertyFillColor, QgsProperty.fromExpression('color_rgb( (@geometry_part_count - 1) * 200, 0, 0 )'))
        self.layer.setRenderer(renderer)

        # Setup rendering check
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(self.mapsettings)
        renderchecker.setControlName('expected_geometry_part_count')
        result = renderchecker.runTest('part_geometry_part_count')

        self.assertTrue(result)

    def testSymbolColor(self):
        # Create rulebased style
        sym1 = QgsFillSymbol.createSimple({'color': '#ff0000', 'outline_color': 'black'})

        renderer = QgsSingleSymbolRenderer(sym1)
        renderer.symbols(QgsRenderContext())[0].symbolLayers()[0].setDataDefinedProperty(QgsSymbolLayer.PropertyFillColor, QgsProperty.fromExpression('set_color_part( @symbol_color, \'value\', "Value" * 4)'))
        self.layer.setRenderer(renderer)

        # Setup rendering check
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(self.mapsettings)
        renderchecker.setControlName('expected_symbol_color_variable')
        result = renderchecker.runTest('symbol_color_variable', 50)

        self.assertTrue(result)


if __name__ == '__main__':
    unittest.main()
