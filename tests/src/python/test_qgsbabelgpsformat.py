# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsAbstractBabelFormat and subclasses.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2021-07'
__copyright__ = 'Copyright 2021, The QGIS Project'

import qgis  # NOQA

from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import (
    Qgis, QgsSettings,
    QgsBabelSimpleImportFormat,
    QgsBabelGpsDeviceFormat,
    QgsApplication,
    QgsBabelFormatRegistry
)
from qgis.testing import start_app, unittest


class TestQgsBabelGpsFormat(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("TestPyQgsAFSProvider.com")
        QCoreApplication.setApplicationName("TestPyQgsAFSProvider")
        QgsSettings().clear()
        start_app()

    def test_simple_format(self):
        """
        Test QgsBabelSimpleImportFormat
        """
        f = QgsBabelSimpleImportFormat('shapefile', Qgis.BabelFormatCapability.Waypoints)
        self.assertEqual(f.capabilities(), Qgis.BabelFormatCapabilities(
            Qgis.BabelFormatCapability.Waypoints | Qgis.BabelFormatCapability.Import))
        f = QgsBabelSimpleImportFormat('shapefile', Qgis.BabelFormatCapabilities(
            Qgis.BabelFormatCapability.Waypoints | Qgis.BabelFormatCapability.Tracks))
        self.assertEqual(f.capabilities(), Qgis.BabelFormatCapabilities(
            Qgis.BabelFormatCapability.Waypoints | Qgis.BabelFormatCapability.Tracks | Qgis.BabelFormatCapability.Import))

        self.assertEqual(
            f.importCommand('babel.exe', Qgis.GpsFeatureType.Waypoint, 'c:/test/test.shp', 'c:/test/test.gpx'),
            ['"babel.exe"',
             '-w',
             '-i',
             'shapefile',
             '-o',
             'gpx',
             '"c:/test/test.shp"',
             '"c:/test/test.gpx"'])
        self.assertEqual(
            f.importCommand('babel.exe', Qgis.GpsFeatureType.Track, 'c:/test/test.shp', 'c:/test/test.gpx'),
            ['"babel.exe"',
             '-t',
             '-i',
             'shapefile',
             '-o',
             'gpx',
             '"c:/test/test.shp"',
             '"c:/test/test.gpx"'])
        self.assertEqual(
            f.importCommand('babel.exe', Qgis.GpsFeatureType.Route, 'c:/test/test.shp', 'c:/test/test.gpx'),
            ['"babel.exe"',
             '-r',
             '-i',
             'shapefile',
             '-o',
             'gpx',
             '"c:/test/test.shp"',
             '"c:/test/test.gpx"'])
        # export not supported
        self.assertEqual(
            f.exportCommand('babel.exe', Qgis.GpsFeatureType.Waypoint, 'c:/test/test.shp', 'c:/test/test.gpx'), [])

    def test_gps_device_format(self):
        """
        Test QgsBabelGpsDeviceFormat
        """
        f = QgsBabelGpsDeviceFormat(
            "%babel -w -i garmin -o gpx %in %out",
            "%babel -w -i gpx -o garmin %in %out",
            "%babel -r -i garmin -o gpx %in %out",
            "%babel -r -i gpx -o garmin %in %out",
            "%babel -t -i garmin -o gpx %in %out",
            "%babel -t -i gpx -o garmin %in %out"
        )
        # TODO -- I suspect that the waypoint/track/route capability should be automatically set/removed
        # depending on whether the corresponding commands are empty!
        # self.assertEqual(f.capabilities(), Qgis.BabelFormatCapabilities(
        #    Qgis.BabelFormatCapability.Waypoints | Qgis.BabelFormatCapability.Import))

        # TODO -- babel command should possibly be quoted (or NOT in QgsBabelSimpleImportFormat)
        self.assertEqual(
            f.importCommand('babel.exe', Qgis.GpsFeatureType.Waypoint, 'c:/test/test.shp', 'c:/test/test.gpx'),
            ['babel.exe',
             '-w',
             '-i',
             'garmin',
             '-o',
             'gpx',
             '"c:/test/test.shp"',
             '"c:/test/test.gpx"'])
        self.assertEqual(
            f.importCommand('babel.exe', Qgis.GpsFeatureType.Track, 'c:/test/test.shp', 'c:/test/test.gpx'),
            ['babel.exe',
             '-t',
             '-i',
             'garmin',
             '-o',
             'gpx',
             '"c:/test/test.shp"',
             '"c:/test/test.gpx"'])
        self.assertEqual(
            f.importCommand('babel.exe', Qgis.GpsFeatureType.Route, 'c:/test/test.shp', 'c:/test/test.gpx'),
            ['babel.exe',
             '-r',
             '-i',
             'garmin',
             '-o',
             'gpx',
             '"c:/test/test.shp"',
             '"c:/test/test.gpx"'])

        self.assertEqual(
            f.exportCommand('babel.exe', Qgis.GpsFeatureType.Waypoint, 'c:/test/test.shp', 'c:/test/test.gpx'),
            ['babel.exe',
             '-w',
             '-i',
             'gpx',
             '-o',
             'garmin',
             '"c:/test/test.shp"',
             '"c:/test/test.gpx"'])
        self.assertEqual(
            f.exportCommand('babel.exe', Qgis.GpsFeatureType.Track, 'c:/test/test.shp', 'c:/test/test.gpx'),
            ['babel.exe',
             '-t',
             '-i',
             'gpx',
             '-o',
             'garmin',
             '"c:/test/test.shp"',
             '"c:/test/test.gpx"'])
        self.assertEqual(
            f.exportCommand('babel.exe', Qgis.GpsFeatureType.Route, 'c:/test/test.shp', 'c:/test/test.gpx'),
            ['babel.exe',
             '-r',
             '-i',
             'gpx',
             '-o',
             'garmin',
             '"c:/test/test.shp"',
             '"c:/test/test.gpx"'])

    def test_registry(self):
        """
        Test QgsBabelFormatRegistry
        """
        self.assertIsNotNone(QgsApplication.gpsBabelFormatRegistry())

        registry = QgsBabelFormatRegistry()
        self.assertIn('Garmin Mapsource', registry.importFormatNames())
        self.assertIn('DNA', registry.importFormatNames())

        self.assertIsNone(registry.importFormat('aaaaaa'))
        self.assertIsNotNone(registry.importFormat('CoPilot Flight Planner'))
        self.assertEqual(registry.importFormat('CoPilot Flight Planner').capabilities(), Qgis.BabelFormatCapabilities(Qgis.BabelFormatCapability.Waypoints | Qgis.BabelFormatCapability.Import))

        # should have only one device by default
        self.assertEqual(registry.deviceNames(), ['Garmin serial'])
        self.assertIsNotNone(registry.deviceFormat('Garmin serial'))
        self.assertEqual(registry.deviceFormat('Garmin serial').importCommand('bb', Qgis.GpsFeatureType.Waypoint, 'in_file.shp', 'out_file.gpx'),
                         ['bb', '-w', '-i', 'garmin', '-o', 'gpx', '"in_file.shp"', '"out_file.gpx"'])


if __name__ == '__main__':
    unittest.main()
