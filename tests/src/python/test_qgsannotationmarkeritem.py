# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsAnnotationMarkerItem.

From build dir, run: ctest -R PyQgsAnnotationMarkerItem -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2020 by Nyall Dawson'
__date__ = '29/07/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

from qgis.PyQt.QtCore import (QSize,
                              QDir)
from qgis.PyQt.QtGui import (QImage,
                             QPainter,
                             QColor)
from qgis.core import (QgsMapSettings,
                       QgsCoordinateTransform,
                       QgsProject,
                       QgsPoint,
                       QgsCoordinateReferenceSystem,
                       QgsMarkerSymbol,
                       QgsRenderChecker,
                       QgsReadWriteContext,
                       QgsRenderContext,
                       QgsAnnotationMarkerItem,
                       QgsRectangle
                       )
from qgis.PyQt.QtXml import QDomDocument

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsAnnotationMarkerItem(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.report = "<h1>Python QgsAnnotationMarkerItem Tests</h1>\n"

    @classmethod
    def tearDownClass(cls):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(cls.report)

    def testBasic(self):
        item = QgsAnnotationMarkerItem(QgsPoint(12, 13))

        self.assertEqual(item.geometry().x(), 12.0)
        self.assertEqual(item.geometry().y(), 13.0)

        item.setGeometry(QgsPoint(1000, 2000))
        item.setZIndex(11)
        self.assertEqual(item.geometry().x(), 1000.0)
        self.assertEqual(item.geometry().y(), 2000.0)
        self.assertEqual(item.zIndex(), 11)

        item.setSymbol(QgsMarkerSymbol.createSimple({'color': '100,200,200', 'size': '3', 'outline_color': 'black'}))
        self.assertEqual(item.symbol()[0].color(), QColor(100, 200, 200))

    def testReadWriteXml(self):
        doc = QDomDocument("testdoc")
        elem = doc.createElement('test')

        item = QgsAnnotationMarkerItem(QgsPoint(12, 13))
        item.setSymbol(QgsMarkerSymbol.createSimple({'color': '100,200,200', 'size': '3', 'outline_color': 'black'}))
        item.setZIndex(11)

        self.assertTrue(item.writeXml(elem, doc, QgsReadWriteContext()))

        s2 = QgsAnnotationMarkerItem.create()
        self.assertTrue(s2.readXml(elem, QgsReadWriteContext()))

        self.assertEqual(s2.geometry().x(), 12.0)
        self.assertEqual(s2.geometry().y(), 13.0)
        self.assertEqual(s2.symbol()[0].color(), QColor(100, 200, 200))
        self.assertEqual(s2.zIndex(), 11)

    def testClone(self):
        item = QgsAnnotationMarkerItem(QgsPoint(12, 13))
        item.setSymbol(QgsMarkerSymbol.createSimple({'color': '100,200,200', 'size': '3', 'outline_color': 'black'}))
        item.setZIndex(11)

        item2 = item.clone()
        self.assertEqual(item2.geometry().x(), 12.0)
        self.assertEqual(item2.geometry().y(), 13.0)
        self.assertEqual(item2.symbol()[0].color(), QColor(100, 200, 200))
        self.assertEqual(item2.zIndex(), 11)

    def testRenderMarker(self):
        item = QgsAnnotationMarkerItem(QgsPoint(12, 13))
        item.setSymbol(QgsMarkerSymbol.createSimple({'color': '100,200,200', 'size': '3', 'outline_color': 'black'}))

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        settings.setExtent(QgsRectangle(10, 10, 16, 16))
        settings.setOutputSize(QSize(300, 300))

        settings.setFlag(QgsMapSettings.Antialiasing, False)

        rc = QgsRenderContext.fromMapSettings(settings)
        image = QImage(200, 200, QImage.Format_ARGB32)
        image.setDotsPerMeterX(96 / 25.4 * 1000)
        image.setDotsPerMeterY(96 / 25.4 * 1000)
        image.fill(QColor(255, 255, 255))
        painter = QPainter(image)
        rc.setPainter(painter)

        try:
            item.render(rc, None)
        finally:
            painter.end()

        self.assertTrue(self.imageCheck('marker_item', 'marker_item', image))

    def testRenderWithTransform(self):
        item = QgsAnnotationMarkerItem(QgsPoint(12, 13))
        item.setSymbol(QgsMarkerSymbol.createSimple({'color': '100,200,200', 'size': '3', 'outline_color': 'black'}))

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        settings.setExtent(QgsRectangle(1250958, 1386945, 1420709, 1532518))
        settings.setOutputSize(QSize(300, 300))

        settings.setFlag(QgsMapSettings.Antialiasing, False)

        rc = QgsRenderContext.fromMapSettings(settings)
        rc.setCoordinateTransform(QgsCoordinateTransform(QgsCoordinateReferenceSystem('EPSG:4326'), settings.destinationCrs(), QgsProject.instance()))
        image = QImage(200, 200, QImage.Format_ARGB32)
        image.setDotsPerMeterX(96 / 25.4 * 1000)
        image.setDotsPerMeterY(96 / 25.4 * 1000)
        image.fill(QColor(255, 255, 255))
        painter = QPainter(image)
        rc.setPainter(painter)

        try:
            item.render(rc, None)
        finally:
            painter.end()

        self.assertTrue(self.imageCheck('marker_item_transform', 'marker_item_transform', image))

    def imageCheck(self, name, reference_image, image):
        TestQgsAnnotationMarkerItem.report += "<h2>Render {}</h2>\n".format(name)
        temp_dir = QDir.tempPath() + '/'
        file_name = temp_dir + 'patch_' + name + ".png"
        image.save(file_name, "PNG")
        checker = QgsRenderChecker()
        checker.setControlPathPrefix("annotation_layer")
        checker.setControlName("expected_" + reference_image)
        checker.setRenderedImage(file_name)
        checker.setColorTolerance(2)
        result = checker.compareImages(name, 20)
        TestQgsAnnotationMarkerItem.report += checker.report()
        return result


if __name__ == '__main__':
    unittest.main()
