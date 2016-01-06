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
"""

__author__ = 'Matthias Kuhn'
__date__ = 'December 2015'
__copyright__ = '(C) 2015, Matthiasd Kuhn'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis
import os

from PyQt4.QtCore import QSize

from qgis.core import (QgsVectorLayer,
                       QgsMapLayerRegistry,
                       QgsRectangle,
                       QgsMultiRenderChecker,
                       QgsSingleSymbolRendererV2,
                       QgsFillSymbolV2,
                       QgsMarkerSymbolV2,
                       QgsRendererCategoryV2,
                       QgsCategorizedSymbolRendererV2,
                       QgsGraduatedSymbolRendererV2,
                       QgsRendererRangeV2,
                       QgsFeatureRequest
                       )
from qgis.testing import (start_app,
                          unittest
                          )

from qgis.testing.mocked import get_iface

from utilities import unitTestDataPath

# Convenience instances in case you may need them
# not used in this test
start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsSingleSymbolRenderer(unittest.TestCase):

    def setUp(self):
        self.iface = get_iface()
        myShpFile = os.path.join(TEST_DATA_DIR, 'polys_overlapping.shp')
        layer = QgsVectorLayer(myShpFile, 'Polys', 'ogr')
        QgsMapLayerRegistry.instance().addMapLayer(layer)

        # Create rulebased style
        sym1 = QgsFillSymbolV2.createSimple({'color': '#fdbf6f'})

        self.renderer = QgsSingleSymbolRendererV2(sym1)
        layer.setRendererV2(self.renderer)

        rendered_layers = [layer.id()]
        self.mapsettings = self.iface.mapCanvas().mapSettings()
        self.mapsettings.setOutputSize(QSize(400, 400))
        self.mapsettings.setOutputDpi(96)
        self.mapsettings.setExtent(QgsRectangle(-163, 22, -70, 52))
        self.mapsettings.setLayers(rendered_layers)

    def tearDown(self):
        QgsMapLayerRegistry.instance().removeAllMapLayers()

    def testOrderBy(self):
        self.renderer.setOrderBy(QgsFeatureRequest.OrderBy([QgsFeatureRequest.OrderByClause('Value', False)]))

        # Setup rendering check
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(self.mapsettings)
        renderchecker.setControlName('expected_singlesymbol_orderby')
        result = renderchecker.runTest('singlesymbol_orderby')

        assert result

if __name__ == '__main__':
    unittest.main()
