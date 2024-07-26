"""QGIS Unit tests for QgsAnnotationPictureItem.

From build dir, run: ctest -R QgsAnnotationPictureItem -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.PyQt.QtCore import QSize, QSizeF
from qgis.PyQt.QtGui import QColor, QImage, QPainter
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    Qgis,
    QgsAnnotationItemEditOperationAddNode,
    QgsAnnotationItemEditOperationDeleteNode,
    QgsAnnotationItemEditOperationMoveNode,
    QgsAnnotationItemEditOperationTranslateItem,
    QgsAnnotationItemEditContext,
    QgsAnnotationItemNode,
    QgsAnnotationPictureItem,
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


class TestQgsAnnotationPictureItem(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "annotation_layer"

    def testBasic(self):
        item = QgsAnnotationPictureItem(Qgis.PictureFormat.Raster, self.get_test_data_path('rgb256x256.png').as_posix(),
                                        QgsRectangle(10, 20, 30, 40))

        self.assertEqual(item.path(), self.get_test_data_path('rgb256x256.png').as_posix())
        self.assertEqual(item.format(), Qgis.PictureFormat.Raster)
        self.assertEqual(item.boundingBox().toString(3), '10.000,20.000 : 30.000,40.000')
        self.assertEqual(item.sizeMode(), Qgis.AnnotationPictureSizeMode.SpatialBounds)

        item.setBounds(QgsRectangle(100, 200, 300, 400))
        item.setZIndex(11)
        item.setPath(Qgis.PictureFormat.SVG, self.get_test_data_path('sample_svg.svg').as_posix())
        item.setLockAspectRatio(False)
        item.setBackgroundEnabled(True)
        item.setFrameEnabled(True)
        item.setSizeMode(Qgis.AnnotationPictureSizeMode.FixedSize)
        item.setFixedSize(QSizeF(56,
                                 57))
        item.setFixedSizeUnit(Qgis.RenderUnit.Inches)
        self.assertEqual(item.bounds().toString(3), '100.000,200.000 : 300.000,400.000')
        self.assertEqual(item.path(), self.get_test_data_path('sample_svg.svg').as_posix())
        self.assertEqual(item.format(), Qgis.PictureFormat.SVG)
        self.assertEqual(item.zIndex(), 11)
        self.assertFalse(item.lockAspectRatio())
        self.assertTrue(item.backgroundEnabled())
        self.assertTrue(item.frameEnabled())
        self.assertEqual(item.sizeMode(),
                         Qgis.AnnotationPictureSizeMode.FixedSize)
        self.assertEqual(item.fixedSize(), QSizeF(56,
                                                  57))
        self.assertEqual(item.fixedSizeUnit(), Qgis.RenderUnit.Inches)

        item.setBackgroundSymbol(QgsFillSymbol.createSimple({'color': '200,100,100', 'outline_color': 'black'}))
        item.setFrameSymbol(QgsFillSymbol.createSimple(
            {'color': '100,200,250', 'outline_color': 'black'}))
        self.assertEqual(item.backgroundSymbol()[0].color(), QColor(200, 100, 100))
        self.assertEqual(item.frameSymbol()[0].color(),
                         QColor(100, 200, 250))

    def test_nodes_spatial_bounds(self):
        """
        Test nodes for item, spatial bounds mode
        """
        item = QgsAnnotationPictureItem(Qgis.PictureFormat.Raster, self.get_test_data_path('rgb256x256.png').as_posix(),
                                        QgsRectangle(10, 20, 30, 40))
        # nodes shouldn't form a closed ring
        self.assertEqual(item.nodesV2(QgsAnnotationItemEditContext()), [
            QgsAnnotationItemNode(QgsVertexId(0, 0, 0), QgsPointXY(10, 20), Qgis.AnnotationItemNodeType.VertexHandle),
            QgsAnnotationItemNode(QgsVertexId(0, 0, 1), QgsPointXY(30, 20), Qgis.AnnotationItemNodeType.VertexHandle),
            QgsAnnotationItemNode(QgsVertexId(0, 0, 2), QgsPointXY(30, 40), Qgis.AnnotationItemNodeType.VertexHandle),
            QgsAnnotationItemNode(QgsVertexId(0, 0, 3), QgsPointXY(10, 40), Qgis.AnnotationItemNodeType.VertexHandle)])

    def test_nodes_fixed_size(self):
        """
        Test nodes for item, fixed size mode
        """
        item = QgsAnnotationPictureItem(Qgis.PictureFormat.Raster, self.get_test_data_path('rgb256x256.png').as_posix(),
                                        QgsRectangle(10, 20, 30, 40))
        item.setSizeMode(Qgis.AnnotationPictureSizeMode.FixedSize)
        self.assertEqual(item.nodesV2(QgsAnnotationItemEditContext()), [
            QgsAnnotationItemNode(QgsVertexId(0, 0, 0), QgsPointXY(20, 30), Qgis.AnnotationItemNodeType.VertexHandle)])

    def test_translate_spatial_bounds(self):
        item = QgsAnnotationPictureItem(Qgis.PictureFormat.Raster,
                                        self.get_test_data_path(
                                            'rgb256x256.png').as_posix(),
                                        QgsRectangle(10, 20, 30, 40))
        self.assertEqual(item.bounds().toString(3), '10.000,20.000 : 30.000,40.000')

        self.assertEqual(item.applyEditV2(QgsAnnotationItemEditOperationTranslateItem('', 100, 200),
                                          QgsAnnotationItemEditContext()),
                         Qgis.AnnotationItemEditOperationResult.Success)
        self.assertEqual(item.bounds().toString(3), '110.000,220.000 : 130.000,240.000')

    def test_translate_fixed_size(self):
        item = QgsAnnotationPictureItem(Qgis.PictureFormat.Raster,
                                        self.get_test_data_path(
                                            'rgb256x256.png').as_posix(),
                                        QgsRectangle(10, 20, 30, 40))
        item.setSizeMode(Qgis.AnnotationPictureSizeMode.FixedSize)
        self.assertEqual(item.bounds().toString(3), '10.000,20.000 : 30.000,40.000')

        self.assertEqual(item.applyEditV2(QgsAnnotationItemEditOperationTranslateItem('', 100, 200),
                                          QgsAnnotationItemEditContext()),
                         Qgis.AnnotationItemEditOperationResult.Success)
        self.assertEqual(item.bounds().toString(3), '110.000,220.000 : 130.000,240.000')

    def test_apply_move_node_edit_spatial_bounds(self):
        item = QgsAnnotationPictureItem(Qgis.PictureFormat.Raster,
                                        self.get_test_data_path(
                                            'rgb256x256.png').as_posix(),
                                        QgsRectangle(10, 20, 30, 40))
        self.assertEqual(item.bounds().toString(3), '10.000,20.000 : 30.000,40.000')

        self.assertEqual(item.applyEditV2(
            QgsAnnotationItemEditOperationMoveNode('', QgsVertexId(0, 0, 1), QgsPoint(30, 20), QgsPoint(17, 18)),
            QgsAnnotationItemEditContext()),
            Qgis.AnnotationItemEditOperationResult.Success)
        self.assertEqual(item.bounds().toString(3), '10.000,18.000 : 17.000,40.000')
        self.assertEqual(item.applyEditV2(
            QgsAnnotationItemEditOperationMoveNode('', QgsVertexId(0, 0, 0), QgsPoint(10, 18), QgsPoint(5, 13)),
            QgsAnnotationItemEditContext()),
            Qgis.AnnotationItemEditOperationResult.Success)
        self.assertEqual(item.bounds().toString(3), '5.000,13.000 : 17.000,40.000')
        self.assertEqual(item.applyEditV2(
            QgsAnnotationItemEditOperationMoveNode('', QgsVertexId(0, 0, 2), QgsPoint(17, 14), QgsPoint(18, 38)),
            QgsAnnotationItemEditContext()),
            Qgis.AnnotationItemEditOperationResult.Success)
        self.assertEqual(item.bounds().toString(3), '5.000,13.000 : 18.000,38.000')
        self.assertEqual(item.applyEditV2(
            QgsAnnotationItemEditOperationMoveNode('', QgsVertexId(0, 0, 3), QgsPoint(5, 38), QgsPoint(2, 39)),
            QgsAnnotationItemEditContext()),
            Qgis.AnnotationItemEditOperationResult.Success)
        self.assertEqual(item.bounds().toString(3), '2.000,13.000 : 18.000,39.000')

    def test_apply_move_node_edit_fixed_size(self):
        item = QgsAnnotationPictureItem(Qgis.PictureFormat.Raster,
                                        self.get_test_data_path(
                                            'rgb256x256.png').as_posix(),
                                        QgsRectangle(10, 20, 30, 40))
        item.setSizeMode(Qgis.AnnotationPictureSizeMode.FixedSize)
        self.assertEqual(item.bounds().toString(3), '10.000,20.000 : 30.000,40.000')

        self.assertEqual(item.applyEditV2(
            QgsAnnotationItemEditOperationMoveNode('', QgsVertexId(0, 0, 0), QgsPoint(30, 20), QgsPoint(17, 18)),
            QgsAnnotationItemEditContext()),
            Qgis.AnnotationItemEditOperationResult.Success)
        self.assertEqual(item.bounds().toString(3), '7.000,8.000 : 27.000,28.000')

    def test_apply_delete_node_edit(self):
        item = QgsAnnotationPictureItem(Qgis.PictureFormat.Raster,
                                        self.get_test_data_path(
                                            'rgb256x256.png').as_posix(),
                                        QgsRectangle(10, 20, 30, 40))

        self.assertEqual(
            item.applyEdit(QgsAnnotationItemEditOperationDeleteNode('', QgsVertexId(0, 0, 1), QgsPoint(14, 13))),
            Qgis.AnnotationItemEditOperationResult.Invalid)

    def test_apply_add_node_edit(self):
        item = QgsAnnotationPictureItem(Qgis.PictureFormat.Raster,
                                        self.get_test_data_path(
                                            'rgb256x256.png').as_posix(),
                                        QgsRectangle(10, 20, 30, 40))

        self.assertEqual(item.applyEdit(QgsAnnotationItemEditOperationAddNode('', QgsPoint(15, 16))),
                         Qgis.AnnotationItemEditOperationResult.Invalid)

    def test_transient_move_operation_spatial_bounds(self):
        item = QgsAnnotationPictureItem(Qgis.PictureFormat.Raster,
                                        self.get_test_data_path(
                                            'rgb256x256.png').as_posix(),
                                        QgsRectangle(10, 20, 30, 40))
        self.assertEqual(item.bounds().toString(3), '10.000,20.000 : 30.000,40.000')

        res = item.transientEditResultsV2(
            QgsAnnotationItemEditOperationMoveNode('', QgsVertexId(0, 0, 1), QgsPoint(30, 20), QgsPoint(17, 18)),
            QgsAnnotationItemEditContext())
        self.assertEqual(res.representativeGeometry().asWkt(), 'Polygon ((10 18, 17 18, 17 40, 10 40, 10 18))')

    def test_transient_move_operation_fixed_size(self):
        item = QgsAnnotationPictureItem(Qgis.PictureFormat.Raster,
                                        self.get_test_data_path(
                                            'rgb256x256.png').as_posix(),
                                        QgsRectangle(10, 20, 30, 40))
        item.setSizeMode(Qgis.AnnotationPictureSizeMode.FixedSize)
        item.setFixedSize(QSizeF(56,
                                 57))
        item.setFixedSizeUnit(Qgis.RenderUnit.Inches)
        self.assertEqual(item.bounds().toString(3), '10.000,20.000 : 30.000,40.000')

        op = QgsAnnotationItemEditOperationMoveNode('', QgsVertexId(0, 0, 1), QgsPoint(30, 20), QgsPoint(17, 18))
        context = QgsAnnotationItemEditContext()
        context.setCurrentItemBounds(QgsRectangle(1, 2, 3, 4))
        res = item.transientEditResultsV2(op, context)
        self.assertEqual(res.representativeGeometry().asWkt(), 'Polygon ((16 17, 18 17, 18 19, 16 19, 16 17))')

    def test_transient_translate_operation_spatial_bounds(self):
        item = QgsAnnotationPictureItem(Qgis.PictureFormat.Raster,
                                        self.get_test_data_path(
                                            'rgb256x256.png').as_posix(),
                                        QgsRectangle(10, 20, 30, 40))
        self.assertEqual(item.bounds().toString(3), '10.000,20.000 : 30.000,40.000')

        res = item.transientEditResultsV2(QgsAnnotationItemEditOperationTranslateItem('', 100, 200),
                                          QgsAnnotationItemEditContext())
        self.assertEqual(res.representativeGeometry().asWkt(),
                         'Polygon ((110 220, 130 220, 130 240, 110 240, 110 220))')

    def test_transient_translate_operation_fixed_size(self):
        item = QgsAnnotationPictureItem(Qgis.PictureFormat.Raster,
                                        self.get_test_data_path(
                                            'rgb256x256.png').as_posix(),
                                        QgsRectangle(10, 20, 30, 40))
        item.setSizeMode(Qgis.AnnotationPictureSizeMode.FixedSize)
        item.setFixedSize(QSizeF(56,
                                 57))
        item.setFixedSizeUnit(Qgis.RenderUnit.Inches)
        self.assertEqual(item.bounds().toString(3), '10.000,20.000 : 30.000,40.000')

        op = QgsAnnotationItemEditOperationTranslateItem('', 100, 200)
        context = QgsAnnotationItemEditContext()
        context.setCurrentItemBounds(QgsRectangle(1, 2, 3, 4))
        res = item.transientEditResultsV2(op, context)
        self.assertEqual(res.representativeGeometry().asWkt(),
                         'Polygon ((119 229, 121 229, 121 231, 119 231, 119 229))')

    def testReadWriteXml(self):
        doc = QDomDocument("testdoc")
        elem = doc.createElement('test')

        item = QgsAnnotationPictureItem(Qgis.PictureFormat.Raster, self.get_test_data_path('rgb256x256.png').as_posix(),
                                        QgsRectangle(10, 20, 30, 40))
        item.setBackgroundEnabled(True)
        item.setBackgroundSymbol(QgsFillSymbol.createSimple({'color': '200,100,100', 'outline_color': 'black'}))
        item.setFrameEnabled(True)
        item.setFrameSymbol(QgsFillSymbol.createSimple({'color': '100,200,150', 'outline_color': 'black'}))
        item.setZIndex(11)
        item.setLockAspectRatio(False)
        item.setSizeMode(Qgis.AnnotationPictureSizeMode.FixedSize)
        item.setFixedSize(QSizeF(56,
                                 57))
        item.setFixedSizeUnit(Qgis.RenderUnit.Inches)

        self.assertTrue(item.writeXml(elem, doc, QgsReadWriteContext()))

        s2 = QgsAnnotationPictureItem.create()
        self.assertTrue(s2.readXml(elem, QgsReadWriteContext()))

        self.assertEqual(s2.bounds().toString(3), '10.000,20.000 : 30.000,40.000')
        self.assertEqual(s2.path(), self.get_test_data_path('rgb256x256.png').as_posix())
        self.assertEqual(s2.format(), Qgis.PictureFormat.Raster)
        self.assertEqual(s2.backgroundSymbol()[0].color(), QColor(200, 100, 100))
        self.assertEqual(s2.frameSymbol()[0].color(),
                         QColor(100, 200, 150))
        self.assertEqual(s2.zIndex(), 11)
        self.assertTrue(s2.frameEnabled())
        self.assertTrue(s2.backgroundEnabled())
        self.assertFalse(s2.lockAspectRatio())
        self.assertEqual(s2.sizeMode(),
                         Qgis.AnnotationPictureSizeMode.FixedSize)
        self.assertEqual(s2.fixedSize(), QSizeF(56,
                                                57))
        self.assertEqual(s2.fixedSizeUnit(), Qgis.RenderUnit.Inches)

    def testClone(self):
        item = QgsAnnotationPictureItem(Qgis.PictureFormat.Raster, self.get_test_data_path('rgb256x256.png').as_posix(),
                                        QgsRectangle(10, 20, 30, 40))
        item.setBackgroundEnabled(True)
        item.setBackgroundSymbol(QgsFillSymbol.createSimple({'color': '200,100,100', 'outline_color': 'black'}))
        item.setFrameEnabled(True)
        item.setFrameSymbol(QgsFillSymbol.createSimple({'color': '100,200,150', 'outline_color': 'black'}))
        item.setZIndex(11)
        item.setLockAspectRatio(False)
        item.setSizeMode(Qgis.AnnotationPictureSizeMode.FixedSize)
        item.setFixedSize(QSizeF(56,
                                 57))
        item.setFixedSizeUnit(Qgis.RenderUnit.Inches)

        s2 = item.clone()
        self.assertEqual(s2.bounds().toString(3), '10.000,20.000 : 30.000,40.000')
        self.assertEqual(s2.path(), self.get_test_data_path('rgb256x256.png').as_posix())
        self.assertEqual(s2.format(), Qgis.PictureFormat.Raster)
        self.assertEqual(s2.backgroundSymbol()[0].color(), QColor(200, 100, 100))
        self.assertEqual(s2.frameSymbol()[0].color(),
                         QColor(100, 200, 150))
        self.assertEqual(s2.zIndex(), 11)
        self.assertTrue(s2.frameEnabled())
        self.assertTrue(s2.backgroundEnabled())
        self.assertFalse(s2.lockAspectRatio())
        self.assertEqual(s2.sizeMode(),
                         Qgis.AnnotationPictureSizeMode.FixedSize)
        self.assertEqual(s2.fixedSize(), QSizeF(56,
                                                57))
        self.assertEqual(s2.fixedSizeUnit(), Qgis.RenderUnit.Inches)

    def testRenderRasterLockedAspect(self):
        item = QgsAnnotationPictureItem(Qgis.PictureFormat.Raster, self.get_test_data_path('rgb256x256.png').as_posix(),
                                        QgsRectangle(12, 13, 16, 15))
        item.setLockAspectRatio(True)

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
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

        self.assertTrue(self.image_check('picture_raster_locked_aspect', 'picture_raster_locked_aspect', image))

    def testRenderRasterUnlockedAspect(self):
        item = QgsAnnotationPictureItem(Qgis.PictureFormat.Raster, self.get_test_data_path('rgb256x256.png').as_posix(),
                                        QgsRectangle(12, 13, 16, 15))
        item.setLockAspectRatio(False)

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
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

        self.assertTrue(self.image_check('picture_raster_unlocked_aspect', 'picture_raster_unlocked_aspect', image))

    def testRenderSvgLockedAspect(self):
        item = QgsAnnotationPictureItem(Qgis.PictureFormat.SVG, self.get_test_data_path('sample_svg.svg').as_posix(),
                                        QgsRectangle(12, 13, 16, 15))
        item.setLockAspectRatio(True)

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
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

        self.assertTrue(self.image_check('picture_svg_locked_aspect', 'picture_svg_locked_aspect', image))

    def testRenderSvgUnlockedAspect(self):
        item = QgsAnnotationPictureItem(Qgis.PictureFormat.SVG, self.get_test_data_path('sample_svg.svg').as_posix(),
                                        QgsRectangle(12, 13, 16, 15))
        item.setLockAspectRatio(False)

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
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

        self.assertTrue(self.image_check('picture_svg_unlocked_aspect', 'picture_svg_unlocked_aspect', image))

    def testRenderWithTransform(self):
        item = QgsAnnotationPictureItem(Qgis.PictureFormat.Raster, self.get_test_data_path('rgb256x256.png').as_posix(),
                                        QgsRectangle(11.5, 13, 12, 13.5))

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        settings.setExtent(QgsRectangle(1250958, 1386945, 1420709, 1532518))
        settings.setOutputSize(QSize(300, 300))

        settings.setFlag(QgsMapSettings.Flag.Antialiasing, False)

        rc = QgsRenderContext.fromMapSettings(settings)
        rc.setCoordinateTransform(
            QgsCoordinateTransform(QgsCoordinateReferenceSystem('EPSG:4326'), settings.destinationCrs(),
                                   QgsProject.instance()))
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

        self.assertTrue(self.image_check('picture_transform', 'picture_transform', image))

    def testRenderFixedSizeRaster(self):
        item = QgsAnnotationPictureItem(Qgis.PictureFormat.Raster, self.get_test_data_path('rgb256x256.png').as_posix(),
                                        QgsRectangle(12, 13, 16, 15))
        item.setLockAspectRatio(True)
        item.setSizeMode(Qgis.AnnotationPictureSizeMode.FixedSize)
        item.setFixedSize(QSizeF(10,
                                 20))
        item.setFixedSizeUnit(Qgis.RenderUnit.Millimeters)

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
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

        self.assertTrue(self.image_check('picture_fixed_size_raster', 'picture_fixed_size_raster', image))

    def testRenderSvgFixedSize(self):
        item = QgsAnnotationPictureItem(Qgis.PictureFormat.SVG, self.get_test_data_path('sample_svg.svg').as_posix(),
                                        QgsRectangle(12, 13, 16, 15))
        item.setLockAspectRatio(True)
        item.setSizeMode(Qgis.AnnotationPictureSizeMode.FixedSize)
        item.setFixedSize(QSizeF(30,
                                 50))
        item.setFixedSizeUnit(Qgis.RenderUnit.Millimeters)

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
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

        self.assertTrue(self.image_check('picture_svg_fixed_size', 'picture_svg_fixed_size', image))

    def testRenderWithTransformFixedSize(self):
        item = QgsAnnotationPictureItem(Qgis.PictureFormat.Raster, self.get_test_data_path('rgb256x256.png').as_posix(),
                                        QgsRectangle(11.5, 13, 12, 13.5))
        item.setSizeMode(Qgis.AnnotationPictureSizeMode.FixedSize)
        item.setFixedSize(QSizeF(10,
                                 20))
        item.setFixedSizeUnit(Qgis.RenderUnit.Millimeters)

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        settings.setExtent(QgsRectangle(1250958, 1386945, 1420709, 1532518))
        settings.setOutputSize(QSize(300, 300))

        settings.setFlag(QgsMapSettings.Flag.Antialiasing, False)

        rc = QgsRenderContext.fromMapSettings(settings)
        rc.setCoordinateTransform(
            QgsCoordinateTransform(QgsCoordinateReferenceSystem('EPSG:4326'), settings.destinationCrs(),
                                   QgsProject.instance()))
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

        self.assertTrue(self.image_check('picture_transform_fixed_size', 'picture_transform_fixed_size', image))

    def testRenderBackgroundFrame(self):
        item = QgsAnnotationPictureItem(Qgis.PictureFormat.Raster, self.get_test_data_path('rgb256x256.png').as_posix(),
                                        QgsRectangle(12, 13, 16, 15))
        item.setLockAspectRatio(True)
        item.setFrameEnabled(True)
        item.setBackgroundEnabled(True)
        item.setBackgroundSymbol(QgsFillSymbol.createSimple({'color': '200,100,100', 'outline_color': 'black'}))
        item.setFrameSymbol(QgsFillSymbol.createSimple(
            {'color': '100,200,250,120', 'outline_color': 'black', 'outline_width': 2}))

        settings = QgsMapSettings()
        settings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
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

        self.assertTrue(self.image_check('picture_frame_background', 'picture_frame_background', image))


if __name__ == '__main__':
    unittest.main()
