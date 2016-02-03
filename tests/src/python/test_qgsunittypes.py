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

import qgis

from utilities import (unittest,
                       TestCase,
                       getQgisTestApp
                       )
from qgis.core import (QgsUnitTypes,
                       QGis,
                       QgsSymbolV2
                       )

getQgisTestApp()


class TestQgsUnitTypes(TestCase):

    def testDistanceUnitType(self):
        """Test QgsUnitTypes::unitType() """
        expected = {QGis.Meters: QgsUnitTypes.Standard,
                    QGis.Feet: QgsUnitTypes.Standard,
                    QGis.Degrees: QgsUnitTypes.Geographic,
                    QGis.UnknownUnit: QgsUnitTypes.UnknownType,
                    QGis.NauticalMiles: QgsUnitTypes.Standard
                    }

        for t in expected.keys():
            self.assertEqual(QgsUnitTypes.unitType(t), expected[t])

    def testEncodeDecodeDistanceUnits(self):
        """Test encoding and decoding distance units"""
        units = [QGis.Meters,
                 QGis.Feet,
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
                 QGis.Feet,
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
        res, ok = QgsUnitTypes.decodeDistanceUnit(' {}  '.format(QgsUnitTypes.toString(QGis.Feet).upper()))
        print ' {}  '.format(QgsUnitTypes.toString(QGis.Feet).upper())
        assert ok
        self.assertEqual(res, QGis.Feet)

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

        expected = {QGis.Meters: {QGis.Meters: 1.0, QGis.Feet: 3.28083989501, QGis.Degrees: 0.00000898315, QGis.NauticalMiles: 0.000539957},
                    QGis.Feet: {QGis.Meters: 0.3048, QGis.Feet: 1.0, QGis.Degrees: 2.73806498599629E-06, QGis.NauticalMiles: 0.000164579},
                    QGis.Degrees: {QGis.Meters: 111319.49079327358, QGis.Feet: 365221.4264871, QGis.Degrees: 1.0, QGis.NauticalMiles: 60.1077164},
                    QGis.NauticalMiles: {QGis.Meters: 1852.0, QGis.Feet: 6076.1154856, QGis.Degrees: 0.0166367990650, QGis.NauticalMiles: 1.0},
                    QGis.UnknownUnit: {QGis.Meters: 1.0, QGis.Feet: 1.0, QGis.Degrees: 1.0, QGis.NauticalMiles: 1.0}
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


if __name__ == "__main__":
    unittest.main()
