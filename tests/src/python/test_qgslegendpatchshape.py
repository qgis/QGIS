"""QGIS Unit tests for QgsLegendPatchShape.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2020 by Nyall Dawson'
__date__ = '05/04/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

from qgis.PyQt.QtCore import QSize, QSizeF
from qgis.PyQt.QtGui import QColor, QImage, QPainter
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    QgsFillSymbol,
    QgsGeometry,
    QgsLegendPatchShape,
    QgsLineSymbol,
    QgsMarkerSymbol,
    QgsReadWriteContext,
    QgsRenderContext,
    QgsStyle,
    QgsSymbol,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLegendPatchShape(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "legend_patch"

    def setUp(self):
        # Create some simple symbols
        self.fill_symbol = QgsFillSymbol.createSimple({'color': '#ffffff', 'outline_color': 'black'})
        self.line_symbol = QgsLineSymbol.createSimple({'color': '#ffffff', 'line_width': '3'})
        self.marker_symbol = QgsMarkerSymbol.createSimple({'color': '#ffffff', 'size': '3', 'outline_color': 'black'})

    def testBasic(self):
        shape = QgsLegendPatchShape(QgsSymbol.SymbolType.Line, QgsGeometry.fromWkt('LineString( 0 0, 1 1)'), False)
        self.assertFalse(shape.isNull())
        self.assertEqual(shape.symbolType(), QgsSymbol.SymbolType.Line)
        self.assertEqual(shape.geometry().asWkt(), 'LineString (0 0, 1 1)')
        self.assertFalse(shape.preserveAspectRatio())

        shape.setSymbolType(QgsSymbol.SymbolType.Marker)
        self.assertEqual(shape.symbolType(), QgsSymbol.SymbolType.Marker)

        shape.setGeometry(QgsGeometry.fromWkt('Multipoint( 1 1, 2 2)'))
        self.assertEqual(shape.geometry().asWkt(), 'MultiPoint ((1 1),(2 2))')

        shape.setPreserveAspectRatio(True)
        self.assertTrue(shape.preserveAspectRatio())

        self.assertTrue(shape.scaleToOutputSize())
        shape.setScaleToOutputSize(False)
        self.assertFalse(shape.scaleToOutputSize())

    @staticmethod
    def polys_to_list(polys):
        return [[[[round(p.x(), 3), round(p.y(), 3)] for p in ring] for ring in poly] for poly in polys]

    def testNull(self):
        shape = QgsLegendPatchShape()
        self.assertTrue(shape.isNull())
        shape.setGeometry(QgsGeometry.fromWkt('Multipoint( 1 1, 2 2)'))
        self.assertFalse(shape.isNull())
        shape.setGeometry(QgsGeometry())
        self.assertTrue(shape.isNull())

    def testDefault(self):
        self.assertEqual(QgsStyle.defaultStyle().defaultPatchAsQPolygonF(QgsSymbol.SymbolType.Hybrid, QSizeF(1, 1)), [])
        self.assertEqual(QgsStyle.defaultStyle().defaultPatchAsQPolygonF(QgsSymbol.SymbolType.Hybrid, QSizeF(10, 10)), [])

        # markers
        self.assertEqual(self.polys_to_list(QgsStyle.defaultStyle().defaultPatchAsQPolygonF(QgsSymbol.SymbolType.Marker, QSizeF(1, 1))), [[[[0.5, 0.5]]]])
        self.assertEqual(self.polys_to_list(QgsStyle.defaultStyle().defaultPatchAsQPolygonF(QgsSymbol.SymbolType.Marker, QSizeF(2, 2))),
                         [[[[1.0, 1.0]]]])
        self.assertEqual(self.polys_to_list(QgsStyle.defaultStyle().defaultPatchAsQPolygonF(QgsSymbol.SymbolType.Marker, QSizeF(10, 2))), [[[[5.0, 1.0]]]])

        # lines
        self.assertEqual(self.polys_to_list(QgsStyle.defaultStyle().defaultPatchAsQPolygonF(QgsSymbol.SymbolType.Line, QSizeF(1, 1))), [[[[0.0, 0.5], [1.0, 0.5]]]])
        self.assertEqual(self.polys_to_list(QgsStyle.defaultStyle().defaultPatchAsQPolygonF(QgsSymbol.SymbolType.Line, QSizeF(10, 2))), [[[[0.0, 1.0], [10.0, 1.0]]]])
        self.assertEqual(self.polys_to_list(QgsStyle.defaultStyle().defaultPatchAsQPolygonF(QgsSymbol.SymbolType.Line, QSizeF(9, 3))), [[[[0.0, 1.5], [9.0, 1.5]]]])

        # fills
        self.assertEqual(self.polys_to_list(QgsStyle.defaultStyle().defaultPatchAsQPolygonF(QgsSymbol.SymbolType.Fill, QSizeF(1, 1))), [[[[0.0, 0.0], [1.0, 0.0], [1.0, 1.0], [0.0, 1.0], [0.0, 0.0]]]])
        self.assertEqual(self.polys_to_list(QgsStyle.defaultStyle().defaultPatchAsQPolygonF(QgsSymbol.SymbolType.Fill, QSizeF(10, 2))), [[[[0.0, 0.0], [10.0, 0.0], [10.0, 2.0], [0.0, 2.0], [0.0, 0.0]]]])

    def testMarkers(self):
        # shouldn't matter what a point geometry is, it will always be rendered in center of symbol patch
        shape = QgsLegendPatchShape(QgsSymbol.SymbolType.Marker, QgsGeometry.fromWkt('Point( 5 5 )'), False)
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Marker, QSizeF(1, 1))), [[[[0.5, 0.5]]]])
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Marker, QSizeF(10, 2))), [[[[5.0, 1.0]]]])

        # requesting different symbol type, should return default
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Fill, QSizeF(1, 1))), [[[[0.0, 0.0], [1.0, 0.0], [1.0, 1.0], [0.0, 1.0], [0.0, 0.0]]]])

        # ... but a multipoint WILL change the result!
        shape = QgsLegendPatchShape(QgsSymbol.SymbolType.Marker, QgsGeometry.fromWkt('MultiPoint((5 5), (1 2))'), False)
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Marker, QSizeF(1, 1))), [[[[1.0, 0.0], [0.0, 1.0]]]])
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Marker, QSizeF(10, 2))), [[[[10.0, 0.0], [0.0, 2.0]]]])

        shape = QgsLegendPatchShape(QgsSymbol.SymbolType.Marker, QgsGeometry.fromWkt('MultiPoint((5 5), (1 2), (4 3))'), False)
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Marker, QSizeF(1, 1))), [[[[1.0, 0.0], [0.0, 1.0], [0.75, 0.667]]]])
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Marker, QSizeF(10, 2))), [[[[10.0, 0.0], [0.0, 2.0], [7.5, 1.333]]]])

    def testPreserveAspect(self):
        # wider
        shape = QgsLegendPatchShape(QgsSymbol.SymbolType.Marker, QgsGeometry.fromWkt('MultiPoint((5 5), (1 2))'))
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Marker, QSizeF(1, 1))), [[[[1.0, 0.125], [0.0, 0.875]]]])
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Marker, QSizeF(10, 2))), [[[[6.333, 0.0], [3.667, 2.0]]]])
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Marker, QSizeF(2, 10))), [[[[2.0, 4.25], [0.0, 5.75]]]])

        # higher
        shape = QgsLegendPatchShape(QgsSymbol.SymbolType.Marker, QgsGeometry.fromWkt('MultiPoint((5 5), (2 1))'))
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Marker, QSizeF(1, 1))), [[[[0.875, 0.0], [0.125, 1.0]]]])
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Marker, QSizeF(10, 2))), [[[[5.75, 0.0], [4.25, 2.0]]]])
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Marker, QSizeF(2, 10))), [[[[2.0, 3.667], [0.0, 6.333]]]])

    def testLines(self):
        shape = QgsLegendPatchShape(QgsSymbol.SymbolType.Line, QgsGeometry.fromWkt('LineString(5 5, 1 2)'), False)
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Line, QSizeF(1, 1))), [[[[1.0, 0.0], [0.0, 1.0]]]])
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Line, QSizeF(10, 2))), [[[[10.0, 0.0], [0.0, 2.0]]]])

        shape = QgsLegendPatchShape(QgsSymbol.SymbolType.Line, QgsGeometry.fromWkt('LineString(1 5, 6 5)'), False)
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Line, QSizeF(1, 1))), [[[[0.0, 0.5], [1.0, 0.5]]]])
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Line, QSizeF(10, 2))), [[[[0.0, 1], [10.0, 1.0]]]])

        shape = QgsLegendPatchShape(QgsSymbol.SymbolType.Line, QgsGeometry.fromWkt('LineString(1 5, 1 10)'), False)
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Line, QSizeF(1, 1))), [[[[0.5, 0.0], [0.5, 1.0]]]])
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Line, QSizeF(10, 2))), [[[[5, 0.0], [5, 2.0]]]])

        # requesting different symbol type, should return default
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Fill, QSizeF(1, 1))), [[[[0.0, 0.0], [1.0, 0.0], [1.0, 1.0], [0.0, 1.0], [0.0, 0.0]]]])

        # circularstring
        shape = QgsLegendPatchShape(QgsSymbol.SymbolType.Line, QgsGeometry.fromWkt('CircularString(5 5, 1 2, 3 4)'), False)
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Line, QSizeF(1, 1)))[0][0][:5],
                         [[0.342, 0.026], [0.35, 0.023], [0.359, 0.02], [0.367, 0.018], [0.375, 0.016]])
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Line, QSizeF(10, 2)))[0][0][:5],
                         [[3.419, 0.051], [3.647, 0.042], [3.875, 0.036], [4.104, 0.034], [4.332, 0.036]])

        # multilinestring
        shape = QgsLegendPatchShape(QgsSymbol.SymbolType.Line, QgsGeometry.fromWkt('MultiLineString((5 5, 1 2),(3 6, 4 2))'), False)
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Line, QSizeF(1, 1))), [[[[1.0, 0.25], [0.0, 1.0]]], [[[0.5, 0.0], [0.75, 1.0]]]])
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Line, QSizeF(10, 2))), [[[[10.0, 0.5], [0.0, 2.0]]], [[[5.0, 0.0], [7.5, 2.0]]]])

    def testFills(self):
        shape = QgsLegendPatchShape(QgsSymbol.SymbolType.Fill, QgsGeometry.fromWkt('Polygon((5 5, 1 2, 3 4, 5 5))'), False)
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Fill, QSizeF(1, 1))), [[[[1.0, 0.0], [0.0, 1.0], [0.5, 0.333], [1.0, 0.0]]]])
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Fill, QSizeF(10, 2))), [[[[10.0, 0.0], [0.0, 2.0], [5.0, 0.667], [10.0, 0.0]]]])

        # requesting different symbol type, should return default
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Line, QSizeF(1, 1))), [[[[0.0, 0.5], [1.0, 0.5]]]])

        # rings
        shape = QgsLegendPatchShape(QgsSymbol.SymbolType.Fill, QgsGeometry.fromWkt('Polygon((5 5, 1 2, 3 4, 5 5), (4.5 4.5, 4.4 4.4, 4.5 4.4, 4.5 4.5))'), False)
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Fill, QSizeF(1, 1))),
                         [[[[1.0, 0.0], [0.0, 1.0], [0.5, 0.333], [1.0, 0.0]], [[0.875, 0.167], [0.85, 0.2], [0.875, 0.2], [0.875, 0.167]]]])
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Fill, QSizeF(10, 2))),
                         [[[[10.0, 0.0], [0.0, 2.0], [5.0, 0.667], [10.0, 0.0]], [[8.75, 0.333], [8.5, 0.4], [8.75, 0.4], [8.75, 0.333]]]])

        # circular
        shape = QgsLegendPatchShape(QgsSymbol.SymbolType.Fill, QgsGeometry.fromWkt('CurvePolygon(CircularString(5 5, 3 4, 1 2, 3 0, 5 5))'), False)
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Fill, QSizeF(1, 1)))[0][0][:5],
                         [[0.746, -0.0], [0.722, 0.009], [0.698, 0.018], [0.675, 0.028], [0.651, 0.038]])
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Fill, QSizeF(10, 2)))[0][0][:5],
                         [[7.459, -0.0], [6.83, 0.04], [6.201, 0.09], [5.574, 0.151], [4.947, 0.223]])

        # multipolygon
        shape = QgsLegendPatchShape(QgsSymbol.SymbolType.Fill, QgsGeometry.fromWkt('MultiPolygon(((5 5, 1 2, 3 4, 5 5), (4.5 4.5, 4.4 4.4, 4.5 4.4, 4.5 4.5)),((10 11, 11 11, 11 10, 10 11)))'), False)
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Fill, QSizeF(1, 1))), [[[[0.4, 0.667], [0.0, 1.0], [0.2, 0.778], [0.4, 0.667]], [[0.35, 0.722], [0.34, 0.733], [0.35, 0.733], [0.35, 0.722]]], [[[0.9, 0.0], [1.0, 0.0], [1.0, 0.111], [0.9, 0.0]]]])
        self.assertEqual(self.polys_to_list(shape.toQPolygonF(QgsSymbol.SymbolType.Fill, QSizeF(10, 2))), [[[[4.0, 1.333], [0.0, 2.0], [2.0, 1.556], [4.0, 1.333]], [[3.5, 1.444], [3.4, 1.467], [3.5, 1.467], [3.5, 1.444]]], [[[9.0, 0.0], [10.0, 0.0], [10.0, 0.222], [9.0, 0.0]]]])

    def testScaledGeometry(self):
        """
        Test scaling geometry
        """
        shape = QgsLegendPatchShape(QgsSymbol.SymbolType.Line, QgsGeometry.fromWkt('LineString(5 5, 1 2)'))

        self.assertEqual(shape.scaledGeometry(QSizeF(20, 30)).asWkt(1), 'LineString (20 7.5, 0 22.5)')
        self.assertEqual(shape.scaledGeometry(QSizeF(200, 300)).asWkt(1), 'LineString (200 75, 0 225)')
        shape.setScaleToOutputSize(False)
        self.assertEqual(shape.scaledGeometry(QSizeF(20, 30)).asWkt(1), 'LineString (5 5, 1 2)')
        self.assertEqual(shape.scaledGeometry(QSizeF(200, 300)).asWkt(1), 'LineString (5 5, 1 2)')

    def testRenderMarker(self):
        shape = QgsLegendPatchShape(QgsSymbol.SymbolType.Marker, QgsGeometry.fromWkt('MultiPoint((5 5), (3 4), (1 2))'), False)
        rendered_image = self.renderPatch(shape)
        self.assertTrue(
            self.image_check('Marker', 'marker_multipoint', rendered_image,
                             color_tolerance=2,
                             allowed_mismatch=20)
        )

    def testRenderMarkerPreserve(self):
        shape = QgsLegendPatchShape(QgsSymbol.SymbolType.Marker, QgsGeometry.fromWkt('MultiPoint((5 5), (3 4), (1 2))'), True)
        rendered_image = self.renderPatch(shape)
        self.assertTrue(
            self.image_check('Marker Preserve', 'marker_multipoint_preserve', rendered_image,
                             color_tolerance=2,
                             allowed_mismatch=20)
        )

    def testRenderLine(self):
        shape = QgsLegendPatchShape(QgsSymbol.SymbolType.Line, QgsGeometry.fromWkt('LineString(5 5, 3 4, 1 2)'), False)
        rendered_image = self.renderPatch(shape)
        self.assertTrue(
            self.image_check('Line', 'line', rendered_image,
                             color_tolerance=2,
                             allowed_mismatch=20)
        )

    def testRenderLinePreserve(self):
        shape = QgsLegendPatchShape(QgsSymbol.SymbolType.Line, QgsGeometry.fromWkt('LineString(5 5, 3 4, 1 2)'), True)
        rendered_image = self.renderPatch(shape)
        self.assertTrue(
            self.image_check('Line Preserve', 'line_preserve', rendered_image,
                             color_tolerance=2,
                             allowed_mismatch=20)
        )

    def testRenderMultiLine(self):
        shape = QgsLegendPatchShape(QgsSymbol.SymbolType.Line, QgsGeometry.fromWkt('MultiLineString((5 5, 3 4, 1 2), ( 6 6, 6 0))'), True)
        rendered_image = self.renderPatch(shape)
        self.assertTrue(
            self.image_check('Multiline', 'multiline', rendered_image,
                             color_tolerance=2,
                             allowed_mismatch=20)
        )

    def testRenderPolygon(self):
        shape = QgsLegendPatchShape(QgsSymbol.SymbolType.Fill, QgsGeometry.fromWkt('Polygon((1 1 , 6 1, 6 6, 1 1),(4 2, 5 3, 4 3, 4 2))'), False)
        rendered_image = self.renderPatch(shape)
        self.assertTrue(
            self.image_check('Polygon', 'polygon', rendered_image,
                             color_tolerance=2,
                             allowed_mismatch=20)
        )

    def testRenderMultiPolygon(self):
        shape = QgsLegendPatchShape(QgsSymbol.SymbolType.Fill, QgsGeometry.fromWkt('MultiPolygon(((1 1 , 6 1, 6 6, 1 1),(4 2, 5 3, 4 3, 4 2)),((1 5, 2 5, 1 6, 1 5)))'), False)
        rendered_image = self.renderPatch(shape)
        self.assertTrue(
            self.image_check('MultiPolygon', 'multipolygon', rendered_image,
                             color_tolerance=2,
                             allowed_mismatch=20)
        )

    def testReadWriteXml(self):
        doc = QDomDocument("testdoc")
        elem = doc.createElement('test')
        shape = QgsLegendPatchShape(QgsSymbol.SymbolType.Line, QgsGeometry.fromWkt('MultiLineString((5 5, 3 4, 1 2), ( 6 6, 6 0))'), False)

        shape.writeXml(elem, doc, QgsReadWriteContext())

        s2 = QgsLegendPatchShape()
        s2.readXml(elem, QgsReadWriteContext())

        self.assertFalse(s2.isNull())
        self.assertEqual(s2.geometry().asWkt(), 'MultiLineString ((5 5, 3 4, 1 2),(6 6, 6 0))')
        self.assertFalse(s2.preserveAspectRatio())
        self.assertEqual(s2.symbolType(), QgsSymbol.SymbolType.Line)

    def renderPatch(self, patch):
        image = QImage(200, 200, QImage.Format.Format_RGB32)
        image.setDotsPerMeterX(int(96 / 25.4 * 1000))
        image.setDotsPerMeterY(int(96 / 25.4 * 1000))

        painter = QPainter()
        painter.begin(image)

        context = QgsRenderContext.fromQPainter(painter)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI

        try:
            image.fill(QColor(0, 0, 0))

            if patch.symbolType() == QgsSymbol.SymbolType.Fill:
                self.fill_symbol.drawPreviewIcon(painter, QSize(200, 200), None, False, None, patch)
            elif patch.symbolType() == QgsSymbol.SymbolType.Line:
                self.line_symbol.drawPreviewIcon(painter, QSize(200, 200), None, False, None, patch)
            elif patch.symbolType() == QgsSymbol.SymbolType.Marker:
                self.marker_symbol.drawPreviewIcon(painter, QSize(200, 200), None, False, None, patch)
        finally:
            painter.end()

        return image


if __name__ == '__main__':
    unittest.main()
