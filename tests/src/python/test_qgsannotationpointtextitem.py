# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsAnnotationPointTextItem.

From build dir, run: ctest -R PyQgsAnnotationPointTextItem -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2020 by Nyall Dawson'
__date__ = '10/08/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

from qgis.PyQt.QtCore import (QSize,
                              QDir,
                              Qt)
from qgis.PyQt.QtGui import (QImage,
                             QPainter,
                             QColor,
                             QTransform)
from qgis.core import (QgsMapSettings,
                       QgsCoordinateTransform,
                       QgsProject,
                       QgsPointXY,
                       QgsCoordinateReferenceSystem,
                       QgsMarkerSymbol,
                       QgsRenderChecker,
                       QgsReadWriteContext,
                       QgsRenderContext,
                       QgsAnnotationPointTextItem,
                       QgsRectangle,
                       QgsTextFormat,
                       QgsAnnotationItemNode,
                       Qgis,
                       QgsVertexId,
                       QgsAnnotationItemEditOperationMoveNode,
                       QgsAnnotationItemEditOperationDeleteNode,
                       QgsAnnotationItemEditOperationAddNode,
                       QgsAnnotationItemEditOperationTranslateItem,
                       QgsPoint
                       )
from qgis.PyQt.QtXml import QDomDocument

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath, getTestFont

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsAnnotationPointTextItem(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.report = "<h1>Python QgsAnnotationPointTextItem Tests</h1>\n"

    @classmethod
    def tearDownClass(cls):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(cls.report)

    def testBasic(self):
        item = QgsAnnotationPointTextItem('my text', QgsPointXY(12, 13))

        self.assertEqual(item.text(), 'my text')
        self.assertEqual(item.point().x(), 12.0)
        self.assertEqual(item.point().y(), 13.0)

        item.setText('tttttt')
        item.setPoint(QgsPointXY(1000, 2000))
        item.setAngle(55)
        item.setAlignment(Qt.AlignRight)
        item.setZIndex(11)

        format = QgsTextFormat()
        format.setSize(37)
        item.setFormat(format)

        self.assertEqual(item.text(), 'tttttt')
        self.assertEqual(item.point().x(), 1000.0)
        self.assertEqual(item.point().y(), 2000.0)
        self.assertEqual(item.angle(), 55.0)
        self.assertEqual(item.alignment(), Qt.AlignRight)
        self.assertEqual(item.zIndex(), 11)
        self.assertEqual(item.format().size(), 37)

    def test_nodes(self):
        """
        Test nodes for item
        """
        item = QgsAnnotationPointTextItem('my text', QgsPointXY(12, 13))
        self.assertEqual(item.nodes(), [QgsAnnotationItemNode(QgsVertexId(0, 0, 0), QgsPointXY(12, 13), Qgis.AnnotationItemNodeType.VertexHandle)])

    def test_transform(self):
        item = QgsAnnotationPointTextItem('my text', QgsPointXY(12, 13))
        self.assertEqual(item.point().asWkt(), 'POINT(12 13)')

        self.assertEqual(item.applyEdit(QgsAnnotationItemEditOperationTranslateItem('', 100, 200)), Qgis.AnnotationItemEditOperationResult.Success)
        self.assertEqual(item.point().asWkt(), 'POINT(112 213)')

    def test_apply_move_node_edit(self):
        item = QgsAnnotationPointTextItem('my text', QgsPointXY(12, 13))
        self.assertEqual(item.point().asWkt(), 'POINT(12 13)')

        self.assertEqual(item.applyEdit(QgsAnnotationItemEditOperationMoveNode('', QgsVertexId(0, 0, 0), QgsPoint(14, 13), QgsPoint(17, 18))), Qgis.AnnotationItemEditOperationResult.Success)
        self.assertEqual(item.point().asWkt(), 'POINT(17 18)')

    def test_transient_move_operation(self):
        item = QgsAnnotationPointTextItem('my text', QgsPointXY(12, 13))
        self.assertEqual(item.point().asWkt(), 'POINT(12 13)')

        res = item.transientEditResults(QgsAnnotationItemEditOperationMoveNode('', QgsVertexId(0, 0, 0), QgsPoint(12, 13), QgsPoint(17, 18)))
        self.assertEqual(res.representativeGeometry().asWkt(), 'Point (17 18)')

    def test_transient_translate_operation(self):
        item = QgsAnnotationPointTextItem('my text', QgsPointXY(12, 13))
        self.assertEqual(item.point().asWkt(), 'POINT(12 13)')

        res = item.transientEditResults(QgsAnnotationItemEditOperationTranslateItem('', 100, 200))
        self.assertEqual(res.representativeGeometry().asWkt(), 'Point (112 213)')

    def test_apply_delete_node_edit(self):
        item = QgsAnnotationPointTextItem('my text', QgsPointXY(12, 13))
        self.assertEqual(item.point().asWkt(), 'POINT(12 13)')

        self.assertEqual(item.applyEdit(QgsAnnotationItemEditOperationDeleteNode('', QgsVertexId(0, 0, 0), QgsPoint(12, 13))), Qgis.AnnotationItemEditOperationResult.ItemCleared)

    def test_apply_add_node_edit(self):
        item = QgsAnnotationPointTextItem('my text', QgsPointXY(12, 13))
        self.assertEqual(item.applyEdit(QgsAnnotationItemEditOperationAddNode('', QgsPoint(13, 14))), Qgis.AnnotationItemEditOperationResult.Invalid)

    def testReadWriteXml(self):
        doc = QDomDocument("testdoc")
        elem = doc.createElement('test')

        item = QgsAnnotationPointTextItem('my text', QgsPointXY(12, 13))
        item.setAngle(55)
        item.setAlignment(Qt.AlignRight)
        item.setZIndex(11)
        format = QgsTextFormat()
        format.setSize(37)
        item.setFormat(format)
        item.setUseSymbologyReferenceScale(True)
        item.setSymbologyReferenceScale(5000)

        self.assertTrue(item.writeXml(elem, doc, QgsReadWriteContext()))

        s2 = QgsAnnotationPointTextItem.create()
        self.assertTrue(s2.readXml(elem, QgsReadWriteContext()))
        self.assertEqual(s2.text(), 'my text')
        self.assertEqual(s2.point().x(), 12.0)
        self.assertEqual(s2.point().y(), 13.0)
        self.assertEqual(s2.angle(), 55.0)
        self.assertEqual(s2.alignment(), Qt.AlignRight)
        self.assertEqual(s2.zIndex(), 11)
        self.assertEqual(s2.format().size(), 37)
        self.assertTrue(s2.useSymbologyReferenceScale())
        self.assertEqual(s2.symbologyReferenceScale(), 5000)

    def testClone(self):
        item = QgsAnnotationPointTextItem('my text', QgsPointXY(12, 13))
        item.setAngle(55)
        item.setAlignment(Qt.AlignRight)
        item.setZIndex(11)
        format = QgsTextFormat()
        format.setSize(37)
        item.setFormat(format)
        item.setUseSymbologyReferenceScale(True)
        item.setSymbologyReferenceScale(5000)

        item2 = item.clone()
        self.assertEqual(item2.text(), 'my text')
        self.assertEqual(item2.point().x(), 12.0)
        self.assertEqual(item2.point().y(), 13.0)
        self.assertEqual(item2.angle(), 55.0)
        self.assertEqual(item2.alignment(), Qt.AlignRight)
        self.assertEqual(item2.zIndex(), 11)
        self.assertEqual(item2.format().size(), 37)
        self.assertTrue(item2.useSymbologyReferenceScale())
        self.assertEqual(item2.symbologyReferenceScale(), 5000)

    def testRenderMarker(self):
        item = QgsAnnotationPointTextItem('my text', QgsPointXY(12.3, 13.2))

        format = QgsTextFormat.fromQFont(getTestFont('Bold'))
        format.setColor(QColor(255, 0, 0))
        format.setOpacity(150 / 255)
        format.setSize(20)
        item.setFormat(format)

        item.setAngle(30)
        item.setAlignment(Qt.AlignRight)

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        settings.setExtent(QgsRectangle(10, 10, 16, 16))
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

        self.assertTrue(self.imageCheck('pointtext_item', 'pointtext_item', image))

    def testRenderMarkerExpression(self):
        item = QgsAnnotationPointTextItem('[% 1 + 1.5 %]', QgsPointXY(12.3, 13.2))

        format = QgsTextFormat.fromQFont(getTestFont('Bold'))
        format.setColor(QColor(255, 0, 0))
        format.setOpacity(150 / 255)
        format.setSize(20)
        item.setFormat(format)

        item.setAngle(30)
        item.setAlignment(Qt.AlignRight)

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        settings.setExtent(QgsRectangle(10, 10, 16, 16))
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

        self.assertTrue(self.imageCheck('pointtext_item_expression', 'pointtext_item_expression', image))

    def testRenderWithTransform(self):
        item = QgsAnnotationPointTextItem('my text', QgsPointXY(12.3, 13.2))

        format = QgsTextFormat.fromQFont(getTestFont('Bold'))
        format.setColor(QColor(255, 0, 0))
        format.setOpacity(150 / 255)
        format.setSize(20)
        item.setFormat(format)

        item.setAngle(30)
        item.setAlignment(Qt.AlignRight)

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

        self.assertTrue(self.imageCheck('pointtext_item_transform', 'pointtext_item_transform', image))

    def imageCheck(self, name, reference_image, image):
        TestQgsAnnotationPointTextItem.report += "<h2>Render {}</h2>\n".format(name)
        temp_dir = QDir.tempPath() + '/'
        file_name = temp_dir + 'patch_' + name + ".png"
        image.save(file_name, "PNG")
        checker = QgsRenderChecker()
        checker.setControlPathPrefix("annotation_layer")
        checker.setControlName("expected_" + reference_image)
        checker.setRenderedImage(file_name)
        checker.setColorTolerance(2)
        result = checker.compareImages(name, 20)
        TestQgsAnnotationPointTextItem.report += checker.report()
        return result


if __name__ == '__main__':
    unittest.main()
