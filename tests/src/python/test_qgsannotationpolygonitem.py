"""QGIS Unit tests for QgsAnnotationPolygonItem.

From build dir, run: ctest -R PyQgsAnnotationPolygonItem -V

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
    QgsAnnotationPolygonItem,
    QgsCircularString,
    QgsCoordinateReferenceSystem,
    QgsCoordinateTransform,
    QgsCurvePolygon,
    QgsFillSymbol,
    QgsLineString,
    QgsMapSettings,
    QgsPoint,
    QgsPointXY,
    QgsPolygon,
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


class TestQgsAnnotationPolygonItem(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "annotation_layer"

    def testBasic(self):
        item = QgsAnnotationPolygonItem(
            QgsPolygon(
                QgsLineString(
                    [
                        QgsPoint(12, 13),
                        QgsPoint(14, 13),
                        QgsPoint(14, 15),
                        QgsPoint(12, 13),
                    ]
                )
            )
        )

        self.assertEqual(
            item.geometry().asWkt(), "Polygon ((12 13, 14 13, 14 15, 12 13))"
        )

        item.setGeometry(
            QgsPolygon(
                QgsLineString(
                    [
                        QgsPoint(22, 23),
                        QgsPoint(24, 23),
                        QgsPoint(24, 25),
                        QgsPoint(22, 23),
                    ]
                )
            )
        )
        item.setZIndex(11)
        self.assertEqual(
            item.geometry().asWkt(), "Polygon ((22 23, 24 23, 24 25, 22 23))"
        )
        self.assertEqual(item.zIndex(), 11)

        item.setSymbol(
            QgsFillSymbol.createSimple(
                {"color": "200,100,100", "outline_color": "black"}
            )
        )
        self.assertEqual(item.symbol()[0].color(), QColor(200, 100, 100))

    def test_nodes(self):
        """
        Test nodes for item
        """
        item = QgsAnnotationPolygonItem(
            QgsPolygon(
                QgsLineString(
                    [
                        QgsPoint(12, 13),
                        QgsPoint(14, 13),
                        QgsPoint(14, 15),
                        QgsPoint(12, 13),
                    ]
                )
            )
        )
        # nodes shouldn't form a closed ring
        self.assertEqual(
            item.nodesV2(QgsAnnotationItemEditContext()),
            [
                QgsAnnotationItemNode(
                    QgsVertexId(0, 0, 0),
                    QgsPointXY(12, 13),
                    Qgis.AnnotationItemNodeType.VertexHandle,
                ),
                QgsAnnotationItemNode(
                    QgsVertexId(0, 0, 1),
                    QgsPointXY(14, 13),
                    Qgis.AnnotationItemNodeType.VertexHandle,
                ),
                QgsAnnotationItemNode(
                    QgsVertexId(0, 0, 2),
                    QgsPointXY(14, 15),
                    Qgis.AnnotationItemNodeType.VertexHandle,
                ),
            ],
        )

    def test_transform(self):
        item = QgsAnnotationPolygonItem(
            QgsPolygon(
                QgsLineString(
                    [
                        QgsPoint(12, 13),
                        QgsPoint(14, 13),
                        QgsPoint(14, 15),
                        QgsPoint(12, 13),
                    ]
                )
            )
        )
        self.assertEqual(
            item.geometry().asWkt(), "Polygon ((12 13, 14 13, 14 15, 12 13))"
        )

        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationTranslateItem("", 100, 200),
                QgsAnnotationItemEditContext(),
            ),
            Qgis.AnnotationItemEditOperationResult.Success,
        )
        self.assertEqual(
            item.geometry().asWkt(), "Polygon ((112 213, 114 213, 114 215, 112 213))"
        )

    def test_apply_move_node_edit(self):
        item = QgsAnnotationPolygonItem(
            QgsPolygon(
                QgsLineString(
                    [
                        QgsPoint(12, 13),
                        QgsPoint(14, 13),
                        QgsPoint(14, 15),
                        QgsPoint(12, 13),
                    ]
                )
            )
        )
        self.assertEqual(
            item.geometry().asWkt(), "Polygon ((12 13, 14 13, 14 15, 12 13))"
        )

        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationMoveNode(
                    "", QgsVertexId(0, 0, 1), QgsPoint(14, 13), QgsPoint(17, 18)
                ),
                QgsAnnotationItemEditContext(),
            ),
            Qgis.AnnotationItemEditOperationResult.Success,
        )
        self.assertEqual(
            item.geometry().asWkt(), "Polygon ((12 13, 17 18, 14 15, 12 13))"
        )
        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationMoveNode(
                    "", QgsVertexId(0, 0, 3), QgsPoint(12, 13), QgsPoint(19, 20)
                ),
                QgsAnnotationItemEditContext(),
            ),
            Qgis.AnnotationItemEditOperationResult.Success,
        )
        self.assertEqual(
            item.geometry().asWkt(), "Polygon ((19 20, 17 18, 14 15, 19 20))"
        )
        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationMoveNode(
                    "", QgsVertexId(0, 0, 4), QgsPoint(14, 15), QgsPoint(19, 20)
                ),
                QgsAnnotationItemEditContext(),
            ),
            Qgis.AnnotationItemEditOperationResult.Invalid,
        )
        self.assertEqual(
            item.geometry().asWkt(), "Polygon ((19 20, 17 18, 14 15, 19 20))"
        )

    def test_apply_delete_node_edit(self):
        item = QgsAnnotationPolygonItem(
            QgsPolygon(
                QgsLineString(
                    [
                        QgsPoint(12, 13),
                        QgsPoint(14, 13),
                        QgsPoint(14, 15),
                        QgsPoint(14.5, 15.5),
                        QgsPoint(14.5, 16.5),
                        QgsPoint(14.5, 17.5),
                        QgsPoint(12, 13),
                    ]
                )
            )
        )
        self.assertEqual(
            item.geometry().asWkt(),
            "Polygon ((12 13, 14 13, 14 15, 14.5 15.5, 14.5 16.5, 14.5 17.5, 12 13))",
        )

        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationDeleteNode(
                    "", QgsVertexId(0, 0, 1), QgsPoint(14, 13)
                ),
                QgsAnnotationItemEditContext(),
            ),
            Qgis.AnnotationItemEditOperationResult.Success,
        )
        self.assertEqual(
            item.geometry().asWkt(),
            "Polygon ((12 13, 14 15, 14.5 15.5, 14.5 16.5, 14.5 17.5, 12 13))",
        )
        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationDeleteNode(
                    "", QgsVertexId(0, 0, 2), QgsPoint(14.5, 15.5)
                ),
                QgsAnnotationItemEditContext(),
            ),
            Qgis.AnnotationItemEditOperationResult.Success,
        )
        self.assertEqual(
            item.geometry().asWkt(),
            "Polygon ((12 13, 14 15, 14.5 16.5, 14.5 17.5, 12 13))",
        )
        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationDeleteNode(
                    "", QgsVertexId(0, 0, 7), QgsPoint(14, 15)
                ),
                QgsAnnotationItemEditContext(),
            ),
            Qgis.AnnotationItemEditOperationResult.Invalid,
        )
        self.assertEqual(
            item.geometry().asWkt(),
            "Polygon ((12 13, 14 15, 14.5 16.5, 14.5 17.5, 12 13))",
        )
        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationDeleteNode(
                    "", QgsVertexId(0, 0, 0), QgsPoint(12, 13)
                ),
                QgsAnnotationItemEditContext(),
            ),
            Qgis.AnnotationItemEditOperationResult.Success,
        )
        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationDeleteNode(
                    "", QgsVertexId(0, 0, 0), QgsPoint(12, 13)
                ),
                QgsAnnotationItemEditContext(),
            ),
            Qgis.AnnotationItemEditOperationResult.ItemCleared,
        )
        self.assertEqual(item.geometry().asWkt(), "Polygon EMPTY")

    def test_apply_add_node_edit(self):
        item = QgsAnnotationPolygonItem(
            QgsPolygon(
                QgsLineString(
                    [
                        QgsPoint(12, 13),
                        QgsPoint(14, 13),
                        QgsPoint(14, 15),
                        QgsPoint(14.5, 15.5),
                        QgsPoint(14.5, 16.5),
                        QgsPoint(14.5, 17.5),
                        QgsPoint(12, 13),
                    ]
                )
            )
        )
        self.assertEqual(
            item.geometry().asWkt(),
            "Polygon ((12 13, 14 13, 14 15, 14.5 15.5, 14.5 16.5, 14.5 17.5, 12 13))",
        )

        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationAddNode("", QgsPoint(15, 16)),
                QgsAnnotationItemEditContext(),
            ),
            Qgis.AnnotationItemEditOperationResult.Success,
        )
        self.assertEqual(
            item.geometry().asWkt(),
            "Polygon ((12 13, 14 13, 14 15, 14.5 15.5, 14.5 16, 14.5 16.5, 14.5 17.5, 12 13))",
        )

    def test_transient_move_operation(self):
        item = QgsAnnotationPolygonItem(
            QgsPolygon(
                QgsLineString(
                    [
                        QgsPoint(12, 13),
                        QgsPoint(14, 13),
                        QgsPoint(14, 15),
                        QgsPoint(12, 13),
                    ]
                )
            )
        )
        self.assertEqual(
            item.geometry().asWkt(), "Polygon ((12 13, 14 13, 14 15, 12 13))"
        )

        res = item.transientEditResultsV2(
            QgsAnnotationItemEditOperationMoveNode(
                "", QgsVertexId(0, 0, 1), QgsPoint(14, 13), QgsPoint(17, 18)
            ),
            QgsAnnotationItemEditContext(),
        )
        self.assertEqual(
            res.representativeGeometry().asWkt(),
            "Polygon ((12 13, 17 18, 14 15, 12 13))",
        )

    def test_transient_translate_operation(self):
        item = QgsAnnotationPolygonItem(
            QgsPolygon(
                QgsLineString(
                    [
                        QgsPoint(12, 13),
                        QgsPoint(14, 13),
                        QgsPoint(14, 15),
                        QgsPoint(12, 13),
                    ]
                )
            )
        )
        self.assertEqual(
            item.geometry().asWkt(), "Polygon ((12 13, 14 13, 14 15, 12 13))"
        )

        res = item.transientEditResultsV2(
            QgsAnnotationItemEditOperationTranslateItem("", 100, 200),
            QgsAnnotationItemEditContext(),
        )
        self.assertEqual(
            res.representativeGeometry().asWkt(),
            "Polygon ((112 213, 114 213, 114 215, 112 213))",
        )

    def testReadWriteXml(self):
        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")

        item = QgsAnnotationPolygonItem(
            QgsPolygon(
                QgsLineString(
                    [
                        QgsPoint(12, 13),
                        QgsPoint(14, 13),
                        QgsPoint(14, 15),
                        QgsPoint(12, 13),
                    ]
                )
            )
        )
        item.setSymbol(
            QgsFillSymbol.createSimple(
                {"color": "200,100,100", "outline_color": "black"}
            )
        )
        item.setZIndex(11)
        item.setUseSymbologyReferenceScale(True)
        item.setSymbologyReferenceScale(5000)

        self.assertTrue(item.writeXml(elem, doc, QgsReadWriteContext()))

        s2 = QgsAnnotationPolygonItem.create()
        self.assertTrue(s2.readXml(elem, QgsReadWriteContext()))

        self.assertEqual(
            s2.geometry().asWkt(), "Polygon ((12 13, 14 13, 14 15, 12 13))"
        )
        self.assertEqual(s2.symbol()[0].color(), QColor(200, 100, 100))
        self.assertEqual(s2.zIndex(), 11)
        self.assertTrue(s2.useSymbologyReferenceScale())
        self.assertEqual(s2.symbologyReferenceScale(), 5000)

    def testClone(self):
        item = QgsAnnotationPolygonItem(
            QgsPolygon(
                QgsLineString(
                    [
                        QgsPoint(12, 13),
                        QgsPoint(14, 13),
                        QgsPoint(14, 15),
                        QgsPoint(12, 13),
                    ]
                )
            )
        )
        item.setSymbol(
            QgsFillSymbol.createSimple(
                {"color": "200,100,100", "outline_color": "black"}
            )
        )
        item.setZIndex(11)
        item.setUseSymbologyReferenceScale(True)
        item.setSymbologyReferenceScale(5000)

        item2 = item.clone()
        self.assertEqual(
            item2.geometry().asWkt(), "Polygon ((12 13, 14 13, 14 15, 12 13))"
        )
        self.assertEqual(item2.symbol()[0].color(), QColor(200, 100, 100))
        self.assertEqual(item2.zIndex(), 11)
        self.assertTrue(item2.useSymbologyReferenceScale())
        self.assertEqual(item2.symbologyReferenceScale(), 5000)

    def testRenderPolygon(self):
        item = QgsAnnotationPolygonItem(
            QgsPolygon(
                QgsLineString(
                    [
                        QgsPoint(12, 13),
                        QgsPoint(14, 13),
                        QgsPoint(14, 15),
                        QgsPoint(12, 13),
                    ]
                )
            )
        )
        item.setSymbol(
            QgsFillSymbol.createSimple(
                {"color": "200,100,100", "outline_color": "black", "outline_width": "2"}
            )
        )

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        settings.setExtent(QgsRectangle(10, 10, 18, 18))
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

        self.assertTrue(self.image_check("polygon_item", "polygon_item", image))

    def testRenderCurvePolygon(self):
        cs = QgsCircularString()
        cs.setPoints(
            [
                QgsPoint(12, 13.2),
                QgsPoint(14, 13.4),
                QgsPoint(14, 15),
                QgsPoint(13, 15.1),
                QgsPoint(12, 13.2),
            ]
        )
        cp = QgsCurvePolygon()
        cp.setExteriorRing(cs)
        item = QgsAnnotationPolygonItem(cp)
        item.setSymbol(
            QgsFillSymbol.createSimple(
                {"color": "200,100,100", "outline_color": "black", "outline_width": "2"}
            )
        )

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        settings.setExtent(QgsRectangle(10, 10, 18, 18))
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

        self.assertTrue(
            self.image_check("curvepolygon_item", "curvepolygon_item", image)
        )

    def testRenderWithTransform(self):
        item = QgsAnnotationPolygonItem(
            QgsPolygon(
                QgsLineString(
                    [
                        QgsPoint(11.5, 13),
                        QgsPoint(12, 13),
                        QgsPoint(12, 13.5),
                        QgsPoint(11.5, 13),
                    ]
                )
            )
        )
        item.setSymbol(
            QgsFillSymbol.createSimple(
                {"color": "200,100,100", "outline_color": "black", "outline_width": "2"}
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
            self.image_check("polygon_item_transform", "polygon_item_transform", image)
        )


if __name__ == "__main__":
    unittest.main()
