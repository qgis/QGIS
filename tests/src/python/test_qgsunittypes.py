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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.testing import unittest
from qgis.core import QgsUnitTypes
from qgis.PyQt.QtCore import QLocale

# enforce C locale because the tests expect it
# (decimal separators / thousand separators)
QLocale.setDefault(QLocale.c())


class TestQgsUnitTypes(unittest.TestCase):

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
                                       msg='got {:.7f}, expected 1.0 when converting from {} to unknown units'.format(res, expected_factor,
                                                                                                                      QgsUnitTypes.toString(from_unit)))

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
                                       msg='got {:.7f}, expected 1.0 when converting from {} to unknown units'.format(res, expected_factor,
                                                                                                                      QgsUnitTypes.toString(from_unit)))

    def testDistanceToAreaUnit(self):
        """Test distanceToAreaUnit conversion"""
        expected = {QgsUnitTypes.DistanceMeters: QgsUnitTypes.AreaSquareMeters,
                    QgsUnitTypes.DistanceKilometers: QgsUnitTypes.AreaSquareKilometers,
                    QgsUnitTypes.DistanceFeet: QgsUnitTypes.AreaSquareFeet,
                    QgsUnitTypes.DistanceYards: QgsUnitTypes.AreaSquareYards,
                    QgsUnitTypes.DistanceMiles: QgsUnitTypes.AreaSquareMiles,
                    QgsUnitTypes.DistanceDegrees: QgsUnitTypes.AreaSquareDegrees,
                    QgsUnitTypes.DistanceUnknownUnit: QgsUnitTypes.AreaUnknownUnit,
                    QgsUnitTypes.DistanceNauticalMiles: QgsUnitTypes.AreaSquareNauticalMiles
                    }

        for t in list(expected.keys()):
            self.assertEqual(QgsUnitTypes.distanceToAreaUnit(t), expected[t])

    def testEncodeDecodeAngleUnits(self):
        """Test encoding and decoding angle units"""
        units = [QgsUnitTypes.AngleDegrees,
                 QgsUnitTypes.AngleRadians,
                 QgsUnitTypes.AngleGon,
                 QgsUnitTypes.AngleMinutesOfArc,
                 QgsUnitTypes.AngleSecondsOfArc,
                 QgsUnitTypes.AngleTurn,
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

        expected = {QgsUnitTypes.AngleDegrees: {QgsUnitTypes.AngleDegrees: 1.0, QgsUnitTypes.AngleRadians: 0.0174533, QgsUnitTypes.AngleGon: 1.1111111, QgsUnitTypes.AngleMinutesOfArc: 60, QgsUnitTypes.AngleSecondsOfArc: 3600, QgsUnitTypes.AngleTurn: 0.00277777777778},
                    QgsUnitTypes.AngleRadians: {QgsUnitTypes.AngleDegrees: 57.2957795, QgsUnitTypes.AngleRadians: 1.0, QgsUnitTypes.AngleGon: 63.6619772, QgsUnitTypes.AngleMinutesOfArc: 3437.7467708, QgsUnitTypes.AngleSecondsOfArc: 206264.8062471, QgsUnitTypes.AngleTurn: 0.159154943092},
                    QgsUnitTypes.AngleGon: {QgsUnitTypes.AngleDegrees: 0.9000000, QgsUnitTypes.AngleRadians: 0.015707968623450838802, QgsUnitTypes.AngleGon: 1.0, QgsUnitTypes.AngleMinutesOfArc: 54.0000000, QgsUnitTypes.AngleSecondsOfArc: 3240.0000000, QgsUnitTypes.AngleTurn: 0.0025},
                    QgsUnitTypes.AngleMinutesOfArc: {QgsUnitTypes.AngleDegrees: 0.016666672633390722247, QgsUnitTypes.AngleRadians: 0.00029088831280398030638, QgsUnitTypes.AngleGon: 0.018518525464057963154, QgsUnitTypes.AngleMinutesOfArc: 1.0, QgsUnitTypes.AngleSecondsOfArc: 60.0, QgsUnitTypes.AngleTurn: 4.62962962962963e-05},
                    QgsUnitTypes.AngleSecondsOfArc: {QgsUnitTypes.AngleDegrees: 0.00027777787722304257169, QgsUnitTypes.AngleRadians: 4.848138546730629518e-6, QgsUnitTypes.AngleGon: 0.0003086420910674814405, QgsUnitTypes.AngleMinutesOfArc: 0.016666672633325253783, QgsUnitTypes.AngleSecondsOfArc: 1.0, QgsUnitTypes.AngleTurn: 7.71604938271605e-07},
                    QgsUnitTypes.AngleTurn: {QgsUnitTypes.AngleDegrees: 360.0, QgsUnitTypes.AngleRadians: 6.2831853071795, QgsUnitTypes.AngleGon: 400.0, QgsUnitTypes.AngleMinutesOfArc: 21600, QgsUnitTypes.AngleSecondsOfArc: 1296000, QgsUnitTypes.AngleTurn: 1}
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
                                       msg='got {:.7f}, expected 1.0 when converting from {} to unknown units'.format(res, expected_factor,
                                                                                                                      QgsUnitTypes.toString(from_unit)))

    def testFormatAngle(self):
        """Test formatting angles"""
        self.assertEqual(QgsUnitTypes.formatAngle(45, 3, QgsUnitTypes.AngleDegrees), '45.000°')
        self.assertEqual(QgsUnitTypes.formatAngle(1, 2, QgsUnitTypes.AngleRadians), '1.00 rad')
        self.assertEqual(QgsUnitTypes.formatAngle(1, 0, QgsUnitTypes.AngleGon), '1 gon')
        self.assertEqual(QgsUnitTypes.formatAngle(1.11111111, 4, QgsUnitTypes.AngleMinutesOfArc), '1.1111′')
        self.assertEqual(QgsUnitTypes.formatAngle(1.99999999, 2, QgsUnitTypes.AngleSecondsOfArc), '2.00″')
        self.assertEqual(QgsUnitTypes.formatAngle(1, 2, QgsUnitTypes.AngleTurn), '1.00 tr')
        self.assertEqual(QgsUnitTypes.formatAngle(1, 2, QgsUnitTypes.AngleUnknownUnit), '1.00')

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
