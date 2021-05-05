# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsAnnotationLayer.

From build dir, run: ctest -R PyQgsAnnotationLayer -V

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
                              QDir,
                              QTemporaryDir)
from qgis.PyQt.QtGui import (QImage,
                             QPainter,
                             QColor)
from qgis.core import (QgsMapSettings,
                       QgsCoordinateTransform,
                       QgsProject,
                       QgsPoint,
                       QgsCoordinateReferenceSystem,
                       QgsFillSymbol,
                       QgsRenderChecker,
                       QgsReadWriteContext,
                       QgsRenderContext,
                       QgsAnnotationPolygonItem,
                       QgsRectangle,
                       QgsLineString,
                       QgsPolygon,
                       QgsAnnotationLayer,
                       QgsAnnotationLineItem,
                       QgsAnnotationMarkerItem,
                       QgsLineSymbol,
                       QgsMarkerSymbol,
                       )
from qgis.PyQt.QtXml import QDomDocument

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsAnnotationLayer(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.report = "<h1>Python QgsAnnotationLayer Tests</h1>\n"

    @classmethod
    def tearDownClass(cls):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(cls.report)

    def testItems(self):
        layer = QgsAnnotationLayer('test', QgsAnnotationLayer.LayerOptions(QgsProject.instance().transformContext()))
        self.assertTrue(layer.isValid())

        self.assertTrue(layer.isEmpty())

        polygon_item_id = layer.addItem(QgsAnnotationPolygonItem(QgsPolygon(QgsLineString([QgsPoint(12, 13), QgsPoint(14, 13), QgsPoint(14, 15), QgsPoint(12, 13)]))))
        linestring_item_id = layer.addItem(QgsAnnotationLineItem(QgsLineString([QgsPoint(11, 13), QgsPoint(12, 13), QgsPoint(12, 15)])))
        marker_item_id = layer.addItem(QgsAnnotationMarkerItem(QgsPoint(12, 13)))

        self.assertEqual(len(layer.items()), 3)
        self.assertFalse(layer.isEmpty())

        self.assertIsInstance(layer.items()[polygon_item_id], QgsAnnotationPolygonItem)
        self.assertIsInstance(layer.items()[linestring_item_id], QgsAnnotationLineItem)
        self.assertIsInstance(layer.items()[marker_item_id], QgsAnnotationMarkerItem)

        self.assertFalse(layer.removeItem('xxxx'))
        self.assertEqual(len(layer.items()), 3)
        self.assertTrue(layer.removeItem(linestring_item_id))
        self.assertEqual(len(layer.items()), 2)
        self.assertIsInstance(layer.items()[polygon_item_id], QgsAnnotationPolygonItem)
        self.assertIsInstance(layer.items()[marker_item_id], QgsAnnotationMarkerItem)
        self.assertFalse(layer.removeItem(linestring_item_id))

        self.assertTrue(layer.removeItem(polygon_item_id))
        self.assertEqual(len(layer.items()), 1)
        self.assertIsInstance(layer.items()[marker_item_id], QgsAnnotationMarkerItem)

        self.assertTrue(layer.removeItem(marker_item_id))
        self.assertEqual(len(layer.items()), 0)
        self.assertTrue(layer.isEmpty())

        layer.addItem(QgsAnnotationPolygonItem(QgsPolygon(QgsLineString([QgsPoint(12, 13), QgsPoint(14, 13), QgsPoint(14, 15), QgsPoint(12, 13)]))))
        layer.addItem(QgsAnnotationLineItem(QgsLineString([QgsPoint(11, 13), QgsPoint(12, 13), QgsPoint(12, 15)])))
        layer.addItem(QgsAnnotationMarkerItem(QgsPoint(12, 13)))

        self.assertEqual(len(layer.items()), 3)
        layer.clear()
        self.assertEqual(len(layer.items()), 0)

    def testReset(self):
        layer = QgsAnnotationLayer('test', QgsAnnotationLayer.LayerOptions(QgsProject.instance().transformContext()))
        self.assertTrue(layer.isValid())
        layer.addItem(QgsAnnotationPolygonItem(QgsPolygon(QgsLineString([QgsPoint(12, 13), QgsPoint(14, 13), QgsPoint(14, 15), QgsPoint(12, 13)]))))
        layer.addItem(QgsAnnotationLineItem(QgsLineString([QgsPoint(11, 13), QgsPoint(12, 13), QgsPoint(12, 15)])))
        layer.addItem(QgsAnnotationMarkerItem(QgsPoint(12, 13)))
        layer.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        layer.setOpacity(0.5)

        layer.reset()
        self.assertEqual(len(layer.items()), 0)
        self.assertEqual(layer.opacity(), 1.0)
        self.assertFalse(layer.crs().isValid())

    def testExtent(self):
        layer = QgsAnnotationLayer('test', QgsAnnotationLayer.LayerOptions(QgsProject.instance().transformContext()))
        self.assertTrue(layer.isValid())

        layer.addItem(QgsAnnotationPolygonItem(QgsPolygon(QgsLineString([QgsPoint(12, 13), QgsPoint(14, 13), QgsPoint(14, 15), QgsPoint(12, 13)]))))
        layer.addItem(QgsAnnotationLineItem(QgsLineString([QgsPoint(11, 13), QgsPoint(12, 13), QgsPoint(12, 15)])))
        layer.addItem(QgsAnnotationMarkerItem(QgsPoint(12, 13)))

        extent = layer.extent()
        self.assertEqual(extent.xMinimum(), 11.0)
        self.assertEqual(extent.xMaximum(), 14.0)
        self.assertEqual(extent.yMinimum(), 13.0)
        self.assertEqual(extent.yMaximum(), 15.0)

        # should have no effect -- item geometries are in layer crs
        layer.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        extent = layer.extent()
        self.assertEqual(extent.xMinimum(), 11.0)
        self.assertEqual(extent.xMaximum(), 14.0)
        self.assertEqual(extent.yMinimum(), 13.0)
        self.assertEqual(extent.yMaximum(), 15.0)

    def testReadWriteXml(self):
        doc = QDomDocument("testdoc")

        layer = QgsAnnotationLayer('test', QgsAnnotationLayer.LayerOptions(QgsProject.instance().transformContext()))
        self.assertTrue(layer.isValid())

        layer.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        polygon_item_id = layer.addItem(QgsAnnotationPolygonItem(QgsPolygon(QgsLineString([QgsPoint(12, 13), QgsPoint(14, 13), QgsPoint(14, 15), QgsPoint(12, 13)]))))
        linestring_item_id = layer.addItem(QgsAnnotationLineItem(QgsLineString([QgsPoint(11, 13), QgsPoint(12, 13), QgsPoint(12, 15)])))
        marker_item_id = layer.addItem(QgsAnnotationMarkerItem(QgsPoint(12, 13)))

        elem = doc.createElement("maplayer")
        self.assertTrue(layer.writeLayerXml(elem, doc, QgsReadWriteContext()))

        layer2 = QgsAnnotationLayer('test2', QgsAnnotationLayer.LayerOptions(QgsProject.instance().transformContext()))
        self.assertTrue(layer2.readLayerXml(elem, QgsReadWriteContext()))
        self.assertEqual(layer2.crs().authid(), 'EPSG:4326')

        self.assertEqual(len(layer2.items()), 3)
        self.assertIsInstance(layer2.items()[polygon_item_id], QgsAnnotationPolygonItem)
        self.assertIsInstance(layer2.items()[linestring_item_id], QgsAnnotationLineItem)
        self.assertIsInstance(layer2.items()[marker_item_id], QgsAnnotationMarkerItem)

    def testClone(self):
        layer = QgsAnnotationLayer('test', QgsAnnotationLayer.LayerOptions(QgsProject.instance().transformContext()))
        self.assertTrue(layer.isValid())

        polygon_item_id = layer.addItem(QgsAnnotationPolygonItem(QgsPolygon(QgsLineString([QgsPoint(12, 13), QgsPoint(14, 13), QgsPoint(14, 15), QgsPoint(12, 13)]))))
        linestring_item_id = layer.addItem(QgsAnnotationLineItem(QgsLineString([QgsPoint(11, 13), QgsPoint(12, 13), QgsPoint(12, 15)])))
        marker_item_id = layer.addItem(QgsAnnotationMarkerItem(QgsPoint(12, 13)))

        layer2 = layer.clone()

        self.assertEqual(len(layer2.items()), 3)
        self.assertIsInstance(layer2.items()[polygon_item_id], QgsAnnotationPolygonItem)
        # should not be the SAME instance of the item -- the item must have been cloned for the cloned layer!
        self.assertNotEqual(layer.items()[polygon_item_id], layer2.items()[polygon_item_id])
        self.assertIsInstance(layer2.items()[linestring_item_id], QgsAnnotationLineItem)
        self.assertIsInstance(layer2.items()[marker_item_id], QgsAnnotationMarkerItem)

    def testProjectMainAnnotationLayer(self):
        p = QgsProject()
        self.assertIsNotNone(p.mainAnnotationLayer())

        # add some items to project annotation layer
        polygon_item_id = p.mainAnnotationLayer().addItem(QgsAnnotationPolygonItem(QgsPolygon(QgsLineString([QgsPoint(12, 13), QgsPoint(14, 13), QgsPoint(14, 15), QgsPoint(12, 13)]))))
        linestring_item_id = p.mainAnnotationLayer().addItem(QgsAnnotationLineItem(QgsLineString([QgsPoint(11, 13), QgsPoint(12, 13), QgsPoint(12, 15)])))
        marker_item_id = p.mainAnnotationLayer().addItem(QgsAnnotationMarkerItem(QgsPoint(12, 13)))

        # save project to xml
        tmpDir = QTemporaryDir()
        tmpFile = "{}/project.qgs".format(tmpDir.path())
        self.assertTrue(p.write(tmpFile))

        # test that annotation layer is cleared with project
        p.clear()
        self.assertEqual(len(p.mainAnnotationLayer().items()), 0)

        # check that main annotation layer is restored on project read
        p2 = QgsProject()
        self.assertTrue(p2.read(tmpFile))
        self.assertEqual(len(p2.mainAnnotationLayer().items()), 3)
        self.assertIsInstance(p2.mainAnnotationLayer().items()[polygon_item_id], QgsAnnotationPolygonItem)
        self.assertIsInstance(p2.mainAnnotationLayer().items()[linestring_item_id], QgsAnnotationLineItem)
        self.assertIsInstance(p2.mainAnnotationLayer().items()[marker_item_id], QgsAnnotationMarkerItem)

    def testRenderLayer(self):
        layer = QgsAnnotationLayer('test', QgsAnnotationLayer.LayerOptions(QgsProject.instance().transformContext()))
        layer.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        self.assertTrue(layer.isValid())

        item = QgsAnnotationPolygonItem(QgsPolygon(QgsLineString([QgsPoint(12, 13), QgsPoint(14, 13), QgsPoint(14, 15), QgsPoint(12, 13)])))
        item.setSymbol(
            QgsFillSymbol.createSimple({'color': '200,100,100', 'outline_color': 'black', 'outline_width': '2'}))
        item.setZIndex(3)
        layer.addItem(item)

        item = QgsAnnotationLineItem(QgsLineString([QgsPoint(11, 13), QgsPoint(12, 13), QgsPoint(12, 15)]))
        item.setSymbol(QgsLineSymbol.createSimple({'color': '#ffff00', 'line_width': '3'}))
        item.setZIndex(2)
        layer.addItem(item)

        item = QgsAnnotationMarkerItem(QgsPoint(12, 13))
        item.setSymbol(QgsMarkerSymbol.createSimple({'color': '100,200,200', 'size': '6', 'outline_color': 'black'}))
        item.setZIndex(1)
        layer.addItem(item)

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        settings.setExtent(QgsRectangle(10, 10, 18, 18))
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
            renderer = layer.createMapRenderer(rc)
            renderer.render()
        finally:
            painter.end()

        self.assertTrue(self.imageCheck('layer_render', 'layer_render', image))

    def testRenderWithTransform(self):
        layer = QgsAnnotationLayer('test', QgsAnnotationLayer.LayerOptions(QgsProject.instance().transformContext()))
        self.assertTrue(layer.isValid())

        item = QgsAnnotationPolygonItem(QgsPolygon(QgsLineString([QgsPoint(11.5, 13), QgsPoint(12, 13), QgsPoint(12, 13.5), QgsPoint(11.5, 13)])))
        item.setSymbol(
            QgsFillSymbol.createSimple({'color': '200,100,100', 'outline_color': 'black', 'outline_width': '2'}))
        item.setZIndex(1)
        layer.addItem(item)

        item = QgsAnnotationLineItem(QgsLineString([QgsPoint(11, 13), QgsPoint(12, 13), QgsPoint(12, 15)]))
        item.setSymbol(QgsLineSymbol.createSimple({'color': '#ffff00', 'line_width': '3'}))
        item.setZIndex(2)
        layer.addItem(item)

        item = QgsAnnotationMarkerItem(QgsPoint(12, 13))
        item.setSymbol(QgsMarkerSymbol.createSimple({'color': '100,200,200', 'size': '6', 'outline_color': 'black'}))
        item.setZIndex(3)
        layer.addItem(item)

        layer.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        settings.setExtent(QgsRectangle(1250958, 1386945, 1420709, 1532518))
        settings.setOutputSize(QSize(300, 300))

        settings.setFlag(QgsMapSettings.Antialiasing, False)

        rc = QgsRenderContext.fromMapSettings(settings)
        rc.setCoordinateTransform(QgsCoordinateTransform(layer.crs(), settings.destinationCrs(), QgsProject.instance()))
        image = QImage(200, 200, QImage.Format_ARGB32)
        image.setDotsPerMeterX(96 / 25.4 * 1000)
        image.setDotsPerMeterY(96 / 25.4 * 1000)
        image.fill(QColor(255, 255, 255))
        painter = QPainter(image)
        rc.setPainter(painter)

        try:
            renderer = layer.createMapRenderer(rc)
            renderer.render()
        finally:
            painter.end()

        self.assertTrue(self.imageCheck('layer_render_transform', 'layer_render_transform', image))

    def imageCheck(self, name, reference_image, image):
        TestQgsAnnotationLayer.report += "<h2>Render {}</h2>\n".format(name)
        temp_dir = QDir.tempPath() + '/'
        file_name = temp_dir + 'patch_' + name + ".png"
        image.save(file_name, "PNG")
        checker = QgsRenderChecker()
        checker.setControlPathPrefix("annotation_layer")
        checker.setControlName("expected_" + reference_image)
        checker.setRenderedImage(file_name)
        checker.setColorTolerance(2)
        result = checker.compareImages(name, 20)
        TestQgsAnnotationLayer.report += checker.report()
        return result


if __name__ == '__main__':
    unittest.main()
