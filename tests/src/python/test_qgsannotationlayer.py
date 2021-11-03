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
from qgis.PyQt.QtXml import QDomDocument
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
                       QgsMapRendererSequentialJob,
                       QgsMapRendererParallelJob,
                       QgsGeometry,
                       QgsAnnotationItemEditOperationMoveNode,
                       QgsVertexId,
                       QgsPointXY,
                       Qgis
                       )
from qgis.testing import start_app, unittest

from utilities import unitTestDataPath, compareWkt

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
        self.assertIsNone(layer.item('xxxx'))
        self.assertIsNone(layer.item(''))

        polygon_item_id = layer.addItem(QgsAnnotationPolygonItem(
            QgsPolygon(QgsLineString([QgsPoint(12, 13), QgsPoint(14, 13), QgsPoint(14, 15), QgsPoint(12, 13)]))))
        linestring_item_id = layer.addItem(
            QgsAnnotationLineItem(QgsLineString([QgsPoint(11, 13), QgsPoint(12, 13), QgsPoint(12, 15)])))
        marker_item_id = layer.addItem(QgsAnnotationMarkerItem(QgsPoint(12, 13)))

        self.assertEqual(len(layer.items()), 3)
        self.assertFalse(layer.isEmpty())

        self.assertIsInstance(layer.items()[polygon_item_id], QgsAnnotationPolygonItem)
        self.assertIsInstance(layer.items()[linestring_item_id], QgsAnnotationLineItem)
        self.assertIsInstance(layer.items()[marker_item_id], QgsAnnotationMarkerItem)

        self.assertIsInstance(layer.item(polygon_item_id), QgsAnnotationPolygonItem)
        self.assertIsInstance(layer.item(linestring_item_id), QgsAnnotationLineItem)
        self.assertIsInstance(layer.item(marker_item_id), QgsAnnotationMarkerItem)
        self.assertIsNone(layer.item('xxxx'))
        self.assertIsNone(layer.item(''))

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

        layer.addItem(QgsAnnotationPolygonItem(
            QgsPolygon(QgsLineString([QgsPoint(12, 13), QgsPoint(14, 13), QgsPoint(14, 15), QgsPoint(12, 13)]))))
        layer.addItem(QgsAnnotationLineItem(QgsLineString([QgsPoint(11, 13), QgsPoint(12, 13), QgsPoint(12, 15)])))
        layer.addItem(QgsAnnotationMarkerItem(QgsPoint(12, 13)))

        self.assertEqual(len(layer.items()), 3)
        layer.clear()
        self.assertEqual(len(layer.items()), 0)

    def testReplaceItem(self):
        layer = QgsAnnotationLayer('test', QgsAnnotationLayer.LayerOptions(QgsProject.instance().transformContext()))

        polygon_item_id = layer.addItem(QgsAnnotationPolygonItem(
            QgsPolygon(QgsLineString([QgsPoint(12, 13), QgsPoint(14, 13), QgsPoint(14, 15), QgsPoint(12, 13)]))))
        linestring_item_id = layer.addItem(
            QgsAnnotationLineItem(QgsLineString([QgsPoint(11, 13), QgsPoint(12, 13), QgsPoint(12, 15)])))
        marker_item_id = layer.addItem(QgsAnnotationMarkerItem(QgsPoint(12, 13)))

        self.assertEqual(layer.item(polygon_item_id).geometry().asWkt(), 'Polygon ((12 13, 14 13, 14 15, 12 13))')
        self.assertEqual(layer.item(linestring_item_id).geometry().asWkt(), 'LineString (11 13, 12 13, 12 15)')
        self.assertEqual(layer.item(marker_item_id).geometry().asWkt(), 'POINT(12 13)')

        layer.replaceItem(linestring_item_id,
                          QgsAnnotationLineItem(QgsLineString([QgsPoint(21, 13), QgsPoint(22, 13), QgsPoint(22, 15)])))
        self.assertEqual(layer.item(polygon_item_id).geometry().asWkt(), 'Polygon ((12 13, 14 13, 14 15, 12 13))')
        self.assertEqual(layer.item(linestring_item_id).geometry().asWkt(), 'LineString (21 13, 22 13, 22 15)')
        self.assertEqual(layer.item(marker_item_id).geometry().asWkt(), 'POINT(12 13)')

    def testReset(self):
        layer = QgsAnnotationLayer('test', QgsAnnotationLayer.LayerOptions(QgsProject.instance().transformContext()))
        self.assertTrue(layer.isValid())
        layer.addItem(QgsAnnotationPolygonItem(
            QgsPolygon(QgsLineString([QgsPoint(12, 13), QgsPoint(14, 13), QgsPoint(14, 15), QgsPoint(12, 13)]))))
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

        layer.addItem(QgsAnnotationPolygonItem(
            QgsPolygon(QgsLineString([QgsPoint(12, 13), QgsPoint(14, 13), QgsPoint(14, 15), QgsPoint(12, 13)]))))
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

    def testItemsInBounds(self):
        layer = QgsAnnotationLayer('test', QgsAnnotationLayer.LayerOptions(QgsProject.instance().transformContext()))
        self.assertTrue(layer.isValid())

        item1uuid = layer.addItem(QgsAnnotationPolygonItem(
            QgsPolygon(QgsLineString([QgsPoint(12, 13), QgsPoint(14, 13), QgsPoint(14, 15), QgsPoint(12, 13)]))))
        item2uuid = layer.addItem(
            QgsAnnotationLineItem(QgsLineString([QgsPoint(11, 13), QgsPoint(12, 13), QgsPoint(12, 150)])))
        item3uuid = layer.addItem(QgsAnnotationMarkerItem(QgsPoint(120, 13)))

        rc = QgsRenderContext()
        self.assertFalse(layer.itemsInBounds(QgsRectangle(-10, -10, -9, 9), rc))
        self.assertCountEqual(layer.itemsInBounds(QgsRectangle(12, 13, 14, 15), rc), [item1uuid, item2uuid])
        self.assertCountEqual(layer.itemsInBounds(QgsRectangle(12, 130, 14, 150), rc), [item2uuid])
        self.assertCountEqual(layer.itemsInBounds(QgsRectangle(110, 0, 120, 20), rc), [item3uuid])

    def testReadWriteXml(self):
        doc = QDomDocument("testdoc")

        layer = QgsAnnotationLayer('test', QgsAnnotationLayer.LayerOptions(QgsProject.instance().transformContext()))
        self.assertTrue(layer.isValid())

        layer.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        polygon_item_id = layer.addItem(QgsAnnotationPolygonItem(
            QgsPolygon(QgsLineString([QgsPoint(12, 13), QgsPoint(14, 13), QgsPoint(14, 15), QgsPoint(12, 13)]))))
        linestring_item_id = layer.addItem(
            QgsAnnotationLineItem(QgsLineString([QgsPoint(11, 13), QgsPoint(12, 13), QgsPoint(12, 15)])))
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

        polygon_item_id = layer.addItem(QgsAnnotationPolygonItem(
            QgsPolygon(QgsLineString([QgsPoint(12, 13), QgsPoint(14, 13), QgsPoint(14, 15), QgsPoint(12, 13)]))))
        linestring_item_id = layer.addItem(
            QgsAnnotationLineItem(QgsLineString([QgsPoint(11, 13), QgsPoint(12, 13), QgsPoint(12, 15)])))
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
        polygon_item_id = p.mainAnnotationLayer().addItem(QgsAnnotationPolygonItem(
            QgsPolygon(QgsLineString([QgsPoint(12, 13), QgsPoint(14, 13), QgsPoint(14, 15), QgsPoint(12, 13)]))))
        linestring_item_id = p.mainAnnotationLayer().addItem(
            QgsAnnotationLineItem(QgsLineString([QgsPoint(11, 13), QgsPoint(12, 13), QgsPoint(12, 15)])))
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

    def test_apply_edit(self):
        """
        Test applying edits to a layer
        """
        layer = QgsAnnotationLayer('test', QgsAnnotationLayer.LayerOptions(QgsProject.instance().transformContext()))
        self.assertTrue(layer.isValid())

        polygon_item_id = layer.addItem(QgsAnnotationPolygonItem(
            QgsPolygon(QgsLineString([QgsPoint(12, 13), QgsPoint(14, 13), QgsPoint(14, 15), QgsPoint(12, 13)]))))
        linestring_item_id = layer.addItem(
            QgsAnnotationLineItem(QgsLineString([QgsPoint(11, 13), QgsPoint(12, 13), QgsPoint(12, 15)])))
        marker_item_id = layer.addItem(QgsAnnotationMarkerItem(QgsPoint(12, 13)))

        rc = QgsRenderContext()
        self.assertCountEqual(layer.itemsInBounds(QgsRectangle(1, 1, 20, 20), rc), [polygon_item_id, linestring_item_id, marker_item_id])

        # can't apply a move to an item which doesn't exist in the layer
        self.assertEqual(layer.applyEdit(QgsAnnotationItemEditOperationMoveNode('xxx', QgsVertexId(0, 0, 2), QgsPoint(14, 15), QgsPoint(19, 15))), Qgis.AnnotationItemEditOperationResult.Invalid)

        # apply move to polygon
        self.assertEqual(layer.applyEdit(
            QgsAnnotationItemEditOperationMoveNode(polygon_item_id, QgsVertexId(0, 0, 2), QgsPoint(14, 15),
                                                   QgsPoint(19, 15))), Qgis.AnnotationItemEditOperationResult.Success)

        self.assertEqual(layer.item(polygon_item_id).geometry().asWkt(), 'Polygon ((12 13, 14 13, 19 15, 12 13))')
        # ensure that spatial index was updated
        self.assertCountEqual(layer.itemsInBounds(QgsRectangle(18, 1, 20, 16), rc), [polygon_item_id])

    def testRenderLayer(self):
        layer = QgsAnnotationLayer('test', QgsAnnotationLayer.LayerOptions(QgsProject.instance().transformContext()))
        layer.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        self.assertTrue(layer.isValid())

        item = QgsAnnotationPolygonItem(
            QgsPolygon(QgsLineString([QgsPoint(12, 13), QgsPoint(14, 13), QgsPoint(14, 15), QgsPoint(12, 13)])))
        item.setSymbol(
            QgsFillSymbol.createSimple({'color': '200,100,100', 'outline_color': 'black', 'outline_width': '2'}))
        item.setZIndex(3)
        i1_id = layer.addItem(item)

        item = QgsAnnotationLineItem(QgsLineString([QgsPoint(11, 13), QgsPoint(12, 13), QgsPoint(12, 15)]))
        item.setSymbol(QgsLineSymbol.createSimple({'color': '#ffff00', 'line_width': '3'}))
        item.setZIndex(2)
        i2_id = layer.addItem(item)

        item = QgsAnnotationMarkerItem(QgsPoint(12, 13))
        item.setSymbol(QgsMarkerSymbol.createSimple({'color': '100,200,200', 'size': '6', 'outline_color': 'black'}))
        item.setZIndex(1)
        i3_id = layer.addItem(item)

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        settings.setExtent(QgsRectangle(10, 10, 18, 18))
        settings.setOutputSize(QSize(300, 300))

        settings.setFlag(QgsMapSettings.Antialiasing, False)

        rc = QgsRenderContext.fromMapSettings(settings)
        image = QImage(200, 200, QImage.Format_ARGB32)
        image.setDotsPerMeterX(int(96 / 25.4 * 1000))
        image.setDotsPerMeterY(int(96 / 25.4 * 1000))
        image.fill(QColor(255, 255, 255))
        painter = QPainter(image)
        rc.setPainter(painter)

        try:
            renderer = layer.createMapRenderer(rc)
            renderer.render()
        finally:
            painter.end()

        self.assertTrue(self.imageCheck('layer_render', 'layer_render', image))

        # also check details of rendered items
        item_details = renderer.takeRenderedItemDetails()
        self.assertEqual([i.layerId() for i in item_details], [layer.id()] * 3)
        self.assertCountEqual([i.itemId() for i in item_details], [i1_id, i2_id, i3_id])
        self.assertEqual([i.boundingBox() for i in item_details if i.itemId() == i1_id][0],
                         QgsRectangle(12, 13, 14, 15))
        self.assertEqual([i.boundingBox() for i in item_details if i.itemId() == i2_id][0],
                         QgsRectangle(11, 13, 12, 15))
        self.assertEqual([i.boundingBox().toString(1) for i in item_details if i.itemId() == i3_id][0],
                         '11.7,12.7 : 12.3,13.3')

    def testRenderWithTransform(self):
        layer = QgsAnnotationLayer('test', QgsAnnotationLayer.LayerOptions(QgsProject.instance().transformContext()))
        self.assertTrue(layer.isValid())

        item = QgsAnnotationPolygonItem(
            QgsPolygon(QgsLineString([QgsPoint(11.5, 13), QgsPoint(12, 13), QgsPoint(12, 13.5), QgsPoint(11.5, 13)])))
        item.setSymbol(
            QgsFillSymbol.createSimple({'color': '200,100,100', 'outline_color': 'black', 'outline_width': '2'}))
        item.setZIndex(1)
        i1_id = layer.addItem(item)

        item = QgsAnnotationLineItem(QgsLineString([QgsPoint(11, 13), QgsPoint(12, 13), QgsPoint(12, 15)]))
        item.setSymbol(QgsLineSymbol.createSimple({'color': '#ffff00', 'line_width': '3'}))
        item.setZIndex(2)
        i2_id = layer.addItem(item)

        item = QgsAnnotationMarkerItem(QgsPoint(12, 13))
        item.setSymbol(QgsMarkerSymbol.createSimple({'color': '100,200,200', 'size': '6', 'outline_color': 'black'}))
        item.setZIndex(3)
        i3_id = layer.addItem(item)

        layer.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        settings.setExtent(QgsRectangle(1250958, 1386945, 1420709, 1532518))
        settings.setOutputSize(QSize(300, 300))

        settings.setFlag(QgsMapSettings.Antialiasing, False)

        rc = QgsRenderContext.fromMapSettings(settings)
        rc.setCoordinateTransform(QgsCoordinateTransform(layer.crs(), settings.destinationCrs(), QgsProject.instance()))
        rc.setExtent(
            rc.coordinateTransform().transformBoundingBox(settings.extent(), QgsCoordinateTransform.ReverseTransform))
        image = QImage(200, 200, QImage.Format_ARGB32)
        image.setDotsPerMeterX(int(96 / 25.4 * 1000))
        image.setDotsPerMeterY(int(96 / 25.4 * 1000))
        image.fill(QColor(255, 255, 255))
        painter = QPainter(image)
        rc.setPainter(painter)

        try:
            renderer = layer.createMapRenderer(rc)
            renderer.render()
        finally:
            painter.end()

        self.assertTrue(self.imageCheck('layer_render_transform', 'layer_render_transform', image))

        # also check details of rendered items
        item_details = renderer.takeRenderedItemDetails()
        self.assertEqual([i.layerId() for i in item_details], [layer.id()] * 3)
        self.assertCountEqual([i.itemId() for i in item_details], [i1_id, i2_id, i3_id])
        self.assertEqual([i.boundingBox() for i in item_details if i.itemId() == i1_id][0],
                         QgsRectangle(11.5, 13, 12, 13.5))
        self.assertEqual([i.boundingBox() for i in item_details if i.itemId() == i2_id][0],
                         QgsRectangle(11, 13, 12, 15))
        self.assertEqual([i.boundingBox().toString(2) for i in item_details if i.itemId() == i3_id][0],
                         '11.94,12.94 : 12.06,13.06')

    def testRenderLayerWithReferenceScale(self):
        layer = QgsAnnotationLayer('test', QgsAnnotationLayer.LayerOptions(QgsProject.instance().transformContext()))
        layer.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        self.assertTrue(layer.isValid())

        item = QgsAnnotationPolygonItem(
            QgsPolygon(QgsLineString([QgsPoint(12, 13), QgsPoint(14, 13), QgsPoint(14, 15), QgsPoint(12, 13)])))
        item.setSymbol(
            QgsFillSymbol.createSimple({'color': '200,100,100', 'outline_color': 'black', 'outline_width': '2'}))
        item.setZIndex(3)
        i1_id = layer.addItem(item)

        item = QgsAnnotationLineItem(QgsLineString([QgsPoint(11, 13), QgsPoint(12, 13), QgsPoint(12, 15)]))
        item.setSymbol(QgsLineSymbol.createSimple({'color': '#ffff00', 'line_width': '3'}))
        item.setZIndex(2)
        i2_id = layer.addItem(item)

        item = QgsAnnotationMarkerItem(QgsPoint(12, 13))
        item.setSymbol(QgsMarkerSymbol.createSimple({'color': '100,200,200', 'size': '6', 'outline_color': 'black'}))
        item.setZIndex(1)
        i3_id = layer.addItem(item)

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        settings.setExtent(QgsRectangle(10, 10, 18, 18))
        settings.setOutputSize(QSize(300, 300))

        settings.setFlag(QgsMapSettings.Antialiasing, False)

        rc = QgsRenderContext.fromMapSettings(settings)

        layer.item(i1_id).setUseSymbologyReferenceScale(True)
        layer.item(i1_id).setSymbologyReferenceScale(rc.rendererScale() * 2)
        # note item 3 has use symbology reference scale set to false, so should be ignored
        layer.item(i2_id).setUseSymbologyReferenceScale(False)
        layer.item(i2_id).setSymbologyReferenceScale(rc.rendererScale() * 2)
        layer.item(i3_id).setUseSymbologyReferenceScale(True)
        layer.item(i3_id).setSymbologyReferenceScale(rc.rendererScale() * 2)

        image = QImage(200, 200, QImage.Format_ARGB32)
        image.setDotsPerMeterX(int(96 / 25.4 * 1000))
        image.setDotsPerMeterY(int(96 / 25.4 * 1000))
        image.fill(QColor(255, 255, 255))
        painter = QPainter(image)
        rc.setPainter(painter)

        try:
            renderer = layer.createMapRenderer(rc)
            renderer.render()
        finally:
            painter.end()

        self.assertTrue(self.imageCheck('layer_render_reference_scale', 'layer_render_reference_scale', image))

        # also check details of rendered items
        item_details = renderer.takeRenderedItemDetails()
        self.assertEqual([i.layerId() for i in item_details], [layer.id()] * 3)
        self.assertCountEqual([i.itemId() for i in item_details], [i1_id, i2_id, i3_id])
        self.assertEqual([i.boundingBox() for i in item_details if i.itemId() == i1_id][0],
                         QgsRectangle(12, 13, 14, 15))
        self.assertEqual([i.boundingBox() for i in item_details if i.itemId() == i2_id][0],
                         QgsRectangle(11, 13, 12, 15))
        self.assertEqual([i.boundingBox().toString(1) for i in item_details if i.itemId() == i3_id][0],
                         '11.4,12.4 : 12.6,13.6')

    def test_render_via_job(self):
        """
        Test rendering an annotation layer via a map render job
        """
        layer = QgsAnnotationLayer('test', QgsAnnotationLayer.LayerOptions(QgsProject.instance().transformContext()))
        self.assertTrue(layer.isValid())

        item = QgsAnnotationPolygonItem(
            QgsPolygon(QgsLineString([QgsPoint(11.5, 13), QgsPoint(12, 13), QgsPoint(12, 13.5), QgsPoint(11.5, 13)])))
        item.setSymbol(
            QgsFillSymbol.createSimple({'color': '200,100,100', 'outline_color': 'black', 'outline_width': '2'}))
        item.setZIndex(1)
        i1_id = layer.addItem(item)

        item = QgsAnnotationLineItem(QgsLineString([QgsPoint(11, 13), QgsPoint(12, 13), QgsPoint(12, 15)]))
        item.setSymbol(QgsLineSymbol.createSimple({'color': '#ffff00', 'line_width': '3'}))
        item.setZIndex(2)
        i2_id = layer.addItem(item)

        item = QgsAnnotationMarkerItem(QgsPoint(12, 13))
        item.setSymbol(QgsMarkerSymbol.createSimple({'color': '100,200,200', 'size': '6', 'outline_color': 'black'}))
        item.setZIndex(3)
        i3_id = layer.addItem(item)

        layer.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        settings.setExtent(QgsRectangle(10, 10, 18, 18))
        settings.setOutputSize(QSize(200, 200))
        settings.setLayers([layer])

        job = QgsMapRendererParallelJob(settings)
        job.start()
        job.waitForFinished()

        # check rendered item results
        item_results = job.takeRenderedItemResults()
        item_details = item_results.renderedItems()
        self.assertEqual(len(item_details), 3)
        self.assertEqual([i.layerId() for i in item_details], [layer.id()] * 3)
        self.assertCountEqual([i.itemId() for i in item_details], [i1_id, i2_id, i3_id])
        self.assertCountEqual(
            [i.itemId() for i in item_results.renderedAnnotationItemsInBounds(QgsRectangle(0, 0, 1, 1))], [])
        self.assertCountEqual(
            [i.itemId() for i in item_results.renderedAnnotationItemsInBounds(QgsRectangle(10, 10, 11, 18))], [i2_id])
        self.assertCountEqual(
            [i.itemId() for i in item_results.renderedAnnotationItemsInBounds(QgsRectangle(10, 10, 12, 18))],
            [i1_id, i2_id, i3_id])

        # bounds should be in map crs
        self.assertEqual([i.boundingBox() for i in item_details if i.itemId() == i1_id][0],
                         QgsRectangle(11.5, 13, 12, 13.5))
        self.assertEqual([i.boundingBox() for i in item_details if i.itemId() == i2_id][0],
                         QgsRectangle(11, 13, 12, 15))
        self.assertEqual([i.boundingBox().toString(1) for i in item_details if i.itemId() == i3_id][0],
                         '11.5,12.5 : 12.5,13.5')

    def test_render_via_job_with_transform(self):
        """
        Test rendering an annotation layer via a map render job
        """
        layer = QgsAnnotationLayer('test', QgsAnnotationLayer.LayerOptions(QgsProject.instance().transformContext()))
        self.assertTrue(layer.isValid())

        item = QgsAnnotationPolygonItem(
            QgsPolygon(QgsLineString([QgsPoint(11.5, 13), QgsPoint(12, 13), QgsPoint(12, 13.5), QgsPoint(11.5, 13)])))
        item.setSymbol(
            QgsFillSymbol.createSimple({'color': '200,100,100', 'outline_color': 'black', 'outline_width': '2'}))
        item.setZIndex(1)
        i1_id = layer.addItem(item)

        item = QgsAnnotationLineItem(QgsLineString([QgsPoint(11, 13), QgsPoint(12, 13), QgsPoint(12, 15)]))
        item.setSymbol(QgsLineSymbol.createSimple({'color': '#ffff00', 'line_width': '3'}))
        item.setZIndex(2)
        i2_id = layer.addItem(item)

        item = QgsAnnotationMarkerItem(QgsPoint(12, 13))
        item.setSymbol(QgsMarkerSymbol.createSimple({'color': '100,200,200', 'size': '6', 'outline_color': 'black'}))
        item.setZIndex(3)
        i3_id = layer.addItem(item)

        layer.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        settings.setExtent(QgsRectangle(1250958, 1386945, 1420709, 1532518))
        settings.setOutputSize(QSize(200, 200))
        settings.setLayers([layer])

        job = QgsMapRendererSequentialJob(settings)
        job.start()
        job.waitForFinished()

        # check rendered item results
        item_results = job.takeRenderedItemResults()
        item_details = item_results.renderedItems()
        self.assertEqual(len(item_details), 3)
        self.assertEqual([i.layerId() for i in item_details], [layer.id()] * 3)
        self.assertCountEqual([i.itemId() for i in item_details], [i1_id, i2_id, i3_id])
        # bounds should be in map crs
        self.assertEqual(
            [QgsGeometry.fromRect(i.boundingBox()).asWkt(0) for i in item_details if i.itemId() == i1_id][0],
            'Polygon ((1280174 1459732, 1335834 1459732, 1335834 1516914, 1280174 1516914, 1280174 1459732))')
        self.assertEqual(
            [QgsGeometry.fromRect(i.boundingBox()).asWkt(0) for i in item_details if i.itemId() == i2_id][0],
            'Polygon ((1224514 1459732, 1335834 1459732, 1335834 1689200, 1224514 1689200, 1224514 1459732))')
        expected = 'Polygon ((1325786 1449684, 1345882 1449684, 1345882 1469780, 1325786 1469780, 1325786 1449684))'
        result = [QgsGeometry.fromRect(i.boundingBox()).asWkt(0) for i in item_details if i.itemId() == i3_id][0]
        self.assertTrue(compareWkt(result, expected, tol=1000), "mismatch Expected:\n{}\nGot:\n{}\n".format(expected,
                                                                                                            result))

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
