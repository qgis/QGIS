"""QGIS Unit tests for QgsGeometryWidget.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "15/02/2023"
__copyright__ = "Copyright 2023, The QGIS Project"

from qgis.PyQt.QtGui import QColor
from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import (
    QgsWkbTypes,
    QgsGeometry,
    QgsReferencedGeometry,
    QgsCoordinateReferenceSystem,
)
from qgis.gui import QgsGeometryWidget
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsGeometryWidget(QgisTestCase):

    def testGeometryValue(self):
        widget = QgsGeometryWidget()

        spy = QSignalSpy(widget.geometryValueChanged)
        self.assertTrue(widget.geometryValue().isNull())

        widget.setGeometryValue(
            QgsReferencedGeometry(
                QgsGeometry.fromWkt("Point( 1 2)"),
                QgsCoordinateReferenceSystem("EPSG:3111"),
            )
        )
        self.assertEqual(len(spy), 1)
        self.assertEqual(
            spy[-1][0],
            QgsReferencedGeometry(
                QgsGeometry.fromWkt("Point( 1 2)"),
                QgsCoordinateReferenceSystem("EPSG:3111"),
            ),
        )

        self.assertEqual(widget.geometryValue().asWkt(), "Point (1 2)")
        self.assertEqual(widget.geometryValue().crs().authid(), "EPSG:3111")

        # same geometry, should be no signal
        widget.setGeometryValue(
            QgsReferencedGeometry(
                QgsGeometry.fromWkt("Point( 1 2)"),
                QgsCoordinateReferenceSystem("EPSG:3111"),
            )
        )
        self.assertEqual(len(spy), 1)

        # different geometry
        widget.setGeometryValue(
            QgsReferencedGeometry(
                QgsGeometry.fromWkt("Point(1 3)"),
                QgsCoordinateReferenceSystem("EPSG:3111"),
            )
        )
        self.assertEqual(len(spy), 2)
        self.assertEqual(
            spy[-1][0],
            QgsReferencedGeometry(
                QgsGeometry.fromWkt("Point(1 3)"),
                QgsCoordinateReferenceSystem("EPSG:3111"),
            ),
        )
        self.assertEqual(
            widget.geometryValue(),
            QgsReferencedGeometry(
                QgsGeometry.fromWkt("Point(1 3)"),
                QgsCoordinateReferenceSystem("EPSG:3111"),
            ),
        )

        # clear
        widget.clearGeometry()
        self.assertEqual(len(spy), 3)
        self.assertEqual(
            spy[-1][0],
            QgsReferencedGeometry(QgsGeometry(), QgsCoordinateReferenceSystem()),
        )
        self.assertEqual(
            widget.geometryValue(),
            QgsReferencedGeometry(QgsGeometry(), QgsCoordinateReferenceSystem()),
        )
        widget.clearGeometry()
        self.assertEqual(len(spy), 3)

    def test_acceptable_types(self):
        w = QgsGeometryWidget()
        self.assertFalse(w.acceptedWkbTypes())

        w.setAcceptedWkbTypes([QgsWkbTypes.Type.PointZ, QgsWkbTypes.Type.PointM])
        self.assertEqual(
            w.acceptedWkbTypes(), [QgsWkbTypes.Type.PointZ, QgsWkbTypes.Type.PointM]
        )


if __name__ == "__main__":
    unittest.main()
