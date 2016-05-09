# -*- coding: utf-8 -*-

"""
***************************************************************************
    test_qgsnullsymbolrenderer.py
    -----------------------------
    Date                 : April 2016
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
__date__ = 'April 2016'
__copyright__ = '(C) 2016, Nyall Dawson'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

import os

from qgis.PyQt.QtCore import QSize

from qgis.core import (QgsVectorLayer,
                       QgsMapLayerRegistry,
                       QgsRectangle,
                       QgsMultiRenderChecker,
                       QgsNullSymbolRenderer,
                       QgsFeatureRequest
                       )
from qgis.testing import start_app, unittest
from qgis.testing.mocked import get_iface
from utilities import unitTestDataPath

# Convenience instances in case you may need them
# not used in this test
start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsNullSymbolRenderer(unittest.TestCase):

    def setUp(self):
        self.iface = get_iface()
        myShpFile = os.path.join(TEST_DATA_DIR, 'polys.shp')
        self.layer = QgsVectorLayer(myShpFile, 'Polys', 'ogr')
        QgsMapLayerRegistry.instance().addMapLayer(self.layer)

        self.renderer = QgsNullSymbolRenderer()
        self.layer.setRendererV2(self.renderer)

        rendered_layers = [self.layer.id()]
        self.mapsettings = self.iface.mapCanvas().mapSettings()
        self.mapsettings.setOutputSize(QSize(400, 400))
        self.mapsettings.setOutputDpi(96)
        self.mapsettings.setExtent(QgsRectangle(-163, 22, -70, 52))
        self.mapsettings.setLayers(rendered_layers)

    def tearDown(self):
        QgsMapLayerRegistry.instance().removeAllMapLayers()

    def testRender(self):
        # test no features are rendered
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(self.mapsettings)
        renderchecker.setControlPathPrefix('null_renderer')
        renderchecker.setControlName('expected_nullrenderer_render')
        result = renderchecker.runTest('nullrenderer_render')
        assert result

    def testSelected(self):
        # select a feature and render
        self.layer.select([1, 2, 3])
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(self.mapsettings)
        renderchecker.setControlPathPrefix('null_renderer')
        renderchecker.setControlName('expected_nullrenderer_selected')
        result = renderchecker.runTest('nullrenderer_selected')
        assert result


if __name__ == '__main__':
    unittest.main()
