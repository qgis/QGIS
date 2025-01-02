"""QGIS Unit tests for QgsAnnotationPointTextItem.

From build dir, run: ctest -R PyQgsAnnotationPointTextItem -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2020 by Nyall Dawson"
__date__ = "10/08/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

from qgis.PyQt.QtCore import QSize, Qt
from qgis.PyQt.QtGui import QColor, QImage, QPainter
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    Qgis,
    QgsAnnotationItemEditContext,
    QgsAnnotationItemEditOperationAddNode,
    QgsAnnotationItemEditOperationDeleteNode,
    QgsAnnotationItemEditOperationMoveNode,
    QgsAnnotationItemEditOperationTranslateItem,
    QgsAnnotationItemNode,
    QgsAnnotationPointTextItem,
    QgsCoordinateReferenceSystem,
    QgsCoordinateTransform,
    QgsMapSettings,
    QgsPoint,
    QgsPointXY,
    QgsProject,
    QgsReadWriteContext,
    QgsRectangle,
    QgsRenderContext,
    QgsTextFormat,
    QgsVertexId,
    QgsCallout,
    QgsBalloonCallout,
    QgsGeometry,
    QgsSimpleLineCallout,
)

import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import getTestFont, unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsAnnotationPointTextItem(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "annotation_layer"

    def testBasic(self):
        item = QgsAnnotationPointTextItem("my text", QgsPointXY(12, 13))

        self.assertEqual(item.text(), "my text")
        self.assertEqual(item.point().x(), 12.0)
        self.assertEqual(item.point().y(), 13.0)

        item.setText("tttttt")
        item.setPoint(QgsPointXY(1000, 2000))
        item.setAngle(55)
        item.setAlignment(Qt.AlignmentFlag.AlignRight)
        item.setRotationMode(Qgis.SymbolRotationMode.RespectMapRotation)
        item.setZIndex(11)

        format = QgsTextFormat()
        format.setSize(37)
        item.setFormat(format)

        self.assertEqual(item.text(), "tttttt")
        self.assertEqual(item.point().x(), 1000.0)
        self.assertEqual(item.point().y(), 2000.0)
        self.assertEqual(item.angle(), 55.0)
        self.assertEqual(item.alignment(), Qt.AlignmentFlag.AlignRight)
        self.assertEqual(item.zIndex(), 11)
        self.assertEqual(item.format().size(), 37)
        self.assertEqual(
            item.rotationMode(), Qgis.SymbolRotationMode.RespectMapRotation
        )

    def test_nodes(self):
        """
        Test nodes for item
        """
        item = QgsAnnotationPointTextItem("my text", QgsPointXY(12, 13))
        context = QgsAnnotationItemEditContext()
        context.setCurrentItemBounds(QgsRectangle(12, 13, 20, 23))
        self.assertEqual(
            item.nodesV2(context),
            [
                QgsAnnotationItemNode(
                    QgsVertexId(0, 0, 0),
                    QgsPointXY(12, 13),
                    Qgis.AnnotationItemNodeType.VertexHandle,
                ),
                QgsAnnotationItemNode(
                    QgsVertexId(0, 0, 1),
                    QgsPointXY(16, 18),
                    Qgis.AnnotationItemNodeType.CalloutHandle,
                ),
            ],
        )

    def test_transform(self):
        item = QgsAnnotationPointTextItem("my text", QgsPointXY(12, 13))
        self.assertEqual(item.point().asWkt(), "POINT(12 13)")

        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationTranslateItem("", 100, 200),
                QgsAnnotationItemEditContext(),
            ),
            Qgis.AnnotationItemEditOperationResult.Success,
        )
        self.assertEqual(item.point().asWkt(), "POINT(112 213)")

    def test_apply_move_node_edit(self):
        item = QgsAnnotationPointTextItem("my text", QgsPointXY(12, 13))
        self.assertEqual(item.point().asWkt(), "POINT(12 13)")

        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationMoveNode(
                    "", QgsVertexId(0, 0, 0), QgsPoint(14, 13), QgsPoint(17, 18)
                ),
                QgsAnnotationItemEditContext(),
            ),
            Qgis.AnnotationItemEditOperationResult.Success,
        )
        self.assertEqual(item.point().asWkt(), "POINT(17 18)")

        # move callout handle
        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationMoveNode(
                    "", QgsVertexId(0, 0, 1), QgsPoint(14, 13), QgsPoint(1, 3)
                ),
                QgsAnnotationItemEditContext(),
            ),
            Qgis.AnnotationItemEditOperationResult.Success,
        )
        self.assertEqual(item.point().asWkt(), "POINT(17 18)")
        self.assertEqual(item.calloutAnchor().asWkt(), "Point (1 3)")
        # callout should have been automatically created
        self.assertIsInstance(item.callout(), QgsCallout)

    def test_transient_move_operation(self):
        item = QgsAnnotationPointTextItem("my text", QgsPointXY(12, 13))
        self.assertEqual(item.point().asWkt(), "POINT(12 13)")

        res = item.transientEditResultsV2(
            QgsAnnotationItemEditOperationMoveNode(
                "", QgsVertexId(0, 0, 0), QgsPoint(12, 13), QgsPoint(17, 18)
            ),
            QgsAnnotationItemEditContext(),
        )
        self.assertEqual(res.representativeGeometry().asWkt(), "Point (17 18)")

    def test_transient_translate_operation(self):
        item = QgsAnnotationPointTextItem("my text", QgsPointXY(12, 13))
        self.assertEqual(item.point().asWkt(), "POINT(12 13)")

        res = item.transientEditResultsV2(
            QgsAnnotationItemEditOperationTranslateItem("", 100, 200),
            QgsAnnotationItemEditContext(),
        )
        self.assertEqual(res.representativeGeometry().asWkt(), "Point (112 213)")

    def test_apply_delete_node_edit(self):
        item = QgsAnnotationPointTextItem("my text", QgsPointXY(12, 13))
        self.assertEqual(item.point().asWkt(), "POINT(12 13)")

        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationDeleteNode(
                    "", QgsVertexId(0, 0, 0), QgsPoint(12, 13)
                ),
                QgsAnnotationItemEditContext(),
            ),
            Qgis.AnnotationItemEditOperationResult.ItemCleared,
        )

    def test_apply_add_node_edit(self):
        item = QgsAnnotationPointTextItem("my text", QgsPointXY(12, 13))
        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationAddNode("", QgsPoint(13, 14)),
                QgsAnnotationItemEditContext(),
            ),
            Qgis.AnnotationItemEditOperationResult.Invalid,
        )

    def testReadWriteXml(self):
        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")

        item = QgsAnnotationPointTextItem("my text", QgsPointXY(12, 13))
        item.setAngle(55)
        item.setAlignment(Qt.AlignmentFlag.AlignRight)
        item.setZIndex(11)
        format = QgsTextFormat()
        format.setSize(37)
        item.setFormat(format)
        item.setUseSymbologyReferenceScale(True)
        item.setSymbologyReferenceScale(5000)
        item.setRotationMode(Qgis.SymbolRotationMode.RespectMapRotation)
        item.setCalloutAnchor(QgsGeometry.fromWkt("Point(1 3)"))
        item.setCallout(QgsBalloonCallout())

        self.assertTrue(item.writeXml(elem, doc, QgsReadWriteContext()))

        s2 = QgsAnnotationPointTextItem.create()
        self.assertTrue(s2.readXml(elem, QgsReadWriteContext()))
        self.assertEqual(s2.text(), "my text")
        self.assertEqual(s2.point().x(), 12.0)
        self.assertEqual(s2.point().y(), 13.0)
        self.assertEqual(s2.angle(), 55.0)
        self.assertEqual(s2.alignment(), Qt.AlignmentFlag.AlignRight)
        self.assertEqual(s2.zIndex(), 11)
        self.assertEqual(s2.format().size(), 37)
        self.assertTrue(s2.useSymbologyReferenceScale())
        self.assertEqual(s2.symbologyReferenceScale(), 5000)
        self.assertEqual(s2.rotationMode(), Qgis.SymbolRotationMode.RespectMapRotation)
        self.assertEqual(s2.calloutAnchor().asWkt(), "Point (1 3)")
        self.assertIsInstance(s2.callout(), QgsBalloonCallout)

    def testClone(self):
        item = QgsAnnotationPointTextItem("my text", QgsPointXY(12, 13))
        item.setAngle(55)
        item.setAlignment(Qt.AlignmentFlag.AlignRight)
        item.setZIndex(11)
        format = QgsTextFormat()
        format.setSize(37)
        item.setFormat(format)
        item.setUseSymbologyReferenceScale(True)
        item.setSymbologyReferenceScale(5000)
        item.setRotationMode(Qgis.SymbolRotationMode.RespectMapRotation)
        item.setCalloutAnchor(QgsGeometry.fromWkt("Point(1 3)"))
        item.setCallout(QgsBalloonCallout())

        item2 = item.clone()
        self.assertEqual(item2.text(), "my text")
        self.assertEqual(item2.point().x(), 12.0)
        self.assertEqual(item2.point().y(), 13.0)
        self.assertEqual(item2.angle(), 55.0)
        self.assertEqual(item2.alignment(), Qt.AlignmentFlag.AlignRight)
        self.assertEqual(item2.zIndex(), 11)
        self.assertEqual(item2.format().size(), 37)
        self.assertTrue(item2.useSymbologyReferenceScale())
        self.assertEqual(item2.symbologyReferenceScale(), 5000)
        self.assertEqual(
            item2.rotationMode(), Qgis.SymbolRotationMode.RespectMapRotation
        )
        self.assertEqual(item2.calloutAnchor().asWkt(), "Point (1 3)")
        self.assertIsInstance(item2.callout(), QgsBalloonCallout)

    def testRenderMarker(self):
        item = QgsAnnotationPointTextItem("my text", QgsPointXY(12.3, 13.2))

        format = QgsTextFormat.fromQFont(getTestFont("Bold"))
        format.setColor(QColor(255, 0, 0))
        format.setOpacity(150 / 255)
        format.setSize(20)
        item.setFormat(format)

        item.setAngle(-30)
        item.setAlignment(Qt.AlignmentFlag.AlignRight)

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        settings.setExtent(QgsRectangle(10, 10, 16, 16))
        settings.setOutputSize(QSize(300, 300))

        settings.setFlag(QgsMapSettings.Flag.Antialiasing, False)

        rc = QgsRenderContext.fromMapSettings(settings)
        rc.setScaleFactor(96 / 25.4)  # 96 DPI
        image = QImage(200, 200, QImage.Format.Format_ARGB32)
        image.setDotsPerMeterX(int(96 / 25.4 * 1000))
        image.setDotsPerMeterY(int(96 / 25.4 * 1000))
        image.fill(QColor(255, 255, 255))
        painter = QPainter(image)
        rc.setPainter(painter)

        try:
            item.render(rc, None)
        finally:
            painter.end()

        self.assertTrue(self.image_check("pointtext_item", "pointtext_item", image))

    def testRenderMapRotation(self):
        item = QgsAnnotationPointTextItem("my text", QgsPointXY(12.3, 13.2))

        format = QgsTextFormat.fromQFont(getTestFont("Bold"))
        format.setColor(QColor(255, 0, 0))
        format.setOpacity(150 / 255)
        format.setSize(20)
        item.setFormat(format)

        item.setAngle(-30)
        item.setAlignment(Qt.AlignmentFlag.AlignRight)

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        settings.setExtent(QgsRectangle(10, 10, 16, 16))
        settings.setOutputSize(QSize(300, 300))
        settings.setRotation(90)

        settings.setFlag(QgsMapSettings.Flag.Antialiasing, False)

        rc = QgsRenderContext.fromMapSettings(settings)
        rc.setScaleFactor(96 / 25.4)  # 96 DPI
        image = QImage(200, 200, QImage.Format.Format_ARGB32)
        image.setDotsPerMeterX(int(96 / 25.4 * 1000))
        image.setDotsPerMeterY(int(96 / 25.4 * 1000))
        image.fill(QColor(255, 255, 255))
        painter = QPainter(image)
        rc.setPainter(painter)

        try:
            item.render(rc, None)
        finally:
            painter.end()

        self.assertTrue(
            self.image_check(
                "pointtext_item_ignore_map_rotation",
                "pointtext_item_ignore_map_rotation",
                image,
            )
        )

    def testRenderRespectMapRotation(self):
        item = QgsAnnotationPointTextItem("my text", QgsPointXY(12.3, 13.2))

        format = QgsTextFormat.fromQFont(getTestFont("Bold"))
        format.setColor(QColor(255, 0, 0))
        format.setOpacity(150 / 255)
        format.setSize(20)
        item.setRotationMode(Qgis.SymbolRotationMode.RespectMapRotation)

        item.setFormat(format)

        item.setAngle(-30)
        item.setAlignment(Qt.AlignmentFlag.AlignRight)

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        settings.setExtent(QgsRectangle(10, 10, 16, 16))
        settings.setOutputSize(QSize(300, 300))
        settings.setRotation(90)

        settings.setFlag(QgsMapSettings.Flag.Antialiasing, False)

        rc = QgsRenderContext.fromMapSettings(settings)
        rc.setScaleFactor(96 / 25.4)  # 96 DPI
        image = QImage(200, 200, QImage.Format.Format_ARGB32)
        image.setDotsPerMeterX(int(96 / 25.4 * 1000))
        image.setDotsPerMeterY(int(96 / 25.4 * 1000))
        image.fill(QColor(255, 255, 255))
        painter = QPainter(image)
        rc.setPainter(painter)

        try:
            item.render(rc, None)
        finally:
            painter.end()

        self.assertTrue(
            self.image_check(
                "pointtext_item_respect_map_rotation",
                "pointtext_item_respect_map_rotation",
                image,
            )
        )

    def testRenderMarkerExpression(self):
        item = QgsAnnotationPointTextItem("[% 1 + 1.5 %]", QgsPointXY(12.3, 13.2))

        format = QgsTextFormat.fromQFont(getTestFont("Bold"))
        format.setColor(QColor(255, 0, 0))
        format.setOpacity(150 / 255)
        format.setSize(20)
        item.setFormat(format)

        item.setAngle(-30)
        item.setAlignment(Qt.AlignmentFlag.AlignRight)

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        settings.setExtent(QgsRectangle(10, 10, 16, 16))
        settings.setOutputSize(QSize(300, 300))

        settings.setFlag(QgsMapSettings.Flag.Antialiasing, False)

        rc = QgsRenderContext.fromMapSettings(settings)
        rc.setScaleFactor(96 / 25.4)  # 96 DPI
        image = QImage(200, 200, QImage.Format.Format_ARGB32)
        image.setDotsPerMeterX(int(96 / 25.4 * 1000))
        image.setDotsPerMeterY(int(96 / 25.4 * 1000))
        image.fill(QColor(255, 255, 255))
        painter = QPainter(image)
        rc.setPainter(painter)

        try:
            item.render(rc, None)
        finally:
            painter.end()

        self.assertTrue(
            self.image_check(
                "pointtext_item_expression", "pointtext_item_expression", image
            )
        )

    def testRenderWithTransform(self):
        item = QgsAnnotationPointTextItem("my text", QgsPointXY(12.3, 13.2))

        format = QgsTextFormat.fromQFont(getTestFont("Bold"))
        format.setColor(QColor(255, 0, 0))
        format.setOpacity(150 / 255)
        format.setSize(20)
        item.setFormat(format)

        item.setAngle(-30)
        item.setAlignment(Qt.AlignmentFlag.AlignRight)

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        settings.setExtent(QgsRectangle(1250958, 1386945, 1420709, 1532518))
        settings.setOutputSize(QSize(300, 300))

        settings.setFlag(QgsMapSettings.Flag.Antialiasing, False)

        rc = QgsRenderContext.fromMapSettings(settings)
        rc.setScaleFactor(96 / 25.4)  # 96 DPI
        rc.setCoordinateTransform(
            QgsCoordinateTransform(
                QgsCoordinateReferenceSystem("EPSG:4326"),
                settings.destinationCrs(),
                QgsProject.instance(),
            )
        )
        image = QImage(200, 200, QImage.Format.Format_ARGB32)
        image.setDotsPerMeterX(int(96 / 25.4 * 1000))
        image.setDotsPerMeterY(int(96 / 25.4 * 1000))
        image.fill(QColor(255, 255, 255))
        painter = QPainter(image)
        rc.setPainter(painter)

        try:
            item.render(rc, None)
        finally:
            painter.end()

        self.assertTrue(
            self.image_check(
                "pointtext_item_transform", "pointtext_item_transform", image
            )
        )

    def testRenderCallout(self):
        item = QgsAnnotationPointTextItem("my text", QgsPointXY(12.3, 13.2))
        item.setCalloutAnchor(QgsGeometry.fromWkt("Point(1 5)"))
        callout = QgsSimpleLineCallout()
        callout.lineSymbol().setWidth(1)
        item.setCallout(callout)

        format = QgsTextFormat.fromQFont(getTestFont("Bold"))
        format.setColor(QColor(255, 0, 0))
        format.setOpacity(150 / 255)
        format.setSize(20)
        item.setFormat(format)

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        settings.setExtent(QgsRectangle(0, 5, 26, 11))
        settings.setOutputSize(QSize(300, 300))

        settings.setFlag(QgsMapSettings.Flag.Antialiasing, False)

        rc = QgsRenderContext.fromMapSettings(settings)
        rc.setScaleFactor(96 / 25.4)  # 96 DPI
        image = QImage(200, 200, QImage.Format.Format_ARGB32)
        image.setDotsPerMeterX(int(96 / 25.4 * 1000))
        image.setDotsPerMeterY(int(96 / 25.4 * 1000))
        image.fill(QColor(255, 255, 255))
        painter = QPainter(image)
        rc.setPainter(painter)

        try:
            item.render(rc, None)
        finally:
            painter.end()

        self.assertTrue(
            self.image_check("pointtext_callout", "pointtext_callout", image)
        )


if __name__ == "__main__":
    unittest.main()
