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
from qgis.core import (
    QgsUnitTypes,
    QGis,
    QgsSymbolV2
)
from qgis.PyQt.QtCore import QLocale

# enforce C locale because the tests expect it
# (decimal separators / thousand separators)
QLocale.setDefault(QLocale.c())


class TestQgsUnitTypes(unittest.TestCase):

    def testDistanceUnitType(self):
        """Test QgsUnitTypes::unitType() """
        expected = {QGis.Meters: QgsUnitTypes.Standard,
                    QGis.Kilometers: QgsUnitTypes.Standard,
                    QGis.Feet: QgsUnitTypes.Standard,
                    QGis.Yards: QgsUnitTypes.Standard,
                    QGis.Miles: QgsUnitTypes.Standard,
                    QGis.Degrees: QgsUnitTypes.Geographic,
                    QGis.UnknownUnit: QgsUnitTypes.UnknownType,
                    QGis.NauticalMiles: QgsUnitTypes.Standard
                    }

        for t in expected.keys():
            self.assertEqual(QgsUnitTypes.unitType(t), expected[t])

    def testEncodeDecodeDistanceUnits(self):
        """Test encoding and decoding distance units"""
        units = [QGis.Meters,
                 QGis.Kilometers,
                 QGis.Feet,
                 QGis.Yards,
                 QGis.Miles,
                 QGis.Degrees,
                 QGis.UnknownUnit,
                 QGis.NauticalMiles]

        for u in units:
            res, ok = QgsUnitTypes.decodeDistanceUnit(QgsUnitTypes.encodeUnit(u))
            assert ok
            self.assertEqual(res, u)

        # Test decoding bad units
        res, ok = QgsUnitTypes.decodeDistanceUnit('bad')
        self.assertFalse(ok)
        self.assertEqual(res, QGis.UnknownUnit)

        # Test that string is cleaned before decoding
        res, ok = QgsUnitTypes.decodeDistanceUnit(' FeEt  ')
        assert ok
        self.assertEqual(res, QGis.Feet)

    def testDistanceUnitsToFromString(self):
        """Test converting distance units to and from translated strings"""
        units = [QGis.Meters,
                 QGis.Kilometers,
                 QGis.Feet,
                 QGis.Yards,
                 QGis.Miles,
                 QGis.Degrees,
                 QGis.UnknownUnit,
                 QGis.NauticalMiles]

        for u in units:
            res, ok = QgsUnitTypes.stringToDistanceUnit(QgsUnitTypes.toString(u))
            assert ok
            self.assertEqual(res, u)

        # Test converting bad strings
        res, ok = QgsUnitTypes.stringToDistanceUnit('bad')
        self.assertFalse(ok)
        self.assertEqual(res, QGis.UnknownUnit)

        # Test that string is cleaned before conversion
        res, ok = QgsUnitTypes.stringToDistanceUnit(' {}  '.format(QgsUnitTypes.toString(QGis.Feet).upper()))
        print(' {}  '.format(QgsUnitTypes.toString(QGis.Feet).upper()))
        assert ok
        self.assertEqual(res, QGis.Feet)

    def testAreaUnitType(self):
        """Test QgsUnitTypes::unitType() for area units """
        expected = {QgsUnitTypes.SquareMeters: QgsUnitTypes.Standard,
                    QgsUnitTypes.SquareKilometers: QgsUnitTypes.Standard,
                    QgsUnitTypes.SquareFeet: QgsUnitTypes.Standard,
                    QgsUnitTypes.SquareYards: QgsUnitTypes.Standard,
                    QgsUnitTypes.SquareMiles: QgsUnitTypes.Standard,
                    QgsUnitTypes.Hectares: QgsUnitTypes.Standard,
                    QgsUnitTypes.Acres: QgsUnitTypes.Standard,
                    QgsUnitTypes.SquareNauticalMiles: QgsUnitTypes.Standard,
                    QgsUnitTypes.SquareDegrees: QgsUnitTypes.Geographic,
                    QgsUnitTypes.UnknownAreaUnit: QgsUnitTypes.UnknownType,
                    }

        for t in expected.keys():
            self.assertEqual(QgsUnitTypes.unitType(t), expected[t])

    def testEncodeDecodeAreaUnits(self):
        """Test encoding and decoding area units"""
        units = [QgsUnitTypes.SquareMeters,
                 QgsUnitTypes.SquareKilometers,
                 QgsUnitTypes.SquareFeet,
                 QgsUnitTypes.SquareYards,
                 QgsUnitTypes.SquareMiles,
                 QgsUnitTypes.Hectares,
                 QgsUnitTypes.Acres,
                 QgsUnitTypes.SquareNauticalMiles,
                 QgsUnitTypes.SquareDegrees,
                 QgsUnitTypes.UnknownAreaUnit]

        for u in units:
            res, ok = QgsUnitTypes.decodeAreaUnit(QgsUnitTypes.encodeUnit(u))
            assert ok
            self.assertEqual(res, u)

        # Test decoding bad units
        res, ok = QgsUnitTypes.decodeAreaUnit('bad')
        self.assertFalse(ok)
        self.assertEqual(res, QgsUnitTypes.UnknownAreaUnit)

        # Test that string is cleaned before decoding
        res, ok = QgsUnitTypes.decodeAreaUnit(' Ha  ')
        assert ok
        self.assertEqual(res, QgsUnitTypes.Hectares)

    def testAreaUnitsToFromString(self):
        """Test converting area units to and from translated strings"""
        units = [QgsUnitTypes.SquareMeters,
                 QgsUnitTypes.SquareKilometers,
                 QgsUnitTypes.SquareFeet,
                 QgsUnitTypes.SquareYards,
                 QgsUnitTypes.SquareMiles,
                 QgsUnitTypes.Hectares,
                 QgsUnitTypes.Acres,
                 QgsUnitTypes.SquareNauticalMiles,
                 QgsUnitTypes.SquareDegrees,
                 QgsUnitTypes.UnknownAreaUnit]

        for u in units:
            res, ok = QgsUnitTypes.stringToAreaUnit(QgsUnitTypes.toString(u))
            assert ok
            self.assertEqual(res, u)

        # Test converting bad strings
        res, ok = QgsUnitTypes.stringToAreaUnit('bad')
        self.assertFalse(ok)
        self.assertEqual(res, QgsUnitTypes.UnknownAreaUnit)

        # Test that string is cleaned before conversion
        res, ok = QgsUnitTypes.stringToAreaUnit(' {}  '.format(QgsUnitTypes.toString(QgsUnitTypes.SquareMiles).upper()))
        assert ok
        self.assertEqual(res, QgsUnitTypes.SquareMiles)

    @unittest.skip("enable for 3.0")
    def testEncodeDecodeSymbolUnits(self):
        """Test encoding and decoding symbol units"""
        units = [QgsSymbolV2.MM,
                 QgsSymbolV2.MapUnit,
                 QgsSymbolV2.Pixel,
                 QgsSymbolV2.Percentage]

        for u in units:
            res, ok = QgsUnitTypes.decodeSymbolUnit(QgsUnitTypes.encodeUnit(u))
            assert ok
            self.assertEqual(res, u)

        # Test decoding bad units
        res, ok = QgsUnitTypes.decodeSymbolUnit('bad')
        self.assertFalse(ok)
        # default units should be MM
        self.assertEqual(res, QgsSymbolV2.MM)

        # Test that string is cleaned before decoding
        res, ok = QgsUnitTypes.decodeSymbolUnit(' PiXeL  ')
        assert ok
        self.assertEqual(res, QgsSymbolV2.Pixel)

    def testFromUnitToUnitFactor(self):
        """Test calculation of conversion factor between units"""

        expected = {QGis.Meters: {QGis.Meters: 1.0, QGis.Kilometers: 0.001, QGis.Feet: 3.28083989501, QGis.Yards: 1.0936133, QGis.Miles: 0.00062136931818182, QGis.Degrees: 0.00000898315, QGis.NauticalMiles: 0.000539957},
                    QGis.Kilometers: {QGis.Meters: 1000.0, QGis.Kilometers: 1.0, QGis.Feet: 3280.8398950, QGis.Yards: 1093.6132983, QGis.Miles: 0.62137121212119317271, QGis.Degrees: 0.0089832, QGis.NauticalMiles: 0.53995682073432482717},
                    QGis.Feet: {QGis.Meters: 0.3048, QGis.Kilometers: 0.0003048, QGis.Feet: 1.0, QGis.Yards: 0.3333333, QGis.Miles: 0.00018939375, QGis.Degrees: 2.73806498599629E-06, QGis.NauticalMiles: 0.000164579},
                    QGis.Yards: {QGis.Meters: 0.9144, QGis.Kilometers: 0.0009144, QGis.Feet: 3.0, QGis.Yards: 1.0, QGis.Miles: 0.000568182, QGis.Degrees: 0.0000082, QGis.NauticalMiles: 0.0004937366590756},
                    QGis.Degrees: {QGis.Meters: 111319.49079327358, QGis.Kilometers: 111.3194908, QGis.Feet: 365221.4264871, QGis.Yards: 121740.4754957, QGis.Miles: 69.1707247, QGis.Degrees: 1.0, QGis.NauticalMiles: 60.1077164},
                    QGis.Miles: {QGis.Meters: 1609.3440000, QGis.Kilometers: 1.6093440, QGis.Feet: 5280.0000000, QGis.Yards: 1760.0000000, QGis.Miles: 1.0, QGis.Degrees: 0.0144570, QGis.NauticalMiles: 0.8689762},
                    QGis.NauticalMiles: {QGis.Meters: 1852.0, QGis.Kilometers: 1.8520000, QGis.Feet: 6076.1154856, QGis.Yards: 2025.3718285, QGis.Miles: 1.1507794, QGis.Degrees: 0.0166367990650, QGis.NauticalMiles: 1.0},
                    QGis.UnknownUnit: {QGis.Meters: 1.0, QGis.Kilometers: 1.0, QGis.Feet: 1.0, QGis.Yards: 1.0, QGis.Miles: 1.0, QGis.Degrees: 1.0, QGis.NauticalMiles: 1.0}
                    }

        for from_unit in expected.keys():
            for to_unit in expected[from_unit].keys():
                expected_factor = expected[from_unit][to_unit]
                res = QgsUnitTypes.fromUnitToUnitFactor(from_unit, to_unit)
                self.assertAlmostEqual(res,
                                       expected_factor,
                                       msg='got {:.7f}, expected {:.7f} when converting from {} to {}'.format(res, expected_factor,
                                                                                                              QgsUnitTypes.toString(from_unit),
                                                                                                              QgsUnitTypes.toString(to_unit)))
                #test conversion to unknown units
                res = QgsUnitTypes.fromUnitToUnitFactor(from_unit, QGis.UnknownUnit)
                self.assertAlmostEqual(res,
                                       1.0,
                                       msg='got {:.7f}, expected 1.0 when converting from {} to unknown units'.format(res, expected_factor,
                                                                                                                      QgsUnitTypes.toString(from_unit)))

    def testAreaFromUnitToUnitFactor(self):
        """Test calculation of conversion factor between areal units"""

        expected = {QgsUnitTypes.SquareMeters: {QgsUnitTypes.SquareMeters: 1.0, QgsUnitTypes.SquareKilometers: 1e-6, QgsUnitTypes.SquareFeet: 10.7639104, QgsUnitTypes.SquareYards: 1.19599, QgsUnitTypes.SquareMiles: 3.86102e-7, QgsUnitTypes.Hectares: 0.0001, QgsUnitTypes.Acres: 0.000247105, QgsUnitTypes.SquareNauticalMiles: 2.91553e-7, QgsUnitTypes.SquareDegrees: 0.000000000080697, QgsUnitTypes.UnknownAreaUnit: 1.0},
                    QgsUnitTypes.SquareKilometers: {QgsUnitTypes.SquareMeters: 1e6, QgsUnitTypes.SquareKilometers: 1, QgsUnitTypes.SquareFeet: 10763910.4167097, QgsUnitTypes.SquareYards: 1195990.04630108, QgsUnitTypes.SquareMiles: 0.386102158, QgsUnitTypes.Hectares: 100, QgsUnitTypes.Acres: 247.105381467, QgsUnitTypes.SquareNauticalMiles: 0.291553349598, QgsUnitTypes.SquareDegrees: 0.000080697034968, QgsUnitTypes.UnknownAreaUnit: 1.0},
                    QgsUnitTypes.SquareFeet: {QgsUnitTypes.SquareMeters: 0.092903, QgsUnitTypes.SquareKilometers: 9.2903e-8, QgsUnitTypes.SquareFeet: 1.0, QgsUnitTypes.SquareYards: 0.11111111111, QgsUnitTypes.SquareMiles: 3.58701e-8, QgsUnitTypes.Hectares: 9.2903e-6, QgsUnitTypes.Acres: 2.29568e-5, QgsUnitTypes.SquareNauticalMiles: 2.70862e-8, QgsUnitTypes.SquareDegrees: 0.000000000007497, QgsUnitTypes.UnknownAreaUnit: 1.0},
                    QgsUnitTypes.SquareYards: {QgsUnitTypes.SquareMeters: 0.836127360, QgsUnitTypes.SquareKilometers: 8.36127e-7, QgsUnitTypes.SquareFeet: 9.0, QgsUnitTypes.SquareYards: 1.0, QgsUnitTypes.SquareMiles: 3.22831e-7, QgsUnitTypes.Hectares: 8.3612736E-5, QgsUnitTypes.Acres: 0.00020661157, QgsUnitTypes.SquareNauticalMiles: 2.43776e-7, QgsUnitTypes.SquareDegrees: 0.000000000067473, QgsUnitTypes.UnknownAreaUnit: 1.0},
                    QgsUnitTypes.SquareMiles: {QgsUnitTypes.SquareMeters: 2589988.110336, QgsUnitTypes.SquareKilometers: 2.589988110, QgsUnitTypes.SquareFeet: 27878400, QgsUnitTypes.SquareYards: 3097600, QgsUnitTypes.SquareMiles: 1.0, QgsUnitTypes.Hectares: 258.998811, QgsUnitTypes.Acres: 640, QgsUnitTypes.SquareNauticalMiles: 0.75511970898, QgsUnitTypes.SquareDegrees: 0.000209004361107, QgsUnitTypes.UnknownAreaUnit: 1.0},
                    QgsUnitTypes.Hectares: {QgsUnitTypes.SquareMeters: 10000, QgsUnitTypes.SquareKilometers: 0.01, QgsUnitTypes.SquareFeet: 107639.1041670972, QgsUnitTypes.SquareYards: 11959.9004630, QgsUnitTypes.SquareMiles: 0.00386102, QgsUnitTypes.Hectares: 1.0, QgsUnitTypes.Acres: 2.471053814, QgsUnitTypes.SquareNauticalMiles: 0.00291553, QgsUnitTypes.SquareDegrees: 0.000000806970350, QgsUnitTypes.UnknownAreaUnit: 1.0},
                    QgsUnitTypes.Acres: {QgsUnitTypes.SquareMeters: 4046.8564224, QgsUnitTypes.SquareKilometers: 0.00404686, QgsUnitTypes.SquareFeet: 43560, QgsUnitTypes.SquareYards: 4840, QgsUnitTypes.SquareMiles: 0.0015625, QgsUnitTypes.Hectares: 0.404685642, QgsUnitTypes.Acres: 1.0, QgsUnitTypes.SquareNauticalMiles: 0.00117987, QgsUnitTypes.SquareDegrees: 0.000000326569314, QgsUnitTypes.UnknownAreaUnit: 1.0},
                    QgsUnitTypes.SquareNauticalMiles: {QgsUnitTypes.SquareMeters: 3429904, QgsUnitTypes.SquareKilometers: 3.4299040, QgsUnitTypes.SquareFeet: 36919179.39391434, QgsUnitTypes.SquareYards: 4102131.04376826, QgsUnitTypes.SquareMiles: 1.324293337, QgsUnitTypes.Hectares: 342.9904000000, QgsUnitTypes.Acres: 847.54773631, QgsUnitTypes.SquareNauticalMiles: 1.0, QgsUnitTypes.SquareDegrees: 0.000276783083025, QgsUnitTypes.UnknownAreaUnit: 1.0},
                    QgsUnitTypes.SquareDegrees: {QgsUnitTypes.SquareMeters: 12392029030.5, QgsUnitTypes.SquareKilometers: 12392.029030499, QgsUnitTypes.SquareFeet: 133386690365.5682220, QgsUnitTypes.SquareYards: 14820743373.9520263, QgsUnitTypes.SquareMiles: 4784.5891573967, QgsUnitTypes.Hectares: 1239202.903050, QgsUnitTypes.Acres: 3062137.060733889, QgsUnitTypes.SquareNauticalMiles: 3612.93757215, QgsUnitTypes.SquareDegrees: 1.0, QgsUnitTypes.UnknownAreaUnit: 1.0}}

        for from_unit in expected.keys():
            for to_unit in expected[from_unit].keys():
                expected_factor = expected[from_unit][to_unit]
                res = QgsUnitTypes.fromUnitToUnitFactor(from_unit, to_unit)
                self.assertAlmostEqual(res,
                                       expected_factor,
                                       msg='got {:.15f}, expected {:.15f} when converting from {} to {}'.format(res, expected_factor,
                                                                                                                QgsUnitTypes.toString(from_unit),
                                                                                                                QgsUnitTypes.toString(to_unit)))
                #test conversion to unknown units
                res = QgsUnitTypes.fromUnitToUnitFactor(from_unit, QgsUnitTypes.UnknownAreaUnit)
                self.assertAlmostEqual(res,
                                       1.0,
                                       msg='got {:.7f}, expected 1.0 when converting from {} to unknown units'.format(res, expected_factor,
                                                                                                                      QgsUnitTypes.toString(from_unit)))

    def testDistanceToAreaUnit(self):
        """Test distanceToAreaUnit conversion"""
        expected = {QGis.Meters: QgsUnitTypes.SquareMeters,
                    QGis.Kilometers: QgsUnitTypes.SquareKilometers,
                    QGis.Feet: QgsUnitTypes.SquareFeet,
                    QGis.Yards: QgsUnitTypes.SquareYards,
                    QGis.Miles: QgsUnitTypes.SquareMiles,
                    QGis.Degrees: QgsUnitTypes.SquareDegrees,
                    QGis.UnknownUnit: QgsUnitTypes.UnknownAreaUnit,
                    QGis.NauticalMiles: QgsUnitTypes.SquareNauticalMiles
                    }

        for t in expected.keys():
            self.assertEqual(QgsUnitTypes.distanceToAreaUnit(t), expected[t])

    def testEncodeDecodeAngleUnits(self):
        """Test encoding and decoding angle units"""
        units = [QgsUnitTypes.AngleDegrees,
                 QgsUnitTypes.Radians,
                 QgsUnitTypes.Gon,
                 QgsUnitTypes.MinutesOfArc,
                 QgsUnitTypes.SecondsOfArc,
                 QgsUnitTypes.Turn,
                 QgsUnitTypes.UnknownAngleUnit]

        for u in units:
            res, ok = QgsUnitTypes.decodeAngleUnit(QgsUnitTypes.encodeUnit(u))
            assert ok, 'could not decode unit {}'.format(QgsUnitTypes.toString(u))
            self.assertEqual(res, u)

        # Test decoding bad units
        res, ok = QgsUnitTypes.decodeAngleUnit('bad')
        self.assertFalse(ok)
        self.assertEqual(res, QgsUnitTypes.UnknownAngleUnit)

        # Test that string is cleaned before decoding
        res, ok = QgsUnitTypes.decodeAngleUnit(' MoA  ')
        assert ok
        self.assertEqual(res, QgsUnitTypes.MinutesOfArc)

    def testAngleToString(self):
        """Test converting angle unit to string"""
        units = [QgsUnitTypes.AngleDegrees,
                 QgsUnitTypes.Radians,
                 QgsUnitTypes.Gon,
                 QgsUnitTypes.MinutesOfArc,
                 QgsUnitTypes.SecondsOfArc,
                 QgsUnitTypes.Turn,
                 QgsUnitTypes.UnknownAngleUnit]

        dupes = set()

        # can't test result as it may be translated, so make sure it's non-empty and not a duplicate
        for u in units:
            s = QgsUnitTypes.toString(u)
            assert len(s) > 0
            self.assertFalse(s in dupes)
            dupes.add(s)

    def testAngleFromUnitToUnitFactor(self):
        """Test calculation of conversion factor between angular units"""

        expected = {QgsUnitTypes.AngleDegrees: {QgsUnitTypes.AngleDegrees: 1.0, QgsUnitTypes.Radians: 0.0174533, QgsUnitTypes.Gon: 1.1111111, QgsUnitTypes.MinutesOfArc: 60, QgsUnitTypes.SecondsOfArc: 3600, QgsUnitTypes.Turn: 0.00277777777778},
                    QgsUnitTypes.Radians: {QgsUnitTypes.AngleDegrees: 57.2957795, QgsUnitTypes.Radians: 1.0, QgsUnitTypes.Gon: 63.6619772, QgsUnitTypes.MinutesOfArc: 3437.7467708, QgsUnitTypes.SecondsOfArc: 206264.8062471, QgsUnitTypes.Turn: 0.159154943092},
                    QgsUnitTypes.Gon: {QgsUnitTypes.AngleDegrees: 0.9000000, QgsUnitTypes.Radians: 0.015707968623450838802, QgsUnitTypes.Gon: 1.0, QgsUnitTypes.MinutesOfArc: 54.0000000, QgsUnitTypes.SecondsOfArc: 3240.0000000, QgsUnitTypes.Turn: 0.0025},
                    QgsUnitTypes.MinutesOfArc: {QgsUnitTypes.AngleDegrees: 0.016666672633390722247, QgsUnitTypes.Radians: 0.00029088831280398030638, QgsUnitTypes.Gon: 0.018518525464057963154, QgsUnitTypes.MinutesOfArc: 1.0, QgsUnitTypes.SecondsOfArc: 60.0, QgsUnitTypes.Turn: 4.62962962962963e-05},
                    QgsUnitTypes.SecondsOfArc: {QgsUnitTypes.AngleDegrees: 0.00027777787722304257169, QgsUnitTypes.Radians: 4.848138546730629518e-6, QgsUnitTypes.Gon: 0.0003086420910674814405, QgsUnitTypes.MinutesOfArc: 0.016666672633325253783, QgsUnitTypes.SecondsOfArc: 1.0, QgsUnitTypes.Turn: 7.71604938271605e-07},
                    QgsUnitTypes.Turn: {QgsUnitTypes.AngleDegrees: 360.0, QgsUnitTypes.Radians: 6.2831853071795, QgsUnitTypes.Gon: 400.0, QgsUnitTypes.MinutesOfArc: 21600, QgsUnitTypes.SecondsOfArc: 1296000, QgsUnitTypes.Turn: 1}
                    }

        for from_unit in expected.keys():
            for to_unit in expected[from_unit].keys():
                expected_factor = expected[from_unit][to_unit]
                res = QgsUnitTypes.fromUnitToUnitFactor(from_unit, to_unit)
                self.assertAlmostEqual(res,
                                       expected_factor,
                                       msg='got {:.7f}, expected {:.7f} when converting from {} to {}'.format(res, expected_factor,
                                                                                                              QgsUnitTypes.toString(from_unit),
                                                                                                              QgsUnitTypes.toString(to_unit)))
                #test conversion to unknown units
                res = QgsUnitTypes.fromUnitToUnitFactor(from_unit, QgsUnitTypes.UnknownAngleUnit)
                self.assertAlmostEqual(res,
                                       1.0,
                                       msg='got {:.7f}, expected 1.0 when converting from {} to unknown units'.format(res, expected_factor,
                                                                                                                      QgsUnitTypes.toString(from_unit)))

    def testFormatAngle(self):
        """Test formatting angles"""
        self.assertEqual(QgsUnitTypes.formatAngle(45, 3, QgsUnitTypes.AngleDegrees), u'45.000°')
        self.assertEqual(QgsUnitTypes.formatAngle(1, 2, QgsUnitTypes.Radians), '1.00 rad')
        self.assertEqual(QgsUnitTypes.formatAngle(1, 0, QgsUnitTypes.Gon), u'1 gon')
        self.assertEqual(QgsUnitTypes.formatAngle(1.11111111, 4, QgsUnitTypes.MinutesOfArc), u'1.1111′')
        self.assertEqual(QgsUnitTypes.formatAngle(1.99999999, 2, QgsUnitTypes.SecondsOfArc), u'2.00″')
        self.assertEqual(QgsUnitTypes.formatAngle(1, 2, QgsUnitTypes.Turn), u'1.00 tr')
        self.assertEqual(QgsUnitTypes.formatAngle(1, 2, QgsUnitTypes.UnknownAngleUnit), u'1.00')

if __name__ == "__main__":
    unittest.main()
