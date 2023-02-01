"""QGIS Unit tests for QgsLegendPatchShapeWidget.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2020 by Nyall Dawson'
__date__ = '20/04/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

from qgis.core import (QgsLegendPatchShape,
                       QgsGeometry,
                       QgsSymbol
                       )
from qgis.gui import QgsLegendPatchShapeWidget
from qgis.PyQt.QtTest import QSignalSpy

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath


start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLegendPatchShapeWidget(unittest.TestCase):

    def testWidget(self):
        shape = QgsLegendPatchShape(QgsSymbol.Line, QgsGeometry.fromWkt('LineString( 0 0, 1 1)'), False)

        widget = QgsLegendPatchShapeWidget(None, shape)
        self.assertEqual(widget.shape().geometry().asWkt(), 'LineString (0 0, 1 1)')
        self.assertFalse(widget.shape().preserveAspectRatio())
        self.assertEqual(widget.shape().symbolType(), QgsSymbol.Line)

        shape = QgsLegendPatchShape(QgsSymbol.Line, QgsGeometry.fromWkt('LineString( 0 0, 1 1)'), True)
        widget = QgsLegendPatchShapeWidget(None, shape)
        self.assertEqual(widget.shape().geometry().asWkt(), 'LineString (0 0, 1 1)')
        self.assertTrue(widget.shape().preserveAspectRatio())
        self.assertEqual(widget.shape().symbolType(), QgsSymbol.Line)

        shape = QgsLegendPatchShape(QgsSymbol.Fill, QgsGeometry.fromWkt('Polygon((5 5, 1 2, 3 4, 5 5))'), False)
        widget = QgsLegendPatchShapeWidget(None, shape)
        self.assertEqual(widget.shape().geometry().asWkt(), 'Polygon ((5 5, 1 2, 3 4, 5 5))')
        self.assertFalse(widget.shape().preserveAspectRatio())
        self.assertEqual(widget.shape().symbolType(), QgsSymbol.Fill)

        shape = QgsLegendPatchShape(QgsSymbol.Marker, QgsGeometry.fromWkt('MultiPoint((5 5), (1 2))'))
        widget = QgsLegendPatchShapeWidget(None, shape)
        self.assertEqual(widget.shape().geometry().asWkt(), 'MultiPoint ((5 5),(1 2))')
        self.assertTrue(widget.shape().preserveAspectRatio())
        self.assertEqual(widget.shape().symbolType(), QgsSymbol.Marker)

    def testSignals(self):
        shape = QgsLegendPatchShape(QgsSymbol.Line, QgsGeometry.fromWkt('LineString( 0 0, 1 1)'), False)

        widget = QgsLegendPatchShapeWidget(None, shape)
        spy = QSignalSpy(widget.changed)
        widget.setShape(shape)
        self.assertEqual(len(spy), 0)
        self.assertFalse(widget.shape().preserveAspectRatio())

        shape = QgsLegendPatchShape(QgsSymbol.Line, QgsGeometry.fromWkt('LineString( 0 0, 1 1)'), True)
        widget.setShape(shape)
        self.assertEqual(len(spy), 1)
        self.assertTrue(widget.shape().preserveAspectRatio())
        self.assertEqual(widget.shape().geometry().asWkt(), 'LineString (0 0, 1 1)')
        self.assertEqual(widget.shape().symbolType(), QgsSymbol.Line)

        shape = QgsLegendPatchShape(QgsSymbol.Line, QgsGeometry.fromWkt('LineString( 0 0, 1 2)'), True)
        widget.setShape(shape)
        self.assertEqual(len(spy), 2)
        self.assertTrue(widget.shape().preserveAspectRatio())
        self.assertEqual(widget.shape().geometry().asWkt(), 'LineString (0 0, 1 2)')
        self.assertEqual(widget.shape().symbolType(), QgsSymbol.Line)

        shape = QgsLegendPatchShape(QgsSymbol.Marker, QgsGeometry.fromWkt('MultiPoint((5 5), (1 2))'), True)
        widget.setShape(shape)
        self.assertEqual(len(spy), 3)
        self.assertTrue(widget.shape().preserveAspectRatio())
        self.assertEqual(widget.shape().geometry().asWkt(), 'MultiPoint ((5 5),(1 2))')
        self.assertEqual(widget.shape().symbolType(), QgsSymbol.Marker)


if __name__ == '__main__':
    unittest.main()
