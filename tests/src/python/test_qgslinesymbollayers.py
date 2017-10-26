# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLineSymbolLayers.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2017-01'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'


import qgis  # NOQA

from qgis.testing import unittest
from qgis.PyQt.QtCore import QDir
from qgis.PyQt.QtGui import (QImage,
                             QPainter,
                             QColor)
from qgis.core import (QgsRenderChecker,
                       QgsSimpleLineSymbolLayer,
                       QgsMapSettings,
                       QgsLineSymbol,
                       QgsGeometry,
                       QgsFeature,
                       QgsRenderContext)


class TestQgsLineSymbolLayers(unittest.TestCase):

    def setUp(self):
        self.report = "<h1>Python QgsLineSymbolLayer Tests</h1>\n"

    def tearDown(self):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(self.report)

    def imageCheck(self, name, reference_image, image):
        self.report += "<h2>Render {}</h2>\n".format(name)
        temp_dir = QDir.tempPath() + '/'
        file_name = temp_dir + 'symbollayer_' + name + ".png"
        image.save(file_name, "PNG")
        checker = QgsRenderChecker()
        checker.setControlPathPrefix("symbol_layer")
        checker.setControlName("expected_" + reference_image)
        checker.setRenderedImage(file_name)
        checker.setColorTolerance(2)
        result = checker.compareImages(name, 0)
        self.report += checker.report()
        print((self.report))
        return result

    def testSimpleLineWithOffset(self):
        """ test that rendering a simple line symbol with offset"""
        layer = QgsSimpleLineSymbolLayer()
        layer.setOffset(1)

        symbol = QgsLineSymbol()
        symbol.changeSymbolLayer(0, layer)

        image = QImage(200, 200, QImage.Format_RGB32)
        painter = QPainter()
        ms = QgsMapSettings()

        geom = QgsGeometry.fromWkt('LineString (0 0, 10 0, 10 10, 0 10, 0 0)')
        f = QgsFeature()
        f.setGeometry(geom)

        extent = geom.constGet().boundingBox()
        # buffer extent by 10%
        extent = extent.buffered((extent.height() + extent.width()) / 20.0)

        ms.setExtent(extent)
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI

        painter.begin(image)
        image.fill(QColor(255, 255, 255))

        symbol.startRender(context)
        symbol.renderFeature(f, context)
        symbol.stopRender(context)
        painter.end()

        self.assertTrue(self.imageCheck('symbol_layer', 'simpleline_offset', image))


if __name__ == '__main__':
    unittest.main()
