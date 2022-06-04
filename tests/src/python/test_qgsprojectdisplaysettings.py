# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsProjectDisplaySettings.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '09/01/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

from qgis.core import (QgsProjectDisplaySettings,
                       QgsReadWriteContext,
                       QgsBearingNumericFormat,
                       QgsGeographicCoordinateNumericFormat,
                       QgsSettings,
                       QgsLocalDefaultSettings)

from qgis.PyQt.QtCore import QCoreApplication

from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument
from qgis.testing import start_app, unittest
from utilities import (unitTestDataPath)

app = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsProjectDisplaySettings(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("TestPyQgsWFSProvider.com")
        QCoreApplication.setApplicationName("TestPyQgsWFSProvider")
        QgsSettings().clear()
        start_app()

    def testBearingFormat(self):
        p = QgsProjectDisplaySettings()

        format = QgsBearingNumericFormat()
        format.setNumberDecimalPlaces(9)
        format.setDirectionFormat(QgsBearingNumericFormat.UseRange0To360)

        spy = QSignalSpy(p.bearingFormatChanged)
        p.setBearingFormat(format)
        self.assertEqual(len(spy), 1)
        self.assertEqual(p.bearingFormat().numberDecimalPlaces(), 9)
        self.assertEqual(p.bearingFormat().directionFormat(), QgsBearingNumericFormat.UseRange0To360)

        format = QgsBearingNumericFormat()
        format.setNumberDecimalPlaces(3)
        format.setDirectionFormat(QgsBearingNumericFormat.UseRangeNegative180ToPositive180)
        p.setBearingFormat(format)
        self.assertEqual(len(spy), 2)
        self.assertEqual(p.bearingFormat().numberDecimalPlaces(), 3)
        self.assertEqual(p.bearingFormat().directionFormat(), QgsBearingNumericFormat.UseRangeNegative180ToPositive180)

    def testGeographicCoordinateFormat(self):
        p = QgsProjectDisplaySettings()

        format = QgsGeographicCoordinateNumericFormat()
        format.setNumberDecimalPlaces(9)
        format.setAngleFormat(QgsGeographicCoordinateNumericFormat.AngleFormat.DegreesMinutesSeconds)

        spy = QSignalSpy(p.geographicCoordinateFormatChanged)
        p.setGeographicCoordinateFormat(format)
        self.assertEqual(len(spy), 1)
        self.assertEqual(p.geographicCoordinateFormat().numberDecimalPlaces(), 9)
        self.assertEqual(p.geographicCoordinateFormat().angleFormat(), QgsGeographicCoordinateNumericFormat.AngleFormat.DegreesMinutesSeconds)

        format = QgsGeographicCoordinateNumericFormat()
        format.setNumberDecimalPlaces(3)
        format.setAngleFormat(QgsGeographicCoordinateNumericFormat.AngleFormat.DegreesMinutes)
        p.setGeographicCoordinateFormat(format)
        self.assertEqual(len(spy), 2)
        self.assertEqual(p.geographicCoordinateFormat().numberDecimalPlaces(), 3)
        self.assertEqual(p.geographicCoordinateFormat().angleFormat(), QgsGeographicCoordinateNumericFormat.AngleFormat.DegreesMinutes)

    def testReset(self):
        """
        Test that resetting inherits local default settings
        """
        format = QgsBearingNumericFormat()
        format.setNumberDecimalPlaces(3)
        format.setDirectionFormat(QgsBearingNumericFormat.UseRangeNegative180ToPositive180)
        p = QgsProjectDisplaySettings()
        p.setBearingFormat(format)
        self.assertEqual(p.bearingFormat().numberDecimalPlaces(), 3)
        self.assertEqual(p.bearingFormat().directionFormat(), QgsBearingNumericFormat.UseRangeNegative180ToPositive180)

        format = QgsGeographicCoordinateNumericFormat()
        format.setNumberDecimalPlaces(7)
        format.setAngleFormat(QgsGeographicCoordinateNumericFormat.AngleFormat.DegreesMinutesSeconds)
        p.setGeographicCoordinateFormat(format)
        self.assertEqual(p.geographicCoordinateFormat().numberDecimalPlaces(), 7)
        self.assertEqual(p.geographicCoordinateFormat().angleFormat(), QgsGeographicCoordinateNumericFormat.AngleFormat.DegreesMinutesSeconds)

        # setup a local default bearing format
        s = QgsLocalDefaultSettings()
        format = QgsBearingNumericFormat()
        format.setNumberDecimalPlaces(9)
        format.setDirectionFormat(QgsBearingNumericFormat.UseRange0To360)
        s.setBearingFormat(format)

        format = QgsGeographicCoordinateNumericFormat()
        format.setNumberDecimalPlaces(5)
        format.setAngleFormat(QgsGeographicCoordinateNumericFormat.AngleFormat.DegreesMinutes)
        s.setGeographicCoordinateFormat(format)

        spy = QSignalSpy(p.bearingFormatChanged)
        spy2 = QSignalSpy(p.geographicCoordinateFormatChanged)
        p.reset()
        self.assertEqual(len(spy), 1)
        self.assertEqual(len(spy2), 1)
        # project should default to local default format
        self.assertEqual(p.bearingFormat().numberDecimalPlaces(), 9)
        self.assertEqual(p.bearingFormat().directionFormat(), QgsBearingNumericFormat.UseRange0To360)

        self.assertEqual(p.geographicCoordinateFormat().numberDecimalPlaces(), 5)
        self.assertEqual(p.geographicCoordinateFormat().angleFormat(), QgsGeographicCoordinateNumericFormat.AngleFormat.DegreesMinutes)

    def testReadWrite(self):
        p = QgsProjectDisplaySettings()

        format = QgsBearingNumericFormat()
        format.setNumberDecimalPlaces(9)
        format.setDirectionFormat(QgsBearingNumericFormat.UseRange0To360)
        p.setBearingFormat(format)

        format = QgsGeographicCoordinateNumericFormat()
        format.setNumberDecimalPlaces(7)
        format.setAngleFormat(QgsGeographicCoordinateNumericFormat.AngleFormat.DegreesMinutesSeconds)
        p.setGeographicCoordinateFormat(format)

        doc = QDomDocument("testdoc")
        elem = p.writeXml(doc, QgsReadWriteContext())

        p2 = QgsProjectDisplaySettings()
        spy = QSignalSpy(p2.bearingFormatChanged)
        spy2 = QSignalSpy(p2.geographicCoordinateFormatChanged)
        self.assertTrue(p2.readXml(elem, QgsReadWriteContext()))
        self.assertEqual(len(spy), 1)
        self.assertEqual(len(spy2), 1)
        self.assertEqual(p2.bearingFormat().numberDecimalPlaces(), 9)
        self.assertEqual(p2.bearingFormat().directionFormat(), QgsBearingNumericFormat.UseRange0To360)
        self.assertEqual(p.geographicCoordinateFormat().numberDecimalPlaces(), 7)
        self.assertEqual(p.geographicCoordinateFormat().angleFormat(), QgsGeographicCoordinateNumericFormat.AngleFormat.DegreesMinutesSeconds)


if __name__ == '__main__':
    unittest.main()
