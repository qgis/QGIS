# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsRasterLayerRenderer

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2020-06'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

import os

from qgis.PyQt.QtCore import QSize, QDir

from qgis.core import (QgsRasterLayer,
                       QgsMapClippingRegion,
                       QgsRectangle,
                       QgsMultiRenderChecker,
                       QgsGeometry,
                       QgsSingleSymbolRenderer,
                       QgsMapSettings,
                       QgsFillSymbol,
                       QgsCoordinateReferenceSystem
                       )
from qgis.testing import start_app, unittest
from utilities import (unitTestDataPath)

# Convenience instances in case you may need them
# not used in this test
start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsRasterLayerRenderer(unittest.TestCase):

    def setUp(self):
        self.report = "<h1>Python QgsRasterLayerRenderer Tests</h1>\n"

    def tearDown(self):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(self.report)

    def testRenderWithPainterClipRegions(self):
        raster_layer = QgsRasterLayer(os.path.join(TEST_DATA_DIR, 'rgb256x256.png'))
        self.assertTrue(raster_layer.isValid())
        raster_layer.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        mapsettings.setExtent(QgsRectangle(0.0001451, -0.0001291, 0.0021493, -0.0021306))
        mapsettings.setLayers([raster_layer])

        region = QgsMapClippingRegion(QgsGeometry.fromWkt('Polygon ((0.00131242078273144 -0.00059281669806561, 0.00066744230712249 -0.00110186995774045, 0.00065145110524788 -0.00152830200772984, 0.00141369839460392 -0.00189076925022083, 0.00210931567614912 -0.00094195793899443, 0.00169354442740946 -0.00067810310806349, 0.00131242078273144 -0.00059281669806561))'))
        region.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.ClipPainterOnly)
        region2 = QgsMapClippingRegion(QgsGeometry.fromWkt('Polygon ((0.00067010750743492 -0.0007740503193111, 0.00064612070462302 -0.00151764120648011, 0.00153629760897587 -0.00158693641460339, 0.0014909892036645 -0.00063812510337699, 0.00106722235398754 -0.00055816909400397, 0.00067010750743492 -0.0007740503193111))'))
        region2.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.ClipToIntersection)
        mapsettings.addClippingRegion(region)
        mapsettings.addClippingRegion(region2)

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('rasterlayerrenderer')
        renderchecker.setControlName('expected_painterclip_region')
        result = renderchecker.runTest('expected_painterclip_region')
        self.report += renderchecker.report()
        self.assertTrue(result)


if __name__ == '__main__':
    unittest.main()
