"""QGIS Unit tests for QgsAnnotationLineTextItem.

From build dir, run: ctest -R PyQgsAnnotationlINETextItem -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2020 by Nyall Dawson'
__date__ = '10/08/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA
from qgis.PyQt.QtCore import QDir, QSize, Qt
from qgis.PyQt.QtGui import QColor, QImage, QPainter
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    Qgis,
    QgsAnnotationItemEditOperationAddNode,
    QgsAnnotationItemEditOperationDeleteNode,
    QgsAnnotationItemEditOperationMoveNode,
    QgsAnnotationItemEditOperationTranslateItem,
    QgsAnnotationItemNode,
    QgsAnnotationLineTextItem,
    QgsCoordinateReferenceSystem,
    QgsCoordinateTransform,
    QgsMapSettings,
    QgsPoint,
    QgsPointXY,
    QgsProject,
    QgsReadWriteContext,
    QgsRectangle,
    QgsMultiRenderChecker,
    QgsRenderContext,
    QgsTextFormat,
    QgsVertexId,
    QgsLineString,
    QgsMapUnitScale,
)
from qgis.testing import start_app, unittest

from utilities import getTestFont, unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsAnnotationLineTextItem(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.report = "<h1>Python QgsAnnotationLineTextItem Tests</h1>\n"

    @classmethod
    def tearDownClass(cls):
        report_file_path = f"{QDir.tempPath()}/qgistest.html"
        with open(report_file_path, 'a') as report_file:
            report_file.write(cls.report)
        super().tearDownClass()

    def testBasic(self):
        item = QgsAnnotationLineTextItem('my text', QgsLineString(((12, 13), (13, 13.1), (14, 13))))

        self.assertEqual(item.text(), 'my text')
        self.assertEqual(item.geometry().asWkt(1), 'LineString (12 13, 13 13.1, 14 13)')

        self.assertEqual(item.offsetFromLine(), 0)
        self.assertEqual(item.offsetFromLineUnit(),
                         Qgis.RenderUnit.Millimeters)

        item.setText('tttttt')
        item.setGeometry(QgsLineString(((12, 13), (13, 13.1))))
        item.setZIndex(11)
        item.setOffsetFromLine(3.4)
        item.setOffsetFromLineUnit(Qgis.RenderUnit.Inches)
        item.setOffsetFromLineMapUnitScale(QgsMapUnitScale(5, 15))

        format = QgsTextFormat()
        format.setSize(37)
        item.setFormat(format)

        self.assertEqual(item.text(), 'tttttt')
        self.assertEqual(item.geometry().asWkt(1), 'LineString (12 13, 13 13.1)')
        self.assertEqual(item.zIndex(), 11)
        self.assertEqual(item.format().size(), 37)
        self.assertEqual(item.offsetFromLine(), 3.4)
        self.assertEqual(item.offsetFromLineUnit(),
                         Qgis.RenderUnit.Inches)
        self.assertEqual(item.offsetFromLineMapUnitScale().minScale,
                         5)
        self.assertEqual(item.offsetFromLineMapUnitScale().maxScale,
                         15)

    def test_nodes(self):
        """
        Test nodes for item
        """
        item = QgsAnnotationLineTextItem('my text', QgsLineString(((12, 13), (13, 13.1), (14, 13))))
        self.assertEqual(item.nodes(), [
            QgsAnnotationItemNode(QgsVertexId(0, 0, 0), QgsPointXY(12, 13),
                                  Qgis.AnnotationItemNodeType.VertexHandle),
            QgsAnnotationItemNode(QgsVertexId(0, 0, 1), QgsPointXY(13, 13.1),
                                  Qgis.AnnotationItemNodeType.VertexHandle),
            QgsAnnotationItemNode(QgsVertexId(0, 0, 2), QgsPointXY(14, 13),
                                  Qgis.AnnotationItemNodeType.VertexHandle)
        ])

    def test_transform(self):
        item = QgsAnnotationLineTextItem('my text', QgsLineString(((12, 13), (13, 13.1), (14, 13))))
        self.assertEqual(item.geometry().asWkt(1), 'LineString (12 13, 13 13.1, 14 13)')

        self.assertEqual(item.applyEdit(QgsAnnotationItemEditOperationTranslateItem('', 100, 200)), Qgis.AnnotationItemEditOperationResult.Success)
        self.assertEqual(item.geometry().asWkt(1), 'LineString (112 213, 113 213.1, 114 213)')

    def test_apply_move_node_edit(self):
        item = QgsAnnotationLineTextItem('my text', QgsLineString(((12, 13), (13, 13.1), (14, 13))))
        self.assertEqual(item.geometry().asWkt(1), 'LineString (12 13, 13 13.1, 14 13)')

        self.assertEqual(item.applyEdit(QgsAnnotationItemEditOperationMoveNode('', QgsVertexId(0, 0, 1), QgsPoint(13, 13.1), QgsPoint(17, 18))), Qgis.AnnotationItemEditOperationResult.Success)
        self.assertEqual(item.geometry().asWkt(1), 'LineString (12 13, 17 18, 14 13)')

    def test_transient_move_operation(self):
        item = QgsAnnotationLineTextItem('my text', QgsLineString(((12, 13), (13, 13.1), (14, 13))))
        self.assertEqual(item.geometry().asWkt(1), 'LineString (12 13, 13 13.1, 14 13)')

        res = item.transientEditResults(QgsAnnotationItemEditOperationMoveNode('', QgsVertexId(0, 0, 1), QgsPoint(13, 13.1), QgsPoint(17, 18)))
        self.assertEqual(res.representativeGeometry().asWkt(1), 'LineString (12 13, 17 18, 14 13)')

    def test_transient_translate_operation(self):
        item = QgsAnnotationLineTextItem('my text', QgsLineString(((12, 13), (13, 13.1), (14, 13))))
        self.assertEqual(item.geometry().asWkt(1), 'LineString (12 13, 13 13.1, 14 13)')

        res = item.transientEditResults(QgsAnnotationItemEditOperationTranslateItem('', 100, 200))
        self.assertEqual(res.representativeGeometry().asWkt(1), 'LineString (112 213, 113 213.1, 114 213)')

    def test_apply_delete_node_edit(self):
        item = QgsAnnotationLineTextItem('my text', QgsLineString(((12, 13), (13, 13.1), (14, 13))))
        self.assertEqual(item.geometry().asWkt(1), 'LineString (12 13, 13 13.1, 14 13)')

        self.assertEqual(item.applyEdit(QgsAnnotationItemEditOperationDeleteNode('', QgsVertexId(0, 0, 1), QgsPoint(13, 13.1))), Qgis.AnnotationItemEditOperationResult.Success)
        self.assertEqual(item.geometry().asWkt(1),
                         'LineString (12 13, 14 13)')

        self.assertEqual(item.applyEdit(QgsAnnotationItemEditOperationDeleteNode('', QgsVertexId(0, 0, 1), QgsPoint(14, 13))), Qgis.AnnotationItemEditOperationResult.ItemCleared)

    def test_apply_add_node_edit(self):
        item = QgsAnnotationLineTextItem('my text', QgsLineString(((12, 13), (13, 13.1), (14, 13))))
        self.assertEqual(item.applyEdit(QgsAnnotationItemEditOperationAddNode('', QgsPoint(12.5, 12.8))), Qgis.AnnotationItemEditOperationResult.Success)
        self.assertEqual(item.geometry().asWkt(1),
                         'LineString (12 13, 12.5 13, 13 13.1, 14 13)')

    def testReadWriteXml(self):
        doc = QDomDocument("testdoc")
        elem = doc.createElement('test')

        item = QgsAnnotationLineTextItem('my text', QgsLineString(((12, 13), (13, 13.1), (14, 13))))
        item.setZIndex(11)
        format = QgsTextFormat()
        format.setSize(37)
        item.setFormat(format)
        item.setUseSymbologyReferenceScale(True)
        item.setSymbologyReferenceScale(5000)
        item.setOffsetFromLine(3.4)
        item.setOffsetFromLineUnit(Qgis.RenderUnit.Inches)
        item.setOffsetFromLineMapUnitScale(QgsMapUnitScale(5, 15))

        self.assertTrue(item.writeXml(elem, doc, QgsReadWriteContext()))

        s2 = QgsAnnotationLineTextItem.create()
        self.assertTrue(s2.readXml(elem, QgsReadWriteContext()))
        self.assertEqual(s2.text(), 'my text')
        self.assertEqual(s2.geometry().asWkt(1), 'LineString (12 13, 13 13.1, 14 13)')
        self.assertEqual(s2.zIndex(), 11)
        self.assertEqual(s2.format().size(), 37)
        self.assertTrue(s2.useSymbologyReferenceScale())
        self.assertEqual(s2.symbologyReferenceScale(), 5000)
        self.assertEqual(s2.offsetFromLine(), 3.4)
        self.assertEqual(s2.offsetFromLineUnit(),
                         Qgis.RenderUnit.Inches)
        self.assertEqual(s2.offsetFromLineMapUnitScale().minScale,
                         5)
        self.assertEqual(s2.offsetFromLineMapUnitScale().maxScale,
                         15)

    def testClone(self):
        item = QgsAnnotationLineTextItem('my text', QgsLineString(((12, 13), (13, 13.1), (14, 13))))
        item.setZIndex(11)
        format = QgsTextFormat()
        format.setSize(37)
        item.setFormat(format)
        item.setUseSymbologyReferenceScale(True)
        item.setSymbologyReferenceScale(5000)
        item.setOffsetFromLine(3.4)
        item.setOffsetFromLineUnit(Qgis.RenderUnit.Inches)
        item.setOffsetFromLineMapUnitScale(QgsMapUnitScale(5, 15))

        item2 = item.clone()
        self.assertEqual(item2.text(), 'my text')
        self.assertEqual(item2.geometry().asWkt(1), 'LineString (12 13, 13 13.1, 14 13)')
        self.assertEqual(item2.zIndex(), 11)
        self.assertEqual(item2.format().size(), 37)
        self.assertTrue(item2.useSymbologyReferenceScale())
        self.assertEqual(item2.symbologyReferenceScale(), 5000)
        self.assertEqual(item2.offsetFromLine(), 3.4)
        self.assertEqual(item2.offsetFromLineUnit(),
                         Qgis.RenderUnit.Inches)
        self.assertEqual(item2.offsetFromLineMapUnitScale().minScale,
                         5)
        self.assertEqual(item2.offsetFromLineMapUnitScale().maxScale,
                         15)

    def testRenderLine(self):
        item = QgsAnnotationLineTextItem('my text', QgsLineString(((12, 13), (13, 13.1), (14, 12))))

        format = QgsTextFormat.fromQFont(getTestFont('Bold'))
        format.setColor(QColor(255, 0, 0))
        format.setOpacity(150 / 255)
        format.setSize(55)
        item.setFormat(format)

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        settings.setExtent(QgsRectangle(11.9, 11.9, 14.5, 14))
        settings.setOutputSize(QSize(600, 300))

        settings.setFlag(QgsMapSettings.Antialiasing, False)

        rc = QgsRenderContext.fromMapSettings(settings)
        rc.setScaleFactor(96 / 25.4)  # 96 DPI
        image = QImage(600, 300, QImage.Format_ARGB32)
        image.setDotsPerMeterX(int(96 / 25.4 * 1000))
        image.setDotsPerMeterY(int(96 / 25.4 * 1000))
        image.fill(QColor(255, 255, 255))
        painter = QPainter(image)
        rc.setPainter(painter)

        try:
            item.render(rc, None)
        finally:
            painter.end()

        self.assertTrue(self.imageCheck('linetext_item', 'linetext_item', image))

    def testRenderLineOffsetPositive(self):
        item = QgsAnnotationLineTextItem('my text', QgsLineString(((12, 13), (13, 13.1), (14, 12))))

        format = QgsTextFormat.fromQFont(getTestFont('Bold'))
        format.setColor(QColor(255, 0, 0))
        format.setOpacity(150 / 255)
        format.setSize(55)
        item.setFormat(format)

        item.setOffsetFromLine(6)

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        settings.setExtent(QgsRectangle(11.9, 11.9, 14.5, 14))
        settings.setOutputSize(QSize(600, 300))

        settings.setFlag(QgsMapSettings.Antialiasing, False)

        rc = QgsRenderContext.fromMapSettings(settings)
        rc.setScaleFactor(96 / 25.4)  # 96 DPI
        image = QImage(600, 300, QImage.Format_ARGB32)
        image.setDotsPerMeterX(int(96 / 25.4 * 1000))
        image.setDotsPerMeterY(int(96 / 25.4 * 1000))
        image.fill(QColor(255, 255, 255))
        painter = QPainter(image)
        rc.setPainter(painter)

        try:
            item.render(rc, None)
        finally:
            painter.end()

        self.assertTrue(self.imageCheck('linetext_item_offset_positive', 'linetext_item_offset_positive', image))

    def testRenderLineOffsetNegative(self):
        item = QgsAnnotationLineTextItem('my text', QgsLineString(((12, 13), (13, 13.1), (14, 12))))

        format = QgsTextFormat.fromQFont(getTestFont('Bold'))
        format.setColor(QColor(255, 0, 0))
        format.setOpacity(150 / 255)
        format.setSize(55)
        item.setFormat(format)

        item.setOffsetFromLine(-6)

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        settings.setExtent(QgsRectangle(11.9, 11.9, 14.5, 14))
        settings.setOutputSize(QSize(600, 300))

        settings.setFlag(QgsMapSettings.Antialiasing, False)

        rc = QgsRenderContext.fromMapSettings(settings)
        rc.setScaleFactor(96 / 25.4)  # 96 DPI
        image = QImage(600, 300, QImage.Format_ARGB32)
        image.setDotsPerMeterX(int(96 / 25.4 * 1000))
        image.setDotsPerMeterY(int(96 / 25.4 * 1000))
        image.fill(QColor(255, 255, 255))
        painter = QPainter(image)
        rc.setPainter(painter)

        try:
            item.render(rc, None)
        finally:
            painter.end()

        self.assertTrue(self.imageCheck('linetext_item_offset_negative', 'linetext_item_offset_negative', image))

    def testRenderLineTruncate(self):
        item = QgsAnnotationLineTextItem('my text', QgsLineString(((12, 13), (13, 13.1), (14, 12))))

        format = QgsTextFormat.fromQFont(getTestFont('Bold'))
        format.setColor(QColor(255, 0, 0))
        format.setOpacity(150 / 255)
        format.setSize(75)
        item.setFormat(format)

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        settings.setExtent(QgsRectangle(11.9, 11.9, 14.5, 14))
        settings.setOutputSize(QSize(600, 300))

        settings.setFlag(QgsMapSettings.Antialiasing, False)

        rc = QgsRenderContext.fromMapSettings(settings)
        rc.setScaleFactor(96 / 25.4)  # 96 DPI
        image = QImage(600, 300, QImage.Format_ARGB32)
        image.setDotsPerMeterX(int(96 / 25.4 * 1000))
        image.setDotsPerMeterY(int(96 / 25.4 * 1000))
        image.fill(QColor(255, 255, 255))
        painter = QPainter(image)
        rc.setPainter(painter)

        try:
            item.render(rc, None)
        finally:
            painter.end()

        self.assertTrue(self.imageCheck('linetext_item_truncate', 'linetext_item_truncate', image))

    def testRenderLineTextExpression(self):
        item = QgsAnnotationLineTextItem('[% 1 + 1.5 %]', QgsLineString(((12, 13), (13, 13.1), (14, 12))))

        format = QgsTextFormat.fromQFont(getTestFont('Bold'))
        format.setColor(QColor(255, 0, 0))
        format.setOpacity(150 / 255)
        format.setSize(55)
        item.setFormat(format)

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        settings.setExtent(QgsRectangle(11.9, 11.9, 14.5, 14))
        settings.setOutputSize(QSize(600, 300))

        settings.setFlag(QgsMapSettings.Antialiasing, False)

        rc = QgsRenderContext.fromMapSettings(settings)
        rc.setScaleFactor(96 / 25.4)  # 96 DPI
        image = QImage(600, 300, QImage.Format_ARGB32)
        image.setDotsPerMeterX(int(96 / 25.4 * 1000))
        image.setDotsPerMeterY(int(96 / 25.4 * 1000))
        image.fill(QColor(255, 255, 255))
        painter = QPainter(image)
        rc.setPainter(painter)

        try:
            item.render(rc, None)
        finally:
            painter.end()

        self.assertTrue(self.imageCheck('linetext_item_expression', 'linetext_item_expression', image))

    def testRenderWithTransform(self):
        item = QgsAnnotationLineTextItem('my text', QgsLineString(
            ((12, 13), (13, 13.1), (14, 12))))

        format = QgsTextFormat.fromQFont(getTestFont('Bold'))
        format.setColor(QColor(255, 0, 0))
        format.setOpacity(150 / 255)
        format.setSize(55)
        item.setFormat(format)

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        settings.setExtent(QgsRectangle(1291958, 1386945, 1420709, 1532518))
        settings.setOutputSize(QSize(600, 300))

        settings.setFlag(QgsMapSettings.Antialiasing, False)

        rc = QgsRenderContext.fromMapSettings(settings)
        rc.setScaleFactor(96 / 25.4)  # 96 DPI
        rc.setCoordinateTransform(QgsCoordinateTransform(QgsCoordinateReferenceSystem('EPSG:4326'), settings.destinationCrs(), QgsProject.instance()))
        image = QImage(600, 300, QImage.Format_ARGB32)
        image.setDotsPerMeterX(int(96 / 25.4 * 1000))
        image.setDotsPerMeterY(int(96 / 25.4 * 1000))
        image.fill(QColor(255, 255, 255))
        painter = QPainter(image)
        rc.setPainter(painter)

        try:
            item.render(rc, None)
        finally:
            painter.end()

        self.assertTrue(self.imageCheck('linetext_item_transform', 'linetext_item_transform', image))

    def imageCheck(self, name, reference_image, image):
        TestQgsAnnotationLineTextItem.report += f"<h2>Render {name}</h2>\n"
        temp_dir = QDir.tempPath() + '/'
        file_name = temp_dir + 'annotation_' + name + ".png"
        image.save(file_name, "PNG")
        checker = QgsMultiRenderChecker()
        checker.setControlPathPrefix("annotation_layer")
        checker.setControlName("expected_" + reference_image)
        checker.setRenderedImage(file_name)
        checker.setColorTolerance(2)
        result = checker.runTest(name, 20)
        TestQgsAnnotationLineTextItem.report += checker.report()
        return result


if __name__ == '__main__':
    unittest.main()
