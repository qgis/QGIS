# -*- coding: utf-8 -*-

"""
***************************************************************************
    test_qgssimplelinesymbollayer.py
    ---------------------
    Date                 : November 2018
    Copyright            : (C) 2018 by Nyall Dawson
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
__date__ = 'November 2018'
__copyright__ = '(C) 2018, Nyall Dawson'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from utilities import unitTestDataPath

from qgis.PyQt.QtCore import QDir
from qgis.PyQt.QtGui import QImage, QColor, QPainter
from qgis.PyQt.QtXml import QDomDocument

from qgis.core import (QgsGeometry,
                       QgsFillSymbol,
                       QgsRenderContext,
                       QgsFeature,
                       QgsMapSettings,
                       QgsRenderChecker,
                       QgsReadWriteContext,
                       QgsSymbolLayerUtils,
                       QgsSimpleLineSymbolLayer,
                       QgsLineSymbolLayer
                       )

from qgis.testing import unittest, start_app

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsSimpleLineSymbolLayer(unittest.TestCase):

    def setUp(self):
        self.report = "<h1>Python QgsSimpleLineSymbolLayer Tests</h1>\n"

    def tearDown(self):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(self.report)

    def testRingFilter(self):
        # test filtering rings during rendering

        s = QgsFillSymbol()
        s.deleteSymbolLayer(0)
        s.appendSymbolLayer(
            QgsSimpleLineSymbolLayer(color=QColor(255, 0, 0), width=2))
        self.assertEqual(s.symbolLayer(0).ringFilter(), QgsLineSymbolLayer.AllRings)
        s.symbolLayer(0).setRingFilter(QgsLineSymbolLayer.ExteriorRingOnly)
        self.assertEqual(s.symbolLayer(0).ringFilter(), QgsLineSymbolLayer.ExteriorRingOnly)

        s2 = s.clone()
        self.assertEqual(s2.symbolLayer(0).ringFilter(), QgsLineSymbolLayer.ExteriorRingOnly)

        doc = QDomDocument()
        context = QgsReadWriteContext()
        element = QgsSymbolLayerUtils.saveSymbol('test', s, doc, context)

        s2 = QgsSymbolLayerUtils.loadSymbol(element, context)
        self.assertEqual(s2.symbolLayer(0).ringFilter(), QgsLineSymbolLayer.ExteriorRingOnly)

        # rendering test
        s3 = QgsFillSymbol()
        s3.deleteSymbolLayer(0)
        s3.appendSymbolLayer(
            QgsSimpleLineSymbolLayer(color=QColor(255, 0, 0), width=2))
        s3.symbolLayer(0).setRingFilter(QgsLineSymbolLayer.ExteriorRingOnly)

        g = QgsGeometry.fromWkt('Polygon((0 0, 10 0, 10 10, 0 10, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1),(8 8, 9 8, 9 9, 8 9, 8 8))')
        rendered_image = self.renderGeometry(s3, g)
        assert self.imageCheck('simpleline_exterioronly', 'simpleline_exterioronly', rendered_image)

        s3.symbolLayer(0).setRingFilter(QgsLineSymbolLayer.InteriorRingsOnly)
        g = QgsGeometry.fromWkt('Polygon((0 0, 10 0, 10 10, 0 10, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1),(8 8, 9 8, 9 9, 8 9, 8 8))')
        rendered_image = self.renderGeometry(s3, g)
        assert self.imageCheck('simpleline_interioronly', 'simpleline_interioronly', rendered_image)

    def renderGeometry(self, symbol, geom):
        f = QgsFeature()
        f.setGeometry(geom)

        image = QImage(200, 200, QImage.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        extent = geom.get().boundingBox()
        # buffer extent by 10%
        if extent.width() > 0:
            extent = extent.buffered((extent.height() + extent.width()) / 20.0)
        else:
            extent = extent.buffered(10)

        ms.setExtent(extent)
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI

        painter.begin(image)
        try:
            image.fill(QColor(0, 0, 0))
            symbol.startRender(context)
            symbol.renderFeature(f, context)
            symbol.stopRender(context)
        finally:
            painter.end()

        return image

    def imageCheck(self, name, reference_image, image):
        self.report += "<h2>Render {}</h2>\n".format(name)
        temp_dir = QDir.tempPath() + '/'
        file_name = temp_dir + 'symbol_' + name + ".png"
        image.save(file_name, "PNG")
        checker = QgsRenderChecker()
        checker.setControlPathPrefix("symbol_simpleline")
        checker.setControlName("expected_" + reference_image)
        checker.setRenderedImage(file_name)
        checker.setColorTolerance(2)
        result = checker.compareImages(name, 20)
        self.report += checker.report()
        print((self.report))
        return result


if __name__ == '__main__':
    unittest.main()
