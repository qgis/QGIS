"""QGIS Unit tests for QgsLayoutItemShape.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2017 by Nyall Dawson"
__date__ = "23/10/2017"
__copyright__ = "Copyright 2017, The QGIS Project"

from qgis.PyQt.QtCore import QRectF
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    QgsFillSymbol,
    QgsLayout,
    QgsLayoutItem,
    QgsLayoutItemShape,
    QgsLayoutMeasurement,
    QgsProject,
    QgsReadWriteContext,
    QgsUnitTypes,
    QgsLayoutItemMap,
    Qgis,
    QgsGeometryGeneratorSymbolLayer,
    QgsRectangle,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from test_qgslayoutitem import LayoutItemTestCase

start_app()


class TestQgsLayoutShape(QgisTestCase, LayoutItemTestCase):

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.item_class = QgsLayoutItemShape

    @classmethod
    def control_path_prefix(cls):
        return "layout_shape"

    def testClipPath(self):
        pr = QgsProject()
        l = QgsLayout(pr)
        shape = QgsLayoutItemShape(l)

        shape.setShapeType(QgsLayoutItemShape.Shape.Rectangle)
        shape.attemptSetSceneRect(QRectF(30, 10, 100, 200))

        # must be a closed polygon, in scene coordinates!
        self.assertEqual(
            shape.clipPath().asWkt(),
            "Polygon ((30 10, 130 10, 130 210, 30 210, 30 10))",
        )
        self.assertTrue(
            int(shape.itemFlags() & QgsLayoutItem.Flag.FlagProvidesClipPath)
        )

        spy = QSignalSpy(shape.clipPathChanged)
        shape.setCornerRadius(
            QgsLayoutMeasurement(10, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        self.assertTrue(
            shape.clipPath()
            .asWkt(0)
            .startswith("Polygon ((30 20, 30 20, 30 19, 30 19, 30 19, 30 19")
        )
        self.assertEqual(len(spy), 1)

        shape.setShapeType(QgsLayoutItemShape.Shape.Ellipse)
        self.assertEqual(len(spy), 2)
        self.assertTrue(
            shape.clipPath()
            .asWkt(0)
            .startswith("Polygon ((130 110, 130 111, 130 113, 130 114")
        )

        shape.setShapeType(QgsLayoutItemShape.Shape.Triangle)
        self.assertEqual(len(spy), 3)
        self.assertEqual(
            shape.clipPath().asWkt(), "Polygon ((30 210, 130 210, 80 10, 30 210))"
        )

        shape.attemptSetSceneRect(QRectF(50, 20, 80, 120))
        self.assertEqual(len(spy), 4)
        self.assertEqual(
            shape.clipPath().asWkt(), "Polygon ((50 140, 130 140, 90 20, 50 140))"
        )

    def testBoundingRectForStrokeSizeOnRestore(self):
        """
        Test that item bounding rect correctly accounts for stroke size on item restore
        """
        pr = QgsProject()
        l = QgsLayout(pr)
        shape = QgsLayoutItemShape(l)

        shape.setShapeType(QgsLayoutItemShape.Shape.Rectangle)
        shape.attemptSetSceneRect(QRectF(30, 10, 100, 200))
        self.assertEqual(shape.boundingRect(), QRectF(-0.15, -0.15, 100.3, 200.3))

        # set a symbol with very wide stroke
        s = QgsFillSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "40", "color": "#ff5588"}
        )
        shape.setSymbol(s)
        # bounding rect for item should include stroke
        self.assertEqual(shape.boundingRect(), QRectF(-20.0, -20.0, 140.0, 240.0))

        # save the shape and restore
        doc = QDomDocument("testdoc")
        parent_elem = doc.createElement("test")
        doc.appendChild(parent_elem)
        self.assertTrue(shape.writeXml(parent_elem, doc, QgsReadWriteContext()))

        item_elem = parent_elem.firstChildElement("LayoutItem")
        self.assertFalse(item_elem.isNull())

        # restore
        shape2 = QgsLayoutItemShape(l)

        self.assertTrue(shape2.readXml(item_elem, doc, QgsReadWriteContext()))

        # bounding rect for item should include stroke
        self.assertEqual(shape2.boundingRect(), QRectF(-20.0, -20.0, 140.0, 240.0))

    def test_generator(self):
        project = QgsProject()
        layout = QgsLayout(project)
        layout.initializeDefaults()

        shape = QgsLayoutItemShape(layout)
        shape.setShapeType(QgsLayoutItemShape.Shape.Rectangle)
        shape.attemptSetSceneRect(QRectF(0, 0, 100, 200))
        layout.addLayoutItem(shape)

        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(0, 0, 10, 10))
        map.zoomToExtent(QgsRectangle(1, 1, 2, 2))
        layout.addLayoutItem(map)

        props = {}
        props["color"] = "green"
        props["style"] = "solid"
        props["style_border"] = "solid"
        props["color_border"] = "red"
        props["width_border"] = "6.0"
        props["joinstyle"] = "miter"

        sub_symbol = QgsFillSymbol.createSimple(props)

        line_symbol = QgsFillSymbol()
        generator = QgsGeometryGeneratorSymbolLayer.create(
            {
                "geometryModifier": "geom_from_wkt('POLYGON((10 10,287 10,287 200,10 200,10 10))')",
                "SymbolType": "Fill",
            }
        )
        generator.setUnits(Qgis.RenderUnit.Millimeters)
        generator.setSubSymbol(sub_symbol)

        line_symbol.changeSymbolLayer(0, generator)
        shape.setSymbol(line_symbol)

        self.assertTrue(self.render_layout_check("layoutshape_generator", layout))


if __name__ == "__main__":
    unittest.main()
