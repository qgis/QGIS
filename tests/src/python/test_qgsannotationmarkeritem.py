"""QGIS Unit tests for QgsAnnotationMarkerItem.

From build dir, run: ctest -R PyQgsAnnotationMarkerItem -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2020 by Nyall Dawson"
__date__ = "29/07/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

from qgis.PyQt.QtCore import QSize
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
    QgsAnnotationMarkerItem,
    QgsCoordinateReferenceSystem,
    QgsCoordinateTransform,
    QgsMapSettings,
    QgsMarkerSymbol,
    QgsPoint,
    QgsPointXY,
    QgsProject,
    QgsReadWriteContext,
    QgsRectangle,
    QgsRenderContext,
    QgsVertexId,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsAnnotationMarkerItem(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "annotation_layer"

    def testBasic(self):
        item = QgsAnnotationMarkerItem(QgsPoint(12, 13))

        self.assertEqual(item.geometry().x(), 12.0)
        self.assertEqual(item.geometry().y(), 13.0)

        item.setGeometry(QgsPoint(1000, 2000))
        item.setZIndex(11)
        self.assertEqual(item.geometry().x(), 1000.0)
        self.assertEqual(item.geometry().y(), 2000.0)
        self.assertEqual(item.zIndex(), 11)

        item.setSymbol(
            QgsMarkerSymbol.createSimple(
                {"color": "100,200,200", "size": "3", "outline_color": "black"}
            )
        )
        self.assertEqual(item.symbol()[0].color(), QColor(100, 200, 200))

    def test_nodes(self):
        """
        Test nodes for item
        """
        item = QgsAnnotationMarkerItem(QgsPoint(12, 13))
        self.assertEqual(
            item.nodesV2(QgsAnnotationItemEditContext()),
            [
                QgsAnnotationItemNode(
                    QgsVertexId(0, 0, 0),
                    QgsPointXY(12, 13),
                    Qgis.AnnotationItemNodeType.VertexHandle,
                )
            ],
        )

    def test_transform(self):
        item = QgsAnnotationMarkerItem(QgsPoint(12, 13))
        self.assertEqual(item.geometry().asWkt(), "POINT(12 13)")

        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationTranslateItem("", 100, 200),
                QgsAnnotationItemEditContext(),
            ),
            Qgis.AnnotationItemEditOperationResult.Success,
        )
        self.assertEqual(item.geometry().asWkt(), "POINT(112 213)")

    def test_apply_move_node_edit(self):
        item = QgsAnnotationMarkerItem(QgsPoint(12, 13))
        self.assertEqual(item.geometry().asWkt(), "POINT(12 13)")

        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationMoveNode(
                    "", QgsVertexId(0, 0, 1), QgsPoint(14, 13), QgsPoint(17, 18)
                ),
                QgsAnnotationItemEditContext(),
            ),
            Qgis.AnnotationItemEditOperationResult.Success,
        )
        self.assertEqual(item.geometry().asWkt(), "POINT(17 18)")

    def test_apply_delete_node_edit(self):
        item = QgsAnnotationMarkerItem(QgsPoint(12, 13))
        self.assertEqual(item.geometry().asWkt(), "POINT(12 13)")

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
        item = QgsAnnotationMarkerItem(QgsPoint(12, 13))
        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationAddNode("", QgsPoint(13, 14)),
                QgsAnnotationItemEditContext(),
            ),
            Qgis.AnnotationItemEditOperationResult.Invalid,
        )

    def test_transient_move_operation(self):
        item = QgsAnnotationMarkerItem(QgsPoint(12, 13))
        self.assertEqual(item.geometry().asWkt(), "POINT(12 13)")

        res = item.transientEditResultsV2(
            QgsAnnotationItemEditOperationMoveNode(
                "", QgsVertexId(0, 0, 0), QgsPoint(12, 13), QgsPoint(17, 18)
            ),
            QgsAnnotationItemEditContext(),
        )
        self.assertEqual(res.representativeGeometry().asWkt(), "Point (17 18)")

    def test_transient_translate_operation(self):
        item = QgsAnnotationMarkerItem(QgsPoint(12, 13))
        self.assertEqual(item.geometry().asWkt(), "POINT(12 13)")

        res = item.transientEditResultsV2(
            QgsAnnotationItemEditOperationTranslateItem("", 100, 200),
            QgsAnnotationItemEditContext(),
        )
        self.assertEqual(res.representativeGeometry().asWkt(), "Point (112 213)")

    def testReadWriteXml(self):
        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")

        item = QgsAnnotationMarkerItem(QgsPoint(12, 13))
        item.setSymbol(
            QgsMarkerSymbol.createSimple(
                {"color": "100,200,200", "size": "3", "outline_color": "black"}
            )
        )
        item.setZIndex(11)
        item.setUseSymbologyReferenceScale(True)
        item.setSymbologyReferenceScale(5000)

        self.assertTrue(item.writeXml(elem, doc, QgsReadWriteContext()))

        s2 = QgsAnnotationMarkerItem.create()
        self.assertTrue(s2.readXml(elem, QgsReadWriteContext()))

        self.assertEqual(s2.geometry().x(), 12.0)
        self.assertEqual(s2.geometry().y(), 13.0)
        self.assertEqual(s2.symbol()[0].color(), QColor(100, 200, 200))
        self.assertEqual(s2.zIndex(), 11)
        self.assertTrue(s2.useSymbologyReferenceScale())
        self.assertEqual(s2.symbologyReferenceScale(), 5000)

    def testClone(self):
        item = QgsAnnotationMarkerItem(QgsPoint(12, 13))
        item.setSymbol(
            QgsMarkerSymbol.createSimple(
                {"color": "100,200,200", "size": "3", "outline_color": "black"}
            )
        )
        item.setZIndex(11)
        item.setUseSymbologyReferenceScale(True)
        item.setSymbologyReferenceScale(5000)

        item2 = item.clone()
        self.assertEqual(item2.geometry().x(), 12.0)
        self.assertEqual(item2.geometry().y(), 13.0)
        self.assertEqual(item2.symbol()[0].color(), QColor(100, 200, 200))
        self.assertEqual(item2.zIndex(), 11)
        self.assertTrue(item2.useSymbologyReferenceScale())
        self.assertEqual(item2.symbologyReferenceScale(), 5000)

    def testRenderMarker(self):
        item = QgsAnnotationMarkerItem(QgsPoint(12, 13))
        item.setSymbol(
            QgsMarkerSymbol.createSimple(
                {"color": "100,200,200", "size": "3", "outline_color": "black"}
            )
        )

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        settings.setExtent(QgsRectangle(10, 10, 16, 16))
        settings.setOutputSize(QSize(300, 300))

        settings.setFlag(QgsMapSettings.Flag.Antialiasing, False)

        rc = QgsRenderContext.fromMapSettings(settings)
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

        self.assertTrue(self.image_check("marker_item", "marker_item", image))

    def testRenderWithTransform(self):
        item = QgsAnnotationMarkerItem(QgsPoint(12, 13))
        item.setSymbol(
            QgsMarkerSymbol.createSimple(
                {"color": "100,200,200", "size": "3", "outline_color": "black"}
            )
        )

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        settings.setExtent(QgsRectangle(1250958, 1386945, 1420709, 1532518))
        settings.setOutputSize(QSize(300, 300))

        settings.setFlag(QgsMapSettings.Flag.Antialiasing, False)

        rc = QgsRenderContext.fromMapSettings(settings)
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
            self.image_check("marker_item_transform", "marker_item_transform", image)
        )


if __name__ == "__main__":
    unittest.main()
