# -*- coding: utf-8 -*-

"""
***************************************************************************
    test_qgsgeometrygeneratorsymbollayerv2.py
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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis
import os

from PyQt.QtCore import QSize

from qgis.core import (QgsVectorLayer,
                       QgsSingleSymbolRendererV2,
                       QgsFillSymbolV2,
                       QgsMarkerSymbolV2,
                       QgsMapLayerRegistry,
                       QgsRectangle,
                       QgsGeometryGeneratorSymbolLayerV2,
                       QgsSymbolV2,
                       QgsMultiRenderChecker
                       )
from utilities import (unitTestDataPath,
                       getQgisTestApp,
                       TestCase,
                       unittest
                       )

# Convenience instances in case you may need them
# not used in this test
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsGeometryGeneratorSymbolLayerV2(TestCase):

    def setUp(self):
        myShpFile = os.path.join(TEST_DATA_DIR, 'polys_overlapping.shp')
        layer = QgsVectorLayer(myShpFile, 'Polygons', 'ogr')
        QgsMapLayerRegistry.instance().addMapLayer(layer)

        # Create style
        sym1 = QgsFillSymbolV2.createSimple({'color': '#fdbf6f'})

        self.renderer = QgsSingleSymbolRendererV2(sym1)
        layer.setRendererV2(self.renderer)
        self.mapsettings = CANVAS.mapSettings()
        self.mapsettings.setOutputSize(QSize(400, 400))
        self.mapsettings.setOutputDpi(96)
        self.mapsettings.setExtent(QgsRectangle(-133, 22, -70, 52))

        rendered_layers = [layer.id()]
        self.mapsettings.setLayers(rendered_layers)

    def tearDown(self):
        QgsMapLayerRegistry.instance().removeAllMapLayers()

    def test_marker(self):
        sym = self.renderer.symbol()
        sym_layer = QgsGeometryGeneratorSymbolLayerV2.create({'geometryModifier': 'centroid($geometry)'})
        sym_layer.setSymbolType(QgsSymbolV2.Marker)
        sym.changeSymbolLayer(0, sym_layer)
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(self.mapsettings)
        renderchecker.setControlName('expected_geometrygenerator_marker')
        self.assertTrue(renderchecker.runTest('geometrygenerator_marker'))

    def test_mixed(self):
        sym = self.renderer.symbol()
        buffer_layer = QgsGeometryGeneratorSymbolLayerV2.create({'geometryModifier': 'buffer($geometry, "value"/15)'})
        buffer_layer.setSymbolType(QgsSymbolV2.Fill)
        buffer_layer.subSymbol()
        sym.appendSymbolLayer(buffer_layer)
        marker_layer = QgsGeometryGeneratorSymbolLayerV2.create({'geometryModifier': 'centroid($geometry)'})
        marker_layer.setSymbolType(QgsSymbolV2.Marker)
        sym.appendSymbolLayer(marker_layer)
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(self.mapsettings)
        renderchecker.setControlName('expected_geometrygenerator_mixed')
        self.assertTrue(renderchecker.runTest('geometrygenerator_mixed'))

if __name__ == '__main__':
    unittest.main()
