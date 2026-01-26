"""QGIS Unit tests for QgsCrsDefinitionWidget.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "12/12/2021"
__copyright__ = "Copyright 2021, The QGIS Project"

from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import QgsCoordinateReferenceSystem
from qgis.gui import QgsCrsDefinitionWidget
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsCrsDefinitionWidget(QgisTestCase):

    def testWidget(self):
        """
        Test widget logic
        """
        w = QgsCrsDefinitionWidget()

        self.assertFalse(w.crs().isValid())
        self.assertEqual(w.format(), QgsCoordinateReferenceSystem.Format.FormatWkt)

        spy = QSignalSpy(w.crsChanged)
        c = QgsCoordinateReferenceSystem("EPSG:3111")

        w.setCrs(c)
        self.assertEqual(w.crs(), c)
        self.assertEqual(len(spy), 1)
        self.assertEqual(w.format(), QgsCoordinateReferenceSystem.Format.FormatWkt)
        self.assertEqual(
            w.definitionString(),
            c.toWkt(QgsCoordinateReferenceSystem.WktVariant.WKT_PREFERRED),
        )

        # native proj string definition
        w.setCrs(c, QgsCoordinateReferenceSystem.Format.FormatProj)
        self.assertEqual(w.crs(), c)
        self.assertEqual(len(spy), 2)
        self.assertEqual(w.format(), QgsCoordinateReferenceSystem.Format.FormatProj)
        self.assertEqual(w.definitionString(), c.toProj())

        # native WKT string definition
        w.setCrs(c, QgsCoordinateReferenceSystem.Format.FormatWkt)
        self.assertEqual(w.crs(), c)
        self.assertEqual(len(spy), 3)
        self.assertEqual(w.format(), QgsCoordinateReferenceSystem.Format.FormatWkt)
        self.assertEqual(
            w.definitionString(),
            c.toWkt(QgsCoordinateReferenceSystem.WktVariant.WKT_PREFERRED),
        )

        # change format
        w.setFormat(QgsCoordinateReferenceSystem.Format.FormatProj)
        self.assertEqual(len(spy), 4)
        self.assertEqual(w.format(), QgsCoordinateReferenceSystem.Format.FormatProj)
        self.assertEqual(w.definitionString(), c.toProj())

        w.setFormat(QgsCoordinateReferenceSystem.Format.FormatWkt)
        # trip through proj string is lossy -- don't compare to previous wkt!
        self.assertEqual(len(spy), 5)
        self.assertEqual(w.format(), QgsCoordinateReferenceSystem.Format.FormatWkt)

    def test_definition_string(self):
        """
        Test definition string logic
        """
        w = QgsCrsDefinitionWidget()

        w.setFormat(QgsCoordinateReferenceSystem.Format.FormatWkt)
        c = QgsCoordinateReferenceSystem("EPSG:3111")
        spy = QSignalSpy(w.crsChanged)

        w.setDefinitionString(
            c.toWkt(QgsCoordinateReferenceSystem.WktVariant.WKT_PREFERRED)
        )
        self.assertEqual(w.crs(), c)
        self.assertEqual(len(spy), 1)
        self.assertEqual(w.format(), QgsCoordinateReferenceSystem.Format.FormatWkt)
        self.assertEqual(
            w.definitionString(),
            c.toWkt(QgsCoordinateReferenceSystem.WktVariant.WKT_PREFERRED),
        )

        c2 = QgsCoordinateReferenceSystem("EPSG:3113")
        w.setDefinitionString(
            c2.toWkt(QgsCoordinateReferenceSystem.WktVariant.WKT_PREFERRED)
        )
        self.assertEqual(w.crs(), c2)
        self.assertEqual(len(spy), 2)
        self.assertEqual(w.format(), QgsCoordinateReferenceSystem.Format.FormatWkt)
        self.assertEqual(
            w.definitionString(),
            c2.toWkt(QgsCoordinateReferenceSystem.WktVariant.WKT_PREFERRED),
        )


if __name__ == "__main__":
    unittest.main()
