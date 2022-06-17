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
    Qt,
    QDir,

)
from qgis.PyQt.QtGui import (
    QColor,
    QImage,
    QPainter,
    QResizeEvent,
    QPixmap
)
from qgis.core import (
    QgsVectorLayer,
    QgsProject,
    QgsRectangle,
    QgsRenderChecker,
    QgsCoordinateReferenceSystem,
    QgsMultiRenderChecker,
    QgsGeometryGeneratorSymbolLayer,
    QgsFillSymbol,
    QgsSingleSymbolRenderer,
    QgsSymbol
)
from qgis.gui import (
    QgsHighlight,
    QgsMapCanvas
)
from qgis.testing import start_app, unittest
from qgis.testing.mocked import get_iface
from utilities import unitTestDataPath

app = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsHighlight(unittest.TestCase):

    def setUp(self):
        self.iface = get_iface()

        self.iface.mapCanvas().viewport().resize(400, 400)
        # For some reason the resizeEvent is not delivered, fake it
        self.iface.mapCanvas().resizeEvent(QResizeEvent(QSize(400, 400), self.iface.mapCanvas().size()))
        self.report = "<h1>Python QgsMapCanvas Tests</h1>\n"

    def tearDown(self):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(self.report)

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

    def test_feature_transformation(self):
        poly_shp = os.path.join(TEST_DATA_DIR, 'polys.shp')
        layer = QgsVectorLayer(poly_shp, 'Layer', 'ogr')

        sub_symbol = QgsFillSymbol.createSimple({'color': '#8888ff', 'outline_style': 'no'})

        sym = QgsFillSymbol()
        buffer_layer = QgsGeometryGeneratorSymbolLayer.create(
            {'geometryModifier': 'buffer($geometry, -0.4)'})
        buffer_layer.setSymbolType(QgsSymbol.Fill)
        buffer_layer.setSubSymbol(sub_symbol)
        sym.changeSymbolLayer(0, buffer_layer)
        layer.setRenderer(QgsSingleSymbolRenderer(sym))

        canvas = QgsMapCanvas()
        canvas.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        canvas.setFrameStyle(0)
        canvas.resize(600, 400)
        self.assertEqual(canvas.width(), 600)
        self.assertEqual(canvas.height(), 400)

        canvas.setLayers([layer])
        canvas.setExtent(QgsRectangle(-11960254, 4247568, -11072454, 4983088))
        canvas.show()

        # need to wait until first redraw can occur (note that we first need to wait till drawing starts!)
        while not canvas.isDrawing():
            app.processEvents()
        canvas.waitWhileRendering()

        feature = layer.getFeature(1)
        self.assertTrue(feature.isValid())

        highlight = QgsHighlight(canvas, feature, layer)
        color = QColor(Qt.red)
        highlight.setColor(color)
        color.setAlpha(50)
        highlight.setFillColor(color)
        highlight.show()
        highlight.show()

        self.assertTrue(self.canvasImageCheck('highlight_transform', 'highlight_transform', canvas))

    def canvasImageCheck(self, name, reference_image, canvas):
        self.report += "<h2>Render {}</h2>\n".format(name)
        temp_dir = QDir.tempPath() + '/'
        file_name = temp_dir + 'rendered_' + name + ".png"

        image = QImage(canvas.size(), QImage.Format_ARGB32)
        painter = QPainter(image)
        canvas.render(painter)
        painter.end()
        image.save(file_name)

        checker = QgsMultiRenderChecker()
        checker.setControlPathPrefix("highlight")
        checker.setControlName("expected_" + reference_image)
        checker.setRenderedImage(file_name)
        checker.setColorTolerance(2)
        result = checker.runTest(name, 20)
        self.report += checker.report()
        print((self.report))
        return result


if __name__ == '__main__':
    unittest.main()
