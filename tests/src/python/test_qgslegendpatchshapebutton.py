"""QGIS Unit tests for QgsLegendPatchShapeButton.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2020 by Nyall Dawson"
__date__ = "20/04/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import QgsGeometry, QgsLegendPatchShape, QgsSymbol
from qgis.gui import QgsLegendPatchShapeButton
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLegendPatchShapeButton(QgisTestCase):

    def testWidget(self):
        widget = QgsLegendPatchShapeButton(dialogTitle="title")
        self.assertTrue(widget.shape().isNull())

        self.assertEqual(widget.dialogTitle(), "title")
        widget.setDialogTitle("title2")
        self.assertEqual(widget.dialogTitle(), "title2")

        widget.setSymbolType(QgsSymbol.SymbolType.Fill)
        self.assertEqual(widget.symbolType(), QgsSymbol.SymbolType.Fill)
        self.assertTrue(widget.shape().isNull())

        shape = QgsLegendPatchShape(
            QgsSymbol.SymbolType.Fill,
            QgsGeometry.fromWkt("Polygon((5 5, 1 2, 3 4, 5 5))"),
            False,
        )
        widget.setShape(shape)
        self.assertEqual(
            widget.shape().geometry().asWkt(), "Polygon ((5 5, 1 2, 3 4, 5 5))"
        )
        self.assertFalse(widget.shape().preserveAspectRatio())
        self.assertEqual(widget.shape().symbolType(), QgsSymbol.SymbolType.Fill)

        # try to set incompatible shape
        shape = QgsLegendPatchShape(
            QgsSymbol.SymbolType.Line,
            QgsGeometry.fromWkt("LineString( 0 0, 1 1)"),
            True,
        )
        widget.setShape(shape)
        # should be back to default
        self.assertTrue(widget.shape().isNull())

        # change type
        widget.setSymbolType(QgsSymbol.SymbolType.Line)
        self.assertEqual(widget.symbolType(), QgsSymbol.SymbolType.Line)
        shape = QgsLegendPatchShape(
            QgsSymbol.SymbolType.Line,
            QgsGeometry.fromWkt("LineString( 0 0, 1 1)"),
            True,
        )
        widget.setShape(shape)
        self.assertEqual(widget.shape().geometry().asWkt(), "LineString (0 0, 1 1)")

        widget.setToDefault()
        self.assertTrue(widget.shape().isNull())

    def testSignals(self):
        shape = QgsLegendPatchShape(
            QgsSymbol.SymbolType.Fill,
            QgsGeometry.fromWkt("Polygon((5 5, 1 2, 3 4, 5 5))"),
            False,
        )

        widget = QgsLegendPatchShapeButton()
        spy = QSignalSpy(widget.changed)
        widget.setSymbolType(QgsSymbol.SymbolType.Fill)
        self.assertEqual(len(spy), 0)
        widget.setShape(shape)
        self.assertEqual(len(spy), 1)

        shape = QgsLegendPatchShape(
            QgsSymbol.SymbolType.Line,
            QgsGeometry.fromWkt("LineString( 0 0, 1 1)"),
            True,
        )
        widget.setShape(shape)
        self.assertEqual(len(spy), 2)
        self.assertTrue(widget.shape().isNull())

        widget.setSymbolType(QgsSymbol.SymbolType.Line)
        self.assertEqual(len(spy), 3)
        shape = QgsLegendPatchShape(
            QgsSymbol.SymbolType.Line,
            QgsGeometry.fromWkt("LineString( 0 0, 1 2)"),
            True,
        )
        widget.setShape(shape)
        self.assertEqual(len(spy), 4)
        self.assertEqual(widget.shape().geometry().asWkt(), "LineString (0 0, 1 2)")

        widget.setToDefault()
        self.assertEqual(len(spy), 5)


if __name__ == "__main__":
    unittest.main()
