# -*- coding: utf-8 -*-

"""
***************************************************************************
    test_qgssinglesymbolrenderer.py
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

From build dir, run: ctest -R PyQgsSingleSymbolRenderer -V

"""

__author__ = 'Matthias Kuhn'
__date__ = 'December 2015'
__copyright__ = '(C) 2015, Matthias Kuhn'

import qgis  # NOQA

import os

from qgis.PyQt.QtCore import QSize

from qgis.core import (QgsVectorLayer,
                       QgsProject,
                       QgsRectangle,
                       QgsMultiRenderChecker,
                       QgsSingleSymbolRenderer,
                       QgsFillSymbol,
                       QgsFeatureRequest,
                       QgsRenderContext
                       )
from qgis.testing import unittest
from qgis.testing.mocked import get_iface
from utilities import unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()


class TestQgsSingleSymbolRenderer(unittest.TestCase):

    def setUp(self):
        self.iface = get_iface()
        myShpFile = os.path.join(TEST_DATA_DIR, 'polys_overlapping.shp')
        layer = QgsVectorLayer(myShpFile, 'Polys', 'ogr')
        QgsProject.instance().addMapLayer(layer)

        # Create rulebased style
        sym1 = QgsFillSymbol.createSimple({'color': '#fdbf6f', 'outline_color': 'black'})

        self.renderer = QgsSingleSymbolRenderer(sym1)
        layer.setRenderer(self.renderer)

        rendered_layers = [layer]
        self.mapsettings = self.iface.mapCanvas().mapSettings()
        self.mapsettings.setOutputSize(QSize(400, 400))
        self.mapsettings.setOutputDpi(96)
        self.mapsettings.setExtent(QgsRectangle(-163, 22, -70, 52))
        self.mapsettings.setLayers(rendered_layers)

    def testOrderBy(self):
        self.renderer.setOrderBy(QgsFeatureRequest.OrderBy([QgsFeatureRequest.OrderByClause('Value', False)]))
        self.renderer.setOrderByEnabled(True)

        # Setup rendering check
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(self.mapsettings)
        renderchecker.setControlName('expected_singlesymbol_orderby')
        self.assertTrue(renderchecker.runTest('singlesymbol_orderby'))

        # disable order by and retest
        self.renderer.setOrderByEnabled(False)
        self.assertTrue(renderchecker.runTest('single'))

    def testUsedAttributes(self):
        ctx = QgsRenderContext.fromMapSettings(self.mapsettings)

        self.assertCountEqual(self.renderer.usedAttributes(ctx), {})

    def test_legend_key_to_expression(self):
        sym1 = QgsFillSymbol.createSimple({'color': '#fdbf6f', 'outline_color': 'black'})
        renderer = QgsSingleSymbolRenderer(sym1)

        exp, ok = renderer.legendKeyToExpression('0', None)
        self.assertTrue(ok)
        self.assertEqual(exp, 'TRUE')

        exp, ok = renderer.legendKeyToExpression('xxxx', None)
        self.assertFalse(ok)


if __name__ == '__main__':
    unittest.main()
