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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA
import os

from qgis.PyQt.QtCore import (
    QSize,
    Qt
)
from qgis.PyQt.QtGui import (
    QColor,
    QImage,
    QPainter
)
from qgis.core import (
    QgsVectorLayer,
    QgsProject,
    QgsRectangle,
    QgsMultiRenderChecker
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

        lines_shp = os.path.join(TEST_DATA_DIR, 'lines.shp')
        self.lines_layer = QgsVectorLayer(lines_shp, 'Lines', 'ogr')
        QgsProject.instance().addMapLayer(self.lines_layer)
        polys_shp = os.path.join(TEST_DATA_DIR, 'polys.shp')
        self.polys_layer = QgsVectorLayer(polys_shp, 'Polygons', 'ogr')
        QgsProject.instance().addMapLayer(self.polys_layer)

        self.iface.mapCanvas().resize(QSize(400, 400))

        self.iface.mapCanvas().setExtent(QgsRectangle(-113, 28, -91, 40))

        self.mapsettings = self.iface.mapCanvas().mapSettings()
        self.mapsettings.setOutputSize(QSize(400, 400))
        self.mapsettings.setOutputDpi(96)
        self.mapsettings.setExtent(QgsRectangle(-113, 28, -91, 40))
        self.mapsettings.setBackgroundColor(QColor("white"))

    def tearDown(self):
        QgsProject.instance().removeAllMapLayers()

    def testLine(self):
        line = next(self.lines_layer.getFeatures()).geometry()
        highlight = QgsHighlight(self.iface.mapCanvas(), line, self.lines_layer)
        color = QColor(Qt.red)
        highlight.setColor(color)
        color.setAlpha(50)
        highlight.setFillColor(color)
        highlight.show()
        image = QImage(QSize(400, 400), QImage.Format_ARGB32)
        painter = QPainter()
        painter.begin(image)
        self.iface.mapCanvas().render(painter)
        painter.end()

    def testPolygon(self):
        poly = next(self.polys_layer.getFeatures()).geometry()
        self.iface.mapCanvas().setExtent(self.polys_layer.extent())
        highlight = QgsHighlight(self.iface.mapCanvas(), poly, self.polys_layer)
        color = QColor(Qt.red)
        highlight.setColor(color)
        color.setAlpha(50)
        highlight.setFillColor(color)
        highlight.show()
        image = QImage(QSize(400, 400), QImage.Format_ARGB32)
        painter = QPainter()
        painter.begin(image)
        self.iface.mapCanvas().render(painter)
        painter.end()



if __name__ == '__main__':
    unittest.main()
