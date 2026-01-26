"""
***************************************************************************
    test_qgslinearreferencingsymbollayer.py
    ---------------------
    Date                 : August 2024
    Copyright            : (C) 2024 by Nyall Dawson
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

import unittest

from qgis.PyQt.QtCore import QPointF, QSize
from qgis.PyQt.QtGui import QColor, QImage, QPainter
from qgis.core import (
    Qgis,
    QgsFeature,
    QgsGeometry,
    QgsLineSymbol,
    QgsMapSettings,
    QgsRenderContext,
    QgsLinearReferencingSymbolLayer,
    QgsTextFormat,
    QgsFontUtils,
    QgsBasicNumericFormat,
    QgsMarkerSymbol,
    QgsFillSymbol,
    QgsVectorLayer,
    QgsSingleSymbolRenderer,
)
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsSimpleLineSymbolLayer(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "symbol_linearref"

    def test_distance_2d(self):
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2"}
        )

        linear_ref = QgsLinearReferencingSymbolLayer()
        linear_ref.setPlacement(Qgis.LinearReferencingPlacement.IntervalCartesian2D)
        linear_ref.setInterval(1)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        text_format = QgsTextFormat.fromQFont(font)
        text_format.setColor(QColor(255, 255, 255))
        linear_ref.setTextFormat(text_format)

        linear_ref.setLabelOffset(QPointF(3, -1))

        s.appendSymbolLayer(linear_ref)

        rendered_image = self.renderGeometry(
            s,
            QgsGeometry.fromWkt(
                "MultiLineStringZM ((6 2 0.2 1.2, 9 2 0.7 0.2, 9 3 0.4 0, 11 5 0.8 0.4))"
            ),
        )
        self.assertTrue(
            self.image_check(
                "distance_2d",
                "distance_2d",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def test_render_using_label_engine(self):
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2"}
        )

        linear_ref = QgsLinearReferencingSymbolLayer()
        linear_ref.setPlacement(Qgis.LinearReferencingPlacement.IntervalCartesian2D)
        linear_ref.setInterval(1)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        text_format = QgsTextFormat.fromQFont(font)
        text_format.setColor(QColor(255, 255, 255))
        linear_ref.setTextFormat(text_format)

        linear_ref.setLabelOffset(QPointF(3, -1))

        s.appendSymbolLayer(linear_ref)

        layer = QgsVectorLayer("LineString", "test", "memory")
        feature = QgsFeature()
        geom = QgsGeometry.fromWkt(
            "MultiLineStringZM ((6 2 0.2 1.2, 9 2 0.7 0.2, 9 3 0.4 0, 11 5 0.8 0.4))"
        )
        feature.setGeometry(geom)
        layer.dataProvider().addFeature(feature)
        layer.setRenderer(QgsSingleSymbolRenderer(s))

        ms = QgsMapSettings()
        ms.setLayers([layer])
        ms.setDestinationCrs(layer.crs())
        extent = geom.get().boundingBox()
        extent = extent.buffered((extent.height() + extent.width()) / 20.0)
        ms.setExtent(extent)
        ms.setOutputSize(QSize(800, 800))

        self.assertTrue(
            self.render_map_settings_check(
                "labeling_engine",
                "labeling_engine",
                ms,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def test_distance_2d_with_z(self):
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2"}
        )

        linear_ref = QgsLinearReferencingSymbolLayer()
        linear_ref.setPlacement(Qgis.LinearReferencingPlacement.IntervalCartesian2D)
        linear_ref.setInterval(1)
        linear_ref.setLabelSource(Qgis.LinearReferencingLabelSource.Z)

        number_format = QgsBasicNumericFormat()
        number_format.setNumberDecimalPlaces(1)
        number_format.setShowTrailingZeros(False)
        linear_ref.setNumericFormat(number_format)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        text_format = QgsTextFormat.fromQFont(font)
        text_format.setColor(QColor(255, 255, 255))
        linear_ref.setTextFormat(text_format)

        linear_ref.setLabelOffset(QPointF(3, -1))

        s.appendSymbolLayer(linear_ref)

        rendered_image = self.renderGeometry(
            s,
            QgsGeometry.fromWkt(
                "MultiLineStringZM ((6 2 0.2 1.2, 9 2 0.7 0.2, 9 3 0.4 0, 11 5 0.8 0.4))"
            ),
        )
        self.assertTrue(
            self.image_check(
                "distance_with_z",
                "distance_with_z",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def test_distance_2d_with_m(self):
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2"}
        )

        linear_ref = QgsLinearReferencingSymbolLayer()
        linear_ref.setPlacement(Qgis.LinearReferencingPlacement.IntervalCartesian2D)
        linear_ref.setInterval(1)
        linear_ref.setLabelSource(Qgis.LinearReferencingLabelSource.M)

        number_format = QgsBasicNumericFormat()
        number_format.setNumberDecimalPlaces(1)
        number_format.setShowTrailingZeros(False)
        linear_ref.setNumericFormat(number_format)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        text_format = QgsTextFormat.fromQFont(font)
        text_format.setColor(QColor(255, 255, 255))
        linear_ref.setTextFormat(text_format)

        linear_ref.setLabelOffset(QPointF(3, -1))

        s.appendSymbolLayer(linear_ref)

        rendered_image = self.renderGeometry(
            s,
            QgsGeometry.fromWkt(
                "MultiLineStringZM ((6 2 0.2 1.2, 9 2 0.7 0.2, 9 3 0.4 0, 11 5 0.8 0.4))"
            ),
        )
        self.assertTrue(
            self.image_check(
                "distance_with_m",
                "distance_with_m",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def test_interpolate_by_z_with_distance(self):
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2"}
        )

        linear_ref = QgsLinearReferencingSymbolLayer()
        linear_ref.setPlacement(Qgis.LinearReferencingPlacement.IntervalZ)
        linear_ref.setInterval(0.3)
        linear_ref.setLabelSource(Qgis.LinearReferencingLabelSource.CartesianDistance2D)

        number_format = QgsBasicNumericFormat()
        number_format.setNumberDecimalPlaces(1)
        number_format.setShowTrailingZeros(False)
        linear_ref.setNumericFormat(number_format)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        text_format = QgsTextFormat.fromQFont(font)
        text_format.setColor(QColor(255, 255, 255))
        linear_ref.setTextFormat(text_format)

        linear_ref.setLabelOffset(QPointF(3, -1))

        s.appendSymbolLayer(linear_ref)

        rendered_image = self.renderGeometry(
            s,
            QgsGeometry.fromWkt(
                "MultiLineStringZM ((6 2 0.2 1.2, 9 2 0.7 0.2, 9 3 0.4 0, 11 5 0.8 0.4))"
            ),
        )
        self.assertTrue(
            self.image_check(
                "placement_by_z_distance",
                "placement_by_z_distance",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def test_interpolate_by_z_with_z(self):
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2"}
        )

        linear_ref = QgsLinearReferencingSymbolLayer()
        linear_ref.setPlacement(Qgis.LinearReferencingPlacement.IntervalZ)
        linear_ref.setInterval(0.3)
        linear_ref.setLabelSource(Qgis.LinearReferencingLabelSource.Z)

        number_format = QgsBasicNumericFormat()
        number_format.setNumberDecimalPlaces(1)
        number_format.setShowTrailingZeros(False)
        linear_ref.setNumericFormat(number_format)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        text_format = QgsTextFormat.fromQFont(font)
        text_format.setColor(QColor(255, 255, 255))
        linear_ref.setTextFormat(text_format)

        linear_ref.setLabelOffset(QPointF(3, -1))

        s.appendSymbolLayer(linear_ref)

        rendered_image = self.renderGeometry(
            s,
            QgsGeometry.fromWkt(
                "MultiLineStringZM ((6 2 0.2 1.2, 9 2 0.7 0.2, 9 3 0.4 0, 11 5 0.8 0.4))"
            ),
        )
        self.assertTrue(
            self.image_check(
                "placement_by_z_z",
                "placement_by_z_z",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def test_interpolate_by_z_with_m(self):
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2"}
        )

        linear_ref = QgsLinearReferencingSymbolLayer()
        linear_ref.setPlacement(Qgis.LinearReferencingPlacement.IntervalZ)
        linear_ref.setInterval(0.3)
        linear_ref.setLabelSource(Qgis.LinearReferencingLabelSource.M)

        number_format = QgsBasicNumericFormat()
        number_format.setNumberDecimalPlaces(1)
        number_format.setShowTrailingZeros(False)
        linear_ref.setNumericFormat(number_format)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        text_format = QgsTextFormat.fromQFont(font)
        text_format.setColor(QColor(255, 255, 255))
        linear_ref.setTextFormat(text_format)

        linear_ref.setLabelOffset(QPointF(3, -1))

        s.appendSymbolLayer(linear_ref)

        rendered_image = self.renderGeometry(
            s,
            QgsGeometry.fromWkt(
                "MultiLineStringZM ((6 2 0.2 1.2, 9 2 0.7 0.2, 9 3 0.4 0, 11 5 0.8 0.4))"
            ),
        )
        self.assertTrue(
            self.image_check(
                "placement_by_z_m",
                "placement_by_z_m",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def test_interpolate_by_m_with_distance(self):
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2"}
        )

        linear_ref = QgsLinearReferencingSymbolLayer()
        linear_ref.setPlacement(Qgis.LinearReferencingPlacement.IntervalM)
        linear_ref.setInterval(0.3)
        linear_ref.setLabelSource(Qgis.LinearReferencingLabelSource.CartesianDistance2D)

        number_format = QgsBasicNumericFormat()
        number_format.setNumberDecimalPlaces(1)
        number_format.setShowTrailingZeros(False)
        linear_ref.setNumericFormat(number_format)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        text_format = QgsTextFormat.fromQFont(font)
        text_format.setColor(QColor(255, 255, 255))
        linear_ref.setTextFormat(text_format)

        linear_ref.setLabelOffset(QPointF(3, -1))

        s.appendSymbolLayer(linear_ref)

        rendered_image = self.renderGeometry(
            s,
            QgsGeometry.fromWkt(
                "MultiLineStringZM ((6 2 0.2 1.2, 9 2 0.7 0.2, 9 3 0.4 0, 11 5 0.8 0.4))"
            ),
        )
        self.assertTrue(
            self.image_check(
                "placement_by_m_distance",
                "placement_by_m_distance",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def test_interpolate_by_m_with_z(self):
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2"}
        )

        linear_ref = QgsLinearReferencingSymbolLayer()
        linear_ref.setPlacement(Qgis.LinearReferencingPlacement.IntervalM)
        linear_ref.setInterval(0.3)
        linear_ref.setLabelSource(Qgis.LinearReferencingLabelSource.Z)

        number_format = QgsBasicNumericFormat()
        number_format.setNumberDecimalPlaces(1)
        number_format.setShowTrailingZeros(False)
        linear_ref.setNumericFormat(number_format)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        text_format = QgsTextFormat.fromQFont(font)
        text_format.setColor(QColor(255, 255, 255))
        linear_ref.setTextFormat(text_format)

        linear_ref.setLabelOffset(QPointF(3, -1))

        s.appendSymbolLayer(linear_ref)

        rendered_image = self.renderGeometry(
            s,
            QgsGeometry.fromWkt(
                "MultiLineStringZM ((6 2 0.2 1.2, 9 2 0.7 0.2, 9 3 0.4 0, 11 5 0.8 0.4))"
            ),
        )
        self.assertTrue(
            self.image_check(
                "placement_by_m_z",
                "placement_by_m_z",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def test_interpolate_by_m_with_m(self):
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2"}
        )

        linear_ref = QgsLinearReferencingSymbolLayer()
        linear_ref.setPlacement(Qgis.LinearReferencingPlacement.IntervalM)
        linear_ref.setInterval(0.3)
        linear_ref.setLabelSource(Qgis.LinearReferencingLabelSource.M)

        number_format = QgsBasicNumericFormat()
        number_format.setNumberDecimalPlaces(1)
        number_format.setShowTrailingZeros(False)
        linear_ref.setNumericFormat(number_format)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        text_format = QgsTextFormat.fromQFont(font)
        text_format.setColor(QColor(255, 255, 255))
        linear_ref.setTextFormat(text_format)

        linear_ref.setLabelOffset(QPointF(3, -1))

        s.appendSymbolLayer(linear_ref)

        rendered_image = self.renderGeometry(
            s,
            QgsGeometry.fromWkt(
                "MultiLineStringZM ((6 2 0.2 1.2, 9 2 0.7 0.2, 9 3 0.4 0, 11 5 0.8 0.4))"
            ),
        )
        self.assertTrue(
            self.image_check(
                "placement_by_m_m",
                "placement_by_m_m",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def test_at_vertex_with_distance(self):
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2"}
        )

        linear_ref = QgsLinearReferencingSymbolLayer()
        linear_ref.setPlacement(Qgis.LinearReferencingPlacement.Vertex)
        linear_ref.setLabelSource(Qgis.LinearReferencingLabelSource.CartesianDistance2D)

        number_format = QgsBasicNumericFormat()
        number_format.setNumberDecimalPlaces(1)
        number_format.setShowTrailingZeros(False)
        linear_ref.setNumericFormat(number_format)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        text_format = QgsTextFormat.fromQFont(font)
        text_format.setColor(QColor(255, 255, 255))
        linear_ref.setTextFormat(text_format)

        linear_ref.setLabelOffset(QPointF(3, -1))

        s.appendSymbolLayer(linear_ref)

        rendered_image = self.renderGeometry(
            s,
            QgsGeometry.fromWkt(
                "MultiLineStringZM ((6 2 0.2 1.2, 9 2 0.7 0.2, 9 3 0.4 0, 11 5 0.8 0.4))"
            ),
        )
        self.assertTrue(
            self.image_check(
                "vertex_distance",
                "vertex_distance",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def test_at_vertex_with_z(self):
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2"}
        )

        linear_ref = QgsLinearReferencingSymbolLayer()
        linear_ref.setPlacement(Qgis.LinearReferencingPlacement.Vertex)
        linear_ref.setLabelSource(Qgis.LinearReferencingLabelSource.Z)

        number_format = QgsBasicNumericFormat()
        number_format.setNumberDecimalPlaces(1)
        number_format.setShowTrailingZeros(False)
        linear_ref.setNumericFormat(number_format)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        text_format = QgsTextFormat.fromQFont(font)
        text_format.setColor(QColor(255, 255, 255))
        linear_ref.setTextFormat(text_format)

        linear_ref.setLabelOffset(QPointF(3, -1))

        s.appendSymbolLayer(linear_ref)

        rendered_image = self.renderGeometry(
            s,
            QgsGeometry.fromWkt(
                "MultiLineStringZM ((6 2 0.2 1.2, 9 2 0.7 0.2, 9 3 0.4 0, 11 5 0.8 0.4))"
            ),
        )
        self.assertTrue(
            self.image_check(
                "vertex_z",
                "vertex_z",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def test_at_vertex_with_m(self):
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2"}
        )

        linear_ref = QgsLinearReferencingSymbolLayer()
        linear_ref.setPlacement(Qgis.LinearReferencingPlacement.Vertex)
        linear_ref.setLabelSource(Qgis.LinearReferencingLabelSource.M)

        number_format = QgsBasicNumericFormat()
        number_format.setNumberDecimalPlaces(1)
        number_format.setShowTrailingZeros(False)
        linear_ref.setNumericFormat(number_format)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        text_format = QgsTextFormat.fromQFont(font)
        text_format.setColor(QColor(255, 255, 255))
        linear_ref.setTextFormat(text_format)

        linear_ref.setLabelOffset(QPointF(3, -1))

        s.appendSymbolLayer(linear_ref)

        rendered_image = self.renderGeometry(
            s,
            QgsGeometry.fromWkt(
                "MultiLineStringZM ((6 2 0.2 1.2, 9 2 0.7 0.2, 9 3 0.4 0, 11 5 0.8 0.4))"
            ),
        )
        self.assertTrue(
            self.image_check(
                "vertex_m",
                "vertex_m",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def test_distance_2d_skip_multiples(self):
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2"}
        )

        linear_ref = QgsLinearReferencingSymbolLayer()
        linear_ref.setPlacement(Qgis.LinearReferencingPlacement.IntervalCartesian2D)
        linear_ref.setInterval(1)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        text_format = QgsTextFormat.fromQFont(font)
        text_format.setColor(QColor(255, 255, 255))
        linear_ref.setTextFormat(text_format)

        linear_ref.setLabelOffset(QPointF(3, -1))
        linear_ref.setSkipMultiplesOf(2)

        s.appendSymbolLayer(linear_ref)

        rendered_image = self.renderGeometry(
            s,
            QgsGeometry.fromWkt(
                "MultiLineStringZM ((6 2 0.2 1.2, 9 2 0.7 0.2, 9 3 0.4 0, 11 5 0.8 0.4))"
            ),
        )
        self.assertTrue(
            self.image_check(
                "skip_multiples",
                "skip_multiples",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def test_distance_2d_numeric_format(self):
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2"}
        )

        linear_ref = QgsLinearReferencingSymbolLayer()
        linear_ref.setPlacement(Qgis.LinearReferencingPlacement.IntervalCartesian2D)
        linear_ref.setInterval(1)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        text_format = QgsTextFormat.fromQFont(font)
        text_format.setColor(QColor(255, 255, 255))
        linear_ref.setTextFormat(text_format)

        linear_ref.setLabelOffset(QPointF(3, -1))

        number_format = QgsBasicNumericFormat()
        number_format.setNumberDecimalPlaces(2)
        number_format.setShowTrailingZeros(True)
        linear_ref.setNumericFormat(number_format)

        s.appendSymbolLayer(linear_ref)

        rendered_image = self.renderGeometry(
            s,
            QgsGeometry.fromWkt(
                "MultiLineStringZM ((6 2 0.2 1.2, 9 2 0.7 0.2, 9 3 0.4 0, 11 5 0.8 0.4))"
            ),
        )
        self.assertTrue(
            self.image_check(
                "numeric_format",
                "numeric_format",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def test_distance_2d_no_rotate(self):
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2"}
        )

        linear_ref = QgsLinearReferencingSymbolLayer()
        linear_ref.setPlacement(Qgis.LinearReferencingPlacement.IntervalCartesian2D)
        linear_ref.setInterval(1)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        text_format = QgsTextFormat.fromQFont(font)
        text_format.setColor(QColor(255, 255, 255))
        linear_ref.setTextFormat(text_format)

        linear_ref.setLabelOffset(QPointF(3, -1))

        linear_ref.setRotateLabels(False)

        s.appendSymbolLayer(linear_ref)

        rendered_image = self.renderGeometry(
            s,
            QgsGeometry.fromWkt(
                "MultiLineStringZM ((6 2 0.2 1.2, 9 2 0.7 0.2, 9 3 0.4 0, 11 5 0.8 0.4))"
            ),
        )
        self.assertTrue(
            self.image_check(
                "no_rotate",
                "no_rotate",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def test_distance_2d_marker(self):
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2"}
        )

        linear_ref = QgsLinearReferencingSymbolLayer()
        linear_ref.setPlacement(Qgis.LinearReferencingPlacement.IntervalCartesian2D)
        linear_ref.setInterval(1)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        text_format = QgsTextFormat.fromQFont(font)
        text_format.setColor(QColor(255, 255, 255))
        linear_ref.setTextFormat(text_format)

        linear_ref.setLabelOffset(QPointF(3, -1))

        linear_ref.setShowMarker(True)
        linear_ref.setSubSymbol(
            QgsMarkerSymbol.createSimple(
                {
                    "color": "#00ff00",
                    "outline_style": "no",
                    "size": "8",
                    "name": "arrow",
                }
            )
        )

        s.appendSymbolLayer(linear_ref)

        rendered_image = self.renderGeometry(
            s,
            QgsGeometry.fromWkt(
                "MultiLineStringZM ((6 2 0.2 1.2, 9 2 0.7 0.2, 9 3 0.4 0, 11 5 0.8 0.4))"
            ),
        )
        self.assertTrue(
            self.image_check(
                "marker",
                "marker",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def test_distance_2d_marker_no_rotate(self):
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2"}
        )

        linear_ref = QgsLinearReferencingSymbolLayer()
        linear_ref.setPlacement(Qgis.LinearReferencingPlacement.IntervalCartesian2D)
        linear_ref.setInterval(1)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        text_format = QgsTextFormat.fromQFont(font)
        text_format.setColor(QColor(255, 255, 255))
        linear_ref.setTextFormat(text_format)

        linear_ref.setLabelOffset(QPointF(3, -1))

        linear_ref.setShowMarker(True)
        linear_ref.setSubSymbol(
            QgsMarkerSymbol.createSimple(
                {
                    "color": "#00ff00",
                    "outline_style": "no",
                    "size": "8",
                    "name": "arrow",
                }
            )
        )
        linear_ref.setRotateLabels(False)

        s.appendSymbolLayer(linear_ref)

        rendered_image = self.renderGeometry(
            s,
            QgsGeometry.fromWkt(
                "MultiLineStringZM ((6 2 0.2 1.2, 9 2 0.7 0.2, 9 3 0.4 0, 11 5 0.8 0.4))"
            ),
        )
        self.assertTrue(
            self.image_check(
                "marker_no_rotate",
                "marker_no_rotate",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def test_multiline(self):
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2"}
        )

        linear_ref = QgsLinearReferencingSymbolLayer()
        linear_ref.setPlacement(Qgis.LinearReferencingPlacement.IntervalCartesian2D)
        linear_ref.setInterval(1)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        text_format = QgsTextFormat.fromQFont(font)
        text_format.setColor(QColor(255, 255, 255))
        linear_ref.setTextFormat(text_format)

        linear_ref.setLabelOffset(QPointF(3, -1))

        s.appendSymbolLayer(linear_ref)

        rendered_image = self.renderGeometry(
            s,
            QgsGeometry.fromWkt(
                "MultiLineStringZM ((6 2 0.2 1.2, 9 2 0.7 0.2, 9 3 0.4 0, 11 5 0.8 0.4),"
                "(16 12 0.2 1.2, 19 12 0.7 0.2))"
            ),
        )
        self.assertTrue(
            self.image_check(
                "multiline",
                "multiline",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def test_polygon(self):
        s = QgsFillSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2"}
        )

        linear_ref = QgsLinearReferencingSymbolLayer()
        linear_ref.setPlacement(Qgis.LinearReferencingPlacement.IntervalCartesian2D)
        linear_ref.setInterval(1)

        font = QgsFontUtils.getStandardTestFont("Bold", 18)
        text_format = QgsTextFormat.fromQFont(font)
        text_format.setColor(QColor(255, 255, 255))
        linear_ref.setTextFormat(text_format)

        linear_ref.setLabelOffset(QPointF(3, -1))

        s.appendSymbolLayer(linear_ref)

        rendered_image = self.renderGeometry(
            s,
            QgsGeometry.fromWkt(
                "Polygon ((6 1, 10 1, 10 -3, 6 -3, 6 1),(7 0, 7 -2, 9 -2, 9 0, 7 0))"
            ),
        )
        self.assertTrue(
            self.image_check(
                "polygon",
                "polygon",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def renderGeometry(self, symbol, geom):
        f = QgsFeature()
        f.setGeometry(geom)

        image = QImage(800, 800, QImage.Format.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        extent = geom.get().boundingBox()
        # buffer extent by 10%
        if extent.width() > 0:
            extent = extent.buffered((extent.height() + extent.width()) / 20.0)
        else:
            extent = extent.buffered(10)

        ms.setExtent(extent)
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI

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
