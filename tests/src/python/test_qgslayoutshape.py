"""QGIS Unit tests for QgsLayoutItemShape.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2017 by Nyall Dawson'
__date__ = '23/10/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import qgis  # NOQA
from qgis.PyQt.QtCore import QRectF
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    QgsLayoutItemShape,
    QgsProject,
    QgsLayout,
    QgsLayoutItem,
    QgsLayoutMeasurement,
    QgsUnitTypes,
    QgsFillSymbol,
    QgsReadWriteContext
)
from qgis.testing import start_app, unittest

from test_qgslayoutitem import LayoutItemTestCase

start_app()


class TestQgsLayoutShape(unittest.TestCase, LayoutItemTestCase):

    @classmethod
    def setUpClass(cls):
        cls.item_class = QgsLayoutItemShape

    def testClipPath(self):
        pr = QgsProject()
        l = QgsLayout(pr)
        shape = QgsLayoutItemShape(l)

        shape.setShapeType(QgsLayoutItemShape.Rectangle)
        shape.attemptSetSceneRect(QRectF(30, 10, 100, 200))

        # must be a closed polygon, in scene coordinates!
        self.assertEqual(shape.clipPath().asWkt(), 'Polygon ((30 10, 130 10, 130 210, 30 210, 30 10))')
        self.assertTrue(int(shape.itemFlags() & QgsLayoutItem.FlagProvidesClipPath))

        spy = QSignalSpy(shape.clipPathChanged)
        shape.setCornerRadius(QgsLayoutMeasurement(10, QgsUnitTypes.LayoutMillimeters))
        self.assertTrue(shape.clipPath().asWkt(0).startswith('Polygon ((30 20, 30 20, 30 19, 30 19, 30 19, 30 19'))
        self.assertEqual(len(spy), 1)

        shape.setShapeType(QgsLayoutItemShape.Ellipse)
        self.assertEqual(len(spy), 2)
        self.assertTrue(shape.clipPath().asWkt(0).startswith('Polygon ((130 110, 130 111, 130 113, 130 114'))

        shape.setShapeType(QgsLayoutItemShape.Triangle)
        self.assertEqual(len(spy), 3)
        self.assertEqual(shape.clipPath().asWkt(), 'Polygon ((30 210, 130 210, 80 10, 30 210))')

        shape.attemptSetSceneRect(QRectF(50, 20, 80, 120))
        self.assertEqual(len(spy), 4)
        self.assertEqual(shape.clipPath().asWkt(), 'Polygon ((50 140, 130 140, 90 20, 50 140))')

    def testBoundingRectForStrokeSizeOnRestore(self):
        """
        Test that item bounding rect correctly accounts for stroke size on item restore
        """
        pr = QgsProject()
        l = QgsLayout(pr)
        shape = QgsLayoutItemShape(l)

        shape.setShapeType(QgsLayoutItemShape.Rectangle)
        shape.attemptSetSceneRect(QRectF(30, 10, 100, 200))
        self.assertEqual(shape.boundingRect(), QRectF(-0.15, -0.15, 100.3, 200.3))

        # set a symbol with very wide stroke
        s = QgsFillSymbol.createSimple({'outline_color': '#ff0000', 'outline_width': '40', 'color': '#ff5588'})
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


if __name__ == '__main__':
    unittest.main()
