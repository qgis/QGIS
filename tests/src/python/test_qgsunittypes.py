# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsUnitTypes

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '03.02.2016'
__copyright__ = 'Copyright 2016, The QGIS Project'

import qgis  # NOQA

from qgis.testing import unittest
from qgis.core import QgsUnitTypes
from qgis.PyQt.QtCore import QLocale


class TestQgsUnitTypes(unittest.TestCase):

    def setUp(self):
        super().setUp()
        # enforce C locale because the tests expect it
        # (decimal separators / thousand separators)
        QLocale.setDefault(QLocale.c())

    def testEncodeDecodeUnitType(self):
        """Test encoding and decoding unit type"""
        units = [QgsUnitTypes.TypeDistance,
                 QgsUnitTypes.TypeArea,
                 QgsUnitTypes.TypeVolume,
                 QgsUnitTypes.TypeTemporal,
                 QgsUnitTypes.TypeUnknown]

        for u in units:
            res, ok = QgsUnitTypes.decodeUnitType(QgsUnitTypes.encodeUnitType(u))
            assert ok
            self.assertEqual(res, u)

        # Test decoding bad units
        res, ok = QgsUnitTypes.decodeUnitType('bad')
        self.assertFalse(ok)
        self.assertEqual(res, QgsUnitTypes.TypeUnknown)

        # Test that string is cleaned before decoding
        res, ok = QgsUnitTypes.decodeUnitType(' volUme  ')
        assert ok
        self.assertEqual(res, QgsUnitTypes.TypeVolume)

    def testDistanceUnitType(self):
        """Test QgsUnitTypes::unitType() """
        expected = {QgsUnitTypes.DistanceMeters: QgsUnitTypes.Standard,
                    QgsUnitTypes.DistanceKilometers: QgsUnitTypes.Standard,
                    QgsUnitTypes.DistanceFeet: QgsUnitTypes.Standard,
                    QgsUnitTypes.DistanceYards: QgsUnitTypes.Standard,
                    QgsUnitTypes.DistanceMiles: QgsUnitTypes.Standard,
                    QgsUnitTypes.DistanceDegrees: QgsUnitTypes.Geographic,
                    QgsUnitTypes.DistanceCentimeters: QgsUnitTypes.Standard,
                    QgsUnitTypes.DistanceMillimeters: QgsUnitTypes.Standard,
                    QgsUnitTypes.DistanceUnknownUnit: QgsUnitTypes.UnknownType,
                    QgsUnitTypes.DistanceNauticalMiles: QgsUnitTypes.Standard
                    }

        for t in list(expected.keys()):
            self.assertEqual(QgsUnitTypes.unitType(t), expected[t])

    def testEncodeDecodeDistanceUnits(self):
        """Test encoding and decoding distance units"""
        units = [QgsUnitTypes.DistanceMeters,
                 QgsUnitTypes.DistanceKilometers,
                 QgsUnitTypes.DistanceFeet,
                 QgsUnitTypes.DistanceYards,
                 QgsUnitTypes.DistanceMiles,
                 QgsUnitTypes.DistanceDegrees,
                 QgsUnitTypes.DistanceCentimeters,
                 QgsUnitTypes.DistanceMillimeters,
                 QgsUnitTypes.DistanceUnknownUnit,
                 QgsUnitTypes.DistanceNauticalMiles]

        for u in units:
            res, ok = QgsUnitTypes.decodeDistanceUnit(QgsUnitTypes.encodeUnit(u))
            assert ok
            self.assertEqual(res, u)

        # Test decoding bad units
        res, ok = QgsUnitTypes.decodeDistanceUnit('bad')
        self.assertFalse(ok)
        self.assertEqual(res, QgsUnitTypes.DistanceUnknownUnit)

        # Test that string is cleaned before decoding
        res, ok = QgsUnitTypes.decodeDistanceUnit(' FeEt  ')
        assert ok
        self.assertEqual(res, QgsUnitTypes.DistanceFeet)

    def testDistanceUnitsToFromString(self):
        """Test converting distance units to and from translated strings"""
        units = [QgsUnitTypes.DistanceMeters,
                 QgsUnitTypes.DistanceKilometers,
                 QgsUnitTypes.DistanceFeet,
                 QgsUnitTypes.DistanceYards,
                 QgsUnitTypes.DistanceMiles,
                 QgsUnitTypes.DistanceDegrees,
                 QgsUnitTypes.DistanceCentimeters,
                 QgsUnitTypes.DistanceMillimeters,
                 QgsUnitTypes.DistanceUnknownUnit,
                 QgsUnitTypes.DistanceNauticalMiles]

        for u in units:
            res, ok = QgsUnitTypes.stringToDistanceUnit(QgsUnitTypes.toString(u))
            assert ok
            self.assertEqual(res, u)

        # Test converting bad strings
        res, ok = QgsUnitTypes.stringToDistanceUnit('bad')
        self.assertFalse(ok)
        self.assertEqual(res, QgsUnitTypes.DistanceUnknownUnit)

        # Test that string is cleaned before conversion
        res, ok = QgsUnitTypes.stringToDistanceUnit(' {}  '.format(QgsUnitTypes.toString(QgsUnitTypes.DistanceFeet).upper()))
        print((' {}  '.format(QgsUnitTypes.toString(QgsUnitTypes.DistanceFeet).upper())))
        assert ok
        self.assertEqual(res, QgsUnitTypes.DistanceFeet)

    def testAreaUnitType(self):
        """Test QgsUnitTypes::unitType() for area units """
        expected = {QgsUnitTypes.AreaSquareMeters: QgsUnitTypes.Standard,
                    QgsUnitTypes.AreaSquareKilometers: QgsUnitTypes.Standard,
                    QgsUnitTypes.AreaSquareFeet: QgsUnitTypes.Standard,
                    QgsUnitTypes.AreaSquareYards: QgsUnitTypes.Standard,
                    QgsUnitTypes.AreaSquareMiles: QgsUnitTypes.Standard,
                    QgsUnitTypes.AreaHectares: QgsUnitTypes.Standard,
                    QgsUnitTypes.AreaAcres: QgsUnitTypes.Standard,
                    QgsUnitTypes.AreaSquareNauticalMiles: QgsUnitTypes.Standard,
                    QgsUnitTypes.AreaSquareDegrees: QgsUnitTypes.Geographic,
                    QgsUnitTypes.AreaSquareCentimeters: QgsUnitTypes.Standard,
                    QgsUnitTypes.AreaSquareMillimeters: QgsUnitTypes.Standard,
                    QgsUnitTypes.AreaUnknownUnit: QgsUnitTypes.UnknownType,
                    }

        for t in list(expected.keys()):
            self.assertEqual(QgsUnitTypes.unitType(t), expected[t])

    def testEncodeDecodeAreaUnits(self):
        """Test encoding and decoding area units"""
        units = [QgsUnitTypes.AreaSquareMeters,
                 QgsUnitTypes.AreaSquareKilometers,
                 QgsUnitTypes.AreaSquareFeet,
                 QgsUnitTypes.AreaSquareYards,
                 QgsUnitTypes.AreaSquareMiles,
                 QgsUnitTypes.AreaHectares,
                 QgsUnitTypes.AreaAcres,
                 QgsUnitTypes.AreaSquareNauticalMiles,
                 QgsUnitTypes.AreaSquareDegrees,
                 QgsUnitTypes.AreaSquareCentimeters,
                 QgsUnitTypes.AreaSquareMillimeters,
                 QgsUnitTypes.AreaUnknownUnit]

        for u in units:
            res, ok = QgsUnitTypes.decodeAreaUnit(QgsUnitTypes.encodeUnit(u))
            assert ok
            self.assertEqual(res, u)

        # Test decoding bad units
        res, ok = QgsUnitTypes.decodeAreaUnit('bad')
        self.assertFalse(ok)
        self.assertEqual(res, QgsUnitTypes.AreaUnknownUnit)

        # Test that string is cleaned before decoding
        res, ok = QgsUnitTypes.decodeAreaUnit(' Ha  ')
        assert ok
        self.assertEqual(res, QgsUnitTypes.AreaHectares)

    def testAreaUnitsToFromString(self):
        """Test converting area units to and from translated strings"""
        units = [QgsUnitTypes.AreaSquareMeters,
                 QgsUnitTypes.AreaSquareKilometers,
                 QgsUnitTypes.AreaSquareFeet,
                 QgsUnitTypes.AreaSquareYards,
                 QgsUnitTypes.AreaSquareMiles,
                 QgsUnitTypes.AreaHectares,
                 QgsUnitTypes.AreaAcres,
                 QgsUnitTypes.AreaSquareNauticalMiles,
                 QgsUnitTypes.AreaSquareDegrees,
                 QgsUnitTypes.AreaSquareCentimeters,
                 QgsUnitTypes.AreaSquareMillimeters,
                 QgsUnitTypes.AreaUnknownUnit]

        for u in units:
            res, ok = QgsUnitTypes.stringToAreaUnit(QgsUnitTypes.toString(u))
            assert ok
            self.assertEqual(res, u)

        # Test converting bad strings
        res, ok = QgsUnitTypes.stringToAreaUnit('bad')
        self.assertFalse(ok)
        self.assertEqual(res, QgsUnitTypes.AreaUnknownUnit)

        # Test that string is cleaned before conversion
        res, ok = QgsUnitTypes.stringToAreaUnit(' {}  '.format(QgsUnitTypes.toString(QgsUnitTypes.AreaSquareMiles).upper()))
        assert ok
        self.assertEqual(res, QgsUnitTypes.AreaSquareMiles)

    def testVolumeUnitType(self):
        """Test QgsUnitTypes::unitType() for volume units """
        expected = {QgsUnitTypes.VolumeCubicMeters: QgsUnitTypes.Standard,
                    QgsUnitTypes.VolumeCubicFeet: QgsUnitTypes.Standard,
                    QgsUnitTypes.VolumeCubicYards: QgsUnitTypes.Standard,
                    QgsUnitTypes.VolumeBarrel: QgsUnitTypes.Standard,
                    QgsUnitTypes.VolumeCubicDecimeter: QgsUnitTypes.Standard,
                    QgsUnitTypes.VolumeLiters: QgsUnitTypes.Standard,
                    QgsUnitTypes.VolumeGallonUS: QgsUnitTypes.Standard,
                    QgsUnitTypes.VolumeCubicInch: QgsUnitTypes.Standard,
                    QgsUnitTypes.VolumeCubicCentimeter: QgsUnitTypes.Standard,
                    QgsUnitTypes.VolumeCubicDegrees: QgsUnitTypes.Geographic,
                    QgsUnitTypes.VolumeUnknownUnit: QgsUnitTypes.UnknownType,
                    }

        for t in list(expected.keys()):
            self.assertEqual(QgsUnitTypes.unitType(t), expected[t])

    def testEncodeDecodeVolumeUnits(self):
        """Test encoding and decoding volume units"""
        units = [QgsUnitTypes.VolumeCubicMeters,
                 QgsUnitTypes.VolumeCubicFeet,
                 QgsUnitTypes.VolumeCubicYards,
                 QgsUnitTypes.VolumeBarrel,
                 QgsUnitTypes.VolumeCubicDecimeter,
                 QgsUnitTypes.VolumeLiters,
                 QgsUnitTypes.VolumeGallonUS,
                 QgsUnitTypes.VolumeCubicInch,
                 QgsUnitTypes.VolumeCubicCentimeter,
                 QgsUnitTypes.VolumeCubicDegrees,
                 QgsUnitTypes.VolumeUnknownUnit]

        for u in units:
            res, ok = QgsUnitTypes.decodeVolumeUnit(QgsUnitTypes.encodeUnit(u))
            assert ok
            self.assertEqual(res, u)

        # Test decoding bad units
        res, ok = QgsUnitTypes.decodeVolumeUnit('bad')
        self.assertFalse(ok)
        self.assertEqual(res, QgsUnitTypes.VolumeUnknownUnit)

        # Test that string is cleaned before decoding
        res, ok = QgsUnitTypes.decodeVolumeUnit(' bbl  ')
        assert ok
        self.assertEqual(res, QgsUnitTypes.VolumeBarrel)

    def testVolumeUnitsToFromString(self):
        """Test converting volume units to and from translated strings"""
        units = [QgsUnitTypes.VolumeCubicMeters,
                 QgsUnitTypes.VolumeCubicFeet,
                 QgsUnitTypes.VolumeCubicYards,
                 QgsUnitTypes.VolumeBarrel,
                 QgsUnitTypes.VolumeCubicDecimeter,
                 QgsUnitTypes.VolumeLiters,
                 QgsUnitTypes.VolumeGallonUS,
                 QgsUnitTypes.VolumeCubicInch,
                 QgsUnitTypes.VolumeCubicCentimeter,
                 QgsUnitTypes.VolumeCubicDegrees,
                 QgsUnitTypes.VolumeUnknownUnit]

        for u in units:
            res, ok = QgsUnitTypes.stringToVolumeUnit(QgsUnitTypes.toString(u))
            assert ok
            self.assertEqual(res, u)

        # Test converting bad strings
        res, ok = QgsUnitTypes.stringToVolumeUnit('bad')
        self.assertFalse(ok)
        self.assertEqual(res, QgsUnitTypes.VolumeUnknownUnit)

        # Test that string is cleaned before conversion
        res, ok = QgsUnitTypes.stringToVolumeUnit(' {}  '.format(QgsUnitTypes.toString(QgsUnitTypes.VolumeBarrel).upper()))
        assert ok
        self.assertEqual(res, QgsUnitTypes.VolumeBarrel)

    def testEncodeDecodeTemporalUnits(self):
        """Test encoding and decoding temporal units"""
        units = [QgsUnitTypes.TemporalMilliseconds,
                 QgsUnitTypes.TemporalSeconds,
                 QgsUnitTypes.TemporalMinutes,
                 QgsUnitTypes.TemporalHours,
                 QgsUnitTypes.TemporalDays,
                 QgsUnitTypes.TemporalWeeks,
                 QgsUnitTypes.TemporalMonths,
                 QgsUnitTypes.TemporalYears,
                 QgsUnitTypes.TemporalDecades,
                 QgsUnitTypes.TemporalCenturies,
                 QgsUnitTypes.TemporalIrregularStep,
                 QgsUnitTypes.TemporalUnknownUnit]

        for u in units:
            res, ok = QgsUnitTypes.decodeTemporalUnit(QgsUnitTypes.encodeUnit(u))
            assert ok
            self.assertEqual(res, u)

        # Test decoding bad units
        res, ok = QgsUnitTypes.decodeTemporalUnit('bad')
        self.assertFalse(ok)
        self.assertEqual(res, QgsUnitTypes.TemporalUnknownUnit)

        # Test that string is cleaned before decoding
        res, ok = QgsUnitTypes.decodeTemporalUnit(' min  ')
        assert ok
        self.assertEqual(res, QgsUnitTypes.TemporalMinutes)

    def testTemporalUnitsToFromString(self):
        """Test converting temporal units to and from translated strings"""
        units = [QgsUnitTypes.TemporalMilliseconds,
                 QgsUnitTypes.TemporalSeconds,
                 QgsUnitTypes.TemporalMinutes,
                 QgsUnitTypes.TemporalHours,
                 QgsUnitTypes.TemporalDays,
                 QgsUnitTypes.TemporalWeeks,
                 QgsUnitTypes.TemporalMonths,
                 QgsUnitTypes.TemporalYears,
                 QgsUnitTypes.TemporalDecades,
                 QgsUnitTypes.TemporalCenturies,
                 QgsUnitTypes.TemporalIrregularStep,
                 QgsUnitTypes.TemporalUnknownUnit]

        for u in units:
            res, ok = QgsUnitTypes.stringToTemporalUnit(QgsUnitTypes.toString(u))
            assert ok
            self.assertEqual(res, u)

        # Test converting bad strings
        res, ok = QgsUnitTypes.stringToTemporalUnit('bad')
        self.assertFalse(ok)
        self.assertEqual(res, QgsUnitTypes.TemporalUnknownUnit)

        # Test that string is cleaned before conversion
        res, ok = QgsUnitTypes.stringToTemporalUnit(' {}  '.format(QgsUnitTypes.toString(QgsUnitTypes.TemporalDecades).upper()))
        assert ok
        self.assertEqual(res, QgsUnitTypes.TemporalDecades)

    def testEncodeDecodeRenderUnits(self):
        """Test encoding and decoding render units"""
        units = [QgsUnitTypes.RenderMillimeters,
                 QgsUnitTypes.RenderMetersInMapUnits,
                 QgsUnitTypes.RenderMapUnits,
                 QgsUnitTypes.RenderPixels,
                 QgsUnitTypes.RenderPercentage,
                 QgsUnitTypes.RenderPoints,
                 QgsUnitTypes.RenderInches]

        for u in units:
            res, ok = QgsUnitTypes.decodeRenderUnit(QgsUnitTypes.encodeUnit(u))
            assert ok
            self.assertEqual(res, u)

        # Test decoding bad units
        res, ok = QgsUnitTypes.decodeRenderUnit('bad')
        self.assertFalse(ok)
        # default units should be MM
        self.assertEqual(res, QgsUnitTypes.RenderMillimeters)

        # Test that string is cleaned before decoding
        res, ok = QgsUnitTypes.decodeRenderUnit(' PiXeL  ')
        assert ok
        self.assertEqual(res, QgsUnitTypes.RenderPixels)

        # check some aliases - used in data defined labeling
        res, ok = QgsUnitTypes.decodeRenderUnit('Meters')
        assert ok
        res, ok = QgsUnitTypes.decodeRenderUnit('MapUnits')
        assert ok
        self.assertEqual(res, QgsUnitTypes.RenderMapUnits)
        res, ok = QgsUnitTypes.decodeRenderUnit('Percent')
        assert ok
        self.assertEqual(res, QgsUnitTypes.RenderPercentage)
        res, ok = QgsUnitTypes.decodeRenderUnit('Points')
        assert ok
        self.assertEqual(res, QgsUnitTypes.RenderPoints)

    def testRenderUnitsString(self):
        """Test converting render units to strings"""
        units = [QgsUnitTypes.RenderMillimeters,
                 QgsUnitTypes.RenderMapUnits,
                 QgsUnitTypes.RenderPixels,
                 QgsUnitTypes.RenderPercentage,
                 QgsUnitTypes.RenderPoints,
                 QgsUnitTypes.RenderInches]

        for u in units:
            self.assertTrue(QgsUnitTypes.toString(u))

    def testFromUnitToUnitFactor(self):
        """Test calculation of conversion factor between units"""

        expected = {
            QgsUnitTypes.DistanceMeters: {
                QgsUnitTypes.DistanceMeters: 1.0,
                QgsUnitTypes.DistanceKilometers: 0.001,
                QgsUnitTypes.DistanceFeet: 3.28083989501,
                QgsUnitTypes.DistanceYards: 1.0936133,
                QgsUnitTypes.DistanceMiles: 0.00062136931818182,
                QgsUnitTypes.DistanceDegrees: 0.00000898315,
                QgsUnitTypes.DistanceNauticalMiles: 0.000539957,
                QgsUnitTypes.DistanceMillimeters: 1000.0,
                QgsUnitTypes.DistanceCentimeters: 100.0
            },
            QgsUnitTypes.DistanceKilometers: {
                QgsUnitTypes.DistanceMeters: 1000.0,
                QgsUnitTypes.DistanceKilometers: 1.0,
                QgsUnitTypes.DistanceFeet: 3280.8398950,
                QgsUnitTypes.DistanceYards: 1093.6132983,
                QgsUnitTypes.DistanceMiles: 0.62137121212119317271,
                QgsUnitTypes.DistanceDegrees: 0.0089832,
                QgsUnitTypes.DistanceNauticalMiles: 0.53995682073432482717,
                QgsUnitTypes.DistanceMillimeters: 1000000.0,
                QgsUnitTypes.DistanceCentimeters: 100000.0
            },
            QgsUnitTypes.DistanceFeet: {
                QgsUnitTypes.DistanceMeters: 0.3048,
                QgsUnitTypes.DistanceKilometers: 0.0003048,
                QgsUnitTypes.DistanceFeet: 1.0,
                QgsUnitTypes.DistanceYards: 0.3333333,
                QgsUnitTypes.DistanceMiles: 0.00018939375,
                QgsUnitTypes.DistanceDegrees: 2.73806498599629E-06,
                QgsUnitTypes.DistanceNauticalMiles: 0.000164579,
                QgsUnitTypes.DistanceMillimeters: 304.8,
                QgsUnitTypes.DistanceCentimeters: 30.48
            },
            QgsUnitTypes.DistanceYards: {
                QgsUnitTypes.DistanceMeters: 0.9144,
                QgsUnitTypes.DistanceKilometers: 0.0009144,
                QgsUnitTypes.DistanceFeet: 3.0,
                QgsUnitTypes.DistanceYards: 1.0,
                QgsUnitTypes.DistanceMiles: 0.000568182,
                QgsUnitTypes.DistanceDegrees: 0.0000082,
                QgsUnitTypes.DistanceNauticalMiles: 0.0004937366590756,
                QgsUnitTypes.DistanceMillimeters: 914.4,
                QgsUnitTypes.DistanceCentimeters: 91.44
            },
            QgsUnitTypes.DistanceDegrees: {
                QgsUnitTypes.DistanceMeters: 111319.49079327358,
                QgsUnitTypes.DistanceKilometers: 111.3194908,
                QgsUnitTypes.DistanceFeet: 365221.4264871,
                QgsUnitTypes.DistanceYards: 121740.4754957,
                QgsUnitTypes.DistanceMiles: 69.1707247,
                QgsUnitTypes.DistanceDegrees: 1.0,
                QgsUnitTypes.DistanceNauticalMiles: 60.1077164,
                QgsUnitTypes.DistanceMillimeters: 111319490.79327358,
                QgsUnitTypes.DistanceCentimeters: 11131949.079327358
            },
            QgsUnitTypes.DistanceMiles: {
                QgsUnitTypes.DistanceMeters: 1609.3440000,
                QgsUnitTypes.DistanceKilometers: 1.6093440,
                QgsUnitTypes.DistanceFeet: 5280.0000000,
                QgsUnitTypes.DistanceYards: 1760.0000000,
                QgsUnitTypes.DistanceMiles: 1.0,
                QgsUnitTypes.DistanceDegrees: 0.0144570,
                QgsUnitTypes.DistanceNauticalMiles: 0.8689762,
                QgsUnitTypes.DistanceMillimeters: 1609344.0,
                QgsUnitTypes.DistanceCentimeters: 160934.4
            },
            QgsUnitTypes.DistanceNauticalMiles: {
                QgsUnitTypes.DistanceMeters: 1852.0,
                QgsUnitTypes.DistanceKilometers: 1.8520000,
                QgsUnitTypes.DistanceFeet: 6076.1154856,
                QgsUnitTypes.DistanceYards: 2025.3718285,
                QgsUnitTypes.DistanceMiles: 1.1507794,
                QgsUnitTypes.DistanceDegrees: 0.0166367990650,
                QgsUnitTypes.DistanceNauticalMiles: 1.0,
                QgsUnitTypes.DistanceMillimeters: 1852000.0,
                QgsUnitTypes.DistanceCentimeters: 185200.0
            },
            QgsUnitTypes.DistanceMillimeters: {
                QgsUnitTypes.DistanceMeters: 0.001,
                QgsUnitTypes.DistanceKilometers: 0.000001,
                QgsUnitTypes.DistanceFeet: 0.00328083989501,
                QgsUnitTypes.DistanceYards: 0.0010936133,
                QgsUnitTypes.DistanceMiles: 0.00000062136931818182,
                QgsUnitTypes.DistanceDegrees: 0.00000000898315,
                QgsUnitTypes.DistanceNauticalMiles: 0.000000539957,
                QgsUnitTypes.DistanceMillimeters: 1.0,
                QgsUnitTypes.DistanceCentimeters: 0.1
            },
            QgsUnitTypes.DistanceCentimeters: {
                QgsUnitTypes.DistanceMeters: 0.01,
                QgsUnitTypes.DistanceKilometers: 0.00001,
                QgsUnitTypes.DistanceFeet: 0.0328083989501,
                QgsUnitTypes.DistanceYards: 0.010936133,
                QgsUnitTypes.DistanceMiles: 0.0000062136931818182,
                QgsUnitTypes.DistanceDegrees: 0.0000000898315,
                QgsUnitTypes.DistanceNauticalMiles: 0.00000539957,
                QgsUnitTypes.DistanceMillimeters: 10.0,
                QgsUnitTypes.DistanceCentimeters: 1.0
            },
            QgsUnitTypes.DistanceUnknownUnit: {
                QgsUnitTypes.DistanceMeters: 1.0,
                QgsUnitTypes.DistanceKilometers: 1.0,
                QgsUnitTypes.DistanceFeet: 1.0,
                QgsUnitTypes.DistanceYards: 1.0,
                QgsUnitTypes.DistanceMiles: 1.0,
                QgsUnitTypes.DistanceDegrees: 1.0,
                QgsUnitTypes.DistanceNauticalMiles: 1.0,
                QgsUnitTypes.DistanceMillimeters: 1.0,
                QgsUnitTypes.DistanceCentimeters: 1.0
            },
        }

        for from_unit in list(expected.keys()):
            for to_unit in list(expected[from_unit].keys()):
                expected_factor = expected[from_unit][to_unit]
                res = QgsUnitTypes.fromUnitToUnitFactor(from_unit, to_unit)
                self.assertAlmostEqual(res,
                                       expected_factor,
                                       msg='got {:.7f}, expected {:.7f} when converting from {} to {}'.format(res, expected_factor,
                                                                                                              QgsUnitTypes.toString(from_unit),
                                                                                                              QgsUnitTypes.toString(to_unit)))
                # test conversion to unknown units
                res = QgsUnitTypes.fromUnitToUnitFactor(from_unit, QgsUnitTypes.DistanceUnknownUnit)
                self.assertAlmostEqual(res,
                                       1.0,
                                       msg='got {:.7f}, expected 1.0 when converting from {} to unknown units'.format(res, QgsUnitTypes.toString(from_unit)))

    def testAreaFromUnitToUnitFactor(self):
        """Test calculation of conversion factor between areal units"""

        expected = {
            QgsUnitTypes.AreaSquareMeters: {
                QgsUnitTypes.AreaSquareMeters: 1.0,
                QgsUnitTypes.AreaSquareKilometers: 1e-6,
                QgsUnitTypes.AreaSquareFeet: 10.7639104,
                QgsUnitTypes.AreaSquareYards: 1.19599,
                QgsUnitTypes.AreaSquareMiles: 3.86102e-7,
                QgsUnitTypes.AreaHectares: 0.0001,
                QgsUnitTypes.AreaAcres: 0.000247105,
                QgsUnitTypes.AreaSquareNauticalMiles: 2.91553e-7,
                QgsUnitTypes.AreaSquareDegrees: 0.000000000080697,
                QgsUnitTypes.AreaSquareMillimeters: 1e6,
                QgsUnitTypes.AreaSquareCentimeters: 1e4,
                QgsUnitTypes.AreaUnknownUnit: 1.0
            },
            QgsUnitTypes.AreaSquareKilometers: {
                QgsUnitTypes.AreaSquareMeters: 1e6,
                QgsUnitTypes.AreaSquareKilometers: 1,
                QgsUnitTypes.AreaSquareFeet: 10763910.4167097,
                QgsUnitTypes.AreaSquareYards: 1195990.04630108,
                QgsUnitTypes.AreaSquareMiles: 0.386102158,
                QgsUnitTypes.AreaHectares: 100,
                QgsUnitTypes.AreaAcres: 247.105381467,
                QgsUnitTypes.AreaSquareNauticalMiles: 0.291553349598,
                QgsUnitTypes.AreaSquareDegrees: 0.000080697034968,
                QgsUnitTypes.AreaSquareMillimeters: 1e12,
                QgsUnitTypes.AreaSquareCentimeters: 1e10,
                QgsUnitTypes.AreaUnknownUnit: 1.0
            },
            QgsUnitTypes.AreaSquareFeet: {
                QgsUnitTypes.AreaSquareMeters: 0.092903,
                QgsUnitTypes.AreaSquareKilometers: 9.2903e-8,
                QgsUnitTypes.AreaSquareFeet: 1.0,
                QgsUnitTypes.AreaSquareYards: 0.11111111111,
                QgsUnitTypes.AreaSquareMiles: 3.58701e-8,
                QgsUnitTypes.AreaHectares: 9.2903e-6,
                QgsUnitTypes.AreaAcres: 2.29568e-5,
                QgsUnitTypes.AreaSquareNauticalMiles: 2.70862e-8,
                QgsUnitTypes.AreaSquareDegrees: 0.000000000007497,
                QgsUnitTypes.AreaSquareMillimeters: 92903.04,
                QgsUnitTypes.AreaSquareCentimeters: 929.0304,
                QgsUnitTypes.AreaUnknownUnit: 1.0
            },
            QgsUnitTypes.AreaSquareYards: {
                QgsUnitTypes.AreaSquareMeters: 0.836127360,
                QgsUnitTypes.AreaSquareKilometers: 8.36127e-7,
                QgsUnitTypes.AreaSquareFeet: 9.0,
                QgsUnitTypes.AreaSquareYards: 1.0,
                QgsUnitTypes.AreaSquareMiles: 3.22831e-7,
                QgsUnitTypes.AreaHectares: 8.3612736E-5,
                QgsUnitTypes.AreaAcres: 0.00020661157,
                QgsUnitTypes.AreaSquareNauticalMiles: 2.43776e-7,
                QgsUnitTypes.AreaSquareDegrees: 0.000000000067473,
                QgsUnitTypes.AreaSquareMillimeters: 836127.360,
                QgsUnitTypes.AreaSquareCentimeters: 8361.27360,
                QgsUnitTypes.AreaUnknownUnit: 1.0
            },
            QgsUnitTypes.AreaSquareMiles: {
                QgsUnitTypes.AreaSquareMeters: 2589988.110336,
                QgsUnitTypes.AreaSquareKilometers: 2.589988110,
                QgsUnitTypes.AreaSquareFeet: 27878400,
                QgsUnitTypes.AreaSquareYards: 3097600,
                QgsUnitTypes.AreaSquareMiles: 1.0,
                QgsUnitTypes.AreaHectares: 258.998811,
                QgsUnitTypes.AreaAcres: 640,
                QgsUnitTypes.AreaSquareNauticalMiles: 0.75511970898,
                QgsUnitTypes.AreaSquareDegrees: 0.000209004361107,
                QgsUnitTypes.AreaSquareMillimeters: 2589988110336.0,
                QgsUnitTypes.AreaSquareCentimeters: 25899881103.36,
                QgsUnitTypes.AreaUnknownUnit: 1.0
            },
            QgsUnitTypes.AreaHectares: {
                QgsUnitTypes.AreaSquareMeters: 10000,
                QgsUnitTypes.AreaSquareKilometers: 0.01,
                QgsUnitTypes.AreaSquareFeet: 107639.1041670972,
                QgsUnitTypes.AreaSquareYards: 11959.9004630,
                QgsUnitTypes.AreaSquareMiles: 0.00386102,
                QgsUnitTypes.AreaHectares: 1.0,
                QgsUnitTypes.AreaAcres: 2.471053814,
                QgsUnitTypes.AreaSquareNauticalMiles: 0.00291553,
                QgsUnitTypes.AreaSquareDegrees: 0.000000806970350,
                QgsUnitTypes.AreaSquareMillimeters: 10000000000.0,
                QgsUnitTypes.AreaSquareCentimeters: 100000000.0,
                QgsUnitTypes.AreaUnknownUnit: 1.0
            },
            QgsUnitTypes.AreaAcres: {
                QgsUnitTypes.AreaSquareMeters: 4046.8564224,
                QgsUnitTypes.AreaSquareKilometers: 0.00404686,
                QgsUnitTypes.AreaSquareFeet: 43560,
                QgsUnitTypes.AreaSquareYards: 4840,
                QgsUnitTypes.AreaSquareMiles: 0.0015625,
                QgsUnitTypes.AreaHectares: 0.404685642,
                QgsUnitTypes.AreaAcres: 1.0,
                QgsUnitTypes.AreaSquareNauticalMiles: 0.00117987,
                QgsUnitTypes.AreaSquareDegrees: 0.000000326569314,
                QgsUnitTypes.AreaSquareMillimeters: 4046856422.4000005,
                QgsUnitTypes.AreaSquareCentimeters: 40468564.224,
                QgsUnitTypes.AreaUnknownUnit: 1.0
            },
            QgsUnitTypes.AreaSquareNauticalMiles: {
                QgsUnitTypes.AreaSquareMeters: 3429904,
                QgsUnitTypes.AreaSquareKilometers: 3.4299040,
                QgsUnitTypes.AreaSquareFeet: 36919179.39391434,
                QgsUnitTypes.AreaSquareYards: 4102131.04376826,
                QgsUnitTypes.AreaSquareMiles: 1.324293337,
                QgsUnitTypes.AreaHectares: 342.9904000000,
                QgsUnitTypes.AreaAcres: 847.54773631,
                QgsUnitTypes.AreaSquareNauticalMiles: 1.0,
                QgsUnitTypes.AreaSquareDegrees: 0.000276783083025,
                QgsUnitTypes.AreaSquareMillimeters: 3429904000000.0,
                QgsUnitTypes.AreaSquareCentimeters: 34299040000.0,
                QgsUnitTypes.AreaUnknownUnit: 1.0
            },
            QgsUnitTypes.AreaSquareDegrees: {
                QgsUnitTypes.AreaSquareMeters: 12392029030.5,
                QgsUnitTypes.AreaSquareKilometers: 12392.029030499,
                QgsUnitTypes.AreaSquareFeet: 133386690365.5682220,
                QgsUnitTypes.AreaSquareYards: 14820743373.9520263,
                QgsUnitTypes.AreaSquareMiles: 4784.5891573967,
                QgsUnitTypes.AreaHectares: 1239202.903050,
                QgsUnitTypes.AreaAcres: 3062137.060733889,
                QgsUnitTypes.AreaSquareNauticalMiles: 3612.93757215,
                QgsUnitTypes.AreaSquareDegrees: 1.0,
                QgsUnitTypes.AreaSquareMillimeters: 12392029030500000.0,
                QgsUnitTypes.AreaSquareCentimeters: 123920290305000.0,
                QgsUnitTypes.AreaUnknownUnit: 1.0
            },
            QgsUnitTypes.AreaSquareMillimeters: {
                QgsUnitTypes.AreaSquareMeters: 1e-6,
                QgsUnitTypes.AreaSquareKilometers: 1e-12,
                QgsUnitTypes.AreaSquareFeet: 0.000010763910417,
                QgsUnitTypes.AreaSquareYards: 0.000001195990046,
                QgsUnitTypes.AreaSquareMiles: 3.861021585424458e-13,
                QgsUnitTypes.AreaHectares: 1e-10,
                QgsUnitTypes.AreaAcres: 2.471053814671653e-10,
                QgsUnitTypes.AreaSquareNauticalMiles: 2.9155334959812287e-13,
                QgsUnitTypes.AreaSquareDegrees: 8.069703496810251e-17,
                QgsUnitTypes.AreaSquareMillimeters: 1.0,
                QgsUnitTypes.AreaSquareCentimeters: 0.01,
                QgsUnitTypes.AreaUnknownUnit: 1.0
            },
            QgsUnitTypes.AreaSquareCentimeters: {
                QgsUnitTypes.AreaSquareMeters: 1e-4,
                QgsUnitTypes.AreaSquareKilometers: 1e-10,
                QgsUnitTypes.AreaSquareFeet: 0.0010763910417,
                QgsUnitTypes.AreaSquareYards: 0.0001195990046,
                QgsUnitTypes.AreaSquareMiles: 3.861021585424458e-11,
                QgsUnitTypes.AreaHectares: 1e-8,
                QgsUnitTypes.AreaAcres: 2.471053814671653e-8,
                QgsUnitTypes.AreaSquareNauticalMiles: 2.9155334959812287e-11,
                QgsUnitTypes.AreaSquareDegrees: 8.069703496810251e-15,
                QgsUnitTypes.AreaSquareMillimeters: 100,
                QgsUnitTypes.AreaSquareCentimeters: 1.0,
                QgsUnitTypes.AreaUnknownUnit: 1.0
            }
        }

        for from_unit in list(expected.keys()):
            for to_unit in list(expected[from_unit].keys()):
                expected_factor = expected[from_unit][to_unit]
                res = QgsUnitTypes.fromUnitToUnitFactor(from_unit, to_unit)
                self.assertAlmostEqual(res,
                                       expected_factor,
                                       msg='got {:.15f}, expected {:.15f} when converting from {} to {}'.format(res, expected_factor,
                                                                                                                QgsUnitTypes.toString(from_unit),
                                                                                                                QgsUnitTypes.toString(to_unit)))
                # test conversion to unknown units
                res = QgsUnitTypes.fromUnitToUnitFactor(from_unit, QgsUnitTypes.AreaUnknownUnit)
                self.assertAlmostEqual(res,
                                       1.0,
                                       msg='got {:.7f}, expected 1.0 when converting from {} to unknown units'.format(res, QgsUnitTypes.toString(from_unit)))

    def testDistanceToAreaUnit(self):
        """Test distanceToAreaUnit conversion"""
        expected = {QgsUnitTypes.DistanceMeters: QgsUnitTypes.AreaSquareMeters,
                    QgsUnitTypes.DistanceKilometers: QgsUnitTypes.AreaSquareKilometers,
                    QgsUnitTypes.DistanceFeet: QgsUnitTypes.AreaSquareFeet,
                    QgsUnitTypes.DistanceYards: QgsUnitTypes.AreaSquareYards,
                    QgsUnitTypes.DistanceMiles: QgsUnitTypes.AreaSquareMiles,
                    QgsUnitTypes.DistanceDegrees: QgsUnitTypes.AreaSquareDegrees,
                    QgsUnitTypes.DistanceCentimeters: QgsUnitTypes.AreaSquareCentimeters,
                    QgsUnitTypes.DistanceMillimeters: QgsUnitTypes.AreaSquareMillimeters,
                    QgsUnitTypes.DistanceUnknownUnit: QgsUnitTypes.AreaUnknownUnit,
                    QgsUnitTypes.DistanceNauticalMiles: QgsUnitTypes.AreaSquareNauticalMiles
                    }

        for t in list(expected.keys()):
            self.assertEqual(QgsUnitTypes.distanceToAreaUnit(t), expected[t])

    def testAreaToDistanceUnit(self):
        """Test areaToDistanceUnit conversion"""
        expected = {QgsUnitTypes.AreaSquareMeters: QgsUnitTypes.DistanceMeters,
                    QgsUnitTypes.AreaSquareKilometers: QgsUnitTypes.DistanceKilometers,
                    QgsUnitTypes.AreaSquareFeet: QgsUnitTypes.DistanceFeet,
                    QgsUnitTypes.AreaSquareYards: QgsUnitTypes.DistanceYards,
                    QgsUnitTypes.AreaSquareMiles: QgsUnitTypes.DistanceMiles,
                    QgsUnitTypes.AreaHectares: QgsUnitTypes.DistanceMeters,
                    QgsUnitTypes.AreaAcres: QgsUnitTypes.DistanceYards,
                    QgsUnitTypes.AreaSquareDegrees: QgsUnitTypes.DistanceDegrees,
                    QgsUnitTypes.AreaSquareCentimeters: QgsUnitTypes.DistanceCentimeters,
                    QgsUnitTypes.AreaSquareMillimeters: QgsUnitTypes.DistanceMillimeters,
                    QgsUnitTypes.AreaUnknownUnit: QgsUnitTypes.DistanceUnknownUnit,
                    QgsUnitTypes.AreaSquareNauticalMiles: QgsUnitTypes.DistanceNauticalMiles
                    }

        for t in list(expected.keys()):
            self.assertEqual(QgsUnitTypes.areaToDistanceUnit(t), expected[t])

    def testVolumeFromUnitToUnitFactor(self):
        """Test calculation of conversion factor between volume units"""

        expected = {
            QgsUnitTypes.VolumeCubicMeters: {
                QgsUnitTypes.VolumeCubicMeters: 1.0,
                QgsUnitTypes.VolumeCubicFeet: 35.314666572222,
                QgsUnitTypes.VolumeCubicYards: 1.307950613786,
                QgsUnitTypes.VolumeBarrel: 6.2898107438466,
                QgsUnitTypes.VolumeCubicDecimeter: 1000,
                QgsUnitTypes.VolumeLiters: 1000,
                QgsUnitTypes.VolumeGallonUS: 264.17205124156,
                QgsUnitTypes.VolumeCubicInch: 61023.7438368,
                QgsUnitTypes.VolumeCubicCentimeter: 1000000,
                QgsUnitTypes.VolumeCubicDegrees: 7.24913798948971e-16,
                QgsUnitTypes.VolumeUnknownUnit: 1.0
            },
            QgsUnitTypes.VolumeCubicFeet: {
                QgsUnitTypes.VolumeCubicMeters: 0.0283168,
                QgsUnitTypes.VolumeCubicFeet: 1.0,
                QgsUnitTypes.VolumeCubicYards: 0.037037,
                QgsUnitTypes.VolumeBarrel: 0.178107622,
                QgsUnitTypes.VolumeCubicDecimeter: 28.31685,
                QgsUnitTypes.VolumeLiters: 28.31685,
                QgsUnitTypes.VolumeGallonUS: 7.48052,
                QgsUnitTypes.VolumeCubicInch: 1728.000629765,
                QgsUnitTypes.VolumeCubicCentimeter: 28316.85,
                QgsUnitTypes.VolumeCubicDegrees: 2.0527272837261945e-17,
                QgsUnitTypes.VolumeUnknownUnit: 1.0
            },
            QgsUnitTypes.VolumeCubicYards: {
                QgsUnitTypes.VolumeCubicMeters: 0.7645549,
                QgsUnitTypes.VolumeCubicFeet: 26.999998234,
                QgsUnitTypes.VolumeCubicYards: 1.0,
                QgsUnitTypes.VolumeBarrel: 4.808905491,
                QgsUnitTypes.VolumeCubicDecimeter: 764.5549,
                QgsUnitTypes.VolumeLiters: 764.5549,
                QgsUnitTypes.VolumeGallonUS: 201.974025549,
                QgsUnitTypes.VolumeCubicInch: 46656.013952472,
                QgsUnitTypes.VolumeCubicCentimeter: 764554.9,
                QgsUnitTypes.VolumeCubicDegrees: 5.542363970640507e-16,
                QgsUnitTypes.VolumeUnknownUnit: 1.0
            },
            QgsUnitTypes.VolumeBarrel: {
                QgsUnitTypes.VolumeCubicMeters: 0.158987294928,
                QgsUnitTypes.VolumeCubicFeet: 5.614582837,
                QgsUnitTypes.VolumeCubicYards: 0.207947526,
                QgsUnitTypes.VolumeBarrel: 1.0,
                QgsUnitTypes.VolumeCubicDecimeter: 158.9873,
                QgsUnitTypes.VolumeLiters: 158.9873,
                QgsUnitTypes.VolumeGallonUS: 41.999998943,
                QgsUnitTypes.VolumeCubicInch: 9702.002677722,
                QgsUnitTypes.VolumeCubicCentimeter: 158987.3,
                QgsUnitTypes.VolumeCubicDegrees: 1.1525208762763973e-16,
                QgsUnitTypes.VolumeUnknownUnit: 1.0
            },
            QgsUnitTypes.VolumeCubicDecimeter: {
                QgsUnitTypes.VolumeCubicMeters: 0.001,
                QgsUnitTypes.VolumeCubicFeet: 0.0353147,
                QgsUnitTypes.VolumeCubicYards: 0.00130795,
                QgsUnitTypes.VolumeBarrel: 0.00628981,
                QgsUnitTypes.VolumeCubicDecimeter: 1.0,
                QgsUnitTypes.VolumeLiters: 1.0,
                QgsUnitTypes.VolumeGallonUS: 0.264172,
                QgsUnitTypes.VolumeCubicInch: 61.02375899,
                QgsUnitTypes.VolumeCubicCentimeter: 1000,
                QgsUnitTypes.VolumeCubicDegrees: 7.24913798948971e-19,
                QgsUnitTypes.VolumeUnknownUnit: 1.0
            },
            QgsUnitTypes.VolumeLiters: {
                QgsUnitTypes.VolumeCubicMeters: 0.001,
                QgsUnitTypes.VolumeCubicFeet: 0.0353147,
                QgsUnitTypes.VolumeCubicYards: 0.00130795,
                QgsUnitTypes.VolumeBarrel: 0.00628981,
                QgsUnitTypes.VolumeCubicDecimeter: 1.0,
                QgsUnitTypes.VolumeLiters: 1.0,
                QgsUnitTypes.VolumeGallonUS: 0.264172,
                QgsUnitTypes.VolumeCubicInch: 61.02375899,
                QgsUnitTypes.VolumeCubicCentimeter: 1000,
                QgsUnitTypes.VolumeCubicDegrees: 7.24913798948971e-19,
                QgsUnitTypes.VolumeUnknownUnit: 1.0
            },
            QgsUnitTypes.VolumeGallonUS: {
                QgsUnitTypes.VolumeCubicMeters: 0.00378541,
                QgsUnitTypes.VolumeCubicFeet: 0.133680547,
                QgsUnitTypes.VolumeCubicYards: 0.00495113,
                QgsUnitTypes.VolumeBarrel: 0.023809524,
                QgsUnitTypes.VolumeCubicDecimeter: 3.785412,
                QgsUnitTypes.VolumeLiters: 3.785412,
                QgsUnitTypes.VolumeGallonUS: 1.0,
                QgsUnitTypes.VolumeCubicInch: 231.000069567,
                QgsUnitTypes.VolumeCubicCentimeter: 3785.412,
                QgsUnitTypes.VolumeCubicDegrees: 2.7440973935070226e-18,
                QgsUnitTypes.VolumeUnknownUnit: 1.0
            },
            QgsUnitTypes.VolumeCubicInch: {
                QgsUnitTypes.VolumeCubicMeters: 1.63871e-5,
                QgsUnitTypes.VolumeCubicFeet: 0.000578704,
                QgsUnitTypes.VolumeCubicYards: 2.14335e-5,
                QgsUnitTypes.VolumeBarrel: 0.000103072,
                QgsUnitTypes.VolumeCubicDecimeter: 0.0163871,
                QgsUnitTypes.VolumeLiters: 0.0163871,
                QgsUnitTypes.VolumeGallonUS: 0.004329,
                QgsUnitTypes.VolumeCubicInch: 1.0,
                QgsUnitTypes.VolumeCubicCentimeter: 16.38706,
                QgsUnitTypes.VolumeCubicDegrees: 1.187916242337679e-20,
                QgsUnitTypes.VolumeUnknownUnit: 1.0
            },
            QgsUnitTypes.VolumeCubicCentimeter: {
                QgsUnitTypes.VolumeCubicMeters: 1e-6,
                QgsUnitTypes.VolumeCubicFeet: 3.53147e-5,
                QgsUnitTypes.VolumeCubicYards: 1.30795e-6,
                QgsUnitTypes.VolumeBarrel: 6.28981e-6,
                QgsUnitTypes.VolumeCubicDecimeter: 0.001,
                QgsUnitTypes.VolumeLiters: 0.001,
                QgsUnitTypes.VolumeGallonUS: 0.000264172,
                QgsUnitTypes.VolumeCubicInch: 0.061023759,
                QgsUnitTypes.VolumeCubicCentimeter: 1.0,
                QgsUnitTypes.VolumeCubicDegrees: 7.24913798948971e-22,
                QgsUnitTypes.VolumeUnknownUnit: 1.0
            },
            QgsUnitTypes.VolumeCubicDegrees: {
                QgsUnitTypes.VolumeCubicMeters: 1379474361572186.2500000,
                QgsUnitTypes.VolumeCubicFeet: 39062363874236.74,
                QgsUnitTypes.VolumeCubicYards: 1054683882564386.8,
                QgsUnitTypes.VolumeBarrel: 219318904165585.66,
                QgsUnitTypes.VolumeCubicDecimeter: 1379474361572.1863,
                QgsUnitTypes.VolumeLiters: 1379474361572.1863,
                QgsUnitTypes.VolumeGallonUS: 5221878801987.693,
                QgsUnitTypes.VolumeCubicInch: 22605446363.083416,
                QgsUnitTypes.VolumeCubicCentimeter: 1379474361.5721862,
                QgsUnitTypes.VolumeCubicDegrees: 1.0,
                QgsUnitTypes.VolumeUnknownUnit: 1.0
            }
        }

        for from_unit in list(expected.keys()):
            for to_unit in list(expected[from_unit].keys()):
                expected_factor = expected[from_unit][to_unit]
                res = QgsUnitTypes.fromUnitToUnitFactor(from_unit, to_unit)
                self.assertAlmostEqual(res,
                                       expected_factor,
                                       msg='got {:.15f}, expected {:.15f} when converting from {} to {}'.format(res, expected_factor,
                                                                                                                QgsUnitTypes.toString(from_unit),
                                                                                                                QgsUnitTypes.toString(to_unit)))
                # test conversion to unknown units
                res = QgsUnitTypes.fromUnitToUnitFactor(from_unit, QgsUnitTypes.VolumeUnknownUnit)
                self.assertAlmostEqual(res,
                                       1.0,
                                       msg='got {:.7f}, expected 1.0 when converting from {} to unknown units'.format(res, QgsUnitTypes.toString(from_unit)))

    def testTemporalFromUnitToUnitFactor(self):
        """Test calculation of conversion factor between temporal units"""

        expected = {
            QgsUnitTypes.TemporalMilliseconds: {
                QgsUnitTypes.TemporalMilliseconds: 1.0,
                QgsUnitTypes.TemporalSeconds: 0.001,
                QgsUnitTypes.TemporalMinutes: 1.66667e-5,
                QgsUnitTypes.TemporalHours: 2.7777777777777776e-07,
                QgsUnitTypes.TemporalDays: 1.157554211999884014e-8,
                QgsUnitTypes.TemporalWeeks: 1.65344e-9,
                QgsUnitTypes.TemporalMonths: 3.805172816249e-10,
                QgsUnitTypes.TemporalYears: 3.170980821917834278e-11,
                QgsUnitTypes.TemporalDecades: 3.170980821917834117e-12,
                QgsUnitTypes.TemporalCenturies: 3.170980821917834319e-13,
                QgsUnitTypes.TemporalUnknownUnit: 1.0,
                QgsUnitTypes.TemporalIrregularStep: 1.0,
            },
            QgsUnitTypes.TemporalSeconds: {
                QgsUnitTypes.TemporalMilliseconds: 1000.0,
                QgsUnitTypes.TemporalSeconds: 1,
                QgsUnitTypes.TemporalMinutes: 0.016666675200000136831,
                QgsUnitTypes.TemporalHours: 0.00027777792000000228051,
                QgsUnitTypes.TemporalDays: 1.157408000000009502e-5,
                QgsUnitTypes.TemporalWeeks: 1.653440000000013514e-6,
                QgsUnitTypes.TemporalMonths: 3.805172816248999917e-7,
                QgsUnitTypes.TemporalYears: 3.170980821917834046e-8,
                QgsUnitTypes.TemporalDecades: 3.170980821917834046e-9,
                QgsUnitTypes.TemporalCenturies: 3.170980821917834149e-10,
                QgsUnitTypes.TemporalUnknownUnit: 1.0,
                QgsUnitTypes.TemporalIrregularStep: 1.0,
            },
            QgsUnitTypes.TemporalMinutes: {
                QgsUnitTypes.TemporalMilliseconds: 60000.0,
                QgsUnitTypes.TemporalSeconds: 60,
                QgsUnitTypes.TemporalMinutes: 1,
                QgsUnitTypes.TemporalHours: 0.016666666666666666,
                QgsUnitTypes.TemporalDays: 0.0006944444444444445,
                QgsUnitTypes.TemporalWeeks: 9.921893245713293505e-5,
                QgsUnitTypes.TemporalMonths: 2.3148148148148147e-05,
                QgsUnitTypes.TemporalYears: 1.902828841643645226e-6,
                QgsUnitTypes.TemporalDecades: 1.902828841643645332e-7,
                QgsUnitTypes.TemporalCenturies: 1.9028288416436452e-8,
                QgsUnitTypes.TemporalUnknownUnit: 1.0,
                QgsUnitTypes.TemporalIrregularStep: 1.0,
            },
            QgsUnitTypes.TemporalHours: {
                QgsUnitTypes.TemporalMilliseconds: 3600000.0,
                QgsUnitTypes.TemporalSeconds: 3600,
                QgsUnitTypes.TemporalMinutes: 60,
                QgsUnitTypes.TemporalHours: 1,
                QgsUnitTypes.TemporalDays: 0.041666700000240003421,
                QgsUnitTypes.TemporalWeeks: 0.0059523857143200008604,
                QgsUnitTypes.TemporalMonths: 0.001388888888888889,
                QgsUnitTypes.TemporalYears: 0.00011407711613050422,
                QgsUnitTypes.TemporalDecades: 1.141553424664109737e-5,
                QgsUnitTypes.TemporalCenturies: 1.141553424664109737e-6,
                QgsUnitTypes.TemporalUnknownUnit: 1.0,
                QgsUnitTypes.TemporalIrregularStep: 1.0,
            },
            QgsUnitTypes.TemporalDays: {
                QgsUnitTypes.TemporalMilliseconds: 8.64e+7,
                QgsUnitTypes.TemporalSeconds: 86400,
                QgsUnitTypes.TemporalMinutes: 1440,
                QgsUnitTypes.TemporalHours: 24,
                QgsUnitTypes.TemporalDays: 1,
                QgsUnitTypes.TemporalWeeks: 0.14285714285714285,
                QgsUnitTypes.TemporalMonths: 0.03333333333333333,
                QgsUnitTypes.TemporalYears: 0.0027378507871321013,
                QgsUnitTypes.TemporalDecades: 0.0002737850787132101,
                QgsUnitTypes.TemporalCenturies: 2.739723287683189167e-5,
                QgsUnitTypes.TemporalUnknownUnit: 1.0,
                QgsUnitTypes.TemporalIrregularStep: 1.0,
            },
            QgsUnitTypes.TemporalWeeks: {
                QgsUnitTypes.TemporalMilliseconds: 6.048e+8,
                QgsUnitTypes.TemporalSeconds: 604800,
                QgsUnitTypes.TemporalMinutes: 10080,
                QgsUnitTypes.TemporalHours: 168,
                QgsUnitTypes.TemporalDays: 7,
                QgsUnitTypes.TemporalWeeks: 1,
                QgsUnitTypes.TemporalMonths: 0.23333333333333334,
                QgsUnitTypes.TemporalYears: 0.019164955509924708,
                QgsUnitTypes.TemporalDecades: 0.0019164955509924709,
                QgsUnitTypes.TemporalCenturies: 0.0001916495550992471,
                QgsUnitTypes.TemporalUnknownUnit: 1.0,
                QgsUnitTypes.TemporalIrregularStep: 1.0,
            },
            QgsUnitTypes.TemporalMonths: {
                QgsUnitTypes.TemporalMilliseconds: 2592000000.0,
                QgsUnitTypes.TemporalSeconds: 2592000.0,
                QgsUnitTypes.TemporalMinutes: 43200.0,
                QgsUnitTypes.TemporalHours: 720.0,
                QgsUnitTypes.TemporalDays: 30.0,
                QgsUnitTypes.TemporalWeeks: 4.285714285714286,
                QgsUnitTypes.TemporalMonths: 1,
                QgsUnitTypes.TemporalYears: 0.08213552361396304,
                QgsUnitTypes.TemporalDecades: 0.008213552361396304,
                QgsUnitTypes.TemporalCenturies: 0.0008213552361396304,
                QgsUnitTypes.TemporalUnknownUnit: 1.0,
                QgsUnitTypes.TemporalIrregularStep: 1.0,
            },
            QgsUnitTypes.TemporalYears: {
                QgsUnitTypes.TemporalMilliseconds: 31557600000.0,
                QgsUnitTypes.TemporalSeconds: 31557600.0,
                QgsUnitTypes.TemporalMinutes: 525960.0,
                QgsUnitTypes.TemporalHours: 8766.0,
                QgsUnitTypes.TemporalDays: 365.25,
                QgsUnitTypes.TemporalWeeks: 52.17857142857143,
                QgsUnitTypes.TemporalMonths: 12.175,
                QgsUnitTypes.TemporalYears: 1,
                QgsUnitTypes.TemporalDecades: 0.1,
                QgsUnitTypes.TemporalCenturies: 0.01,
                QgsUnitTypes.TemporalUnknownUnit: 1.0,
                QgsUnitTypes.TemporalIrregularStep: 1.0,
            },
            QgsUnitTypes.TemporalDecades: {
                QgsUnitTypes.TemporalMilliseconds: 315576000000.0,
                QgsUnitTypes.TemporalSeconds: 315576000.0,
                QgsUnitTypes.TemporalMinutes: 5259600.0,
                QgsUnitTypes.TemporalHours: 87660.0,
                QgsUnitTypes.TemporalDays: 3652.5,
                QgsUnitTypes.TemporalWeeks: 521.7857142857143,
                QgsUnitTypes.TemporalMonths: 121.75,
                QgsUnitTypes.TemporalYears: 10,
                QgsUnitTypes.TemporalDecades: 1,
                QgsUnitTypes.TemporalCenturies: 0.1,
                QgsUnitTypes.TemporalUnknownUnit: 1.0,
                QgsUnitTypes.TemporalIrregularStep: 1.0,
            },
            QgsUnitTypes.TemporalCenturies: {
                QgsUnitTypes.TemporalMilliseconds: 3155760000000.0,
                QgsUnitTypes.TemporalSeconds: 3155760000.0,
                QgsUnitTypes.TemporalMinutes: 52596000.0,
                QgsUnitTypes.TemporalHours: 876600.0,
                QgsUnitTypes.TemporalDays: 36525.0,
                QgsUnitTypes.TemporalWeeks: 5217.857142857143,
                QgsUnitTypes.TemporalMonths: 1217.5,
                QgsUnitTypes.TemporalYears: 100,
                QgsUnitTypes.TemporalDecades: 10,
                QgsUnitTypes.TemporalCenturies: 1,
                QgsUnitTypes.TemporalUnknownUnit: 1.0,
                QgsUnitTypes.TemporalIrregularStep: 1.0,
            }
        }

        for from_unit in list(expected.keys()):
            for to_unit in list(expected[from_unit].keys()):
                expected_factor = expected[from_unit][to_unit]
                res = QgsUnitTypes.fromUnitToUnitFactor(from_unit, to_unit)
                self.assertAlmostEqual(res,
                                       expected_factor,
                                       msg='got {:.15f}, expected {:.15f} when converting from {} to {}'.format(res, expected_factor,
                                                                                                                QgsUnitTypes.toString(from_unit),
                                                                                                                QgsUnitTypes.toString(to_unit)))
                # test conversion to unknown units
                res = QgsUnitTypes.fromUnitToUnitFactor(from_unit, QgsUnitTypes.TemporalUnknownUnit)
                self.assertAlmostEqual(res,
                                       1.0,
                                       msg='got {:.7f}, expected 1.0 when converting from {} to unknown units'.format(res, QgsUnitTypes.toString(from_unit)))
                res = QgsUnitTypes.fromUnitToUnitFactor(from_unit, QgsUnitTypes.TemporalIrregularStep)
                self.assertAlmostEqual(res,
                                       1.0,
                                       msg='got {:.7f}, expected 1.0 when converting from {} to unknown units'.format(res, QgsUnitTypes.toString(from_unit)))

    def testDistanceToVolumeUnit(self):
        """Test distanceToVolumeUnit conversion"""
        expected = {QgsUnitTypes.DistanceMeters: QgsUnitTypes.VolumeCubicMeters,
                    QgsUnitTypes.DistanceKilometers: QgsUnitTypes.VolumeCubicMeters,
                    QgsUnitTypes.DistanceFeet: QgsUnitTypes.VolumeCubicFeet,
                    QgsUnitTypes.DistanceYards: QgsUnitTypes.VolumeCubicYards,
                    QgsUnitTypes.DistanceMiles: QgsUnitTypes.VolumeCubicFeet,
                    QgsUnitTypes.DistanceDegrees: QgsUnitTypes.VolumeCubicDegrees,
                    QgsUnitTypes.DistanceCentimeters: QgsUnitTypes.VolumeCubicCentimeter,
                    QgsUnitTypes.DistanceMillimeters: QgsUnitTypes.VolumeCubicCentimeter,
                    QgsUnitTypes.DistanceUnknownUnit: QgsUnitTypes.VolumeUnknownUnit,
                    QgsUnitTypes.DistanceNauticalMiles: QgsUnitTypes.VolumeCubicFeet
                    }

        for t in list(expected.keys()):
            self.assertEqual(QgsUnitTypes.distanceToVolumeUnit(t), expected[t])

    def testVolumeToDistanceUnit(self):
        """Test volumeToDistanceUnit conversion"""
        expected = {QgsUnitTypes.VolumeCubicMeters: QgsUnitTypes.DistanceMeters,
                    QgsUnitTypes.VolumeCubicFeet: QgsUnitTypes.DistanceFeet,
                    QgsUnitTypes.VolumeCubicYards: QgsUnitTypes.DistanceYards,
                    QgsUnitTypes.VolumeBarrel: QgsUnitTypes.DistanceFeet,
                    QgsUnitTypes.VolumeCubicDecimeter: QgsUnitTypes.DistanceCentimeters,
                    QgsUnitTypes.VolumeLiters: QgsUnitTypes.DistanceMeters,
                    QgsUnitTypes.VolumeGallonUS: QgsUnitTypes.DistanceFeet,
                    QgsUnitTypes.VolumeCubicInch: QgsUnitTypes.DistanceFeet,
                    QgsUnitTypes.VolumeCubicCentimeter: QgsUnitTypes.DistanceCentimeters,
                    QgsUnitTypes.VolumeCubicDegrees: QgsUnitTypes.DistanceDegrees
                    }

        for t in list(expected.keys()):
            self.assertEqual(QgsUnitTypes.volumeToDistanceUnit(t), expected[t])

    def testEncodeDecodeAngleUnits(self):
        """Test encoding and decoding angle units"""
        units = [QgsUnitTypes.AngleDegrees,
                 QgsUnitTypes.AngleRadians,
                 QgsUnitTypes.AngleGon,
                 QgsUnitTypes.AngleMinutesOfArc,
                 QgsUnitTypes.AngleSecondsOfArc,
                 QgsUnitTypes.AngleTurn,
                 QgsUnitTypes.AngleMilliradiansSI,
                 QgsUnitTypes.AngleMilNATO,
                 QgsUnitTypes.AngleUnknownUnit]

        for u in units:
            res, ok = QgsUnitTypes.decodeAngleUnit(QgsUnitTypes.encodeUnit(u))
            assert ok, 'could not decode unit {}'.format(QgsUnitTypes.toString(u))
            self.assertEqual(res, u)

        # Test decoding bad units
        res, ok = QgsUnitTypes.decodeAngleUnit('bad')
        self.assertFalse(ok)
        self.assertEqual(res, QgsUnitTypes.AngleUnknownUnit)

        # Test that string is cleaned before decoding
        res, ok = QgsUnitTypes.decodeAngleUnit(' MoA  ')
        assert ok
        self.assertEqual(res, QgsUnitTypes.AngleMinutesOfArc)

    def testAngleToString(self):
        """Test converting angle unit to string"""
        units = [QgsUnitTypes.AngleDegrees,
                 QgsUnitTypes.AngleRadians,
                 QgsUnitTypes.AngleGon,
                 QgsUnitTypes.AngleMinutesOfArc,
                 QgsUnitTypes.AngleSecondsOfArc,
                 QgsUnitTypes.AngleTurn,
                 QgsUnitTypes.AngleMilliradiansSI,
                 QgsUnitTypes.AngleMilNATO,
                 QgsUnitTypes.AngleUnknownUnit]

        dupes = set()

        # can't test result as it may be translated, so make sure it's non-empty and not a duplicate
        for u in units:
            s = QgsUnitTypes.toString(u)
            assert len(s) > 0
            self.assertFalse(s in dupes)
            dupes.add(s)

    def testAngleFromUnitToUnitFactor(self):
        """Test calculation of conversion factor between angular units"""

        expected = {QgsUnitTypes.AngleDegrees: {QgsUnitTypes.AngleDegrees: 1.0, QgsUnitTypes.AngleRadians: 0.0174533, QgsUnitTypes.AngleGon: 1.1111111, QgsUnitTypes.AngleMinutesOfArc: 60, QgsUnitTypes.AngleSecondsOfArc: 3600, QgsUnitTypes.AngleTurn: 0.00277777777778, QgsUnitTypes.AngleMilliradiansSI: 17.453292519943297, QgsUnitTypes.AngleMilNATO: 17.77777777777778},
                    QgsUnitTypes.AngleRadians: {QgsUnitTypes.AngleDegrees: 57.2957795, QgsUnitTypes.AngleRadians: 1.0, QgsUnitTypes.AngleGon: 63.6619772, QgsUnitTypes.AngleMinutesOfArc: 3437.7467708, QgsUnitTypes.AngleSecondsOfArc: 206264.8062471, QgsUnitTypes.AngleTurn: 0.159154943092, QgsUnitTypes.AngleMilliradiansSI: 1000.0, QgsUnitTypes.AngleMilNATO: 1018.5916357881301},
                    QgsUnitTypes.AngleGon: {QgsUnitTypes.AngleDegrees: 0.9000000, QgsUnitTypes.AngleRadians: 0.015707968623450838802, QgsUnitTypes.AngleGon: 1.0, QgsUnitTypes.AngleMinutesOfArc: 54.0000000, QgsUnitTypes.AngleSecondsOfArc: 3240.0000000, QgsUnitTypes.AngleTurn: 0.0025, QgsUnitTypes.AngleMilliradiansSI: 15.707963267948967, QgsUnitTypes.AngleMilNATO: 16},
                    QgsUnitTypes.AngleMinutesOfArc: {QgsUnitTypes.AngleDegrees: 0.016666672633390722247, QgsUnitTypes.AngleRadians: 0.00029088831280398030638, QgsUnitTypes.AngleGon: 0.018518525464057963154, QgsUnitTypes.AngleMinutesOfArc: 1.0, QgsUnitTypes.AngleSecondsOfArc: 60.0, QgsUnitTypes.AngleTurn: 4.62962962962963e-05, QgsUnitTypes.AngleMilliradiansSI: 0.29088820866572157, QgsUnitTypes.AngleMilNATO: 0.29629629629629634},
                    QgsUnitTypes.AngleSecondsOfArc: {QgsUnitTypes.AngleDegrees: 0.00027777787722304257169, QgsUnitTypes.AngleRadians: 4.848138546730629518e-6, QgsUnitTypes.AngleGon: 0.0003086420910674814405, QgsUnitTypes.AngleMinutesOfArc: 0.016666672633325253783, QgsUnitTypes.AngleSecondsOfArc: 1.0, QgsUnitTypes.AngleTurn: 7.71604938271605e-07, QgsUnitTypes.AngleMilliradiansSI: 0.0048481482527009582897, QgsUnitTypes.AngleMilNATO: 0.0049382716049382715},
                    QgsUnitTypes.AngleTurn: {QgsUnitTypes.AngleDegrees: 360.0, QgsUnitTypes.AngleRadians: 6.2831853071795, QgsUnitTypes.AngleGon: 400.0, QgsUnitTypes.AngleMinutesOfArc: 21600, QgsUnitTypes.AngleSecondsOfArc: 1296000, QgsUnitTypes.AngleTurn: 1, QgsUnitTypes.AngleMilliradiansSI: 6283.185307179586, QgsUnitTypes.AngleMilNATO: 6400},
                    QgsUnitTypes.AngleMilliradiansSI: {QgsUnitTypes.AngleDegrees: 0.057295779513082325, QgsUnitTypes.AngleRadians: 0.001, QgsUnitTypes.AngleGon: 0.06366197723675814, QgsUnitTypes.AngleMinutesOfArc: 3.4377467707849396, QgsUnitTypes.AngleSecondsOfArc: 206.26480624709637, QgsUnitTypes.AngleTurn: 0.0015707963267948967, QgsUnitTypes.AngleMilliradiansSI: 1.0, QgsUnitTypes.AngleMilNATO: 1.0185916357881302},
                    QgsUnitTypes.AngleMilNATO: {QgsUnitTypes.AngleDegrees: 0.05625,
                                                QgsUnitTypes.AngleRadians: 0.0009817477042468104,
                                                QgsUnitTypes.AngleGon: 0.0625,
                                                QgsUnitTypes.AngleMinutesOfArc: 3.375,
                                                QgsUnitTypes.AngleSecondsOfArc: 202.5,
                                                QgsUnitTypes.AngleTurn: 0.000015625,
                                                QgsUnitTypes.AngleMilliradiansSI: 0.9817477042468102,
                                                QgsUnitTypes.AngleMilNATO: 1.0}
                    }

        for from_unit in list(expected.keys()):
            for to_unit in list(expected[from_unit].keys()):
                expected_factor = expected[from_unit][to_unit]
                res = QgsUnitTypes.fromUnitToUnitFactor(from_unit, to_unit)
                self.assertAlmostEqual(res,
                                       expected_factor,
                                       msg='got {:.7f}, expected {:.7f} when converting from {} to {}'.format(res, expected_factor,
                                                                                                              QgsUnitTypes.toString(from_unit),
                                                                                                              QgsUnitTypes.toString(to_unit)))
                # test conversion to unknown units
                res = QgsUnitTypes.fromUnitToUnitFactor(from_unit, QgsUnitTypes.AngleUnknownUnit)
                self.assertAlmostEqual(res,
                                       1.0,
                                       msg='got {:.7f}, expected 1.0 when converting from {} to unknown units'.format(res, QgsUnitTypes.toString(from_unit)))

    def testFormatAngle(self):
        """Test formatting angles"""
        self.assertEqual(QgsUnitTypes.formatAngle(45, 3, QgsUnitTypes.AngleDegrees), '45.000°')
        self.assertEqual(QgsUnitTypes.formatAngle(1, 2, QgsUnitTypes.AngleRadians), '1.00 rad')
        self.assertEqual(QgsUnitTypes.formatAngle(1, 0, QgsUnitTypes.AngleGon), '1 gon')
        self.assertEqual(QgsUnitTypes.formatAngle(1.11111111, 4, QgsUnitTypes.AngleMinutesOfArc), '1.1111′')
        self.assertEqual(QgsUnitTypes.formatAngle(1.99999999, 2, QgsUnitTypes.AngleSecondsOfArc), '2.00″')
        self.assertEqual(QgsUnitTypes.formatAngle(1, 2, QgsUnitTypes.AngleTurn), '1.00 tr')
        self.assertEqual(QgsUnitTypes.formatAngle(1, 2, QgsUnitTypes.AngleMilliradiansSI), '1.00 millirad')
        self.assertEqual(QgsUnitTypes.formatAngle(1, 2, QgsUnitTypes.AngleMilNATO), '1.00 mil')
        self.assertEqual(QgsUnitTypes.formatAngle(1, 2, QgsUnitTypes.AngleUnknownUnit), '1.00')

        self.assertEqual(QgsUnitTypes.formatAngle(45, -1, QgsUnitTypes.AngleDegrees), '45°')
        self.assertEqual(QgsUnitTypes.formatAngle(1, -1, QgsUnitTypes.AngleRadians), '1.00 rad')
        self.assertEqual(QgsUnitTypes.formatAngle(1, -1, QgsUnitTypes.AngleGon), '1 gon')
        self.assertEqual(QgsUnitTypes.formatAngle(1.11111111, -1, QgsUnitTypes.AngleMinutesOfArc), '1′')
        self.assertEqual(QgsUnitTypes.formatAngle(1.99999999, -1, QgsUnitTypes.AngleSecondsOfArc), '2″')
        self.assertEqual(QgsUnitTypes.formatAngle(1, -1, QgsUnitTypes.AngleTurn), '1.000 tr')
        self.assertEqual(QgsUnitTypes.formatAngle(1, -1, QgsUnitTypes.AngleMilliradiansSI), '1 millirad')
        self.assertEqual(QgsUnitTypes.formatAngle(1, -1, QgsUnitTypes.AngleMilNATO), '1 mil')
        self.assertEqual(QgsUnitTypes.formatAngle(1, -1, QgsUnitTypes.AngleUnknownUnit), '1.00')

    def testFormatDistance(self):
        """Test formatting distances"""
        # keep base unit
        self.assertEqual(QgsUnitTypes.formatDistance(100, 3, QgsUnitTypes.DistanceMeters, True), '100.000 m')
        self.assertEqual(QgsUnitTypes.formatDistance(10, 2, QgsUnitTypes.DistanceKilometers, True), '10.00 km')
        self.assertEqual(QgsUnitTypes.formatDistance(1, 0, QgsUnitTypes.DistanceFeet, True), '1 ft')
        self.assertEqual(QgsUnitTypes.formatDistance(1.11111111, 4, QgsUnitTypes.DistanceNauticalMiles, True), '1.1111 NM')
        self.assertEqual(QgsUnitTypes.formatDistance(1.99999999, 2, QgsUnitTypes.DistanceYards, True), '2.00 yd')
        self.assertEqual(QgsUnitTypes.formatDistance(1, 2, QgsUnitTypes.DistanceMiles, True), '1.00 mi')
        self.assertEqual(QgsUnitTypes.formatDistance(1, 2, QgsUnitTypes.DistanceDegrees, True), '1.00 deg')
        self.assertEqual(QgsUnitTypes.formatDistance(1, 2, QgsUnitTypes.DistanceCentimeters, True), '1.00 cm')
        self.assertEqual(QgsUnitTypes.formatDistance(1, 2, QgsUnitTypes.DistanceMillimeters, True), '1.00 mm')
        self.assertEqual(QgsUnitTypes.formatDistance(1, 2, QgsUnitTypes.DistanceUnknownUnit, True), '1.00')

        # don't keep base unit
        self.assertEqual(QgsUnitTypes.formatDistance(10, 3, QgsUnitTypes.DistanceMeters, False), '10.000 m')
        self.assertEqual(QgsUnitTypes.formatDistance(1001, 3, QgsUnitTypes.DistanceMeters, False), '1.001 km')
        self.assertEqual(QgsUnitTypes.formatDistance(0.05, 2, QgsUnitTypes.DistanceMeters, False), '5.00 cm')
        self.assertEqual(QgsUnitTypes.formatDistance(0.005, 2, QgsUnitTypes.DistanceMeters, False), '5.00 mm')
        self.assertEqual(QgsUnitTypes.formatDistance(10, 2, QgsUnitTypes.DistanceKilometers, False), '10.00 km')
        self.assertEqual(QgsUnitTypes.formatDistance(0.5, 2, QgsUnitTypes.DistanceKilometers, False), '500.00 m')
        self.assertEqual(QgsUnitTypes.formatDistance(10, 2, QgsUnitTypes.DistanceFeet, False), '10.00 ft')
        self.assertEqual(QgsUnitTypes.formatDistance(6000, 2, QgsUnitTypes.DistanceFeet, False), '1.14 mi')
        self.assertEqual(QgsUnitTypes.formatDistance(10, 2, QgsUnitTypes.DistanceYards, False), '10.00 yd')
        self.assertEqual(QgsUnitTypes.formatDistance(2500, 2, QgsUnitTypes.DistanceYards, False), '1.42 mi')
        self.assertEqual(QgsUnitTypes.formatDistance(1, 2, QgsUnitTypes.DistanceMiles, False), '1.00 mi')
        self.assertEqual(QgsUnitTypes.formatDistance(0.5, 2, QgsUnitTypes.DistanceMiles, False), '2640.00 ft')
        self.assertEqual(QgsUnitTypes.formatDistance(1.11111111, 4, QgsUnitTypes.DistanceNauticalMiles, False), '1.1111 NM')
        self.assertEqual(QgsUnitTypes.formatDistance(0.001, 4, QgsUnitTypes.DistanceDegrees, False), '0.0010 deg')
        self.assertEqual(QgsUnitTypes.formatDistance(100, 2, QgsUnitTypes.DistanceCentimeters, False), '100.00 cm')
        self.assertEqual(QgsUnitTypes.formatDistance(1000, 2, QgsUnitTypes.DistanceMillimeters, False), '1000.00 mm')
        self.assertEqual(QgsUnitTypes.formatDistance(1, 2, QgsUnitTypes.DistanceUnknownUnit, False), '1.00')

        # small values should not be displayed as zeroes, instead fallback to scientific notation
        self.assertEqual(QgsUnitTypes.formatDistance(0.00168478, 2, QgsUnitTypes.DistanceMeters, False), '1.68 mm')
        self.assertEqual(QgsUnitTypes.formatDistance(0.00000168478, 2, QgsUnitTypes.DistanceMeters, False), '1.68e-06 m')
        self.assertEqual(QgsUnitTypes.formatDistance(0.00168478, 2, QgsUnitTypes.DistanceMeters, True), '1.68e-03 m')

        # test different locales
        QLocale.setDefault(QLocale(QLocale.Italian))
        self.assertEqual(QgsUnitTypes.formatDistance(10, 3, QgsUnitTypes.DistanceMeters, False), '10,000 m')
        self.assertEqual(QgsUnitTypes.formatDistance(0.5, 2, QgsUnitTypes.DistanceMiles, False), '2.640,00 ft')

    def testFormatArea(self):
        """Test formatting areas"""
        # keep base unit
        self.assertEqual(QgsUnitTypes.formatArea(100, 3, QgsUnitTypes.AreaSquareMeters, True), '100.000 m²')
        self.assertEqual(QgsUnitTypes.formatArea(10, 2, QgsUnitTypes.AreaSquareKilometers, True), '10.00 km²')
        self.assertEqual(QgsUnitTypes.formatArea(1, 0, QgsUnitTypes.AreaSquareFeet, True), '1 ft²')
        self.assertEqual(QgsUnitTypes.formatArea(1.11111111, 4, QgsUnitTypes.AreaSquareYards, True), '1.1111 yd²')
        self.assertEqual(QgsUnitTypes.formatArea(1.99999999, 2, QgsUnitTypes.AreaSquareMiles, True), '2.00 mi²')
        self.assertEqual(QgsUnitTypes.formatArea(1, 2, QgsUnitTypes.AreaHectares, True), '1.00 ha')
        self.assertEqual(QgsUnitTypes.formatArea(1, 2, QgsUnitTypes.AreaAcres, True), '1.00 ac')
        self.assertEqual(QgsUnitTypes.formatArea(1, 2, QgsUnitTypes.AreaSquareNauticalMiles, True), '1.00 NM²')
        self.assertEqual(QgsUnitTypes.formatArea(1, 2, QgsUnitTypes.AreaSquareDegrees, True), '1.00 deg²')
        self.assertEqual(QgsUnitTypes.formatArea(1, 2, QgsUnitTypes.AreaSquareCentimeters, True), '1.00 cm²')
        self.assertEqual(QgsUnitTypes.formatArea(1, 2, QgsUnitTypes.AreaSquareMillimeters, True), '1.00 mm²')
        self.assertEqual(QgsUnitTypes.formatArea(1, 2, QgsUnitTypes.AreaUnknownUnit, True), '1.00')

        # don't keep base unit
        self.assertEqual(QgsUnitTypes.formatArea(100, 2, QgsUnitTypes.AreaSquareMeters, False), '100.00 m²')
        self.assertEqual(QgsUnitTypes.formatArea(2000000, 2, QgsUnitTypes.AreaSquareMeters, False), '2.00 km²')
        self.assertEqual(QgsUnitTypes.formatArea(10001, 2, QgsUnitTypes.AreaSquareMeters, False), '1.00 ha')
        self.assertEqual(QgsUnitTypes.formatArea(100, 2, QgsUnitTypes.AreaSquareKilometers, False), '100.00 km²')
        self.assertEqual(QgsUnitTypes.formatArea(0.5, 2, QgsUnitTypes.AreaSquareKilometers, False), '0.50 km²')
        self.assertEqual(QgsUnitTypes.formatArea(27879000, 2, QgsUnitTypes.AreaSquareFeet, False), '1.00 mi²')
        self.assertEqual(QgsUnitTypes.formatArea(2787, 2, QgsUnitTypes.AreaSquareFeet, False), '2787.00 ft²')
        self.assertEqual(QgsUnitTypes.formatArea(3099000, 2, QgsUnitTypes.AreaSquareYards, False), '1.00 mi²')
        self.assertEqual(QgsUnitTypes.formatArea(309, 2, QgsUnitTypes.AreaSquareYards, False), '309.00 yd²')
        self.assertEqual(QgsUnitTypes.formatArea(10, 2, QgsUnitTypes.AreaSquareMiles, False), '10.00 mi²')
        self.assertEqual(QgsUnitTypes.formatArea(0.05, 2, QgsUnitTypes.AreaSquareMiles, False), '0.05 mi²')
        self.assertEqual(QgsUnitTypes.formatArea(10, 2, QgsUnitTypes.AreaHectares, False), '10.00 ha')
        self.assertEqual(QgsUnitTypes.formatArea(110, 2, QgsUnitTypes.AreaHectares, False), '1.10 km²')
        self.assertEqual(QgsUnitTypes.formatArea(10, 2, QgsUnitTypes.AreaAcres, False), '10.00 ac')
        self.assertEqual(QgsUnitTypes.formatArea(650, 2, QgsUnitTypes.AreaAcres, False), '1.02 mi²')
        self.assertEqual(QgsUnitTypes.formatArea(0.01, 2, QgsUnitTypes.AreaSquareNauticalMiles, False), '0.01 NM²')
        self.assertEqual(QgsUnitTypes.formatArea(100, 2, QgsUnitTypes.AreaSquareNauticalMiles, False), '100.00 NM²')
        self.assertEqual(QgsUnitTypes.formatArea(0.0001, 4, QgsUnitTypes.AreaSquareDegrees, False), '0.0001 deg²')
        self.assertEqual(QgsUnitTypes.formatArea(0.0001, 4, QgsUnitTypes.AreaSquareDegrees, False), '0.0001 deg²')
        self.assertEqual(QgsUnitTypes.formatArea(1000, 4, QgsUnitTypes.AreaSquareMillimeters, False), '0.0010 m²')
        self.assertEqual(QgsUnitTypes.formatArea(100, 3, QgsUnitTypes.AreaSquareCentimeters, False), '0.010 m²')
        self.assertEqual(QgsUnitTypes.formatArea(10, 2, QgsUnitTypes.AreaUnknownUnit, False), '10.00')

        # small values should not be displayed as zeroes, instead fallback to scientific notation
        self.assertEqual(QgsUnitTypes.formatArea(0.00168478, 4, QgsUnitTypes.AreaSquareMeters, False), '0.0017 m²')
        self.assertEqual(QgsUnitTypes.formatArea(0.00168478, 2, QgsUnitTypes.AreaSquareMeters, False), '1.68e-03 m²')
        self.assertEqual(QgsUnitTypes.formatArea(0.00168478, 2, QgsUnitTypes.AreaSquareMeters, True), '1.68e-03 m²')

        # test different locales
        QLocale.setDefault(QLocale(QLocale.Italian))
        self.assertEqual(QgsUnitTypes.formatArea(100, 2, QgsUnitTypes.AreaSquareKilometers, False), '100,00 km²')
        self.assertEqual(QgsUnitTypes.formatArea(2787, 2, QgsUnitTypes.AreaSquareFeet, False), '2.787,00 ft²')

    def testEncodeDecodeLayoutUnits(self):
        """Test encoding and decoding layout units"""
        units = [QgsUnitTypes.LayoutMillimeters,
                 QgsUnitTypes.LayoutCentimeters,
                 QgsUnitTypes.LayoutMeters,
                 QgsUnitTypes.LayoutInches,
                 QgsUnitTypes.LayoutFeet,
                 QgsUnitTypes.LayoutPoints,
                 QgsUnitTypes.LayoutPicas,
                 QgsUnitTypes.LayoutPixels]

        for u in units:
            res, ok = QgsUnitTypes.decodeLayoutUnit(QgsUnitTypes.encodeUnit(u))
            assert ok
            self.assertEqual(res, u)

        # Test decoding bad units
        res, ok = QgsUnitTypes.decodeLayoutUnit('bad')
        self.assertFalse(ok)
        # default units should be MM
        self.assertEqual(res, QgsUnitTypes.LayoutMillimeters)

        # Test that string is cleaned before decoding
        res, ok = QgsUnitTypes.decodeLayoutUnit(' px  ')
        assert ok
        self.assertEqual(res, QgsUnitTypes.LayoutPixels)

    def testAbbreviateRenderUnits(self):
        """Test abbreviating render units"""
        units = [QgsUnitTypes.RenderMillimeters,
                 QgsUnitTypes.RenderMapUnits,
                 QgsUnitTypes.RenderPixels,
                 QgsUnitTypes.RenderPercentage,
                 QgsUnitTypes.RenderPoints,
                 QgsUnitTypes.RenderInches,
                 QgsUnitTypes.RenderUnknownUnit,
                 QgsUnitTypes.RenderMetersInMapUnits]

        used = set()
        for u in units:
            self.assertTrue(QgsUnitTypes.toString(u))
            self.assertTrue(QgsUnitTypes.toAbbreviatedString(u))
            self.assertFalse(QgsUnitTypes.toAbbreviatedString(u) in used)
            used.add(QgsUnitTypes.toAbbreviatedString(u))

    def testAbbreviateLayoutUnits(self):
        """Test abbreviating layout units"""
        units = [QgsUnitTypes.LayoutMillimeters,
                 QgsUnitTypes.LayoutCentimeters,
                 QgsUnitTypes.LayoutMeters,
                 QgsUnitTypes.LayoutInches,
                 QgsUnitTypes.LayoutFeet,
                 QgsUnitTypes.LayoutPoints,
                 QgsUnitTypes.LayoutPicas,
                 QgsUnitTypes.LayoutPixels]

        used = set()
        for u in units:
            self.assertTrue(QgsUnitTypes.toString(u))
            self.assertTrue(QgsUnitTypes.toAbbreviatedString(u))
            self.assertFalse(QgsUnitTypes.toAbbreviatedString(u) in used)
            used.add(QgsUnitTypes.toAbbreviatedString(u))


if __name__ == "__main__":
    unittest.main()
