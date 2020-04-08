# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLegendPatchShape.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2020 by Nyall Dawson'
__date__ = '05/04/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

from qgis.PyQt.QtCore import (QSizeF,
                              QPointF)
from qgis.PyQt.QtGui import QPolygonF
from qgis.core import (QgsLegendPatchShape,
                       QgsGeometry,
                       QgsSymbol
                       )
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath


start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLegendPatchShape(unittest.TestCase):

    def testBasic(self):
        shape = QgsLegendPatchShape(QgsSymbol.Line, QgsGeometry.fromWkt('LineString( 0 0, 1 1)'), False)
        self.assertFalse(shape.isNull())
        self.assertEqual(shape.symbolType(), QgsSymbol.Line)
        self.assertEqual(shape.geometry().asWkt(), 'LineString (0 0, 1 1)')
        self.assertFalse(shape.preserveAspectRatio())

        shape.setSymbolType(QgsSymbol.Marker)
        self.assertEqual(shape.symbolType(), QgsSymbol.Marker)

        shape.setGeometry(QgsGeometry.fromWkt('Multipoint( 1 1, 2 2)'))
        self.assertEqual(shape.geometry().asWkt(), 'MultiPoint ((1 1),(2 2))')

        shape.setPreserveAspectRatio(True)
        self.assertTrue(shape.preserveAspectRatio())

    @staticmethod
    def polys_to_list(polys):
        return [[[p.x(), p.y()] for p in poly] for poly in polys]

    def testNull(self):
        shape = QgsLegendPatchShape()
        self.assertTrue(shape.isNull())
        shape.setGeometry(QgsGeometry.fromWkt('Multipoint( 1 1, 2 2)'))
        self.assertFalse(shape.isNull())
        shape.setGeometry(QgsGeometry())
        self.assertTrue(shape.isNull())

    def testDefault(self):
        self.assertEqual(QgsLegendPatchShape.defaultPatch(QgsSymbol.Hybrid, QSizeF(1, 1)), [])
        self.assertEqual(QgsLegendPatchShape.defaultPatch(QgsSymbol.Hybrid, QSizeF(10, 10)), [])

        # markers
        self.assertEqual(self.polys_to_list(QgsLegendPatchShape.defaultPatch(QgsSymbol.Marker, QSizeF(1, 1))), [[[0.5, 0.5]]])
        self.assertEqual(self.polys_to_list(QgsLegendPatchShape.defaultPatch(QgsSymbol.Marker, QSizeF(10, 2))), [[[5.0, 1.0]]])

        # lines
        self.assertEqual(self.polys_to_list(QgsLegendPatchShape.defaultPatch(QgsSymbol.Line, QSizeF(1, 1))), [[[0.0, 0.5], [1.0, 0.5]]])
        self.assertEqual(self.polys_to_list(QgsLegendPatchShape.defaultPatch(QgsSymbol.Line, QSizeF(10, 2))), [[[0.0, 1.5], [10.0, 1.5]]])
        self.assertEqual(self.polys_to_list(QgsLegendPatchShape.defaultPatch(QgsSymbol.Line, QSizeF(9, 3))), [[[0.0, 1.5], [9.0, 1.5]]])

        # fills
        self.assertEqual(self.polys_to_list(QgsLegendPatchShape.defaultPatch(QgsSymbol.Fill, QSizeF(1, 1))), [[[0.0, 0.0], [1.0, 0.0], [1.0, 1.0], [0.0, 1.0], [0.0, 0.0]]])
        self.assertEqual(self.polys_to_list(QgsLegendPatchShape.defaultPatch(QgsSymbol.Fill, QSizeF(10, 2))), [[[0.0, 0.0], [10.0, 0.0], [10.0, 2.0], [0.0, 2.0], [0.0, 0.0]]])


if __name__ == '__main__':
    unittest.main()
