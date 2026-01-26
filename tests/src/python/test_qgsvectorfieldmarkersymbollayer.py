"""
***************************************************************************
    test_qgsvectorfieldmarkersymbollayer.py
    ---------------------
    Date                 : January 2021
    Copyright            : (C) 2021 by Nyall Dawson
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
__date__ = "November 2021"
__copyright__ = "(C) 2021, Nyall Dawson"

from qgis.PyQt.QtCore import QVariant
from qgis.PyQt.QtGui import QColor, QImage, QPainter
from qgis.core import (
    QgsFeature,
    QgsField,
    QgsFields,
    QgsGeometry,
    QgsLineSymbol,
    QgsMapSettings,
    QgsMarkerSymbol,
    QgsRenderContext,
    QgsVectorFieldSymbolLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsVectorFieldMarkerSymbolLayer(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "symbol_vectorfield"

    def testRender(self):
        # test rendering
        s = QgsMarkerSymbol()
        s.deleteSymbolLayer(0)

        field_marker = QgsVectorFieldSymbolLayer()
        field_marker.setXAttribute("x")
        field_marker.setYAttribute("y")
        field_marker.setScale(4)

        field_marker.setSubSymbol(
            QgsLineSymbol.createSimple({"color": "#ff0000", "width": "2"})
        )

        s.appendSymbolLayer(field_marker.clone())

        g = QgsGeometry.fromWkt("Point(5 4)")
        fields = QgsFields()
        fields.append(QgsField("x", QVariant.Double))
        fields.append(QgsField("y", QVariant.Double))
        f = QgsFeature(fields)
        f.setAttributes([2, 3])
        f.setGeometry(g)

        rendered_image = self.renderFeature(s, f)
        self.assertTrue(
            self.image_check(
                "vectorfield",
                "vectorfield",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testMapRotation(self):
        # test rendering with map rotation
        s = QgsMarkerSymbol()
        s.deleteSymbolLayer(0)

        field_marker = QgsVectorFieldSymbolLayer()
        field_marker.setXAttribute("x")
        field_marker.setYAttribute("y")
        field_marker.setScale(4)

        field_marker.setSubSymbol(
            QgsLineSymbol.createSimple({"color": "#ff0000", "width": "2"})
        )

        s.appendSymbolLayer(field_marker.clone())

        g = QgsGeometry.fromWkt("Point(5 4)")
        fields = QgsFields()
        fields.append(QgsField("x", QVariant.Double))
        fields.append(QgsField("y", QVariant.Double))
        f = QgsFeature(fields)
        f.setAttributes([2, 3])
        f.setGeometry(g)

        rendered_image = self.renderFeature(s, f, map_rotation=45)
        self.assertTrue(
            self.image_check(
                "rotated_map",
                "rotated_map",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testHeight(self):
        # test rendering
        s = QgsMarkerSymbol()
        s.deleteSymbolLayer(0)

        field_marker = QgsVectorFieldSymbolLayer()
        field_marker.setXAttribute("x")
        field_marker.setYAttribute("y")
        field_marker.setScale(4)
        field_marker.setVectorFieldType(
            QgsVectorFieldSymbolLayer.VectorFieldType.Height
        )

        field_marker.setSubSymbol(
            QgsLineSymbol.createSimple({"color": "#ff0000", "width": "2"})
        )

        s.appendSymbolLayer(field_marker.clone())

        g = QgsGeometry.fromWkt("Point(5 4)")
        fields = QgsFields()
        fields.append(QgsField("x", QVariant.Double))
        fields.append(QgsField("y", QVariant.Double))
        f = QgsFeature(fields)
        f.setAttributes([2, 3])
        f.setGeometry(g)

        rendered_image = self.renderFeature(s, f)
        self.assertTrue(
            self.image_check(
                "height",
                "height",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testPolar(self):
        # test rendering
        s = QgsMarkerSymbol()
        s.deleteSymbolLayer(0)

        field_marker = QgsVectorFieldSymbolLayer()
        field_marker.setXAttribute("x")
        field_marker.setYAttribute("y")
        field_marker.setVectorFieldType(QgsVectorFieldSymbolLayer.VectorFieldType.Polar)
        field_marker.setScale(1)

        field_marker.setSubSymbol(
            QgsLineSymbol.createSimple({"color": "#ff0000", "width": "2"})
        )

        s.appendSymbolLayer(field_marker.clone())

        g = QgsGeometry.fromWkt("Point(5 4)")
        fields = QgsFields()
        fields.append(QgsField("x", QVariant.Double))
        fields.append(QgsField("y", QVariant.Double))
        f = QgsFeature(fields)
        f.setAttributes([6, 135])
        f.setGeometry(g)

        rendered_image = self.renderFeature(s, f)
        self.assertTrue(
            self.image_check(
                "polar", "polar", rendered_image, color_tolerance=2, allowed_mismatch=20
            )
        )

    def testPolarAnticlockwise(self):
        # test rendering
        s = QgsMarkerSymbol()
        s.deleteSymbolLayer(0)

        field_marker = QgsVectorFieldSymbolLayer()
        field_marker.setXAttribute("x")
        field_marker.setYAttribute("y")
        field_marker.setVectorFieldType(QgsVectorFieldSymbolLayer.VectorFieldType.Polar)
        field_marker.setAngleOrientation(
            QgsVectorFieldSymbolLayer.AngleOrientation.CounterclockwiseFromEast
        )
        field_marker.setScale(1)

        field_marker.setSubSymbol(
            QgsLineSymbol.createSimple({"color": "#ff0000", "width": "2"})
        )

        s.appendSymbolLayer(field_marker.clone())

        g = QgsGeometry.fromWkt("Point(5 4)")
        fields = QgsFields()
        fields.append(QgsField("x", QVariant.Double))
        fields.append(QgsField("y", QVariant.Double))
        f = QgsFeature(fields)
        f.setAttributes([6, 135])
        f.setGeometry(g)

        rendered_image = self.renderFeature(s, f)
        self.assertTrue(
            self.image_check(
                "anticlockwise_polar",
                "anticlockwise_polar",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testPolarRadians(self):
        # test rendering
        s = QgsMarkerSymbol()
        s.deleteSymbolLayer(0)

        field_marker = QgsVectorFieldSymbolLayer()
        field_marker.setXAttribute("x")
        field_marker.setYAttribute("y")
        field_marker.setVectorFieldType(QgsVectorFieldSymbolLayer.VectorFieldType.Polar)
        field_marker.setScale(1)
        field_marker.setAngleUnits(QgsVectorFieldSymbolLayer.AngleUnits.Radians)

        field_marker.setSubSymbol(
            QgsLineSymbol.createSimple({"color": "#ff0000", "width": "2"})
        )

        s.appendSymbolLayer(field_marker.clone())

        g = QgsGeometry.fromWkt("Point(5 4)")
        fields = QgsFields()
        fields.append(QgsField("x", QVariant.Double))
        fields.append(QgsField("y", QVariant.Double))
        f = QgsFeature(fields)
        f.setAttributes([6, 135])
        f.setGeometry(g)

        rendered_image = self.renderFeature(s, f)
        self.assertTrue(
            self.image_check(
                "radians_polar",
                "radians_polar",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def renderFeature(self, symbol, f, buffer=20, map_rotation=0):
        image = QImage(200, 200, QImage.Format.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        extent = f.geometry().constGet().boundingBox()
        # buffer extent by 10%
        if extent.width() > 0:
            extent = extent.buffered((extent.height() + extent.width()) / buffer)
        else:
            extent = extent.buffered(buffer / 2)

        ms.setExtent(extent)
        ms.setOutputSize(image.size())
        ms.setRotation(map_rotation)
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.expressionContext().setFeature(f)

        painter.begin(image)
        try:
            image.fill(QColor(0, 0, 0))
            symbol.startRender(context, f.fields())
            symbol.renderFeature(f, context)
            symbol.stopRender(context)
        finally:
            painter.end()

        return image


if __name__ == "__main__":
    unittest.main()
