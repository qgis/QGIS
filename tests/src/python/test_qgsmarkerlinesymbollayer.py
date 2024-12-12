"""
***************************************************************************
    test_qgsmarkerlinesymbollayer.py
    ---------------------
    Date                 : November 2018
    Copyright            : (C) 2018 by Nyall Dawson
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

__author__ = "Nyall Dawson"
__date__ = "November 2018"
__copyright__ = "(C) 2018, Nyall Dawson"

import os

from qgis.PyQt.QtCore import QSize, Qt
from qgis.PyQt.QtGui import QColor, QImage, QPainter
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    Qgis,
    QgsFeature,
    QgsFillSymbol,
    QgsFontMarkerSymbolLayer,
    QgsFontUtils,
    QgsGeometry,
    QgsGeometryGeneratorSymbolLayer,
    QgsLineSymbol,
    QgsLineSymbolLayer,
    QgsMapSettings,
    QgsMarkerLineSymbolLayer,
    QgsMarkerSymbol,
    QgsProperty,
    QgsReadWriteContext,
    QgsRectangle,
    QgsRenderContext,
    QgsSimpleMarkerSymbolLayer,
    QgsSingleSymbolRenderer,
    QgsSymbol,
    QgsSymbolLayer,
    QgsSymbolLayerUtils,
    QgsTemplatedLineSymbolLayerBase,
    QgsUnitTypes,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsMarkerLineSymbolLayer(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "symbol_markerline"

    def testWidth(self):
        ms = QgsMapSettings()
        extent = QgsRectangle(100, 200, 100, 200)
        ms.setExtent(extent)
        ms.setOutputSize(QSize(400, 400))
        context = QgsRenderContext.fromMapSettings(ms)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        ms.setExtent(QgsRectangle(100, 150, 100, 150))
        ms.setOutputDpi(ms.outputDpi() * 2)
        context2 = QgsRenderContext.fromMapSettings(ms)
        context2.setScaleFactor(300 / 25.4)

        s = QgsFillSymbol()
        s.deleteSymbolLayer(0)

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacement(QgsMarkerLineSymbolLayer.Placement.FirstVertex)
        marker = QgsSimpleMarkerSymbolLayer(
            QgsSimpleMarkerSymbolLayer.Shape.Triangle, 10
        )
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)

        self.assertEqual(marker_line.width(), 10)
        self.assertAlmostEqual(marker_line.width(context), 37.795275590551185, 3)
        self.assertAlmostEqual(marker_line.width(context2), 118.11023622047244, 3)

        marker_line.subSymbol().setSizeUnit(QgsUnitTypes.RenderUnit.RenderPixels)
        self.assertAlmostEqual(marker_line.width(context), 10.0, 3)
        self.assertAlmostEqual(marker_line.width(context2), 10.0, 3)

    def testMultiplePlacements(self):
        line_symbol = QgsLineSymbol()
        line_symbol.deleteSymbolLayer(0)
        line_symbol.appendSymbolLayer(QgsMarkerLineSymbolLayer())
        line_symbol[0].setPlacements(
            Qgis.MarkerLinePlacements(
                Qgis.MarkerLinePlacement.FirstVertex
                | Qgis.MarkerLinePlacement.LastVertex
            )
        )

        marker = QgsSimpleMarkerSymbolLayer(
            QgsSimpleMarkerSymbolLayer.Shape.Triangle, 4
        )
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        line_symbol[0].setSubSymbol(marker_symbol)

        g = QgsGeometry.fromWkt("LineString(0 0, 10 0, 10 10, 0 10)")
        rendered_image = self.renderGeometry(line_symbol, g)
        self.assertTrue(
            self.image_check(
                "markerline_multiple_placement",
                "markerline_multiple_placement",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testFirstVertexNoRespectMultipart(self):
        line_symbol = QgsLineSymbol()
        line_symbol.deleteSymbolLayer(0)
        line_symbol.appendSymbolLayer(QgsMarkerLineSymbolLayer())
        line_symbol[0].setPlacements(
            Qgis.MarkerLinePlacements(Qgis.MarkerLinePlacement.FirstVertex)
        )

        marker = QgsSimpleMarkerSymbolLayer(
            QgsSimpleMarkerSymbolLayer.Shape.Triangle, 4
        )
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        line_symbol[0].setSubSymbol(marker_symbol)
        line_symbol[0].setPlaceOnEveryPart(True)

        g = QgsGeometry.fromWkt(
            "MultiLineString((0 0, 10 0, 10 10, 0 10),(3 3, 7 3, 7 7, 3 7))"
        )
        rendered_image = self.renderGeometry(line_symbol, g)
        self.assertTrue(
            self.image_check(
                "markerline_first_no_respect_multipart",
                "markerline_first_no_respect_multipart",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testFirstVertexRespectMultipart(self):
        line_symbol = QgsLineSymbol()
        line_symbol.deleteSymbolLayer(0)
        line_symbol.appendSymbolLayer(QgsMarkerLineSymbolLayer())
        line_symbol[0].setPlacements(
            Qgis.MarkerLinePlacements(Qgis.MarkerLinePlacement.FirstVertex)
        )

        marker = QgsSimpleMarkerSymbolLayer(
            QgsSimpleMarkerSymbolLayer.Shape.Triangle, 4
        )
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        line_symbol[0].setSubSymbol(marker_symbol)
        line_symbol[0].setPlaceOnEveryPart(False)

        g = QgsGeometry.fromWkt(
            "MultiLineString((0 0, 10 0, 10 10, 0 10),(3 3, 7 3, 7 7, 3 7))"
        )
        rendered_image = self.renderGeometry(line_symbol, g)
        self.assertTrue(
            self.image_check(
                "markerline_first_respect_multipart",
                "markerline_first_respect_multipart",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testLastVertexNoRespectMultipart(self):
        line_symbol = QgsLineSymbol()
        line_symbol.deleteSymbolLayer(0)
        line_symbol.appendSymbolLayer(QgsMarkerLineSymbolLayer())
        line_symbol[0].setPlacements(
            Qgis.MarkerLinePlacements(Qgis.MarkerLinePlacement.LastVertex)
        )

        marker = QgsSimpleMarkerSymbolLayer(
            QgsSimpleMarkerSymbolLayer.Shape.Triangle, 4
        )
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        line_symbol[0].setSubSymbol(marker_symbol)
        line_symbol[0].setPlaceOnEveryPart(True)

        g = QgsGeometry.fromWkt(
            "MultiLineString((0 0, 10 0, 10 10, 0 10),(3 3, 7 3, 7 7, 3 7))"
        )
        rendered_image = self.renderGeometry(line_symbol, g)
        self.assertTrue(
            self.image_check(
                "markerline_last_no_respect_multipart",
                "markerline_last_no_respect_multipart",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testLastVertexRespectMultipart(self):
        line_symbol = QgsLineSymbol()
        line_symbol.deleteSymbolLayer(0)
        line_symbol.appendSymbolLayer(QgsMarkerLineSymbolLayer())
        line_symbol[0].setPlacements(
            Qgis.MarkerLinePlacements(Qgis.MarkerLinePlacement.LastVertex)
        )

        marker = QgsSimpleMarkerSymbolLayer(
            QgsSimpleMarkerSymbolLayer.Shape.Triangle, 4
        )
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        line_symbol[0].setSubSymbol(marker_symbol)
        line_symbol[0].setPlaceOnEveryPart(False)

        g = QgsGeometry.fromWkt(
            "MultiLineString((0 0, 10 0, 10 10, 0 10),(3 3, 7 3, 7 7, 3 7))"
        )
        rendered_image = self.renderGeometry(line_symbol, g)
        self.assertTrue(
            self.image_check(
                "markerline_last_respect_multipart",
                "markerline_last_respect_multipart",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testFirstLastVertexNoRespectMultipart(self):
        line_symbol = QgsLineSymbol()
        line_symbol.deleteSymbolLayer(0)
        line_symbol.appendSymbolLayer(QgsMarkerLineSymbolLayer())
        line_symbol[0].setPlacements(
            Qgis.MarkerLinePlacements(
                Qgis.MarkerLinePlacement.FirstVertex
                | Qgis.MarkerLinePlacement.LastVertex
            )
        )

        marker = QgsSimpleMarkerSymbolLayer(
            QgsSimpleMarkerSymbolLayer.Shape.Triangle, 4
        )
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        line_symbol[0].setSubSymbol(marker_symbol)
        line_symbol[0].setPlaceOnEveryPart(True)

        g = QgsGeometry.fromWkt(
            "MultiLineString((0 0, 10 0, 10 10, 0 10),(3 3, 7 3, 7 7, 3 7))"
        )
        rendered_image = self.renderGeometry(line_symbol, g)
        self.assertTrue(
            self.image_check(
                "markerline_first_last_no_respect_multipart",
                "markerline_first_last_no_respect_multipart",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testFirstLastVertexRespectMultipart(self):
        line_symbol = QgsLineSymbol()
        line_symbol.deleteSymbolLayer(0)
        line_symbol.appendSymbolLayer(QgsMarkerLineSymbolLayer())
        line_symbol[0].setPlacements(
            Qgis.MarkerLinePlacements(
                Qgis.MarkerLinePlacement.FirstVertex
                | Qgis.MarkerLinePlacement.LastVertex
            )
        )

        marker = QgsSimpleMarkerSymbolLayer(
            QgsSimpleMarkerSymbolLayer.Shape.Triangle, 4
        )
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        line_symbol[0].setSubSymbol(marker_symbol)
        line_symbol[0].setPlaceOnEveryPart(False)

        g = QgsGeometry.fromWkt(
            "MultiLineString((0 0, 10 0, 10 10, 0 10),(3 3, 7 3, 7 7, 3 7))"
        )
        rendered_image = self.renderGeometry(line_symbol, g)
        self.assertTrue(
            self.image_check(
                "markerline_first_last_respect_multipart",
                "markerline_first_last_respect_multipart",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testInnerVerticesLine(self):
        line_symbol = QgsLineSymbol()
        line_symbol.deleteSymbolLayer(0)
        line_symbol.appendSymbolLayer(QgsMarkerLineSymbolLayer())
        line_symbol[0].setPlacements(
            Qgis.MarkerLinePlacements(Qgis.MarkerLinePlacement.InnerVertices)
        )

        marker = QgsSimpleMarkerSymbolLayer(
            QgsSimpleMarkerSymbolLayer.Shape.Triangle, 4
        )
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        line_symbol[0].setSubSymbol(marker_symbol)

        g = QgsGeometry.fromWkt("LineString(0 0, 10 0, 10 10, 0 10)")
        rendered_image = self.renderGeometry(line_symbol, g)
        self.assertTrue(
            self.image_check(
                "markerline_inner_vertices_line",
                "markerline_inner_vertices_line",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testInnerVerticesPolygon(self):
        fill_symbol = QgsFillSymbol()
        fill_symbol.deleteSymbolLayer(0)
        fill_symbol.appendSymbolLayer(QgsMarkerLineSymbolLayer())
        fill_symbol[0].setPlacements(
            Qgis.MarkerLinePlacements(Qgis.MarkerLinePlacement.InnerVertices)
        )

        marker = QgsSimpleMarkerSymbolLayer(
            QgsSimpleMarkerSymbolLayer.Shape.Triangle, 4
        )
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        fill_symbol[0].setSubSymbol(marker_symbol)

        g = QgsGeometry.fromWkt("Polygon((0 0, 10 0, 10 10, 0 10, 0 0))")
        rendered_image = self.renderGeometry(fill_symbol, g)
        self.assertTrue(
            self.image_check(
                "markerline_inner_vertices_polygon",
                "markerline_inner_vertices_polygon",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testRingFilter(self):
        # test filtering rings during rendering
        s = QgsFillSymbol()
        s.deleteSymbolLayer(0)

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacement(QgsMarkerLineSymbolLayer.Placement.FirstVertex)
        marker = QgsSimpleMarkerSymbolLayer(
            QgsSimpleMarkerSymbolLayer.Shape.Triangle, 4
        )
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)

        s.appendSymbolLayer(marker_line.clone())
        self.assertEqual(
            s.symbolLayer(0).ringFilter(), QgsLineSymbolLayer.RenderRingFilter.AllRings
        )
        s.symbolLayer(0).setRingFilter(
            QgsLineSymbolLayer.RenderRingFilter.ExteriorRingOnly
        )
        self.assertEqual(
            s.symbolLayer(0).ringFilter(),
            QgsLineSymbolLayer.RenderRingFilter.ExteriorRingOnly,
        )

        s2 = s.clone()
        self.assertEqual(
            s2.symbolLayer(0).ringFilter(),
            QgsLineSymbolLayer.RenderRingFilter.ExteriorRingOnly,
        )

        doc = QDomDocument()
        context = QgsReadWriteContext()
        element = QgsSymbolLayerUtils.saveSymbol("test", s, doc, context)

        s2 = QgsSymbolLayerUtils.loadSymbol(element, context)
        self.assertEqual(
            s2.symbolLayer(0).ringFilter(),
            QgsLineSymbolLayer.RenderRingFilter.ExteriorRingOnly,
        )

        # rendering test
        s3 = QgsFillSymbol()
        s3.deleteSymbolLayer(0)
        s3.appendSymbolLayer(QgsMarkerLineSymbolLayer())
        s3.symbolLayer(0).setRingFilter(
            QgsLineSymbolLayer.RenderRingFilter.ExteriorRingOnly
        )
        s3.symbolLayer(0).setAverageAngleLength(0)

        g = QgsGeometry.fromWkt(
            "Polygon((0 0, 10 0, 10 10, 0 10, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1),(8 8, 9 8, 9 9, 8 9, 8 8))"
        )
        rendered_image = self.renderGeometry(s3, g)
        self.assertTrue(
            self.image_check(
                "markerline_exterioronly",
                "markerline_exterioronly",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

        s3.symbolLayer(0).setRingFilter(
            QgsLineSymbolLayer.RenderRingFilter.InteriorRingsOnly
        )
        g = QgsGeometry.fromWkt(
            "Polygon((0 0, 10 0, 10 10, 0 10, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1),(8 8, 9 8, 9 9, 8 9, 8 8))"
        )
        rendered_image = self.renderGeometry(s3, g)
        self.assertTrue(
            self.image_check(
                "markerline_interioronly",
                "markerline_interioronly",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testRingNumberVariable(self):
        # test test geometry_ring_num variable
        s3 = QgsFillSymbol()
        s3.deleteSymbolLayer(0)
        s3.appendSymbolLayer(QgsMarkerLineSymbolLayer())
        s3.symbolLayer(0).subSymbol()[0].setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyFillColor,
            QgsProperty.fromExpression(
                "case when @geometry_ring_num=0 then 'green' when @geometry_ring_num=1 then 'blue' when @geometry_ring_num=2 then 'red' end"
            ),
        )
        s3.symbolLayer(0).setAverageAngleLength(0)

        g = QgsGeometry.fromWkt(
            "Polygon((0 0, 10 0, 10 10, 0 10, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1),(8 8, 9 8, 9 9, 8 9, 8 8))"
        )
        rendered_image = self.renderGeometry(s3, g)
        self.assertTrue(
            self.image_check(
                "markerline_ring_num",
                "markerline_ring_num",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testPartNum(self):
        # test geometry_part_num variable
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        sym_layer = QgsGeometryGeneratorSymbolLayer.create(
            {"geometryModifier": "segments_to_lines($geometry)"}
        )
        sym_layer.setSymbolType(QgsSymbol.SymbolType.Line)
        s.appendSymbolLayer(sym_layer)

        marker_line = QgsMarkerLineSymbolLayer(False)
        marker_line.setPlacement(QgsMarkerLineSymbolLayer.Placement.FirstVertex)
        f = QgsFontUtils.getStandardTestFont("Bold", 24)
        marker = QgsFontMarkerSymbolLayer(f.family(), "x", 24, QColor(255, 255, 0))
        marker.setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyCharacter,
            QgsProperty.fromExpression("@geometry_part_num"),
        )
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)
        marker_line.setAverageAngleLength(0)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, marker_line)
        sym_layer.setSubSymbol(line_symbol)

        # rendering test
        g = QgsGeometry.fromWkt("LineString(0 0, 10 0, 10 10, 0 10)")
        rendered_image = self.renderGeometry(s, g, buffer=4)
        self.assertTrue(
            self.image_check(
                "part_num_variable",
                "part_num_variable",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

        marker.setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyCharacter,
            QgsProperty.fromExpression("@geometry_part_count"),
        )

        # rendering test
        g = QgsGeometry.fromWkt("LineString(0 0, 10 0, 10 10, 0 10)")
        rendered_image = self.renderGeometry(s, g, buffer=4)
        self.assertTrue(
            self.image_check(
                "part_count_variable",
                "part_count_variable",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testPartNumPolygon(self):
        # test geometry_part_num variable
        s = QgsFillSymbol()

        marker_line = QgsMarkerLineSymbolLayer(False)
        marker_line.setPlacement(QgsMarkerLineSymbolLayer.Placement.FirstVertex)
        f = QgsFontUtils.getStandardTestFont("Bold", 24)
        marker = QgsFontMarkerSymbolLayer(f.family(), "x", 24, QColor(255, 255, 0))
        marker.setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyCharacter,
            QgsProperty.fromExpression("@geometry_part_num"),
        )
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)
        marker_line.setAverageAngleLength(0)
        s.changeSymbolLayer(0, marker_line)

        # rendering test - a polygon with a smaller part first
        g = QgsGeometry.fromWkt(
            "MultiPolygon(((0 0, 2 0, 2 2, 0 0)),((10 0, 10 10, 0 10, 10 0)))"
        )
        rendered_image = self.renderGeometry(s, g, buffer=4)
        self.assertTrue(
            self.image_check(
                "poly_part_num_variable",
                "poly_part_num_variable",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testCompoundCurve(self):
        # test rendering compound curve with markers at vertices and curve points
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacement(QgsMarkerLineSymbolLayer.Placement.Vertex)
        marker = QgsSimpleMarkerSymbolLayer(
            QgsSimpleMarkerSymbolLayer.Shape.Triangle, 4
        )
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)

        s.appendSymbolLayer(marker_line.clone())

        marker_line2 = QgsMarkerLineSymbolLayer(True)
        marker_line2.setPlacement(QgsMarkerLineSymbolLayer.Placement.CurvePoint)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Shape.Square, 4)
        marker.setColor(QColor(0, 255, 0))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line2.setSubSymbol(marker_symbol)

        s.appendSymbolLayer(marker_line2.clone())

        # rendering test
        g = QgsGeometry.fromWkt(
            "CompoundCurve (CircularString (2606642.3863534671254456 1228883.61571401031687856, 2606656.45901552261784673 1228882.30281259422190487, 2606652.60236761253327131 1228873.80998155777342618, 2606643.65822671446949244 1228875.45110832806676626, 2606642.3863534671254456 1228883.65674217976629734))"
        )
        self.assertFalse(g.isNull())
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "markerline_compoundcurve",
                "markerline_compoundcurve",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testCompoundCurveInnerVertices(self):
        # test rendering compound curve with markers at inner vertices and curve points
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacement(QgsMarkerLineSymbolLayer.Placement.InnerVertices)
        marker = QgsSimpleMarkerSymbolLayer(
            QgsSimpleMarkerSymbolLayer.Shape.Triangle, 4
        )
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)

        s.appendSymbolLayer(marker_line.clone())

        # rendering test
        g = QgsGeometry.fromWkt(
            "CompoundCurve (CircularString (2606642.3863534671254456 1228883.61571401031687856, 2606656.45901552261784673 1228882.30281259422190487, 2606652.60236761253327131 1228873.80998155777342618, 2606643.65822671446949244 1228875.45110832806676626, 2606642.3863534671254456 1228883.65674217976629734))"
        )
        self.assertFalse(g.isNull())
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "markerline_compoundcurve_inner_vertices",
                "markerline_compoundcurve_inner_vertices",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testMultiCurve(self):
        # test rendering multi curve with markers at vertices and curve points
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacement(QgsMarkerLineSymbolLayer.Placement.Vertex)
        marker = QgsSimpleMarkerSymbolLayer(
            QgsSimpleMarkerSymbolLayer.Shape.Triangle, 4
        )
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)

        s.appendSymbolLayer(marker_line.clone())

        marker_line2 = QgsMarkerLineSymbolLayer(True)
        marker_line2.setPlacement(QgsMarkerLineSymbolLayer.Placement.CurvePoint)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Shape.Square, 4)
        marker.setColor(QColor(0, 255, 0))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line2.setSubSymbol(marker_symbol)

        s.appendSymbolLayer(marker_line2.clone())

        # rendering test
        g = QgsGeometry.fromWkt(
            "MultiCurve (CompoundCurve (CircularString (2606668.74491960229352117 1228910.0701227153185755, 2606667.84593895543366671 1228899.48981202743016183, 2606678.70285907341167331 1228879.78139015776105225, 2606701.64743852475658059 1228866.43043032777495682, 2606724.96578619908541441 1228864.70617623627185822)),LineString (2606694.16802780656144023 1228913.44624055083841085, 2606716.84054400492459536 1228890.51009044284000993, 2606752.43112175865098834 1228906.59175890940241516))"
        )
        self.assertFalse(g.isNull())
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "markerline_multicurve",
                "markerline_multicurve",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testCurvePolygon(self):
        # test rendering curve polygon with markers at vertices and curve points
        s = QgsFillSymbol()
        s.deleteSymbolLayer(0)

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacement(QgsMarkerLineSymbolLayer.Placement.Vertex)
        marker = QgsSimpleMarkerSymbolLayer(
            QgsSimpleMarkerSymbolLayer.Shape.Triangle, 4
        )
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)

        s.appendSymbolLayer(marker_line.clone())

        marker_line2 = QgsMarkerLineSymbolLayer(True)
        marker_line2.setPlacement(QgsMarkerLineSymbolLayer.Placement.CurvePoint)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Shape.Square, 4)
        marker.setColor(QColor(0, 255, 0))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line2.setSubSymbol(marker_symbol)

        s.appendSymbolLayer(marker_line2.clone())

        # rendering test
        g = QgsGeometry.fromWkt(
            "CurvePolygon (CompoundCurve (CircularString (2606711.1353147104382515 1228875.77055342611856759, 2606715.00784672703593969 1228870.79158369055949152, 2606721.16240653907880187 1228873.35022091586142778),(2606721.16240653907880187 1228873.35022091586142778, 2606711.1353147104382515 1228875.77055342611856759)))"
        )
        self.assertFalse(g.isNull())
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "markerline_curvepolygon",
                "markerline_curvepolygon",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testMultiSurve(self):
        # test rendering multisurface with markers at vertices and curve points
        s = QgsFillSymbol()
        s.deleteSymbolLayer(0)

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacement(QgsMarkerLineSymbolLayer.Placement.Vertex)
        marker = QgsSimpleMarkerSymbolLayer(
            QgsSimpleMarkerSymbolLayer.Shape.Triangle, 4
        )
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)

        s.appendSymbolLayer(marker_line.clone())

        marker_line2 = QgsMarkerLineSymbolLayer(True)
        marker_line2.setPlacement(QgsMarkerLineSymbolLayer.Placement.CurvePoint)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Shape.Square, 4)
        marker.setColor(QColor(0, 255, 0))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line2.setSubSymbol(marker_symbol)

        s.appendSymbolLayer(marker_line2.clone())

        # rendering test
        g = QgsGeometry.fromWkt(
            "MultiSurface (CurvePolygon (CompoundCurve (CircularString (2606664.83926784340292215 1228868.83649749564938247, 2606666.84044930292293429 1228872.22980518848635256, 2606668.05855975672602654 1228875.62311288132332265, 2606674.45363963954150677 1228870.05460794945247471, 2606680.58769585331901908 1228866.00874108518473804, 2606680.7182076876051724 1228865.05165429995395243, 2606679.97864062618464231 1228864.61661485210061073, 2606671.93041084241122007 1228867.87941071065142751, 2606664.83926784340292215 1228868.79299355088733137),(2606664.83926784340292215 1228868.79299355088733137, 2606664.83926784340292215 1228868.83649749564938247))),Polygon ((2606677.23432376980781555 1228875.74241803237237036, 2606674.27243852382525802 1228874.75512295053340495, 2606675.61874999897554517 1228871.97274590120650828, 2606678.84989754017442465 1228870.35717213083989918, 2606680.64497950719669461 1228873.31905737658962607, 2606677.23432376980781555 1228875.74241803237237036)))"
        )
        self.assertFalse(g.isNull())
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "markerline_multisurface",
                "markerline_multisurface",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testMultiSurfaceOnePart(self):
        # test rendering multisurface with one part with markers at vertices and curve points
        s = QgsFillSymbol()
        s.deleteSymbolLayer(0)

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacement(QgsMarkerLineSymbolLayer.Placement.Vertex)
        marker = QgsSimpleMarkerSymbolLayer(
            QgsSimpleMarkerSymbolLayer.Shape.Triangle, 4
        )
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)

        s.appendSymbolLayer(marker_line.clone())

        marker_line2 = QgsMarkerLineSymbolLayer(True)
        marker_line2.setPlacement(QgsMarkerLineSymbolLayer.Placement.CurvePoint)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Shape.Square, 4)
        marker.setColor(QColor(0, 255, 0))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line2.setSubSymbol(marker_symbol)

        s.appendSymbolLayer(marker_line2.clone())

        # rendering test
        g = QgsGeometry.fromWkt(
            "MultiSurface (CurvePolygon (CompoundCurve (CircularString (2606664.83926784340292215 1228868.83649749564938247, 2606666.84044930292293429 1228872.22980518848635256, 2606668.05855975672602654 1228875.62311288132332265, 2606674.45363963954150677 1228870.05460794945247471, 2606680.58769585331901908 1228866.00874108518473804, 2606680.7182076876051724 1228865.05165429995395243, 2606679.97864062618464231 1228864.61661485210061073, 2606671.93041084241122007 1228867.87941071065142751, 2606664.83926784340292215 1228868.79299355088733137),(2606664.83926784340292215 1228868.79299355088733137, 2606664.83926784340292215 1228868.83649749564938247))))"
        )
        self.assertFalse(g.isNull())
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "markerline_one_part_multisurface",
                "markerline_one_part_multisurface",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testMarkerAverageAngle(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacement(QgsTemplatedLineSymbolLayerBase.Placement.Interval)
        marker_line.setInterval(6)
        marker = QgsSimpleMarkerSymbolLayer(
            QgsSimpleMarkerSymbolLayer.Shape.Triangle, 4
        )
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)
        marker_line.setAverageAngleLength(60)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, marker_line)

        s.appendSymbolLayer(marker_line.clone())

        g = QgsGeometry.fromWkt("LineString(0 0, 10 10, 10 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "markerline_average_angle",
                "markerline_average_angle",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testMarkerAverageAngleRing(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacement(QgsTemplatedLineSymbolLayerBase.Placement.Interval)
        marker_line.setInterval(6)
        marker = QgsSimpleMarkerSymbolLayer(
            QgsSimpleMarkerSymbolLayer.Shape.Triangle, 4
        )
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)
        marker_line.setAverageAngleLength(60)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, marker_line)

        s.appendSymbolLayer(marker_line.clone())

        g = QgsGeometry.fromWkt("LineString(0 0, 0 10, 10 10, 10 0, 0 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "markerline_ring_average_angle",
                "markerline_ring_average_angle",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testMarkerAverageAngleCenter(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacement(QgsTemplatedLineSymbolLayerBase.Placement.CentralPoint)
        marker = QgsSimpleMarkerSymbolLayer(
            QgsSimpleMarkerSymbolLayer.Shape.Triangle, 4
        )
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)
        marker_line.setAverageAngleLength(60)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, marker_line)

        s.appendSymbolLayer(marker_line.clone())

        g = QgsGeometry.fromWkt("LineString(0 0, 10 10, 10 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "markerline_center_average_angle",
                "markerline_center_average_angle",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testRingNoDupe(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacement(QgsTemplatedLineSymbolLayerBase.Placement.Interval)
        marker_line.setInterval(10)
        marker_line.setIntervalUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Shape.Circle, 4)
        marker.setColor(QColor(255, 0, 0, 100))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, marker_line)

        s.appendSymbolLayer(marker_line.clone())

        g = QgsGeometry.fromWkt("LineString(0 0, 0 10, 10 10, 10 0, 0 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "markerline_ring_no_dupes",
                "markerline_ring_no_dupes",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testSinglePoint(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacement(QgsTemplatedLineSymbolLayerBase.Placement.Interval)
        marker_line.setInterval(1000)
        marker_line.setIntervalUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Shape.Circle, 4)
        marker.setColor(QColor(255, 0, 0, 100))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, marker_line)

        s.appendSymbolLayer(marker_line.clone())

        g = QgsGeometry.fromWkt("LineString(0 0, 0 10, 10 10)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "markerline_single",
                "markerline_single",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testNoPoint(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacement(QgsTemplatedLineSymbolLayerBase.Placement.Interval)
        marker_line.setOffsetAlongLine(1000)
        marker_line.setIntervalUnit(QgsUnitTypes.RenderUnit.RenderMapUnits)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Shape.Circle, 4)
        marker.setColor(QColor(255, 0, 0, 100))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, marker_line)

        s.appendSymbolLayer(marker_line.clone())

        g = QgsGeometry.fromWkt("LineString(0 0, 0 10, 10 10)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "markerline_none",
                "markerline_none",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testFirstVertexOffsetPercentage(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacements(Qgis.MarkerLinePlacement.FirstVertex)
        marker_line.setOffsetAlongLine(10)
        marker_line.setOffsetAlongLineUnit(QgsUnitTypes.RenderUnit.RenderPercentage)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Shape.Circle, 4)
        marker.setColor(QColor(255, 0, 0, 100))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, marker_line)

        s.appendSymbolLayer(marker_line.clone())

        g = QgsGeometry.fromWkt("LineString(0 0, 0 10, 10 10)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "markerline_first_offset_percent",
                "markerline_first_offset_percent",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testClosedRingFirstVertexOffsetLarge(self):
        """
        Test that an offset larger than the length of a closed line effectively "loops" around the ring
        """
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacements(Qgis.MarkerLinePlacement.FirstVertex)
        marker_line.setOffsetAlongLine(110)
        marker_line.setOffsetAlongLineUnit(QgsUnitTypes.RenderUnit.RenderPercentage)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Shape.Circle, 4)
        marker.setColor(QColor(255, 0, 0, 100))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, marker_line)

        s.appendSymbolLayer(marker_line.clone())

        g = QgsGeometry.fromWkt("LineString(0 0, 0 10, 10 10, 0 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "markerline_first_large_offset",
                "markerline_first_large_offset",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testClosedRingFirstVertexOffsetNegative(self):
        """
        Test that a negative offset of a closed line effectively "loops" around the ring
        """
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacements(Qgis.MarkerLinePlacement.FirstVertex)
        marker_line.setOffsetAlongLine(-20)
        marker_line.setOffsetAlongLineUnit(QgsUnitTypes.RenderUnit.RenderPercentage)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Shape.Circle, 4)
        marker.setColor(QColor(255, 0, 0, 100))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, marker_line)

        s.appendSymbolLayer(marker_line.clone())

        g = QgsGeometry.fromWkt("LineString(0 0, 0 10, 10 10, 0 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "markerline_first_negative_offset",
                "markerline_first_negative_offset",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testIntervalOffsetPercentage(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacements(Qgis.MarkerLinePlacement.Interval)
        marker_line.setInterval(6)
        marker_line.setOffsetAlongLine(50)
        marker_line.setOffsetAlongLineUnit(QgsUnitTypes.RenderUnit.RenderPercentage)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Shape.Circle, 4)
        marker.setColor(QColor(255, 0, 0, 100))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, marker_line)

        s.appendSymbolLayer(marker_line.clone())

        g = QgsGeometry.fromWkt("LineString(0 0, 0 10, 10 10)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "markerline_interval_offset_percent",
                "markerline_interval_offset_percent",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testClosedRingIntervalOffsetLarge(self):
        """
        Test that an offset larger than the length of a closed line effectively "loops" around the ring
        """
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacements(Qgis.MarkerLinePlacement.Interval)
        marker_line.setInterval(6)
        marker_line.setOffsetAlongLine(150)
        marker_line.setOffsetAlongLineUnit(QgsUnitTypes.RenderUnit.RenderPercentage)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Shape.Circle, 4)
        marker.setColor(QColor(255, 0, 0, 100))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, marker_line)

        s.appendSymbolLayer(marker_line.clone())

        g = QgsGeometry.fromWkt("LineString(0 0, 0 10, 10 10, 0 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "markerline_interval_large_offset",
                "markerline_interval_large_offset",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testClosedRingIntervalOffsetNegative(self):
        """
        Test that a negative offset for a closed line effectively "loops" around the ring
        """
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacements(Qgis.MarkerLinePlacement.Interval)
        marker_line.setInterval(6)
        marker_line.setOffsetAlongLine(-50)
        marker_line.setOffsetAlongLineUnit(QgsUnitTypes.RenderUnit.RenderPercentage)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Shape.Circle, 4)
        marker.setColor(QColor(255, 0, 0, 100))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, marker_line)

        s.appendSymbolLayer(marker_line.clone())

        g = QgsGeometry.fromWkt("LineString(0 0, 0 10, 10 10, 0 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "markerline_interval_large_offset",
                "markerline_interval_large_offset",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testCenterSegment(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacement(
            QgsTemplatedLineSymbolLayerBase.Placement.SegmentCenter
        )
        marker = QgsSimpleMarkerSymbolLayer(
            QgsSimpleMarkerSymbolLayer.Shape.Triangle, 4
        )
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, marker_line)

        s.appendSymbolLayer(marker_line.clone())

        g = QgsGeometry.fromWkt("LineString(0 0, 10 0, 0 10)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "markerline_segmentcenter",
                "markerline_segmentcenter",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testMarkerDataDefinedAngleLine(self):
        """Test issue https://github.com/qgis/QGIS/issues/38716"""

        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setRotateSymbols(True)
        marker_line.setPlacement(QgsTemplatedLineSymbolLayerBase.Placement.CentralPoint)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Shape.Arrow, 10)
        marker.setAngle(90)
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, marker_line)

        s.appendSymbolLayer(marker_line.clone())

        g = QgsGeometry.fromWkt("LineString(0 0, 10 10, 20 20)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "markerline_center_angle_dd",
                "markerline_center_angle_dd",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

        # Now with DD

        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setRotateSymbols(True)
        marker_line.setPlacement(QgsTemplatedLineSymbolLayerBase.Placement.CentralPoint)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Shape.Arrow, 10)
        # Note: set this to a different value than the reference test (90)
        marker.setAngle(30)
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        # This is the same value of the reference test
        marker_symbol.setDataDefinedAngle(QgsProperty.fromExpression("90"))
        marker_line.setSubSymbol(marker_symbol)
        line_symbol = QgsLineSymbol()
        line_symbol.changeSymbolLayer(0, marker_line)

        s.appendSymbolLayer(marker_line.clone())

        g = QgsGeometry.fromWkt("LineString(0 0, 10 10, 20 20)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "markerline_center_angle_dd",
                "markerline_center_angle_dd",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testDataDefinedAnglePolygon(self):
        # test rendering curve polygon with markers at vertices and curve points
        s = QgsFillSymbol()

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacement(QgsMarkerLineSymbolLayer.Placement.SegmentCenter)
        marker = QgsSimpleMarkerSymbolLayer(
            QgsSimpleMarkerSymbolLayer.Shape.Triangle, 4
        )
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker.setAngle(90)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_line.setSubSymbol(marker_symbol)

        s.appendSymbolLayer(marker_line.clone())

        g = QgsGeometry.fromWkt("Polygon (LineString (0 5, 5 0, 10 5, 5 10, 0 5))")
        self.assertFalse(g.isNull())

        # rendering test with non data-defined angle
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "markerline_datadefinedanglepolygon",
                "markerline_datadefinedanglepolygon",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

        s = QgsFillSymbol()

        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacement(QgsMarkerLineSymbolLayer.Placement.SegmentCenter)
        marker = QgsSimpleMarkerSymbolLayer(
            QgsSimpleMarkerSymbolLayer.Shape.Triangle, 4
        )
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.PenStyle.NoPen)
        marker.setAngle(38)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_symbol.setDataDefinedAngle(QgsProperty.fromExpression("90"))
        marker_line.setSubSymbol(marker_symbol)

        s.appendSymbolLayer(marker_line.clone())

        # rendering test with data-defined angle
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "markerline_datadefinedanglepolygon",
                "markerline_datadefinedanglepolygon",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testOpacityWithDataDefinedColor(self):
        line_shp = os.path.join(TEST_DATA_DIR, "lines.shp")
        line_layer = QgsVectorLayer(line_shp, "Lines", "ogr")
        self.assertTrue(line_layer.isValid())

        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)
        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacement(QgsTemplatedLineSymbolLayerBase.Placement.CentralPoint)
        simple_marker = QgsSimpleMarkerSymbolLayer(
            QgsSimpleMarkerSymbolLayer.Shape.Circle, 10
        )
        simple_marker.setColor(QColor(0, 255, 0))
        simple_marker.setStrokeColor(QColor(255, 0, 0))
        simple_marker.setStrokeWidth(1)
        simple_marker.setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyFillColor,
            QgsProperty.fromExpression("if(Name='Arterial', 'red', 'green')"),
        )
        simple_marker.setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyStrokeColor,
            QgsProperty.fromExpression("if(Name='Arterial', 'magenta', 'blue')"),
        )

        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, simple_marker)
        marker_symbol.setOpacity(0.5)
        marker_line.setSubSymbol(marker_symbol)
        s.appendSymbolLayer(marker_line.clone())

        # set opacity on both the symbol and subsymbol, to test that they get combined
        s.setOpacity(0.5)

        line_layer.setRenderer(QgsSingleSymbolRenderer(s))

        ms = QgsMapSettings()
        ms.setOutputSize(QSize(400, 400))
        ms.setOutputDpi(96)
        ms.setExtent(QgsRectangle(-118.5, 19.0, -81.4, 50.4))
        ms.setLayers([line_layer])

        # Test rendering
        self.assertTrue(
            self.render_map_settings_check(
                "markerline_opacityddcolor", "markerline_opacityddcolor", ms
            )
        )

    def testDataDefinedOpacity(self):
        line_shp = os.path.join(TEST_DATA_DIR, "lines.shp")
        line_layer = QgsVectorLayer(line_shp, "Lines", "ogr")
        self.assertTrue(line_layer.isValid())

        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)
        marker_line = QgsMarkerLineSymbolLayer(True)
        marker_line.setPlacement(QgsTemplatedLineSymbolLayerBase.Placement.CentralPoint)
        simple_marker = QgsSimpleMarkerSymbolLayer(
            QgsSimpleMarkerSymbolLayer.Shape.Circle, 10
        )
        simple_marker.setColor(QColor(0, 255, 0))
        simple_marker.setStrokeColor(QColor(255, 0, 0))
        simple_marker.setStrokeWidth(1)
        simple_marker.setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyFillColor,
            QgsProperty.fromExpression("if(Name='Arterial', 'red', 'green')"),
        )
        simple_marker.setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyStrokeColor,
            QgsProperty.fromExpression("if(Name='Arterial', 'magenta', 'blue')"),
        )

        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, simple_marker)
        marker_symbol.setOpacity(0.5)
        marker_line.setSubSymbol(marker_symbol)
        s.appendSymbolLayer(marker_line.clone())

        s.setDataDefinedProperty(
            QgsSymbol.Property.PropertyOpacity,
            QgsProperty.fromExpression('if("Value" = 1, 25, 50)'),
        )

        line_layer.setRenderer(QgsSingleSymbolRenderer(s))

        ms = QgsMapSettings()
        ms.setOutputSize(QSize(400, 400))
        ms.setOutputDpi(96)
        ms.setExtent(QgsRectangle(-118.5, 19.0, -81.4, 50.4))
        ms.setLayers([line_layer])

        # Test rendering
        self.assertTrue(
            self.render_map_settings_check(
                "markerline_ddopacity", "markerline_ddopacity", ms
            )
        )

    def renderGeometry(self, symbol, geom, buffer=20):
        f = QgsFeature()
        f.setGeometry(geom)

        image = QImage(200, 200, QImage.Format.Format_RGB32)

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


if __name__ == "__main__":
    unittest.main()
