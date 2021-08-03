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
        QCoreApplication.setOrganizationDomain("TestPyQgsBabelGpsFormat.com")
        QCoreApplication.setApplicationName("TestPyQgsBabelGpsFormat")
        QgsSettings().clear()
        start_app()

    def test_simple_format(self):
        """
        Test QgsBabelSimpleImportFormat
        """
        f = QgsBabelSimpleImportFormat('shapefile', 'ESRI Shapefile', Qgis.BabelFormatCapability.Waypoints, ['shp', 'shx'])
        self.assertEqual(f.name(), 'shapefile')
        self.assertEqual(f.description(), 'ESRI Shapefile')
        self.assertEqual(f.extensions(), ['shp', 'shx'])
        self.assertEqual(f.capabilities(), Qgis.BabelFormatCapabilities(
            Qgis.BabelFormatCapability.Waypoints | Qgis.BabelFormatCapability.Import))
        f = QgsBabelSimpleImportFormat('shapefile', 'ESRI Shapefile', Qgis.BabelFormatCapabilities(
            Qgis.BabelFormatCapability.Waypoints | Qgis.BabelFormatCapability.Tracks))
        self.assertEqual(f.capabilities(), Qgis.BabelFormatCapabilities(
            Qgis.BabelFormatCapability.Waypoints | Qgis.BabelFormatCapability.Tracks | Qgis.BabelFormatCapability.Import))

        self.assertEqual(
            f.importCommand('babel.exe', Qgis.GpsFeatureType.Waypoint, 'c:/test/test.shp', 'c:/test/test.gpx'),
            ['babel.exe',
             '-w',
             '-i',
             'shapefile',
             '-o',
             'gpx',
             'c:/test/test.shp',
             'c:/test/test.gpx'])
        self.assertEqual(
            f.importCommand('babel.exe', Qgis.GpsFeatureType.Track, 'c:/test/test.shp', 'c:/test/test.gpx'),
            ['babel.exe',
             '-t',
             '-i',
             'shapefile',
             '-o',
             'gpx',
             'c:/test/test.shp',
             'c:/test/test.gpx'])
        self.assertEqual(
            f.importCommand('babel.exe', Qgis.GpsFeatureType.Route, 'c:/test/test.shp', 'c:/test/test.gpx'),
            ['babel.exe',
             '-r',
             '-i',
             'shapefile',
             '-o',
             'gpx',
             'c:/test/test.shp',
             'c:/test/test.gpx'])

        # with quoted paths
        self.assertEqual(
            f.importCommand('babel.exe', Qgis.GpsFeatureType.Route, 'c:/test/test.shp', 'c:/test/test.gpx', Qgis.BabelCommandFlag.QuoteFilePaths),
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
        # waypoint/track/route capability should be automatically set/removed
        # depending on whether the corresponding commands are empty
        self.assertEqual(f.capabilities(), Qgis.BabelFormatCapabilities(
            Qgis.BabelFormatCapability.Waypoints | Qgis.BabelFormatCapability.Import
            | Qgis.BabelFormatCapability.Export | Qgis.BabelFormatCapability.Tracks | Qgis.BabelFormatCapability.Routes))

        self.assertEqual(
            f.importCommand('babel.exe', Qgis.GpsFeatureType.Waypoint, 'c:/test/test.shp', 'c:/test/test.gpx'),
            ['babel.exe',
             '-w',
             '-i',
             'garmin',
             '-o',
             'gpx',
             'c:/test/test.shp',
             'c:/test/test.gpx'])
        self.assertEqual(
            f.importCommand('babel.exe', Qgis.GpsFeatureType.Track, 'c:/test/test.shp', 'c:/test/test.gpx'),
            ['babel.exe',
             '-t',
             '-i',
             'garmin',
             '-o',
             'gpx',
             'c:/test/test.shp',
             'c:/test/test.gpx'])
        self.assertEqual(
            f.importCommand('babel.exe', Qgis.GpsFeatureType.Route, 'c:/test/test.shp', 'c:/test/test.gpx'),
            ['babel.exe',
             '-r',
             '-i',
             'garmin',
             '-o',
             'gpx',
             'c:/test/test.shp',
             'c:/test/test.gpx'])

        self.assertEqual(
            f.exportCommand('babel.exe', Qgis.GpsFeatureType.Waypoint, 'c:/test/test.shp', 'c:/test/test.gpx'),
            ['babel.exe',
             '-w',
             '-i',
             'gpx',
             '-o',
             'garmin',
             'c:/test/test.shp',
             'c:/test/test.gpx'])
        self.assertEqual(
            f.exportCommand('babel.exe', Qgis.GpsFeatureType.Track, 'c:/test/test.shp', 'c:/test/test.gpx'),
            ['babel.exe',
             '-t',
             '-i',
             'gpx',
             '-o',
             'garmin',
             'c:/test/test.shp',
             'c:/test/test.gpx'])
        self.assertEqual(
            f.exportCommand('babel.exe', Qgis.GpsFeatureType.Route, 'c:/test/test.shp', 'c:/test/test.gpx'),
            ['babel.exe',
             '-r',
             '-i',
             'gpx',
             '-o',
             'garmin',
             'c:/test/test.shp',
             'c:/test/test.gpx'])

        # with quoted paths
        self.assertEqual(
            f.exportCommand('babel.exe', Qgis.GpsFeatureType.Route, 'c:/test/test.shp', 'c:/test/test.gpx', Qgis.BabelCommandFlag.QuoteFilePaths),
            ['"babel.exe"',
             '-r',
             '-i',
             'gpx',
             '-o',
             'garmin',
             '"c:/test/test.shp"',
             '"c:/test/test.gpx"'])

        # waypoint/track/route capability should be automatically set/removed
        # depending on whether the corresponding commands are empty
        f = QgsBabelGpsDeviceFormat(
            "%babel -w -i garmin -o gpx %in %out",
            None,
            None,
            None,
            None,
            None
        )
        self.assertEqual(f.capabilities(), Qgis.BabelFormatCapabilities(
            Qgis.BabelFormatCapability.Waypoints | Qgis.BabelFormatCapability.Import))
        f = QgsBabelGpsDeviceFormat(
            None,
            "%babel -w -i gpx -o garmin %in %out",
            None,
            None,
            None,
            None
        )
        self.assertEqual(f.capabilities(), Qgis.BabelFormatCapabilities(
            Qgis.BabelFormatCapability.Waypoints | Qgis.BabelFormatCapability.Export))
        f = QgsBabelGpsDeviceFormat(
            None,
            None,
            "%babel -r -i garmin -o gpx %in %out",
            None,
            None,
            None
        )
        self.assertEqual(f.capabilities(), Qgis.BabelFormatCapabilities(
            Qgis.BabelFormatCapability.Routes | Qgis.BabelFormatCapability.Import))
        f = QgsBabelGpsDeviceFormat(
            None,
            None,
            None,
            "%babel -r -i gpx -o garmin %in %out",
            None,
            None
        )
        self.assertEqual(f.capabilities(), Qgis.BabelFormatCapabilities(
            Qgis.BabelFormatCapability.Routes | Qgis.BabelFormatCapability.Export))
        f = QgsBabelGpsDeviceFormat(
            None,
            None,
            None,
            None,
            "%babel -t -i garmin -o gpx %in %out",
            None
        )
        self.assertEqual(f.capabilities(), Qgis.BabelFormatCapabilities(
            Qgis.BabelFormatCapability.Tracks | Qgis.BabelFormatCapability.Import))
        f = QgsBabelGpsDeviceFormat(
            None,
            None,
            None,
            None,
            None,
            "%babel -t -i gpx -o garmin %in %out"
        )
        self.assertEqual(f.capabilities(), Qgis.BabelFormatCapabilities(
            Qgis.BabelFormatCapability.Tracks | Qgis.BabelFormatCapability.Export))

    def test_registry(self):
        """
        Test QgsBabelFormatRegistry
        """
        self.assertIsNotNone(QgsApplication.gpsBabelFormatRegistry())

        registry = QgsBabelFormatRegistry()
        self.assertIn('garmin_poi', registry.importFormatNames())
        self.assertIn('dna', registry.importFormatNames())

        self.assertIsNone(registry.importFormat('aaaaaa'))
        self.assertIsNotNone(registry.importFormat('dna'))
        self.assertEqual(registry.importFormat('dna').name(), 'dna')
        self.assertEqual(registry.importFormat('dna').description(), 'Navitrak DNA marker format')
        self.assertEqual(registry.importFormat('dna').extensions(), ['dna'])

        self.assertIsNone(registry.importFormatByDescription('aaaaaa'))
        self.assertEqual(registry.importFormatByDescription('Navitrak DNA marker format').name(), 'dna')
        self.assertEqual(registry.importFormatByDescription('navitrak dna marker format').name(), 'dna')
        self.assertEqual(registry.importFormatByDescription('PocketFMS flightplan (.xml)').name(), 'pocketfms_fp')
        # see explanation in QgsBabelFormatRegistry::importFileFilter()!
        self.assertEqual(registry.importFormatByDescription('PocketFMS flightplan [.xml]').name(), 'pocketfms_fp')

        self.assertIn(';;ESRI shapefile (*.shp);;', registry.importFileFilter())
        self.assertIn(';;PocketFMS flightplan [.xml] (*.xml);;', registry.importFileFilter())

        # should have only one device by default
        self.assertEqual(registry.deviceNames(), ['Garmin serial'])
        self.assertIsNotNone(registry.deviceFormat('Garmin serial'))
        self.assertEqual(registry.deviceFormat('Garmin serial').importCommand('bb', Qgis.GpsFeatureType.Waypoint, 'in_file.shp', 'out_file.gpx'),
                         ['bb', '-w', '-i', 'garmin', '-o', 'gpx', 'in_file.shp', 'out_file.gpx'])


if __name__ == '__main__':
    unittest.main()
