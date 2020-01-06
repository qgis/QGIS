# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsNumericFormat

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '6/01/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

from qgis.core import (QgsFallbackNumericFormat,
                       QgsBasicNumericFormat,
                       QgsNumericFormatContext,
                       QgsBearingNumericFormat,
                       QgsPercentageNumericFormat,
                       QgsNumericFormatRegistry,
                       QgsNumericFormat)

from qgis.testing import start_app, unittest

start_app()


class TestFormat(QgsNumericFormat):

    def id(self):
        return 'test'

    def formatDouble(self, value, context):
        return 'xxx' + str(value)

    def clone(self):
        return TestFormat()

    def create(self, configuration):
        return TestFormat()

    def configuration(self):
        return {}


class TestQgsNumericFormat(unittest.TestCase):

    def testFallbackFormat(self):
        """ test fallback formatter """
        f = QgsFallbackNumericFormat()
        context = QgsNumericFormatContext()
        self.assertEqual(f.formatDouble(5, context), '5')
        self.assertEqual(f.formatDouble(5.5, context), '5.5')
        self.assertEqual(f.formatDouble(-5, context), '-5')
        self.assertEqual(f.formatDouble(-5.5, context), '-5.5')

    def testBasicFormat(self):
        """ test basic formatter """
        f = QgsBasicNumericFormat()
        context = QgsNumericFormatContext()
        self.assertEqual(f.formatDouble(0, context), '0')
        self.assertEqual(f.formatDouble(5, context), '5')
        self.assertEqual(f.formatDouble(5.5, context), '5.5')
        self.assertEqual(f.formatDouble(-5, context), '-5')
        self.assertEqual(f.formatDouble(-5.5, context), '-5.5')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-55,555,555.5')
        context.setDecimalSeparator('x')
        self.assertEqual(f.formatDouble(0, context), '0')
        self.assertEqual(f.formatDouble(-5.5, context), '-5x5')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-55,555,555x5')
        context.setThousandsSeparator('y')
        self.assertEqual(f.formatDouble(-5.5, context), '-5x5')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-55y555y555x5')
        f.setShowThousandsSeparator(False)
        self.assertEqual(f.formatDouble(-5.5, context), '-5x5')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-55555555x5')
        context.setDecimalSeparator('.')
        f.setNumberDecimalPlaces(0)
        self.assertEqual(f.formatDouble(0, context), '0')
        self.assertEqual(f.formatDouble(5.5, context), '6')
        self.assertEqual(f.formatDouble(55555555.5, context), '55555556')
        self.assertEqual(f.formatDouble(55555555.123456, context), '55555555')
        self.assertEqual(f.formatDouble(-5.5, context), '-6')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-55555556')
        f.setNumberDecimalPlaces(3)
        self.assertEqual(f.formatDouble(0, context), '0')
        self.assertEqual(f.formatDouble(5.5, context), '5.5')
        self.assertEqual(f.formatDouble(55555555.5, context), '55555555.5')
        self.assertEqual(f.formatDouble(55555555.123456, context), '55555555.123')
        self.assertEqual(f.formatDouble(-5.5, context), '-5.5')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-55555555.5')
        f.setShowTrailingZeros(True)
        self.assertEqual(f.formatDouble(0, context), '0.000')
        self.assertEqual(f.formatDouble(5, context), '5.000')
        self.assertEqual(f.formatDouble(-5, context), '-5.000')
        self.assertEqual(f.formatDouble(5.5, context), '5.500')
        self.assertEqual(f.formatDouble(55555555.5, context), '55555555.500')
        self.assertEqual(f.formatDouble(55555555.123456, context), '55555555.123')
        self.assertEqual(f.formatDouble(-5.5, context), '-5.500')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-55555555.500')
        f.setShowPlusSign(True)
        self.assertEqual(f.formatDouble(0, context), '0.000')
        self.assertEqual(f.formatDouble(5, context), '+5.000')
        self.assertEqual(f.formatDouble(-5, context), '-5.000')
        self.assertEqual(f.formatDouble(5.5, context), '+5.500')
        self.assertEqual(f.formatDouble(55555555.5, context), '+55555555.500')
        self.assertEqual(f.formatDouble(55555555.123456, context), '+55555555.123')
        self.assertEqual(f.formatDouble(-5.5, context), '-5.500')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-55555555.500')

    def testBearingFormat(self):
        """ test bearing formatter """
        f = QgsBearingNumericFormat()
        f.setDirectionFormat(QgsBearingNumericFormat.UseRange0To180WithEWDirectionalSuffix)
        context = QgsNumericFormatContext()
        self.assertEqual(f.formatDouble(0, context), '0°')
        self.assertEqual(f.formatDouble(90, context), '90°E')
        self.assertEqual(f.formatDouble(180, context), '180°')
        self.assertEqual(f.formatDouble(270, context), '90°W')
        self.assertEqual(f.formatDouble(300, context), '60°W')
        self.assertEqual(f.formatDouble(5, context), '5°E')
        self.assertEqual(f.formatDouble(5.5, context), '5.5°E')
        self.assertEqual(f.formatDouble(-5, context), '5°W')
        self.assertEqual(f.formatDouble(-5.5, context), '5.5°W')
        context.setDecimalSeparator('x')
        self.assertEqual(f.formatDouble(0, context), '0°')
        self.assertEqual(f.formatDouble(-5.5, context), '5x5°W')
        self.assertEqual(f.formatDouble(180, context), '180°')
        context.setDecimalSeparator('.')
        f.setNumberDecimalPlaces(0)
        self.assertEqual(f.formatDouble(0, context), '0°')
        self.assertEqual(f.formatDouble(5.5, context), '6°E')
        self.assertEqual(f.formatDouble(-5.5, context), '6°W')
        self.assertEqual(f.formatDouble(180, context), '180°')
        f.setNumberDecimalPlaces(3)
        self.assertEqual(f.formatDouble(0, context), '0°')
        self.assertEqual(f.formatDouble(5.5, context), '5.5°E')
        self.assertEqual(f.formatDouble(-5.5, context), '5.5°W')
        self.assertEqual(f.formatDouble(180, context), '180°')
        f.setShowTrailingZeros(True)
        self.assertEqual(f.formatDouble(0, context), '0.000°E') # todo - fix and avoid E
        self.assertEqual(f.formatDouble(5, context), '5.000°E')
        self.assertEqual(f.formatDouble(-5, context), '5.000°W')
        self.assertEqual(f.formatDouble(5.5, context), '5.500°E')
        self.assertEqual(f.formatDouble(-5.5, context), '5.500°W')
        self.assertEqual(f.formatDouble(180, context), '180.000°E') # todo fix and avoid E

        f = QgsBearingNumericFormat()
        f.setDirectionFormat(QgsBearingNumericFormat.UseRangeNegative180ToPositive180)
        self.assertEqual(f.formatDouble(0, context), '0°')
        self.assertEqual(f.formatDouble(90, context), '90°')
        self.assertEqual(f.formatDouble(180, context), '180°')
        self.assertEqual(f.formatDouble(270, context), '-90°')
        self.assertEqual(f.formatDouble(5, context), '5°')
        self.assertEqual(f.formatDouble(5.5, context), '5.5°')
        self.assertEqual(f.formatDouble(-5, context), '-5°')
        self.assertEqual(f.formatDouble(-5.5, context), '-5.5°')
        context.setDecimalSeparator('x')
        self.assertEqual(f.formatDouble(0, context), '0°')
        self.assertEqual(f.formatDouble(-5.5, context), '-5x5°')
        self.assertEqual(f.formatDouble(180, context), '180°')
        context.setDecimalSeparator('.')
        f.setNumberDecimalPlaces(0)
        self.assertEqual(f.formatDouble(0, context), '0°')
        self.assertEqual(f.formatDouble(5.5, context), '6°')
        self.assertEqual(f.formatDouble(-5.5, context), '-6°')
        self.assertEqual(f.formatDouble(180, context), '180°')
        f.setNumberDecimalPlaces(3)
        self.assertEqual(f.formatDouble(0, context), '0°')
        self.assertEqual(f.formatDouble(5.5, context), '5.5°')
        self.assertEqual(f.formatDouble(-5.5, context), '-5.5°')
        self.assertEqual(f.formatDouble(180, context), '180°')
        f.setShowTrailingZeros(True)
        self.assertEqual(f.formatDouble(0, context), '0.000°')
        self.assertEqual(f.formatDouble(5, context), '5.000°')
        self.assertEqual(f.formatDouble(-5, context), '-5.000°')
        self.assertEqual(f.formatDouble(5.5, context), '5.500°')
        self.assertEqual(f.formatDouble(-5.5, context), '-5.500°')
        self.assertEqual(f.formatDouble(180, context), '180.000°')

        f = QgsBearingNumericFormat()
        f.setDirectionFormat(QgsBearingNumericFormat.UseRange0To360)
        self.assertEqual(f.formatDouble(0, context), '0°')
        self.assertEqual(f.formatDouble(90, context), '90°')
        self.assertEqual(f.formatDouble(180, context), '180°')
        self.assertEqual(f.formatDouble(270, context), '270°')
        self.assertEqual(f.formatDouble(5, context), '5°')
        self.assertEqual(f.formatDouble(5.5, context), '5.5°')
        self.assertEqual(f.formatDouble(-5, context), '355°')
        self.assertEqual(f.formatDouble(-5.5, context), '354.5°')
        context.setDecimalSeparator('x')
        self.assertEqual(f.formatDouble(0, context), '0°')
        self.assertEqual(f.formatDouble(-5.5, context), '354x5°')
        self.assertEqual(f.formatDouble(180, context), '180°')
        context.setDecimalSeparator('.')
        f.setNumberDecimalPlaces(0)
        self.assertEqual(f.formatDouble(0, context), '0°')
        self.assertEqual(f.formatDouble(5.5, context), '6°')
        self.assertEqual(f.formatDouble(-5.5, context), '354°')
        self.assertEqual(f.formatDouble(180, context), '180°')
        f.setNumberDecimalPlaces(3)
        self.assertEqual(f.formatDouble(0, context), '0°')
        self.assertEqual(f.formatDouble(5.5, context), '5.5°')
        self.assertEqual(f.formatDouble(-5.5, context), '354.5°')
        self.assertEqual(f.formatDouble(180, context), '180°')
        f.setShowTrailingZeros(True)
        self.assertEqual(f.formatDouble(0, context), '0.000°')
        self.assertEqual(f.formatDouble(5, context), '5.000°')
        self.assertEqual(f.formatDouble(-5, context), '355.000°')
        self.assertEqual(f.formatDouble(5.5, context), '5.500°')
        self.assertEqual(f.formatDouble(-5.5, context), '354.500°')
        self.assertEqual(f.formatDouble(180, context), '180.000°')

    def testPercentageFormat(self):
        """ test percentage formatter """
        f = QgsPercentageNumericFormat()
        f.setInputValues(QgsPercentageNumericFormat.ValuesArePercentage)
        context = QgsNumericFormatContext()
        self.assertEqual(f.formatDouble(0, context), '0%')
        self.assertEqual(f.formatDouble(5, context), '5%')
        self.assertEqual(f.formatDouble(5.5, context), '5.5%')
        self.assertEqual(f.formatDouble(-5, context), '-5%')
        self.assertEqual(f.formatDouble(-5.5, context), '-5.5%')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-55,555,555.5%')
        context.setDecimalSeparator('x')
        self.assertEqual(f.formatDouble(0, context), '0%')
        self.assertEqual(f.formatDouble(-5.5, context), '-5x5%')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-55,555,555x5%')
        context.setThousandsSeparator('y')
        self.assertEqual(f.formatDouble(-5.5, context), '-5x5%')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-55y555y555x5%')
        f.setShowThousandsSeparator(False)
        self.assertEqual(f.formatDouble(-5.5, context), '-5x5%')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-55555555x5%')
        context.setDecimalSeparator('.')
        f.setNumberDecimalPlaces(0)
        self.assertEqual(f.formatDouble(0, context), '0%')
        self.assertEqual(f.formatDouble(5.5, context), '6%')
        self.assertEqual(f.formatDouble(55555555.5, context), '55555556%')
        self.assertEqual(f.formatDouble(55555555.123456, context), '55555555%')
        self.assertEqual(f.formatDouble(-5.5, context), '-6%')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-55555556%')
        f.setNumberDecimalPlaces(3)
        self.assertEqual(f.formatDouble(0, context), '0%')
        self.assertEqual(f.formatDouble(5.5, context), '5.5%')
        self.assertEqual(f.formatDouble(55555555.5, context), '55555555.5%')
        self.assertEqual(f.formatDouble(55555555.123456, context), '55555555.123%')
        self.assertEqual(f.formatDouble(-5.5, context), '-5.5%')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-55555555.5%')
        f.setShowTrailingZeros(True)
        self.assertEqual(f.formatDouble(0, context), '0.000%')
        self.assertEqual(f.formatDouble(5, context), '5.000%')
        self.assertEqual(f.formatDouble(-5, context), '-5.000%')
        self.assertEqual(f.formatDouble(5.5, context), '5.500%')
        self.assertEqual(f.formatDouble(55555555.5, context), '55555555.500%')
        self.assertEqual(f.formatDouble(55555555.123456, context), '55555555.123%')
        self.assertEqual(f.formatDouble(-5.5, context), '-5.500%')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-55555555.500%')
        f.setShowPlusSign(True)
        self.assertEqual(f.formatDouble(0, context), '0.000%')
        self.assertEqual(f.formatDouble(5, context), '+5.000%')
        self.assertEqual(f.formatDouble(-5, context), '-5.000%')
        self.assertEqual(f.formatDouble(5.5, context), '+5.500%')
        self.assertEqual(f.formatDouble(55555555.5, context), '+55555555.500%')
        self.assertEqual(f.formatDouble(55555555.123456, context), '+55555555.123%')
        self.assertEqual(f.formatDouble(-5.5, context), '-5.500%')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-55555555.500%')

        f = QgsPercentageNumericFormat()
        f.setInputValues(QgsPercentageNumericFormat.ValuesAreFractions)
        context = QgsNumericFormatContext()
        self.assertEqual(f.formatDouble(0, context), '0%')
        self.assertEqual(f.formatDouble(5, context), '500%')
        self.assertEqual(f.formatDouble(5.5, context), '550%')
        self.assertEqual(f.formatDouble(-5, context), '-500%')
        self.assertEqual(f.formatDouble(-5.5, context), '-550%')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-5,555,555,550%')
        context.setDecimalSeparator('x')
        self.assertEqual(f.formatDouble(0, context), '0%')
        self.assertEqual(f.formatDouble(-5.5, context), '-550%')
        self.assertEqual(f.formatDouble(-0.005, context), '-0x5%')
        context.setThousandsSeparator('y')
        self.assertEqual(f.formatDouble(-5.5, context), '-550%')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-5y555y555y550%')
        f.setShowThousandsSeparator(False)
        self.assertEqual(f.formatDouble(-5.5, context), '-550%')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-5555555550%')
        context.setDecimalSeparator('.')
        f.setNumberDecimalPlaces(0)
        self.assertEqual(f.formatDouble(0, context), '0%')
        self.assertEqual(f.formatDouble(5.5, context), '550%')
        self.assertEqual(f.formatDouble(55555555.5, context), '5555555550%')
        self.assertEqual(f.formatDouble(0.123456, context), '12%')
        self.assertEqual(f.formatDouble(-5.5, context), '-550%')
        self.assertEqual(f.formatDouble(-0.123456, context), '-12%')
        f.setNumberDecimalPlaces(3)
        self.assertEqual(f.formatDouble(0, context), '0%')
        self.assertEqual(f.formatDouble(5.5, context), '550%')
        self.assertEqual(f.formatDouble(55555555.5, context), '5555555550%')
        self.assertEqual(f.formatDouble(0.123456, context), '12.346%')
        self.assertEqual(f.formatDouble(-5.5, context), '-550%')
        self.assertEqual(f.formatDouble(-0.123456, context), '-12.346%')
        f.setShowTrailingZeros(True)
        self.assertEqual(f.formatDouble(0, context), '0.000%')
        self.assertEqual(f.formatDouble(5, context), '500.000%')
        self.assertEqual(f.formatDouble(-5, context), '-500.000%')
        self.assertEqual(f.formatDouble(0.5, context), '50.000%')
        self.assertEqual(f.formatDouble(55555555.5, context), '5555555550.000%')
        self.assertEqual(f.formatDouble(0.123456, context), '12.346%')
        self.assertEqual(f.formatDouble(-5.5, context), '-550.000%')
        self.assertEqual(f.formatDouble(-1234.5, context), '-123450.000%')
        f.setShowPlusSign(True)
        self.assertEqual(f.formatDouble(0, context), '0.000%')
        self.assertEqual(f.formatDouble(5, context), '+500.000%')
        self.assertEqual(f.formatDouble(-5, context), '-500.000%')
        self.assertEqual(f.formatDouble(5.5, context), '+550.000%')
        self.assertEqual(f.formatDouble(-5.5, context), '-550.000%')

    def testRegistry(self):
        registry = QgsNumericFormatRegistry()
        self.assertTrue(registry.formats())
        for f in registry.formats():
            self.assertEqual(registry.format(f).id(), f)

        self.assertNotIn('default', registry.formats())
        registry.addFormat(TestFormat())
        self.assertIn('test', registry.formats())
        self.assertTrue(isinstance(registry.format('test'), TestFormat))
        self.assertTrue(isinstance(registry.create('test', {}), TestFormat))

        registry.removeFormat('test')

        self.assertNotIn('test', registry.formats())
        self.assertTrue(isinstance(registry.format('test'), QgsFallbackNumericFormat))
        self.assertTrue(isinstance(registry.create('test', {}), QgsFallbackNumericFormat))

        self.assertTrue(isinstance(registry.fallbackFormat(), QgsFallbackNumericFormat))


if __name__ == '__main__':
    unittest.main()
