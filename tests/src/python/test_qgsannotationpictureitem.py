"""QGIS Unit tests for QgsAnnotationPictureItem.

From build dir, run: ctest -R QgsAnnotationPictureItem -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import unittest

from qgis.core import (
    Qgis,
    QgsAnnotationItemEditContext,
    QgsAnnotationItemEditOperationAddNode,
    QgsAnnotationItemEditOperationDeleteNode,
    QgsAnnotationItemEditOperationMoveNode,
    QgsAnnotationItemEditOperationRotateItem,
    QgsAnnotationItemEditOperationTranslateItem,
    QgsAnnotationItemNode,
    QgsAnnotationPictureItem,
    QgsBalloonCallout,
    QgsCallout,
    QgsCircularString,
    QgsCoordinateReferenceSystem,
    QgsCoordinateTransform,
    QgsCurvePolygon,
    QgsFillSymbol,
    QgsGeometry,
    QgsLineString,
    QgsMapSettings,
    QgsPoint,
    QgsPointXY,
    QgsPolygon,
    QgsProject,
    QgsReadWriteContext,
    QgsRectangle,
    QgsRenderContext,
    QgsSimpleLineCallout,
    QgsVertexId,
)
from qgis.PyQt.QtCore import QSize, QSizeF
from qgis.PyQt.QtGui import QColor, QImage, QPainter
from qgis.PyQt.QtXml import QDomDocument
from qgis.testing import QgisTestCase, start_app
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsAnnotationPictureItem(QgisTestCase):
    @classmethod
    def control_path_prefix(cls):
        return "annotation_layer"

    def testBasic(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(10, 20, 30, 40),
        )

        self.assertEqual(
            item.path(), self.get_test_data_path("rgb256x256.png").as_posix()
        )
        self.assertEqual(item.format(), Qgis.PictureFormat.Raster)
        self.assertEqual(
            item.boundingBox().toString(3), "10.000,20.000 : 30.000,40.000"
        )
        self.assertEqual(
            item.placementMode(), Qgis.AnnotationPlacementMode.SpatialBounds
        )

        # rotation defaults
        self.assertEqual(item.rotation(), 0)
        self.assertEqual(item.rotationMode(), Qgis.SymbolRotationMode.IgnoreMapRotation)

        item.setBounds(QgsRectangle(100, 200, 300, 400))
        item.setZIndex(11)
        item.setPath(
            Qgis.PictureFormat.SVG, self.get_test_data_path("sample_svg.svg").as_posix()
        )
        item.setLockAspectRatio(False)
        item.setBackgroundEnabled(True)
        item.setFrameEnabled(True)
        item.setPlacementMode(Qgis.AnnotationPlacementMode.FixedSize)
        item.setFixedSize(QSizeF(56, 57))
        item.setFixedSizeUnit(Qgis.RenderUnit.Inches)
        item.setOffsetFromCallout(QSizeF(13.6, 17.2))
        item.setOffsetFromCalloutUnit(Qgis.RenderUnit.Inches)
        item.setRotation(45)
        item.setRotationMode(Qgis.SymbolRotationMode.RespectMapRotation)

        self.assertEqual(item.bounds().toString(3), "100.000,200.000 : 300.000,400.000")
        self.assertEqual(
            item.path(), self.get_test_data_path("sample_svg.svg").as_posix()
        )
        self.assertEqual(item.format(), Qgis.PictureFormat.SVG)
        self.assertEqual(item.zIndex(), 11)
        self.assertFalse(item.lockAspectRatio())
        self.assertTrue(item.backgroundEnabled())
        self.assertTrue(item.frameEnabled())
        self.assertEqual(item.placementMode(), Qgis.AnnotationPlacementMode.FixedSize)
        self.assertEqual(item.fixedSize(), QSizeF(56, 57))
        self.assertEqual(item.fixedSizeUnit(), Qgis.RenderUnit.Inches)
        self.assertEqual(item.offsetFromCallout(), QSizeF(13.6, 17.2))
        self.assertEqual(item.offsetFromCalloutUnit(), Qgis.RenderUnit.Inches)
        self.assertEqual(item.rotation(), 45)
        self.assertEqual(
            item.rotationMode(), Qgis.SymbolRotationMode.RespectMapRotation
        )

        item.setBackgroundSymbol(
            QgsFillSymbol.createSimple(
                {"color": "200,100,100", "outline_color": "black"}
            )
        )
        item.setFrameSymbol(
            QgsFillSymbol.createSimple(
                {"color": "100,200,250", "outline_color": "black"}
            )
        )
        self.assertEqual(item.backgroundSymbol()[0].color(), QColor(200, 100, 100))
        self.assertEqual(item.frameSymbol()[0].color(), QColor(100, 200, 250))

    def test_applied_rotation(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(10, 20, 30, 40),
        )
        item.setRotation(30)

        # IgnoreMapRotation (default): map rotation is ignored
        self.assertEqual(item.rotationMode(), Qgis.SymbolRotationMode.IgnoreMapRotation)
        self.assertEqual(item.appliedRotation(0), 30)
        self.assertEqual(item.appliedRotation(45), 30)

        # RespectMapRotation: map rotation is added to the item rotation
        item.setRotationMode(Qgis.SymbolRotationMode.RespectMapRotation)
        self.assertEqual(item.appliedRotation(0), 30)
        self.assertEqual(item.appliedRotation(45), 75)
        self.assertEqual(item.appliedRotation(-10), 20)

        # RespectMapRotation is ignored when the item is relative to the map frame
        item.setPlacementMode(Qgis.AnnotationPlacementMode.RelativeToMapFrame)
        self.assertEqual(item.appliedRotation(0), 30)
        self.assertEqual(item.appliedRotation(45), 30)

    def test_nodes_spatial_bounds(self):
        """
        Test nodes for item, spatial bounds mode
        """
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(10, 20, 30, 40),
        )
        # nodes shouldn't form a closed ring
        self.assertEqual(
            item.nodesV2(QgsAnnotationItemEditContext()),
            [
                QgsAnnotationItemNode(
                    QgsVertexId(0, 0, 0),
                    QgsPointXY(10, 20),
                    Qgis.AnnotationItemNodeType.VertexHandle,
                ),
                QgsAnnotationItemNode(
                    QgsVertexId(0, 0, 1),
                    QgsPointXY(30, 20),
                    Qgis.AnnotationItemNodeType.VertexHandle,
                ),
                QgsAnnotationItemNode(
                    QgsVertexId(0, 0, 2),
                    QgsPointXY(30, 40),
                    Qgis.AnnotationItemNodeType.VertexHandle,
                ),
                QgsAnnotationItemNode(
                    QgsVertexId(0, 0, 3),
                    QgsPointXY(10, 40),
                    Qgis.AnnotationItemNodeType.VertexHandle,
                ),
                QgsAnnotationItemNode(
                    QgsVertexId(1, 0, 0),
                    QgsPointXY(20, 30),
                    Qgis.AnnotationItemNodeType.CalloutHandle,
                ),
            ],
        )

    def test_nodes_fixed_size(self):
        """
        Test nodes for item, fixed size mode
        """
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(10, 20, 30, 40),
        )
        item.setPlacementMode(Qgis.AnnotationPlacementMode.FixedSize)
        context = QgsAnnotationItemEditContext()
        context.setCurrentItemBounds(QgsRectangle(10, 20, 30, 40))
        self.assertEqual(
            item.nodesV2(context),
            [
                QgsAnnotationItemNode(
                    QgsVertexId(0, 0, 0),
                    QgsPointXY(20, 30),
                    Qgis.AnnotationItemNodeType.VertexHandle,
                ),
                QgsAnnotationItemNode(
                    QgsVertexId(1, 0, 0),
                    QgsPointXY(10, 20),
                    Qgis.AnnotationItemNodeType.CalloutHandle,
                ),
            ],
        )

    def test_nodes_relative_to_map(self):
        """
        Test nodes for item, relative to map mode
        """
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(0.25, 0.75, 30, 40),
        )
        item.setPlacementMode(Qgis.AnnotationPlacementMode.RelativeToMapFrame)
        context = QgsAnnotationItemEditContext()
        context.setCurrentItemBounds(QgsRectangle(10, 20, 30, 40))
        self.assertEqual(
            item.nodesV2(context),
            [
                QgsAnnotationItemNode(
                    QgsVertexId(0, 0, 0),
                    QgsPointXY(20, 30),
                    Qgis.AnnotationItemNodeType.VertexHandle,
                )
            ],
        )

    def test_translate_spatial_bounds(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(10, 20, 30, 40),
        )
        self.assertEqual(item.bounds().toString(3), "10.000,20.000 : 30.000,40.000")

        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationTranslateItem("", 100, 200),
                QgsAnnotationItemEditContext(),
            ),
            Qgis.AnnotationItemEditOperationResult.Success,
        )
        self.assertEqual(item.bounds().toString(3), "110.000,220.000 : 130.000,240.000")

    def test_apply_rotate(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(10, 20, 30, 40),
        )
        self.assertEqual(item.rotation(), 0)

        # a rotate operation accumulates onto the item's rotation
        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationRotateItem("", 30),
                QgsAnnotationItemEditContext(),
            ),
            Qgis.AnnotationItemEditOperationResult.Success,
        )
        self.assertEqual(item.rotation(), 30)

        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationRotateItem("", 20),
                QgsAnnotationItemEditContext(),
            ),
            Qgis.AnnotationItemEditOperationResult.Success,
        )
        self.assertEqual(item.rotation(), 50)

        # rotation wraps around at 360 degrees
        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationRotateItem("", 350),
                QgsAnnotationItemEditContext(),
            ),
            Qgis.AnnotationItemEditOperationResult.Success,
        )
        self.assertEqual(item.rotation(), 40)

        # the bounds are not affected by a rotation
        self.assertEqual(item.bounds().toString(3), "10.000,20.000 : 30.000,40.000")

    def test_translate_fixed_size(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(10, 20, 30, 40),
        )
        item.setPlacementMode(Qgis.AnnotationPlacementMode.FixedSize)
        self.assertEqual(item.bounds().toString(3), "10.000,20.000 : 30.000,40.000")

        context = QgsAnnotationItemEditContext()
        render_context = QgsRenderContext()
        render_context.setScaleFactor(5)
        context.setRenderContext(render_context)

        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationTranslateItem("", 100, 200), context
            ),
            Qgis.AnnotationItemEditOperationResult.Success,
        )
        self.assertEqual(item.bounds().toString(3), "110.000,220.000 : 130.000,240.000")
        self.assertEqual(item.offsetFromCallout(), QSizeF())

    def test_translate_fixed_size_with_callout_anchor(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(10, 20, 30, 40),
        )
        item.setCalloutAnchor(QgsGeometry.fromWkt("Point(1 3)"))
        item.setCallout(QgsBalloonCallout())
        item.setPlacementMode(Qgis.AnnotationPlacementMode.FixedSize)
        self.assertEqual(item.bounds().toString(3), "10.000,20.000 : 30.000,40.000")

        context = QgsAnnotationItemEditContext()
        render_context = QgsRenderContext()
        render_context.setScaleFactor(5)
        context.setRenderContext(render_context)

        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationTranslateItem("", 100, 200, 50, 30),
                context,
            ),
            Qgis.AnnotationItemEditOperationResult.Success,
        )
        # should affect callout offset only
        self.assertEqual(item.offsetFromCallout(), QSizeF(9, 5))
        self.assertEqual(item.offsetFromCalloutUnit(), Qgis.RenderUnit.Millimeters)

    def test_translate_spatial_bounds_rotated_preview(self):
        """
        The transient (rubber band) geometry used while dragging a rotated item must
        be rotated to match how the item is rendered, otherwise the move preview shows
        an axis-aligned rectangle with the wrong angle.
        """
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            # 40 wide, 10 tall, centered on (100, 100)
            QgsRectangle(80, 95, 120, 105),
        )
        item.setRotation(90)

        settings = QgsMapSettings()
        settings.setExtent(QgsRectangle(0, 0, 200, 200))
        settings.setOutputSize(QSize(200, 200))
        context = QgsAnnotationItemEditContext()
        context.setRenderContext(QgsRenderContext.fromMapSettings(settings))

        res = item.transientEditResultsV2(
            QgsAnnotationItemEditOperationTranslateItem("", 20, 30), context
        )
        self.assertIsNotNone(res)
        bounds = res.representativeGeometry().boundingBox()
        # a 90 degree rotation swaps the footprint: 40x10 becomes 10x40, centered on
        # the translated center (120, 130)
        self.assertAlmostEqual(bounds.width(), 10, delta=0.5)
        self.assertAlmostEqual(bounds.height(), 40, delta=0.5)
        self.assertAlmostEqual(bounds.center().x(), 120, delta=0.5)
        self.assertAlmostEqual(bounds.center().y(), 130, delta=0.5)

    def test_translate_relative_to_map(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(0.2, 0.8, 30, 40),
        )
        item.setPlacementMode(Qgis.AnnotationPlacementMode.RelativeToMapFrame)
        self.assertEqual(item.bounds().toString(3), "0.200,0.800 : 30.000,40.000")

        context = QgsAnnotationItemEditContext()
        render_context = QgsRenderContext()
        render_context.setScaleFactor(5)
        render_context.setOutputSize(QSize(1000, 600))
        context.setRenderContext(render_context)

        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationTranslateItem("", 100, 200, 100, 200),
                context,
            ),
            Qgis.AnnotationItemEditOperationResult.Success,
        )
        self.assertEqual(item.bounds().toString(3), "0.300,1.133 : 30.100,40.333")
        self.assertEqual(item.offsetFromCallout(), QSizeF())

    def test_apply_move_node_edit_spatial_bounds(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(10, 20, 30, 40),
        )
        self.assertEqual(item.bounds().toString(3), "10.000,20.000 : 30.000,40.000")

        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationMoveNode(
                    "", QgsVertexId(0, 0, 1), QgsPoint(30, 20), QgsPoint(17, 18)
                ),
                QgsAnnotationItemEditContext(),
            ),
            Qgis.AnnotationItemEditOperationResult.Success,
        )
        self.assertEqual(item.bounds().toString(3), "10.000,18.000 : 17.000,40.000")
        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationMoveNode(
                    "", QgsVertexId(0, 0, 0), QgsPoint(10, 18), QgsPoint(5, 13)
                ),
                QgsAnnotationItemEditContext(),
            ),
            Qgis.AnnotationItemEditOperationResult.Success,
        )
        self.assertEqual(item.bounds().toString(3), "5.000,13.000 : 17.000,40.000")
        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationMoveNode(
                    "", QgsVertexId(0, 0, 2), QgsPoint(17, 14), QgsPoint(18, 38)
                ),
                QgsAnnotationItemEditContext(),
            ),
            Qgis.AnnotationItemEditOperationResult.Success,
        )
        self.assertEqual(item.bounds().toString(3), "5.000,13.000 : 18.000,38.000")
        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationMoveNode(
                    "", QgsVertexId(0, 0, 3), QgsPoint(5, 38), QgsPoint(2, 39)
                ),
                QgsAnnotationItemEditContext(),
            ),
            Qgis.AnnotationItemEditOperationResult.Success,
        )
        self.assertEqual(item.bounds().toString(3), "2.000,13.000 : 18.000,39.000")

        # move callout handle
        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationMoveNode(
                    "", QgsVertexId(1, 0, 0), QgsPoint(14, 13), QgsPoint(1, 3)
                ),
                QgsAnnotationItemEditContext(),
            ),
            Qgis.AnnotationItemEditOperationResult.Success,
        )
        self.assertEqual(item.bounds().toString(3), "2.000,13.000 : 18.000,39.000")
        self.assertEqual(item.calloutAnchor().asWkt(), "Point (1 3)")
        # callout should have been automatically created
        self.assertIsInstance(item.callout(), QgsCallout)

    def test_apply_move_node_edit_fixed_size(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(10, 20, 30, 40),
        )
        item.setPlacementMode(Qgis.AnnotationPlacementMode.FixedSize)
        self.assertEqual(item.bounds().toString(3), "10.000,20.000 : 30.000,40.000")

        context = QgsAnnotationItemEditContext()
        render_context = QgsRenderContext()
        render_context.setScaleFactor(5)
        context.setRenderContext(render_context)

        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationMoveNode(
                    "", QgsVertexId(0, 0, 0), QgsPoint(30, 20), QgsPoint(17, 18)
                ),
                context,
            ),
            Qgis.AnnotationItemEditOperationResult.Success,
        )
        self.assertEqual(item.bounds().toString(3), "7.000,8.000 : 27.000,28.000")

        # move callout handle
        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationMoveNode(
                    "", QgsVertexId(1, 0, 0), QgsPoint(14, 13), QgsPoint(1, 3)
                ),
                QgsAnnotationItemEditContext(),
            ),
            Qgis.AnnotationItemEditOperationResult.Success,
        )
        self.assertEqual(item.bounds().toString(3), "7.000,8.000 : 27.000,28.000")
        self.assertEqual(item.calloutAnchor().asWkt(), "Point (1 3)")
        # callout should have been automatically created
        self.assertIsInstance(item.callout(), QgsCallout)

    def test_apply_move_node_edit_relative_to_map(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(0.2, 0.8, 30, 40),
        )
        item.setPlacementMode(Qgis.AnnotationPlacementMode.RelativeToMapFrame)
        self.assertEqual(item.bounds().toString(3), "0.200,0.800 : 30.000,40.000")

        context = QgsAnnotationItemEditContext()
        render_context = QgsRenderContext()
        render_context.setScaleFactor(5)
        render_context.setOutputSize(QSize(2000, 600))
        context.setRenderContext(render_context)

        self.assertEqual(
            item.applyEditV2(
                QgsAnnotationItemEditOperationMoveNode(
                    "",
                    QgsVertexId(0, 0, 0),
                    QgsPoint(30, 20),
                    QgsPoint(17, 18),
                    100,
                    200,
                ),
                context,
            ),
            Qgis.AnnotationItemEditOperationResult.Success,
        )
        self.assertEqual(item.bounds().toString(3), "0.250,1.133 : 30.050,40.333")

    def test_apply_delete_node_edit(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(10, 20, 30, 40),
        )

        self.assertEqual(
            item.applyEdit(
                QgsAnnotationItemEditOperationDeleteNode(
                    "", QgsVertexId(0, 0, 1), QgsPoint(14, 13)
                )
            ),
            Qgis.AnnotationItemEditOperationResult.Invalid,
        )

    def test_apply_add_node_edit(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(10, 20, 30, 40),
        )

        self.assertEqual(
            item.applyEdit(QgsAnnotationItemEditOperationAddNode("", QgsPoint(15, 16))),
            Qgis.AnnotationItemEditOperationResult.Invalid,
        )

    def test_transient_move_operation_spatial_bounds(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(10, 20, 30, 40),
        )
        self.assertEqual(item.bounds().toString(3), "10.000,20.000 : 30.000,40.000")

        res = item.transientEditResultsV2(
            QgsAnnotationItemEditOperationMoveNode(
                "", QgsVertexId(0, 0, 1), QgsPoint(30, 20), QgsPoint(17, 18)
            ),
            QgsAnnotationItemEditContext(),
        )
        self.assertEqual(
            res.representativeGeometry().asWkt(),
            "Polygon ((10 18, 17 18, 17 40, 10 40, 10 18))",
        )

        # move callout handle
        res = item.transientEditResultsV2(
            QgsAnnotationItemEditOperationMoveNode(
                "", QgsVertexId(1, 0, 0), QgsPoint(14, 13), QgsPoint(1, 3)
            ),
            QgsAnnotationItemEditContext(),
        )
        self.assertEqual(res.representativeGeometry().asWkt(), "Point (1 3)")

    def test_transient_move_operation_fixed_size(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(10, 20, 30, 40),
        )
        item.setPlacementMode(Qgis.AnnotationPlacementMode.FixedSize)
        item.setFixedSize(QSizeF(56, 57))
        item.setFixedSizeUnit(Qgis.RenderUnit.Inches)
        self.assertEqual(item.bounds().toString(3), "10.000,20.000 : 30.000,40.000")

        op = QgsAnnotationItemEditOperationMoveNode(
            "", QgsVertexId(0, 0, 1), QgsPoint(30, 20), QgsPoint(17, 18)
        )
        context = QgsAnnotationItemEditContext()
        context.setCurrentItemBounds(QgsRectangle(1, 2, 3, 4))
        res = item.transientEditResultsV2(op, context)
        self.assertEqual(
            res.representativeGeometry().asWkt(),
            "Polygon ((16 17, 18 17, 18 19, 16 19, 16 17))",
        )

        # move callout handle
        res = item.transientEditResultsV2(
            QgsAnnotationItemEditOperationMoveNode(
                "", QgsVertexId(1, 0, 0), QgsPoint(14, 13), QgsPoint(1, 3)
            ),
            QgsAnnotationItemEditContext(),
        )
        self.assertEqual(res.representativeGeometry().asWkt(), "Point (1 3)")

    def test_transient_move_operation_relative_map(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(0.2, 0.8, 30, 40),
        )
        item.setPlacementMode(Qgis.AnnotationPlacementMode.RelativeToMapFrame)
        item.setFixedSize(QSizeF(56, 57))
        item.setFixedSizeUnit(Qgis.RenderUnit.Inches)
        self.assertEqual(item.bounds().toString(3), "0.200,0.800 : 30.000,40.000")

        op = QgsAnnotationItemEditOperationMoveNode(
            "", QgsVertexId(0, 0, 1), QgsPoint(30, 20), QgsPoint(17, 18), 100, 200
        )
        render_context = QgsRenderContext()
        render_context.setScaleFactor(5)
        render_context.setOutputSize(QSize(2000, 600))

        context = QgsAnnotationItemEditContext()
        context.setRenderContext(render_context)
        context.setCurrentItemBounds(QgsRectangle(1, 2, 3, 4))
        res = item.transientEditResultsV2(op, context)
        self.assertEqual(
            res.representativeGeometry().asWkt(),
            "Polygon ((-12 0, -10 0, -10 2, -12 2, -12 0))",
        )

    def test_transient_translate_operation_spatial_bounds(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(10, 20, 30, 40),
        )
        self.assertEqual(item.bounds().toString(3), "10.000,20.000 : 30.000,40.000")

        res = item.transientEditResultsV2(
            QgsAnnotationItemEditOperationTranslateItem("", 100, 200),
            QgsAnnotationItemEditContext(),
        )
        self.assertEqual(
            res.representativeGeometry().asWkt(),
            "Polygon ((110 220, 130 220, 130 240, 110 240, 110 220))",
        )

    def test_transient_translate_operation_fixed_size(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(10, 20, 30, 40),
        )
        item.setPlacementMode(Qgis.AnnotationPlacementMode.FixedSize)
        item.setFixedSize(QSizeF(56, 57))
        item.setFixedSizeUnit(Qgis.RenderUnit.Inches)
        self.assertEqual(item.bounds().toString(3), "10.000,20.000 : 30.000,40.000")

        op = QgsAnnotationItemEditOperationTranslateItem("", 100, 200)
        context = QgsAnnotationItemEditContext()
        context.setCurrentItemBounds(QgsRectangle(1, 2, 3, 4))
        render_context = QgsRenderContext()
        render_context.setScaleFactor(5)
        context.setRenderContext(render_context)

        res = item.transientEditResultsV2(op, context)
        self.assertEqual(
            res.representativeGeometry().asWkt(),
            "Polygon ((119 229, 121 229, 121 231, 119 231, 119 229))",
        )

    def test_transient_translate_operation_fixed_size_with_callout_anchor(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(10, 20, 30, 40),
        )
        item.setPlacementMode(Qgis.AnnotationPlacementMode.FixedSize)
        item.setFixedSize(QSizeF(56, 57))
        item.setFixedSizeUnit(Qgis.RenderUnit.Inches)
        item.setCalloutAnchor(QgsGeometry.fromWkt("Point(1 3)"))
        item.setCallout(QgsBalloonCallout())
        self.assertEqual(item.bounds().toString(3), "10.000,20.000 : 30.000,40.000")

        op = QgsAnnotationItemEditOperationTranslateItem("", 100, 200, 50, 30)
        context = QgsAnnotationItemEditContext()
        context.setCurrentItemBounds(QgsRectangle(1, 2, 3, 4))
        render_context = QgsRenderContext()
        render_context.setScaleFactor(0.5)
        context.setRenderContext(render_context)

        res = item.transientEditResultsV2(op, context)
        self.assertEqual(
            res.representativeGeometry().asWkt(2),
            "Polygon ((50.5 -26.5, 761.7 -26.5, 761.7 -750.4, 50.5 -750.4, 50.5 -26.5))",
        )

    def test_transient_translate_operation_relative_map(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(0.2, 0.8, 30, 40),
        )
        item.setPlacementMode(Qgis.AnnotationPlacementMode.RelativeToMapFrame)
        item.setFixedSize(QSizeF(56, 57))
        item.setFixedSizeUnit(Qgis.RenderUnit.Inches)
        self.assertEqual(item.bounds().toString(3), "0.200,0.800 : 30.000,40.000")

        op = QgsAnnotationItemEditOperationTranslateItem("", 100, 200, 100, 200)
        context = QgsAnnotationItemEditContext()
        context.setCurrentItemBounds(QgsRectangle(1, 2, 3, 4))
        render_context = QgsRenderContext()
        render_context.setScaleFactor(5)
        context.setRenderContext(render_context)

        res = item.transientEditResultsV2(op, context)
        self.assertEqual(
            res.representativeGeometry().asWkt(),
            "Polygon ((101 202, 103 202, 103 204, 101 204, 101 202))",
        )

    def testReadWriteXml(self):
        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")

        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(10, 20, 30, 40),
        )
        item.setBackgroundEnabled(True)
        item.setBackgroundSymbol(
            QgsFillSymbol.createSimple(
                {"color": "200,100,100", "outline_color": "black"}
            )
        )
        item.setFrameEnabled(True)
        item.setFrameSymbol(
            QgsFillSymbol.createSimple(
                {"color": "100,200,150", "outline_color": "black"}
            )
        )
        item.setZIndex(11)
        item.setLockAspectRatio(False)
        item.setPlacementMode(Qgis.AnnotationPlacementMode.FixedSize)
        item.setFixedSize(QSizeF(56, 57))
        item.setFixedSizeUnit(Qgis.RenderUnit.Inches)
        item.setCalloutAnchor(QgsGeometry.fromWkt("Point(1 3)"))
        item.setCallout(QgsBalloonCallout())
        item.setOffsetFromCallout(QSizeF(13.6, 17.2))
        item.setOffsetFromCalloutUnit(Qgis.RenderUnit.Inches)
        item.setRotation(45)
        item.setRotationMode(Qgis.SymbolRotationMode.RespectMapRotation)

        self.assertTrue(item.writeXml(elem, doc, QgsReadWriteContext()))

        s2 = QgsAnnotationPictureItem.create()
        self.assertTrue(s2.readXml(elem, QgsReadWriteContext()))

        self.assertEqual(s2.bounds().toString(3), "10.000,20.000 : 30.000,40.000")
        self.assertEqual(
            s2.path(), self.get_test_data_path("rgb256x256.png").as_posix()
        )
        self.assertEqual(s2.format(), Qgis.PictureFormat.Raster)
        self.assertEqual(s2.backgroundSymbol()[0].color(), QColor(200, 100, 100))
        self.assertEqual(s2.frameSymbol()[0].color(), QColor(100, 200, 150))
        self.assertEqual(s2.zIndex(), 11)
        self.assertTrue(s2.frameEnabled())
        self.assertTrue(s2.backgroundEnabled())
        self.assertFalse(s2.lockAspectRatio())
        self.assertEqual(s2.placementMode(), Qgis.AnnotationPlacementMode.FixedSize)
        self.assertEqual(s2.fixedSize(), QSizeF(56, 57))
        self.assertEqual(s2.fixedSizeUnit(), Qgis.RenderUnit.Inches)
        self.assertEqual(s2.calloutAnchor().asWkt(), "Point (1 3)")
        self.assertIsInstance(s2.callout(), QgsBalloonCallout)
        self.assertEqual(s2.offsetFromCallout(), QSizeF(13.6, 17.2))
        self.assertEqual(s2.offsetFromCalloutUnit(), Qgis.RenderUnit.Inches)
        self.assertEqual(s2.rotation(), 45)
        self.assertEqual(s2.rotationMode(), Qgis.SymbolRotationMode.RespectMapRotation)

    def testClone(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(10, 20, 30, 40),
        )
        item.setBackgroundEnabled(True)
        item.setBackgroundSymbol(
            QgsFillSymbol.createSimple(
                {"color": "200,100,100", "outline_color": "black"}
            )
        )
        item.setFrameEnabled(True)
        item.setFrameSymbol(
            QgsFillSymbol.createSimple(
                {"color": "100,200,150", "outline_color": "black"}
            )
        )
        item.setZIndex(11)
        item.setLockAspectRatio(False)
        item.setPlacementMode(Qgis.AnnotationPlacementMode.FixedSize)
        item.setFixedSize(QSizeF(56, 57))
        item.setFixedSizeUnit(Qgis.RenderUnit.Inches)
        item.setCalloutAnchor(QgsGeometry.fromWkt("Point(1 3)"))
        item.setCallout(QgsBalloonCallout())
        item.setOffsetFromCallout(QSizeF(13.6, 17.2))
        item.setOffsetFromCalloutUnit(Qgis.RenderUnit.Inches)
        item.setRotation(45)
        item.setRotationMode(Qgis.SymbolRotationMode.RespectMapRotation)

        s2 = item.clone()
        self.assertEqual(s2.bounds().toString(3), "10.000,20.000 : 30.000,40.000")
        self.assertEqual(
            s2.path(), self.get_test_data_path("rgb256x256.png").as_posix()
        )
        self.assertEqual(s2.format(), Qgis.PictureFormat.Raster)
        self.assertEqual(s2.backgroundSymbol()[0].color(), QColor(200, 100, 100))
        self.assertEqual(s2.frameSymbol()[0].color(), QColor(100, 200, 150))
        self.assertEqual(s2.zIndex(), 11)
        self.assertTrue(s2.frameEnabled())
        self.assertTrue(s2.backgroundEnabled())
        self.assertFalse(s2.lockAspectRatio())
        self.assertEqual(s2.placementMode(), Qgis.AnnotationPlacementMode.FixedSize)
        self.assertEqual(s2.fixedSize(), QSizeF(56, 57))
        self.assertEqual(s2.fixedSizeUnit(), Qgis.RenderUnit.Inches)
        self.assertEqual(s2.calloutAnchor().asWkt(), "Point (1 3)")
        self.assertIsInstance(s2.callout(), QgsBalloonCallout)
        self.assertEqual(s2.offsetFromCallout(), QSizeF(13.6, 17.2))
        self.assertEqual(s2.offsetFromCalloutUnit(), Qgis.RenderUnit.Inches)
        self.assertEqual(s2.rotation(), 45)
        self.assertEqual(s2.rotationMode(), Qgis.SymbolRotationMode.RespectMapRotation)

    def testRenderRasterLockedAspect(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(12, 13, 16, 15),
        )
        item.setLockAspectRatio(True)

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
            self.image_check(
                "picture_raster_locked_aspect", "picture_raster_locked_aspect", image
            )
        )

    def testRenderRasterUnlockedAspect(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(12, 13, 16, 15),
        )
        item.setLockAspectRatio(False)

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
            self.image_check(
                "picture_raster_unlocked_aspect",
                "picture_raster_unlocked_aspect",
                image,
            )
        )

    def testRenderSvgLockedAspect(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.SVG,
            self.get_test_data_path("sample_svg.svg").as_posix(),
            QgsRectangle(12, 13, 16, 15),
        )
        item.setLockAspectRatio(True)

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
            self.image_check(
                "picture_svg_locked_aspect", "picture_svg_locked_aspect", image
            )
        )

    def testRenderSvgUnlockedAspect(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.SVG,
            self.get_test_data_path("sample_svg.svg").as_posix(),
            QgsRectangle(12, 13, 16, 15),
        )
        item.setLockAspectRatio(False)

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
            self.image_check(
                "picture_svg_unlocked_aspect", "picture_svg_unlocked_aspect", image
            )
        )

    def testRenderWithTransform(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(11.5, 13, 12, 13.5),
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
            self.image_check("picture_transform", "picture_transform", image)
        )

    def testRenderCallout(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(12, 13, 16, 15),
        )
        item.setLockAspectRatio(True)

        callout = QgsSimpleLineCallout()
        callout.lineSymbol().setWidth(1)
        item.setCallout(callout)
        item.setCalloutAnchor(QgsGeometry.fromWkt("Point(11 12)"))

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        settings.setExtent(QgsRectangle(10, 8, 18, 16))
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

        self.assertTrue(self.image_check("picture_callout", "picture_callout", image))

    def test_render_rotated_callout(self):
        """
        Test that a callout connects to the rotated item rather than to the
        unrotated bounds (the callout rotates its rectangle around the top-left
        corner and with the opposite sign to QPainter, so the item must
        compensate for its center rotation).
        """
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            # non-square bounds so the rotation pivot matters
            QgsRectangle(80, 95, 120, 105),
        )
        item.setRotation(90)

        callout = QgsSimpleLineCallout()
        callout.lineSymbol().setColor(QColor(0, 255, 0))
        callout.lineSymbol().setWidth(1)
        item.setCallout(callout)
        # anchor straight above the item center
        item.setCalloutAnchor(QgsGeometry.fromWkt("Point(100 160)"))

        settings = QgsMapSettings()
        # 1 map unit == 1 pixel, y flipped: pixel = (x, 200 - y)
        settings.setExtent(QgsRectangle(0, 0, 200, 200))
        settings.setOutputSize(QSize(200, 200))
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

        # The 40x10 item rotated 90 degrees around its center (100, 100) covers
        # the screen region x in [95, 105], y in [80, 120]. The callout therefore
        # leaves the rotated item at its top edge middle (100, 80) and runs up to
        # the anchor at (100, 40): a vertical green line at x == 100. Without the
        # rotation compensation the callout would attach near (90, 55) instead.
        on_line = image.pixelColor(100, 60)
        self.assertGreater(on_line.green(), 200)
        self.assertLess(on_line.red(), 100)
        self.assertLess(on_line.blue(), 100)

        # the buggy attachment location must remain background (white)
        off_line = image.pixelColor(115, 55)
        self.assertEqual(off_line, QColor(255, 255, 255))

    def test_render_rotated_balloon_callout(self):
        """
        Test that a balloon callout is rotated together with the item body. The
        balloon shape itself ignores the angle argument, so the whole callout must
        be rendered through a rotated painter; otherwise an axis-aligned balloon is
        drawn around the unrotated bounds and pokes out beside the rotated item.
        """
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            # wide and short, so after a 90 degree rotation it becomes tall and narrow
            QgsRectangle(70, 96, 130, 104),
        )
        item.setRotation(90)

        callout = QgsBalloonCallout()
        callout.fillSymbol().setColor(QColor(0, 0, 255))
        # no margins by default: the balloon body is hidden behind the opaque image,
        # only the wedge pointing to the anchor is visible
        item.setCallout(callout)
        item.setCalloutAnchor(QgsGeometry.fromWkt("Point(100 160)"))

        settings = QgsMapSettings()
        # 1 map unit == 1 pixel, y flipped: pixel = (x, 200 - y)
        settings.setExtent(QgsRectangle(0, 0, 200, 200))
        settings.setOutputSize(QSize(200, 200))
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

        # The rotated item covers the screen region x in [96, 104], y in [70, 130].
        # The balloon wedge therefore leaves the item top edge (around 100, 70) and
        # runs up towards the anchor at (100, 40): probe a point on that wedge.
        on_wedge = image.pixelColor(100, 55)
        self.assertGreater(on_wedge.blue(), 200)
        self.assertLess(on_wedge.red(), 100)
        self.assertLess(on_wedge.green(), 100)

        # If the balloon were not rotated, an axis-aligned balloon around the
        # unrotated bounds (x in [70, 130], y in [96, 104]) would fill this point,
        # which lies beside the rotated item and must stay background.
        beside_item = image.pixelColor(80, 100)
        self.assertEqual(beside_item, QColor(255, 255, 255))

    def testRenderFixedSizeRaster(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(12, 13, 16, 15),
        )
        item.setLockAspectRatio(True)
        item.setPlacementMode(Qgis.AnnotationPlacementMode.FixedSize)
        item.setFixedSize(QSizeF(10, 20))
        item.setFixedSizeUnit(Qgis.RenderUnit.Millimeters)

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
            self.image_check(
                "picture_fixed_size_raster", "picture_fixed_size_raster", image
            )
        )

    def testRenderFixedSizeCallout(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(12, 13, 16, 15),
        )
        item.setLockAspectRatio(True)
        item.setPlacementMode(Qgis.AnnotationPlacementMode.FixedSize)
        item.setFixedSize(QSizeF(10, 20))
        item.setFixedSizeUnit(Qgis.RenderUnit.Millimeters)

        callout = QgsSimpleLineCallout()
        callout.lineSymbol().setWidth(1)
        item.setCallout(callout)
        item.setCalloutAnchor(QgsGeometry.fromWkt("Point(11 12)"))
        item.setOffsetFromCallout(QSizeF(60, -80))
        item.setOffsetFromCalloutUnit(Qgis.RenderUnit.Points)

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        settings.setExtent(QgsRectangle(10, 8, 18, 16))
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
            self.image_check(
                "picture_fixed_size_callout", "picture_fixed_size_callout", image
            )
        )

    def testRenderSvgFixedSize(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.SVG,
            self.get_test_data_path("sample_svg.svg").as_posix(),
            QgsRectangle(12, 13, 16, 15),
        )
        item.setLockAspectRatio(True)
        item.setPlacementMode(Qgis.AnnotationPlacementMode.FixedSize)
        item.setFixedSize(QSizeF(30, 50))
        item.setFixedSizeUnit(Qgis.RenderUnit.Millimeters)

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
            self.image_check("picture_svg_fixed_size", "picture_svg_fixed_size", image)
        )

    def testRenderWithTransformFixedSize(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(11.5, 13, 12, 13.5),
        )
        item.setPlacementMode(Qgis.AnnotationPlacementMode.FixedSize)
        item.setFixedSize(QSizeF(10, 20))
        item.setFixedSizeUnit(Qgis.RenderUnit.Millimeters)

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
            self.image_check(
                "picture_transform_fixed_size", "picture_transform_fixed_size", image
            )
        )

    def testRenderRelativeMapRaster(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle.fromCenterAndSize(QgsPointXY(0.3, 0.7), 1, 1),
        )
        item.setLockAspectRatio(True)
        item.setPlacementMode(Qgis.AnnotationPlacementMode.RelativeToMapFrame)
        item.setFixedSize(QSizeF(10, 20))
        item.setFixedSizeUnit(Qgis.RenderUnit.Millimeters)

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        settings.setExtent(QgsRectangle(10, 10, 18, 18))
        settings.setOutputSize(QSize(300, 300))

        settings.setFlag(QgsMapSettings.Flag.Antialiasing, False)

        rc = QgsRenderContext.fromMapSettings(settings)
        image = QImage(300, 300, QImage.Format.Format_ARGB32)
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
                "picture_relative_to_map", "picture_relative_to_map", image
            )
        )

    def testRenderBackgroundFrame(self):
        item = QgsAnnotationPictureItem(
            Qgis.PictureFormat.Raster,
            self.get_test_data_path("rgb256x256.png").as_posix(),
            QgsRectangle(12, 13, 16, 15),
        )
        item.setLockAspectRatio(True)
        item.setFrameEnabled(True)
        item.setBackgroundEnabled(True)
        item.setBackgroundSymbol(
            QgsFillSymbol.createSimple(
                {"color": "200,100,100", "outline_color": "black"}
            )
        )
        item.setFrameSymbol(
            QgsFillSymbol.createSimple(
                {
                    "color": "100,200,250,120",
                    "outline_color": "black",
                    "outline_width": 2,
                }
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
            self.image_check(
                "picture_frame_background", "picture_frame_background", image
            )
        )


if __name__ == "__main__":
    unittest.main()
