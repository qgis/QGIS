"""QGIS Unit tests for QgsLocalDefaultSettings.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "09/01/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import (
    QgsBearingNumericFormat,
    QgsGeographicCoordinateNumericFormat,
    QgsLocalDefaultSettings,
    QgsSettings,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()


class TestQgsLocalDefaultSettings(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("TestPyQgsWFSProvider.com")
        QCoreApplication.setApplicationName("TestPyQgsWFSProvider")
        QgsSettings().clear()
        start_app()

    def testBearingFormat(self):
        s = QgsLocalDefaultSettings()

        format = QgsBearingNumericFormat()
        format.setNumberDecimalPlaces(9)
        format.setDirectionFormat(
            QgsBearingNumericFormat.FormatDirectionOption.UseRange0To360
        )

        s.setBearingFormat(format)
        self.assertEqual(s.bearingFormat().numberDecimalPlaces(), 9)
        self.assertEqual(
            s.bearingFormat().directionFormat(),
            QgsBearingNumericFormat.FormatDirectionOption.UseRange0To360,
        )

        format = QgsBearingNumericFormat()
        format.setNumberDecimalPlaces(3)
        format.setDirectionFormat(
            QgsBearingNumericFormat.FormatDirectionOption.UseRangeNegative180ToPositive180
        )
        s.setBearingFormat(format)
        self.assertEqual(s.bearingFormat().numberDecimalPlaces(), 3)
        self.assertEqual(
            s.bearingFormat().directionFormat(),
            QgsBearingNumericFormat.FormatDirectionOption.UseRangeNegative180ToPositive180,
        )

        # new settings object, should persist.
        s2 = QgsLocalDefaultSettings()
        self.assertEqual(s2.bearingFormat().numberDecimalPlaces(), 3)
        self.assertEqual(
            s2.bearingFormat().directionFormat(),
            QgsBearingNumericFormat.FormatDirectionOption.UseRangeNegative180ToPositive180,
        )

    def testGeographicCoordinateFormat(self):
        s = QgsLocalDefaultSettings()

        format = QgsGeographicCoordinateNumericFormat()
        format.setAngleFormat(
            QgsGeographicCoordinateNumericFormat.AngleFormat.DegreesMinutes
        )

        s.setGeographicCoordinateFormat(format)
        self.assertEqual(
            s.geographicCoordinateFormat().angleFormat(),
            QgsGeographicCoordinateNumericFormat.AngleFormat.DegreesMinutes,
        )

        format = QgsGeographicCoordinateNumericFormat()
        format.setNumberDecimalPlaces(3)
        format.setAngleFormat(
            QgsGeographicCoordinateNumericFormat.AngleFormat.DegreesMinutesSeconds
        )
        s.setGeographicCoordinateFormat(format)
        self.assertEqual(s.geographicCoordinateFormat().numberDecimalPlaces(), 3)
        self.assertEqual(
            s.geographicCoordinateFormat().angleFormat(),
            QgsGeographicCoordinateNumericFormat.AngleFormat.DegreesMinutesSeconds,
        )

        # new settings object, should persist.
        s2 = QgsLocalDefaultSettings()
        self.assertEqual(s2.geographicCoordinateFormat().numberDecimalPlaces(), 3)
        self.assertEqual(
            s2.geographicCoordinateFormat().angleFormat(),
            QgsGeographicCoordinateNumericFormat.AngleFormat.DegreesMinutesSeconds,
        )


if __name__ == "__main__":
    unittest.main()
