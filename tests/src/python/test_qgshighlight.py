# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsHighlight.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Matthias Kuhn'
__date__ = '8.11.2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import qgis  # NOQA
import os
import tempfile
import shutil

from qgis.PyQt.QtCore import (
    QSize,
    Qt
)
from qgis.PyQt.QtGui import (
    QColor,
    QImage,
    QPainter,
    QResizeEvent
)
from qgis.core import (
    QgsVectorLayer,
    QgsProject,
    QgsRectangle,
    QgsRenderChecker
)
from qgis.gui import QgsHighlight
from qgis.testing import start_app, unittest
from qgis.testing.mocked import get_iface
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsHighlight(unittest.TestCase):

    def setUp(self):
        self.iface = get_iface()

        self.iface.mapCanvas().viewport().resize(400, 400)
        # For some reason the resizeEvent is not delivered, fake it
        self.iface.mapCanvas().resizeEvent(QResizeEvent(QSize(400, 400), self.iface.mapCanvas().size()))

    def tearDown(self):
        QgsProject.instance().removeAllMapLayers()

    def runTestForLayer(self, layer, testname):
        tempdir = tempfile.mkdtemp()

        layer = QgsVectorLayer(layer, 'Layer', 'ogr')
        QgsProject.instance().addMapLayer(layer)
        self.iface.mapCanvas().setExtent(layer.extent())

        geom = next(layer.getFeatures()).geometry()

        highlight = QgsHighlight(self.iface.mapCanvas(), geom, layer)
        color = QColor(Qt.red)
        highlight.setColor(color)
        highlight.setWidth(2)
        color.setAlpha(50)
        highlight.setFillColor(color)
        highlight.show()

        image = QImage(QSize(400, 400), QImage.Format_ARGB32)
        image.fill(Qt.white)
        painter = QPainter()
        painter.begin(image)
        self.iface.mapCanvas().render(painter)
        painter.end()
        control_image = os.path.join(tempdir, 'highlight_{}.png'.format(testname))
        image.save(control_image)
        checker = QgsRenderChecker()
        checker.setControlPathPrefix("highlight")
        checker.setControlName("expected_highlight_{}".format(testname))
        checker.setRenderedImage(control_image)
        self.assertTrue(checker.compareImages("highlight_{}".format(testname)))
        shutil.rmtree(tempdir)

    def testLine(self):
        lines_shp = os.path.join(TEST_DATA_DIR, 'lines.shp')
        self.runTestForLayer(lines_shp, 'lines')

    def testPolygon(self):
        polys_shp = os.path.join(TEST_DATA_DIR, 'polys.shp')
        self.runTestForLayer(polys_shp, 'polygons')

    def testBugfix48471(self):
        """ Test scenario of https://github.com/qgis/QGIS/issues/48471 """

        lines_shp = os.path.join(TEST_DATA_DIR, 'lines.shp')
        layer = QgsVectorLayer(lines_shp, 'Layer', 'ogr')
        QgsProject.instance().addMapLayer(layer)
        self.iface.mapCanvas().setExtent(layer.extent())

        geom = next(layer.getFeatures()).geometry()

        highlight = QgsHighlight(self.iface.mapCanvas(), geom, layer)
        highlight.setBuffer(12345)

        try:
            found = False
            for item in self.iface.mapCanvas().scene().items():
                if isinstance(item, QgsHighlight):
                    if item.buffer() == 12345:
                        found = True
            self.assertTrue(found)
        finally:
            self.iface.mapCanvas().scene().removeItem(highlight)


if __name__ == '__main__':
    unittest.main()
