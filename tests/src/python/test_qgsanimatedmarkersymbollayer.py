# -*- coding: utf-8 -*-

"""
***************************************************************************
    test_qgsanimatedmarkersymbollayer.py
    ---------------------
    Date                 : April 2022
    Copyright            : (C) 2022 by Nyall Dawson
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
__date__ = 'April 2022'
__copyright__ = '(C) 2022, Nyall Dawson'

import qgis  # NOQA

import os
from utilities import unitTestDataPath

from qgis.PyQt.QtCore import QDir, Qt, QSize
from qgis.PyQt.QtGui import QImage, QColor, QPainter
from qgis.PyQt.QtXml import QDomDocument

from qgis.core import (Qgis,
                       QgsGeometry,
                       QgsFillSymbol,
                       QgsRenderContext,
                       QgsFeature,
                       QgsMapSettings,
                       QgsRenderChecker,
                       QgsVectorLayer,
                       QgsReadWriteContext,
                       QgsSymbolLayerUtils,
                       QgsSimpleMarkerSymbolLayer,
                       QgsLineSymbolLayer,
                       QgsTemplatedLineSymbolLayerBase,
                       QgsMarkerLineSymbolLayer,
                       QgsMarkerSymbol,
                       QgsGeometryGeneratorSymbolLayer,
                       QgsSymbol,
                       QgsFontMarkerSymbolLayer,
                       QgsFontUtils,
                       QgsLineSymbol,
                       QgsSymbolLayer,
                       QgsProperty,
                       QgsRectangle,
                       QgsUnitTypes,
                       QgsMultiRenderChecker,
                       QgsSingleSymbolRenderer,
                       QgsAnimatedMarkerSymbolLayer,
                       QgsMarkerSymbol
                       )

from qgis.testing import unittest, start_app

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsAnimatedMarkerSymbolLayer(unittest.TestCase):

    def setUp(self):
        self.report = "<h1>Python QgsAnimatedMarkerSymbolLayer Tests</h1>\n"

    def tearDown(self):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(self.report)

    def testRenderFrame1(self):
        point_shp = os.path.join(TEST_DATA_DIR, 'points.shp')
        point_layer = QgsVectorLayer(point_shp, 'Lines', 'ogr')
        self.assertTrue(point_layer.isValid())

        marker_symbol = QgsMarkerSymbol()
        marker_symbol.deleteSymbolLayer(0)
        marker_symbol.appendSymbolLayer(
            QgsAnimatedMarkerSymbolLayer())
        marker_symbol[0].setPath(os.path.join(TEST_DATA_DIR, 'qgis_logo_animated.gif'))
        marker_symbol[0].setSize(20)

        point_layer.setRenderer(QgsSingleSymbolRenderer(marker_symbol))

        ms = QgsMapSettings()
        ms.setOutputSize(QSize(200, 200))
        ms.setOutputDpi(96)
        ms.setExtent(QgsRectangle(-118.5, 19.0, -81.4, 50.4))
        ms.setLayers([point_layer])
        ms.setFrameRate(10)
        ms.setCurrentFrame(1)

        # Test rendering
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(ms)
        renderchecker.setControlPathPrefix('symbol_animatedmarker')
        renderchecker.setControlName('expected_animatedmarker_frame1')
        res = renderchecker.runTest('expected_animatedmarker_frame1')
        self.report += renderchecker.report()
        self.assertTrue(res)

    def testRenderFrame2(self):
        point_shp = os.path.join(TEST_DATA_DIR, 'points.shp')
        point_layer = QgsVectorLayer(point_shp, 'Lines', 'ogr')
        self.assertTrue(point_layer.isValid())

        marker_symbol = QgsMarkerSymbol()
        marker_symbol.deleteSymbolLayer(0)
        marker_symbol.appendSymbolLayer(
            QgsAnimatedMarkerSymbolLayer())
        marker_symbol[0].setPath(os.path.join(TEST_DATA_DIR, 'qgis_logo_animated.gif'))
        marker_symbol[0].setSize(20)

        point_layer.setRenderer(QgsSingleSymbolRenderer(marker_symbol))

        ms = QgsMapSettings()
        ms.setOutputSize(QSize(200, 200))
        ms.setOutputDpi(96)
        ms.setExtent(QgsRectangle(-118.5, 19.0, -81.4, 50.4))
        ms.setLayers([point_layer])
        ms.setFrameRate(10)
        ms.setCurrentFrame(2)

        # Test rendering
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(ms)
        renderchecker.setControlPathPrefix('symbol_animatedmarker')
        renderchecker.setControlName('expected_animatedmarker_frame2')
        res = renderchecker.runTest('expected_animatedmarker_frame2')
        self.report += renderchecker.report()
        self.assertTrue(res)

    def imageCheck(self, name, reference_image, image):
        self.report += "<h2>Render {}</h2>\n".format(name)
        temp_dir = QDir.tempPath() + '/'
        file_name = temp_dir + 'symbol_' + name + ".png"
        image.save(file_name, "PNG")
        checker = QgsRenderChecker()
        checker.setControlPathPrefix("symbol_animatedmarker")
        checker.setControlName("expected_" + reference_image)
        checker.setRenderedImage(file_name)
        checker.setColorTolerance(2)
        result = checker.compareImages(name, 20)
        self.report += checker.report()
        print((self.report))
        return result


if __name__ == '__main__':
    unittest.main()
