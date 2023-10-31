"""
***************************************************************************
    test_qgsrasterlinesymbollayer.py
    ---------------------
    Date                 : March 2019
    Copyright            : (C) 2019 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Nyall Dawson'
__date__ = 'March 2019'
__copyright__ = '(C) 2019, Nyall Dawson'


import qgis  # NOQA
from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtGui import QColor, QImage, QPainter
from qgis.core import (
    Qgis,
    QgsFeature,
    QgsGeometry,
    QgsLineSymbol,
    QgsMapSettings,
    QgsRasterLineSymbolLayer,
    QgsRenderContext,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsRasterLineSymbolLayer(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "symbol_rasterline"

    def testRender(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + '/raster_brush.png')
        raster_line.setWidth(8)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.image_check('rasterline_render', 'rasterline_render', rendered_image))

    def testRenderHairline(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + '/raster_brush.png')
        raster_line.setWidth(0)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.image_check('rasterline_hairline', 'rasterline_hairline', rendered_image))

    def testRenderClosedRing(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + '/raster_brush.png')
        raster_line.setWidth(8)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 10 0, 10 10, 0 10, 0 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.image_check('rasterline_closed', 'rasterline_closed', rendered_image))

    def testRenderOpacity(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + '/raster_brush.png')
        raster_line.setWidth(8)
        raster_line.setOpacity(0.5)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.image_check('rasterline_opacity', 'rasterline_opacity', rendered_image))

    def testRenderFlatCap(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + '/raster_brush.png')
        raster_line.setWidth(15)
        raster_line.setPenCapStyle(Qt.FlatCap)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.image_check('rasterline_flatcap', 'rasterline_flatcap', rendered_image))

    def testRenderSquareCap(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + '/raster_brush.png')
        raster_line.setWidth(15)
        raster_line.setPenCapStyle(Qt.SquareCap)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt('LineString(2 2, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.image_check('rasterline_squarecap', 'rasterline_squarecap', rendered_image))

    def testRenderMiterJoin(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + '/raster_brush.png')
        raster_line.setWidth(15)
        raster_line.setPenJoinStyle(Qt.MiterJoin)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt('LineString(0 15, 0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.image_check('rasterline_miterjoin', 'rasterline_miterjoin', rendered_image))

    def testRenderBevelJoin(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + '/raster_brush.png')
        raster_line.setWidth(15)
        raster_line.setPenJoinStyle(Qt.BevelJoin)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt('LineString(2 2, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.image_check('rasterline_beveljoin', 'rasterline_beveljoin', rendered_image))

    def testLineOffset(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + '/raster_brush.png')
        raster_line.setWidth(5)
        raster_line.setOffset(5)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt('LineString(2 2, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.image_check('rasterline_offset', 'rasterline_offset', rendered_image))

    def renderGeometry(self, symbol, geom, buffer=20):
        f = QgsFeature()
        f.setGeometry(geom)

        image = QImage(200, 200, QImage.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        extent = geom.get().boundingBox()
        # buffer extent by 10%
        if extent.width() > 0:
            extent = extent.buffered((extent.height() + extent.width()) / buffer)
        else:
            extent = extent.buffered(buffer / 2)

        ms.setExtent(extent)
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.expressionContext().setFeature(f)

        painter.begin(image)
        try:
            image.fill(QColor(0, 0, 0))
            symbol.startRender(context)
            symbol.renderFeature(f, context)
            symbol.stopRender(context)
        finally:
            painter.end()

        return image

    def testRenderBrushPath(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + '/raster_brush.png')
        raster_line.setWidth(8)
        raster_line.setMode(Qgis.RasterLineSymbolLayerMode.BrushPath)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.image_check('rasterline_brush_path', 'rasterline_brush_path', rendered_image))

    def testRenderBrushPathViewport(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + '/raster_brush.png')
        raster_line.setWidth(8)
        raster_line.setMode(Qgis.RasterLineSymbolLayerMode.BrushPath)
        raster_line.setCoordinateMode(Qgis.SymbolCoordinateReference.Viewport)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.image_check('rasterline_brush_path_viewport', 'rasterline_brush_path_viewport', rendered_image))

    def testRenderBrushPathWidth(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + '/raster_brush.png')
        raster_line.setWidth(8)
        raster_line.setMode(Qgis.RasterLineSymbolLayerMode.BrushPath)
        raster_line.setImageWidth(20)
        raster_line.setImageWidthUnit(Qgis.RenderUnit.Millimeters)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.image_check('rasterline_brush_path_width', 'rasterline_brush_path_width', rendered_image))

    def testRenderBrushPathHeight(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + '/raster_brush.png')
        raster_line.setWidth(8)
        raster_line.setMode(Qgis.RasterLineSymbolLayerMode.BrushPath)
        raster_line.setImageHeight(10)
        raster_line.setImageHeightUnit(Qgis.RenderUnit.Millimeters)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.image_check('rasterline_brush_path_height', 'rasterline_brush_path_height', rendered_image))

    def testRenderBrushPathWidthHeight(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + '/raster_brush.png')
        raster_line.setWidth(8)
        raster_line.setMode(Qgis.RasterLineSymbolLayerMode.BrushPath)
        raster_line.setImageWidth(30)
        raster_line.setImageWidthUnit(Qgis.RenderUnit.Millimeters)
        raster_line.setImageHeight(10)
        raster_line.setImageHeightUnit(Qgis.RenderUnit.Millimeters)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt('LineString(0 0, 10 10, 10 0)')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.image_check('rasterline_brush_path_width_height', 'rasterline_brush_path_width_height', rendered_image))


if __name__ == '__main__':
    unittest.main()
