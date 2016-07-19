# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsComposerLegend.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2016 by Nyall Dawson'
__date__ = '13/07/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.PyQt.QtCore import QRectF
from qgis.PyQt.QtGui import QColor

from qgis.core import (QgsComposerLegend,
                       QgsComposerMap,
                       QgsComposition,
                       QgsMapSettings,
                       QgsVectorLayer,
                       QgsMapLayerRegistry,
                       QgsMarkerSymbolV2,
                       QgsSingleSymbolRendererV2
                       )
from qgis.testing import (start_app,
                          unittest
                          )
from utilities import unitTestDataPath
from qgscompositionchecker import QgsCompositionChecker
import os

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsComposerLegend(unittest.TestCase):

    def __init__(self, methodName):
        """Run once on class initialization."""
        unittest.TestCase.__init__(self, methodName)
        point_path = os.path.join(TEST_DATA_DIR, 'points.shp')
        self.point_layer = QgsVectorLayer(point_path, 'points', 'ogr')
        QgsMapLayerRegistry.instance().addMapLayers([self.point_layer])

    def testInitialSizeSymbolMapUnits(self):
        """Test initial size of legend with a symbol size in map units"""

        marker_symbol = QgsMarkerSymbolV2.createSimple({'color': '#ff0000', 'outline_style': 'no', 'size': '5', 'size_unit': 'MapUnit'})

        self.point_layer.setRendererV2(QgsSingleSymbolRendererV2(marker_symbol))

        s = QgsMapSettings()
        s.setLayers([self.point_layer.id()])
        s.setCrsTransformEnabled(False)
        composition = QgsComposition(s)
        composition.setPaperSize(297, 210)

        composer_map = QgsComposerMap(composition, 20, 20, 80, 80)
        composer_map.setFrameEnabled(True)
        composition.addComposerMap(composer_map)
        composer_map.setNewExtent(self.point_layer.extent())

        legend = QgsComposerLegend(composition)
        legend.setSceneRect(QRectF(120, 20, 80, 80))
        legend.setFrameEnabled(True)
        legend.setFrameOutlineWidth(2)
        legend.setBackgroundColor(QColor(200, 200, 200))
        legend.setTitle('')
        composition.addComposerLegend(legend)
        legend.setComposerMap(composer_map)

        checker = QgsCompositionChecker(
            'composer_legend_mapunits', composition)
        checker.setControlPathPrefix("composer_legend")
        result, message = checker.testComposition()
        self.assertTrue(result, message)


if __name__ == '__main__':
    unittest.main()
