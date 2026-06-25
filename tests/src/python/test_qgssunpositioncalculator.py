"""QGIS Unit tests for QgsSunPositionCalculator.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import unittest

from qgis.core import (
    QgsCoordinateReferenceSystem,
    QgsCoordinateTransformContext,
    QgsInvalidArgumentException,
    QgsPointXY,
    QgsSunPositionCalculator,
)
from qgis.PyQt.QtCore import QDate, QDateTime, Qt, QTime
from qgis.testing import QgisTestCase


class TestQgsSunPositionCalculator(QgisTestCase):
    def test_basic_calculation(self):
        point = QgsPointXY(0.0, 51.48)
        dt = QDateTime(QDate(2026, 4, 22), QTime(12, 0, 0), Qt.TimeSpec.UTC)

        result = QgsSunPositionCalculator.calculate(
            point,
            QgsCoordinateReferenceSystem("EPSG:4326"),
            QgsCoordinateTransformContext(),
            dt,
            0.0,
        )
        self.assertAlmostEqual(result.azimuth, 180.58312, places=3)
        self.assertAlmostEqual(result.apparentElevation, 50.82780006, places=3)

        self.assertEqual(
            result.solarMidnightBefore,
            QDateTime(QDate(2026, 4, 21), QTime(23, 58, 35), Qt.TimeSpec.UTC),
        )
        self.assertEqual(
            result.solarTransit,
            QDateTime(QDate(2026, 4, 22), QTime(11, 58, 29), Qt.TimeSpec.UTC),
        )
        self.assertEqual(
            result.solarMidnightAfter,
            QDateTime(QDate(2026, 4, 22), QTime(23, 58, 24), Qt.TimeSpec.UTC),
        )
        self.assertEqual(
            result.sunrise,
            QDateTime(QDate(2026, 4, 22), QTime(4, 49, 51), Qt.TimeSpec.UTC),
        )
        self.assertEqual(
            result.sunset,
            QDateTime(QDate(2026, 4, 22), QTime(19, 8, 14), Qt.TimeSpec.UTC),
        )
        self.assertEqual(
            result.civilDawn,
            QDateTime(QDate(2026, 4, 22), QTime(4, 13, 22), Qt.TimeSpec.UTC),
        )
        self.assertEqual(
            result.civilDusk,
            QDateTime(QDate(2026, 4, 22), QTime(19, 44, 55), Qt.TimeSpec.UTC),
        )
        self.assertEqual(
            result.nauticalDawn,
            QDateTime(QDate(2026, 4, 22), QTime(3, 25, 16), Qt.TimeSpec.UTC),
        )
        self.assertEqual(
            result.nauticalDusk,
            QDateTime(QDate(2026, 4, 22), QTime(20, 33, 27), Qt.TimeSpec.UTC),
        )
        self.assertEqual(
            result.astronomicalDawn,
            QDateTime(QDate(2026, 4, 22), QTime(2, 31, 27), Qt.TimeSpec.UTC),
        )
        self.assertEqual(
            result.astronomicalDusk,
            QDateTime(QDate(2026, 4, 22), QTime(21, 28, 26), Qt.TimeSpec.UTC),
        )

    def test_projected_crs(self):
        point = QgsPointXY(111319.49, 111325.14)
        dt = QDateTime(QDate(2026, 4, 22), QTime(12, 0, 0), Qt.TimeSpec.UTC)

        result = QgsSunPositionCalculator.calculate(
            point,
            QgsCoordinateReferenceSystem("EPSG:3857"),
            QgsCoordinateTransformContext(),
            dt,
            0.0,
        )
        self.assertAlmostEqual(result.azimuth, 353.16502477, places=3)
        self.assertAlmostEqual(result.apparentElevation, 78.623678642, places=3)
        self.assertEqual(
            result.sunrise,
            QDateTime(QDate(2026, 4, 22), QTime(5, 50, 18), Qt.TimeSpec.UTC),
        )
        self.assertEqual(
            result.sunset,
            QDateTime(QDate(2026, 4, 22), QTime(17, 58, 43), Qt.TimeSpec.UTC),
        )

    def test_polar_summer(self):
        # test a location above the Arctic circle during summer solstice
        point = QgsPointXY(15.0, 80.0)  # Svalbard
        dt = QDateTime(QDate(2026, 6, 21), QTime(12, 0, 0), Qt.TimeSpec.UTC)

        result = QgsSunPositionCalculator.calculate(
            point,
            QgsCoordinateReferenceSystem("EPSG:4326"),
            QgsCoordinateTransformContext(),
            dt,
            0.0,
        )
        # the sun does not set here in summer, so sunrise and sunset should be invalid
        self.assertFalse(result.sunrise.isValid())
        self.assertFalse(result.sunset.isValid())

        # solar transit (solar noon) still happens every day
        self.assertEqual(
            result.solarTransit,
            QDateTime(QDate(2026, 6, 21), QTime(11, 1, 49), Qt.TimeSpec.UTC),
        )

    def test_polar_winter(self):
        # Test a location above the Arctic circle during winter solstice
        point = QgsPointXY(15.0, 80.0)  # Svalbard
        dt = QDateTime(QDate(2026, 12, 21), QTime(12, 0, 0), Qt.TimeSpec.UTC)

        result = QgsSunPositionCalculator.calculate(
            point,
            QgsCoordinateReferenceSystem("EPSG:4326"),
            QgsCoordinateTransformContext(),
            dt,
            0.0,
        )

        # The sun does not rise here in winter, so sunrise and sunset should be invalid
        self.assertFalse(result.sunrise.isValid())
        self.assertFalse(result.sunset.isValid())

        # The sun stays below the horizon, so elevation should be negative
        self.assertLess(result.apparentElevation, 0.0)

    def test_invalid_parameters(self):
        dt = QDateTime(QDate(2026, 4, 22), QTime(12, 0, 0), Qt.TimeSpec.UTC)

        # Invalid latitude (valid range is -90 to 90)
        with self.assertRaises(QgsInvalidArgumentException):
            QgsSunPositionCalculator.calculate(
                QgsPointXY(0.0, 100.0),
                QgsCoordinateReferenceSystem("EPSG:4326"),
                QgsCoordinateTransformContext(),
                dt,
                0.0,
            )

        # Invalid longitude
        with self.assertRaises(QgsInvalidArgumentException):
            QgsSunPositionCalculator.calculate(
                QgsPointXY(100000, 0),
                QgsCoordinateReferenceSystem("EPSG:4326"),
                QgsCoordinateTransformContext(),
                dt,
                0.0,
            )

        # Invalid pressure
        with self.assertRaises(QgsInvalidArgumentException):
            QgsSunPositionCalculator.calculate(
                QgsPointXY(0, 0),
                QgsCoordinateReferenceSystem("EPSG:4326"),
                QgsCoordinateTransformContext(),
                dt,
                0.0,
                pressure=-999.0,
            )

        # Invalid temperature
        with self.assertRaises(QgsInvalidArgumentException):
            QgsSunPositionCalculator.calculate(
                QgsPointXY(0, 0),
                QgsCoordinateReferenceSystem("EPSG:4326"),
                QgsCoordinateTransformContext(),
                dt,
                0.0,
                temperature=-999.0,
            )


if __name__ == "__main__":
    unittest.main()
