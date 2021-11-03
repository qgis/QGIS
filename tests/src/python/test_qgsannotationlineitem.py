# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsAnnotationLineItem.

From build dir, run: ctest -R PyQgsAnnotationLineItem -V

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
                             QColor,
                             QTransform)
from qgis.core import (QgsMapSettings,
                       QgsCoordinateTransform,
                       QgsProject,
                       QgsPoint,
                       QgsCoordinateReferenceSystem,
                       QgsLineSymbol,
                       QgsRenderChecker,
                       QgsReadWriteContext,
                       QgsRenderContext,
                       QgsAnnotationLineItem,
                       QgsRectangle,
                       QgsLineString,
                       QgsCircularString,
                       QgsAnnotationItemNode,
                       QgsPointXY,
                       Qgis,
                       QgsVertexId,
                       QgsAnnotationItemEditOperationMoveNode,
                       QgsAnnotationItemEditOperationDeleteNode,
                       QgsAnnotationItemEditOperationTranslateItem,
                       QgsAnnotationItemEditOperationAddNode
                       )
from qgis.PyQt.QtXml import QDomDocument

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsAnnotationLineItem(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.report = "<h1>Python QgsAnnotationLineItem Tests</h1>\n"

    @classmethod
    def tearDownClass(cls):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(cls.report)

    def testBasic(self):
        item = QgsAnnotationLineItem(QgsLineString([QgsPoint(12, 13), QgsPoint(14, 13), QgsPoint(14, 15)]))

        self.assertEqual(item.geometry().asWkt(), 'LineString (12 13, 14 13, 14 15)')

        item.setGeometry(QgsLineString([QgsPoint(22, 23), QgsPoint(24, 23), QgsPoint(24, 25)]))
        item.setZIndex(11)
        self.assertEqual(item.geometry().asWkt(), 'LineString (22 23, 24 23, 24 25)')
        self.assertEqual(item.zIndex(), 11)

        item.setSymbol(QgsLineSymbol.createSimple({'color': '#ffff00', 'line_width': '3'}))
        self.assertEqual(item.symbol()[0].color(), QColor(255, 255, 0))

    def test_nodes(self):
        """
        Test nodes for item
        """
        item = QgsAnnotationLineItem(QgsLineString([QgsPoint(12, 13), QgsPoint(14, 13), QgsPoint(14, 15)]))
        self.assertEqual(item.nodes(), [QgsAnnotationItemNode(QgsVertexId(0, 0, 0), QgsPointXY(12, 13), Qgis.AnnotationItemNodeType.VertexHandle),
                                        QgsAnnotationItemNode(QgsVertexId(0, 0, 1), QgsPointXY(14, 13), Qgis.AnnotationItemNodeType.VertexHandle),
                                        QgsAnnotationItemNode(QgsVertexId(0, 0, 2), QgsPointXY(14, 15), Qgis.AnnotationItemNodeType.VertexHandle)])

    def test_transform(self):
        item = QgsAnnotationLineItem(QgsLineString([QgsPoint(12, 13), QgsPoint(14, 13), QgsPoint(14, 15)]))
        self.assertEqual(item.geometry().asWkt(), 'LineString (12 13, 14 13, 14 15)')

        self.assertEqual(item.applyEdit(QgsAnnotationItemEditOperationTranslateItem('', 100, 200)), Qgis.AnnotationItemEditOperationResult.Success)
        self.assertEqual(item.geometry().asWkt(), 'LineString (112 213, 114 213, 114 215)')

    def test_apply_move_node_edit(self):
        item = QgsAnnotationLineItem(QgsLineString([QgsPoint(12, 13), QgsPoint(14, 13), QgsPoint(14, 15)]))
        self.assertEqual(item.geometry().asWkt(), 'LineString (12 13, 14 13, 14 15)')

        self.assertEqual(item.applyEdit(QgsAnnotationItemEditOperationMoveNode('', QgsVertexId(0, 0, 1), QgsPoint(14, 13), QgsPoint(17, 18))), Qgis.AnnotationItemEditOperationResult.Success)
        self.assertEqual(item.geometry().asWkt(), 'LineString (12 13, 17 18, 14 15)')
        self.assertEqual(item.applyEdit(QgsAnnotationItemEditOperationMoveNode('', QgsVertexId(0, 0, 2), QgsPoint(14, 15), QgsPoint(19, 20))), Qgis.AnnotationItemEditOperationResult.Success)
        self.assertEqual(item.geometry().asWkt(), 'LineString (12 13, 17 18, 19 20)')
        self.assertEqual(item.applyEdit(QgsAnnotationItemEditOperationMoveNode('', QgsVertexId(0, 0, 3), QgsPoint(14, 15), QgsPoint(19, 20))), Qgis.AnnotationItemEditOperationResult.Invalid)
        self.assertEqual(item.geometry().asWkt(), 'LineString (12 13, 17 18, 19 20)')

    def test_apply_delete_node_edit(self):
        item = QgsAnnotationLineItem(QgsLineString([QgsPoint(12, 13), QgsPoint(14, 13), QgsPoint(14, 15), QgsPoint(16, 17)]))
        self.assertEqual(item.geometry().asWkt(), 'LineString (12 13, 14 13, 14 15, 16 17)')

        self.assertEqual(item.applyEdit(QgsAnnotationItemEditOperationDeleteNode('', QgsVertexId(0, 0, 1), QgsPoint(14, 13))), Qgis.AnnotationItemEditOperationResult.Success)
        self.assertEqual(item.geometry().asWkt(), 'LineString (12 13, 14 15, 16 17)')
        self.assertEqual(item.applyEdit(QgsAnnotationItemEditOperationDeleteNode('', QgsVertexId(0, 0, 0), QgsPoint(12, 13))), Qgis.AnnotationItemEditOperationResult.Success)
        self.assertEqual(item.geometry().asWkt(), 'LineString (14 15, 16 17)')
        self.assertEqual(item.applyEdit(QgsAnnotationItemEditOperationDeleteNode('', QgsVertexId(0, 0, 3), QgsPoint(14, 15))), Qgis.AnnotationItemEditOperationResult.Invalid)
        self.assertEqual(item.geometry().asWkt(), 'LineString (14 15, 16 17)')
        self.assertEqual(item.applyEdit(QgsAnnotationItemEditOperationDeleteNode('', QgsVertexId(0, 0, 1), QgsPoint(16, 17))), Qgis.AnnotationItemEditOperationResult.ItemCleared)
        self.assertEqual(item.geometry().asWkt(), 'LineString EMPTY')

    def test_apply_add_node_edit(self):
        item = QgsAnnotationLineItem(QgsLineString([QgsPoint(12, 13), QgsPoint(14, 13), QgsPoint(14, 15), QgsPoint(16, 17)]))
        self.assertEqual(item.geometry().asWkt(), 'LineString (12 13, 14 13, 14 15, 16 17)')

        self.assertEqual(item.applyEdit(QgsAnnotationItemEditOperationAddNode('', QgsPoint(15, 16))), Qgis.AnnotationItemEditOperationResult.Success)
        self.assertEqual(item.geometry().asWkt(), 'LineString (12 13, 14 13, 14 15, 15 16, 16 17)')

    def test_transient_move_operation(self):
        item = QgsAnnotationLineItem(QgsLineString([QgsPoint(12, 13), QgsPoint(14, 13), QgsPoint(14, 15)]))
        self.assertEqual(item.geometry().asWkt(), 'LineString (12 13, 14 13, 14 15)')

        res = item.transientEditResults(QgsAnnotationItemEditOperationMoveNode('', QgsVertexId(0, 0, 1), QgsPoint(14, 13), QgsPoint(17, 18)))
        self.assertEqual(res.representativeGeometry().asWkt(), 'LineString (12 13, 17 18, 14 15)')

    def test_transient_translate_operation(self):
        item = QgsAnnotationLineItem(QgsLineString([QgsPoint(12, 13), QgsPoint(14, 13), QgsPoint(14, 15)]))
        self.assertEqual(item.geometry().asWkt(), 'LineString (12 13, 14 13, 14 15)')

        res = item.transientEditResults(QgsAnnotationItemEditOperationTranslateItem('', 100, 200))
        self.assertEqual(res.representativeGeometry().asWkt(), 'LineString (112 213, 114 213, 114 215)')

    def testReadWriteXml(self):
        doc = QDomDocument("testdoc")
        elem = doc.createElement('test')

        item = QgsAnnotationLineItem(QgsLineString([QgsPoint(12, 13), QgsPoint(14, 13), QgsPoint(14, 15)]))
        item.setSymbol(QgsLineSymbol.createSimple({'color': '#ffff00', 'line_width': '3'}))
        item.setZIndex(11)
        item.setUseSymbologyReferenceScale(True)
        item.setSymbologyReferenceScale(5000)

        self.assertTrue(item.writeXml(elem, doc, QgsReadWriteContext()))

        s2 = QgsAnnotationLineItem.create()
        self.assertTrue(s2.readXml(elem, QgsReadWriteContext()))

        self.assertEqual(s2.geometry().asWkt(), 'LineString (12 13, 14 13, 14 15)')
        self.assertEqual(s2.symbol()[0].color(), QColor(255, 255, 0))
        self.assertEqual(s2.zIndex(), 11)
        self.assertTrue(s2.useSymbologyReferenceScale())
        self.assertEqual(s2.symbologyReferenceScale(), 5000)

    def testClone(self):
        item = QgsAnnotationLineItem(QgsLineString([QgsPoint(12, 13), QgsPoint(14, 13), QgsPoint(14, 15)]))
        item.setSymbol(QgsLineSymbol.createSimple({'color': '#ffff00', 'line_width': '3'}))
        item.setZIndex(11)
        item.setUseSymbologyReferenceScale(True)
        item.setSymbologyReferenceScale(5000)

        item2 = item.clone()
        self.assertEqual(item2.geometry().asWkt(), 'LineString (12 13, 14 13, 14 15)')
        self.assertEqual(item2.symbol()[0].color(), QColor(255, 255, 0))
        self.assertEqual(item2.zIndex(), 11)
        self.assertTrue(item2.useSymbologyReferenceScale())
        self.assertEqual(item2.symbologyReferenceScale(), 5000)

    def testRenderLineString(self):
        item = QgsAnnotationLineItem(QgsLineString([QgsPoint(12, 13), QgsPoint(14, 13), QgsPoint(14, 15)]))
        item.setSymbol(QgsLineSymbol.createSimple({'color': '#ffff00', 'line_width': '3'}))

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
            item.render(rc, None)
        finally:
            painter.end()

        self.assertTrue(self.imageCheck('linestring_item', 'linestring_item', image))

    def testRenderCurve(self):
        item = QgsAnnotationLineItem(QgsCircularString(QgsPoint(12, 13.2), QgsPoint(14, 13.4), QgsPoint(14, 15)))
        item.setSymbol(QgsLineSymbol.createSimple({'color': '#ffff00', 'line_width': '3'}))

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
            item.render(rc, None)
        finally:
            painter.end()

        self.assertTrue(self.imageCheck('line_circularstring', 'line_circularstring', image))

    def testRenderWithTransform(self):
        item = QgsAnnotationLineItem(QgsLineString([QgsPoint(11, 13), QgsPoint(12, 13), QgsPoint(12, 15)]))
        item.setSymbol(QgsLineSymbol.createSimple({'color': '#ffff00', 'line_width': '3'}))

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        settings.setExtent(QgsRectangle(1250958, 1386945, 1420709, 1532518))
        settings.setOutputSize(QSize(300, 300))

        settings.setFlag(QgsMapSettings.Antialiasing, False)

        rc = QgsRenderContext.fromMapSettings(settings)
        rc.setCoordinateTransform(QgsCoordinateTransform(QgsCoordinateReferenceSystem('EPSG:4326'), settings.destinationCrs(), QgsProject.instance()))
        image = QImage(200, 200, QImage.Format_ARGB32)
        image.setDotsPerMeterX(int(96 / 25.4 * 1000))
        image.setDotsPerMeterY(int(96 / 25.4 * 1000))
        image.fill(QColor(255, 255, 255))
        painter = QPainter(image)
        rc.setPainter(painter)

        try:
            item.render(rc, None)
        finally:
            painter.end()

        self.assertTrue(self.imageCheck('linestring_item_transform', 'linestring_item_transform', image))

    def imageCheck(self, name, reference_image, image):
        TestQgsAnnotationLineItem.report += "<h2>Render {}</h2>\n".format(name)
        temp_dir = QDir.tempPath() + '/'
        file_name = temp_dir + 'patch_' + name + ".png"
        image.save(file_name, "PNG")
        checker = QgsRenderChecker()
        checker.setControlPathPrefix("annotation_layer")
        checker.setControlName("expected_" + reference_image)
        checker.setRenderedImage(file_name)
        checker.setColorTolerance(2)
        result = checker.compareImages(name, 20)
        TestQgsAnnotationLineItem.report += checker.report()
        return result


if __name__ == '__main__':
    unittest.main()
