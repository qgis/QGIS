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
                       QgsScientificNumericFormat,
                       QgsCurrencyNumericFormat,
                       QgsNumericFormatRegistry,
                       QgsNumericFormat,
                       QgsFractionNumericFormat,
                       QgsGeographicCoordinateNumericFormat,
                       QgsReadWriteContext)
from qgis.testing import start_app, unittest
from qgis.PyQt.QtXml import QDomDocument

start_app()


class TestFormat(QgsNumericFormat):

    def id(self):
        return 'test'

    def formatDouble(self, value, context):
        return 'xxx' + str(value)

    def visibleName(self):
        return 'Test'

    def clone(self):
        return TestFormat()

    def create(self, configuration, context):
        return TestFormat()

    def configuration(self, context):
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

        f2 = f.clone()
        self.assertIsInstance(f2, QgsFallbackNumericFormat)

        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        f2.writeXml(elem, doc, QgsReadWriteContext())

        f3 = QgsNumericFormatRegistry().createFromXml(elem, QgsReadWriteContext())
        self.assertIsInstance(f3, QgsFallbackNumericFormat)

    def testEquality(self):
        f = QgsBasicNumericFormat()
        f2 = QgsBasicNumericFormat()
        self.assertEqual(f, f2)
        f2.setShowPlusSign(True)
        self.assertNotEqual(f, f2)
        f.setShowPlusSign(True)
        self.assertEqual(f, f2)
        self.assertNotEqual(f, QgsCurrencyNumericFormat())

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
        context.setDecimalSeparator('☕')
        self.assertEqual(f.formatDouble(0, context), '0')
        self.assertEqual(f.formatDouble(-5.5, context), '-5☕5')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-55,555,555☕5')
        context.setThousandsSeparator('⚡')
        self.assertEqual(f.formatDouble(-5.5, context), '-5☕5')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-55⚡555⚡555☕5')
        f.setShowThousandsSeparator(False)
        self.assertEqual(f.formatDouble(-5.5, context), '-5☕5')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-55555555☕5')
        context.setDecimalSeparator('.')
        f.setDecimalSeparator('⛹')
        self.assertEqual(f.formatDouble(-5.5, context), '-5⛹5')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-55555555⛹5')
        f.setNumberDecimalPlaces(0)
        self.assertEqual(f.formatDouble(0, context), '0')
        self.assertEqual(f.formatDouble(5.5, context), '6')
        self.assertEqual(f.formatDouble(55555555.5, context), '55555556')
        self.assertEqual(f.formatDouble(55555555.123456, context), '55555555')
        self.assertEqual(f.formatDouble(-5.5, context), '-6')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-55555556')
        f.setNumberDecimalPlaces(3)
        self.assertEqual(f.formatDouble(0, context), '0')
        self.assertEqual(f.formatDouble(5.5, context), '5⛹5')
        self.assertEqual(f.formatDouble(55555555.5, context), '55555555⛹5')
        self.assertEqual(f.formatDouble(55555555.123456, context), '55555555⛹123')
        self.assertEqual(f.formatDouble(-5.5, context), '-5⛹5')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-55555555⛹5')
        f.setShowTrailingZeros(True)
        self.assertEqual(f.formatDouble(0, context), '0⛹000')
        self.assertEqual(f.formatDouble(5, context), '5⛹000')
        self.assertEqual(f.formatDouble(-5, context), '-5⛹000')
        self.assertEqual(f.formatDouble(5.5, context), '5⛹500')
        self.assertEqual(f.formatDouble(55555555.5, context), '55555555⛹500')
        self.assertEqual(f.formatDouble(55555555.123456, context), '55555555⛹123')
        self.assertEqual(f.formatDouble(-5.5, context), '-5⛹500')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-55555555⛹500')
        f.setShowPlusSign(True)
        self.assertEqual(f.formatDouble(0, context), '0⛹000')
        self.assertEqual(f.formatDouble(5, context), '+5⛹000')
        self.assertEqual(f.formatDouble(-5, context), '-5⛹000')
        self.assertEqual(f.formatDouble(5.5, context), '+5⛹500')
        self.assertEqual(f.formatDouble(55555555.5, context), '+55555555⛹500')
        self.assertEqual(f.formatDouble(55555555.123456, context), '+55555555⛹123')
        self.assertEqual(f.formatDouble(-5.5, context), '-5⛹500')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-55555555⛹500')
        context.setPositiveSign('w')
        self.assertEqual(f.formatDouble(5, context), 'w5⛹000')
        self.assertEqual(f.formatDouble(-5, context), '-5⛹000')
        self.assertEqual(f.formatDouble(5.5, context), 'w5⛹500')

        f.setShowPlusSign(False)
        f.setRoundingType(QgsBasicNumericFormat.SignificantFigures)
        self.assertEqual(f.formatDouble(0, context), '0⛹00')
        self.assertEqual(f.formatDouble(5, context), '5⛹00')
        self.assertEqual(f.formatDouble(-5, context), '-5⛹00')
        self.assertEqual(f.formatDouble(5.5, context), '5⛹50')
        self.assertEqual(f.formatDouble(1231.23123123123123, context), '1230')
        self.assertEqual(f.formatDouble(123.123123123123123, context), '123')
        self.assertEqual(f.formatDouble(12.3123123123123123, context), '12⛹3')
        self.assertEqual(f.formatDouble(1.23123123123123123, context), '1⛹23')
        self.assertEqual(f.formatDouble(-1231.23123123123123, context), '-1230')
        self.assertEqual(f.formatDouble(-123.123123123123123, context), '-123')
        self.assertEqual(f.formatDouble(-12.3123123123123123, context), '-12⛹3')
        self.assertEqual(f.formatDouble(-1.23123123123123123, context), '-1⛹23')
        self.assertEqual(f.formatDouble(100, context), '100')
        self.assertEqual(f.formatDouble(1000, context), '1000')
        self.assertEqual(f.formatDouble(1001, context), '1000')
        self.assertEqual(f.formatDouble(9999, context), '10000')
        self.assertEqual(f.formatDouble(10, context), '10⛹0')
        self.assertEqual(f.formatDouble(1, context), '1⛹00')
        self.assertEqual(f.formatDouble(0.00000123456, context), '0⛹00000123')
        self.assertEqual(f.formatDouble(55555555.5, context), '55600000')
        self.assertEqual(f.formatDouble(55555555.123456, context), '55600000')
        self.assertEqual(f.formatDouble(-5.5, context), '-5⛹50')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-55600000')

        f.setThousandsSeparator('✅')
        f.setShowThousandsSeparator(True)
        self.assertEqual(f.formatDouble(-55555555.5, context), '-55✅600✅000')
        f.setShowThousandsSeparator(False)

        f.setShowPlusSign(True)

        f2 = f.clone()
        self.assertIsInstance(f2, QgsBasicNumericFormat)

        self.assertEqual(f2.showTrailingZeros(), f.showTrailingZeros())
        self.assertEqual(f2.showPlusSign(), f.showPlusSign())
        self.assertEqual(f2.numberDecimalPlaces(), f.numberDecimalPlaces())
        self.assertEqual(f2.showThousandsSeparator(), f.showThousandsSeparator())
        self.assertEqual(f2.roundingType(), f.roundingType())
        self.assertEqual(f2.thousandsSeparator(), f.thousandsSeparator())
        self.assertEqual(f2.decimalSeparator(), f.decimalSeparator())

        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        f2.writeXml(elem, doc, QgsReadWriteContext())

        f3 = QgsNumericFormatRegistry().createFromXml(elem, QgsReadWriteContext())
        self.assertIsInstance(f3, QgsBasicNumericFormat)

        self.assertEqual(f3.showTrailingZeros(), f.showTrailingZeros())
        self.assertEqual(f3.showPlusSign(), f.showPlusSign())
        self.assertEqual(f3.numberDecimalPlaces(), f.numberDecimalPlaces())
        self.assertEqual(f3.showThousandsSeparator(), f.showThousandsSeparator())
        self.assertEqual(f3.roundingType(), f.roundingType())
        self.assertEqual(f3.thousandsSeparator(), f.thousandsSeparator())
        self.assertEqual(f3.decimalSeparator(), f.decimalSeparator())

    def testCurrencyFormat(self):
        """ test currency formatter """
        f = QgsCurrencyNumericFormat()
        f.setPrefix('$')
        context = QgsNumericFormatContext()
        f.setShowTrailingZeros(False)
        self.assertEqual(f.formatDouble(0, context), '$0')
        self.assertEqual(f.formatDouble(5, context), '$5')
        self.assertEqual(f.formatDouble(5.5, context), '$5.5')
        self.assertEqual(f.formatDouble(-5, context), '-$5')
        self.assertEqual(f.formatDouble(-5.5, context), '-$5.5')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-$55,555,555.5')
        context.setDecimalSeparator('x')
        self.assertEqual(f.formatDouble(0, context), '$0')
        self.assertEqual(f.formatDouble(-5.5, context), '-$5x5')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-$55,555,555x5')
        context.setThousandsSeparator('y')
        self.assertEqual(f.formatDouble(-5.5, context), '-$5x5')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-$55y555y555x5')
        f.setShowThousandsSeparator(False)
        self.assertEqual(f.formatDouble(-5.5, context), '-$5x5')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-$55555555x5')
        context.setDecimalSeparator('.')
        f.setNumberDecimalPlaces(0)
        self.assertEqual(f.formatDouble(0, context), '$0')
        self.assertEqual(f.formatDouble(5.5, context), '$6')
        self.assertEqual(f.formatDouble(55555555.5, context), '$55555556')
        self.assertEqual(f.formatDouble(55555555.123456, context), '$55555555')
        self.assertEqual(f.formatDouble(-5.5, context), '-$6')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-$55555556')
        f.setNumberDecimalPlaces(3)
        self.assertEqual(f.formatDouble(0, context), '$0')
        self.assertEqual(f.formatDouble(5.5, context), '$5.5')
        self.assertEqual(f.formatDouble(55555555.5, context), '$55555555.5')
        self.assertEqual(f.formatDouble(55555555.123456, context), '$55555555.123')
        self.assertEqual(f.formatDouble(-5.5, context), '-$5.5')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-$55555555.5')
        f.setShowTrailingZeros(True)
        self.assertEqual(f.formatDouble(0, context), '$0.000')
        self.assertEqual(f.formatDouble(5, context), '$5.000')
        self.assertEqual(f.formatDouble(-5, context), '-$5.000')
        self.assertEqual(f.formatDouble(5.5, context), '$5.500')
        self.assertEqual(f.formatDouble(55555555.5, context), '$55555555.500')
        self.assertEqual(f.formatDouble(55555555.123456, context), '$55555555.123')
        self.assertEqual(f.formatDouble(-5.5, context), '-$5.500')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-$55555555.500')
        f.setShowPlusSign(True)
        self.assertEqual(f.formatDouble(0, context), '$0.000')
        self.assertEqual(f.formatDouble(5, context), '+$5.000')
        self.assertEqual(f.formatDouble(-5, context), '-$5.000')
        self.assertEqual(f.formatDouble(5.5, context), '+$5.500')
        self.assertEqual(f.formatDouble(55555555.5, context), '+$55555555.500')
        self.assertEqual(f.formatDouble(55555555.123456, context), '+$55555555.123')
        self.assertEqual(f.formatDouble(-5.5, context), '-$5.500')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-$55555555.500')

        f.setSuffix('AUD')
        self.assertEqual(f.formatDouble(0, context), '$0.000AUD')
        self.assertEqual(f.formatDouble(5, context), '+$5.000AUD')
        self.assertEqual(f.formatDouble(-5, context), '-$5.000AUD')
        self.assertEqual(f.formatDouble(5.5, context), '+$5.500AUD')
        self.assertEqual(f.formatDouble(55555555.5, context), '+$55555555.500AUD')
        self.assertEqual(f.formatDouble(55555555.123456, context), '+$55555555.123AUD')
        self.assertEqual(f.formatDouble(-5.5, context), '-$5.500AUD')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-$55555555.500AUD')

        f2 = f.clone()
        self.assertIsInstance(f2, QgsCurrencyNumericFormat)

        self.assertEqual(f2.showTrailingZeros(), f.showTrailingZeros())
        self.assertEqual(f2.showPlusSign(), f.showPlusSign())
        self.assertEqual(f2.numberDecimalPlaces(), f.numberDecimalPlaces())
        self.assertEqual(f2.showThousandsSeparator(), f.showThousandsSeparator())
        self.assertEqual(f2.prefix(), f.prefix())
        self.assertEqual(f2.suffix(), f.suffix())

        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        f2.writeXml(elem, doc, QgsReadWriteContext())

        f3 = QgsNumericFormatRegistry().createFromXml(elem, QgsReadWriteContext())
        self.assertIsInstance(f3, QgsCurrencyNumericFormat)

        self.assertEqual(f3.showTrailingZeros(), f.showTrailingZeros())
        self.assertEqual(f3.showPlusSign(), f.showPlusSign())
        self.assertEqual(f3.numberDecimalPlaces(), f.numberDecimalPlaces())
        self.assertEqual(f3.showThousandsSeparator(), f.showThousandsSeparator())
        self.assertEqual(f3.prefix(), f.prefix())
        self.assertEqual(f3.suffix(), f.suffix())

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
        self.assertEqual(f.formatDouble(0, context), '0.000°E')  # todo - fix and avoid E
        self.assertEqual(f.formatDouble(5, context), '5.000°E')
        self.assertEqual(f.formatDouble(-5, context), '5.000°W')
        self.assertEqual(f.formatDouble(5.5, context), '5.500°E')
        self.assertEqual(f.formatDouble(-5.5, context), '5.500°W')
        self.assertEqual(f.formatDouble(180, context), '180.000°E')  # todo fix and avoid E

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
        self.assertEqual(f.formatDouble(-5.4, context), '355°')
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

        f2 = f.clone()
        self.assertIsInstance(f2, QgsBearingNumericFormat)

        self.assertEqual(f2.showTrailingZeros(), f.showTrailingZeros())
        self.assertEqual(f2.showPlusSign(), f.showPlusSign())
        self.assertEqual(f2.numberDecimalPlaces(), f.numberDecimalPlaces())
        self.assertEqual(f2.showThousandsSeparator(), f.showThousandsSeparator())
        self.assertEqual(f2.directionFormat(), f.directionFormat())

        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        f2.writeXml(elem, doc, QgsReadWriteContext())

        f3 = QgsNumericFormatRegistry().createFromXml(elem, QgsReadWriteContext())
        self.assertIsInstance(f3, QgsBearingNumericFormat)

        self.assertEqual(f3.showTrailingZeros(), f.showTrailingZeros())
        self.assertEqual(f3.showPlusSign(), f.showPlusSign())
        self.assertEqual(f3.numberDecimalPlaces(), f.numberDecimalPlaces())
        self.assertEqual(f3.showThousandsSeparator(), f.showThousandsSeparator())
        self.assertEqual(f3.directionFormat(), f.directionFormat())

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

        context.setPercent('p')
        self.assertEqual(f.formatDouble(0, context), '0.000p')
        self.assertEqual(f.formatDouble(5, context), '+500.000p')
        self.assertEqual(f.formatDouble(-5, context), '-500.000p')

        f2 = f.clone()
        self.assertIsInstance(f2, QgsPercentageNumericFormat)

        self.assertEqual(f2.showTrailingZeros(), f.showTrailingZeros())
        self.assertEqual(f2.showPlusSign(), f.showPlusSign())
        self.assertEqual(f2.numberDecimalPlaces(), f.numberDecimalPlaces())
        self.assertEqual(f2.showThousandsSeparator(), f.showThousandsSeparator())
        self.assertEqual(f2.inputValues(), f.inputValues())

        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        f2.writeXml(elem, doc, QgsReadWriteContext())

        f3 = QgsNumericFormatRegistry().createFromXml(elem, QgsReadWriteContext())
        self.assertIsInstance(f3, QgsPercentageNumericFormat)

        self.assertEqual(f3.showTrailingZeros(), f.showTrailingZeros())
        self.assertEqual(f3.showPlusSign(), f.showPlusSign())
        self.assertEqual(f3.numberDecimalPlaces(), f.numberDecimalPlaces())
        self.assertEqual(f3.showThousandsSeparator(), f.showThousandsSeparator())
        self.assertEqual(f3.inputValues(), f.inputValues())

    def testScientificFormat(self):
        """ test scientific formatter """
        f = QgsScientificNumericFormat()
        context = QgsNumericFormatContext()
        self.assertEqual(f.formatDouble(0, context), '0e+00')
        self.assertEqual(f.formatDouble(5, context), '5e+00')
        self.assertEqual(f.formatDouble(5.5, context), '5.5e+00')
        self.assertEqual(f.formatDouble(-5, context), '-5e+00')
        self.assertEqual(f.formatDouble(-5.5, context), '-5.5e+00')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-5.555556e+07')
        context.setDecimalSeparator('x')
        self.assertEqual(f.formatDouble(0, context), '0e+00')
        self.assertEqual(f.formatDouble(-5.5, context), '-5x5e+00')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-5x555556e+07')
        context.setDecimalSeparator('.')

        # places must be at least 1 for scientific notation!
        f.setNumberDecimalPlaces(0)
        self.assertEqual(f.numberDecimalPlaces(), 1)
        self.assertEqual(f.formatDouble(0, context), '0e+00')
        self.assertEqual(f.formatDouble(5.5, context), '5.5e+00')
        self.assertEqual(f.formatDouble(55555555.5, context), '5.6e+07')
        self.assertEqual(f.formatDouble(55555555.123456, context), '5.6e+07')
        self.assertEqual(f.formatDouble(-5.5, context), '-5.5e+00')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-5.6e+07')

        f.setNumberDecimalPlaces(3)
        self.assertEqual(f.formatDouble(0, context), '0e+00')
        self.assertEqual(f.formatDouble(5.5, context), '5.5e+00')
        self.assertEqual(f.formatDouble(55555555.5, context), '5.556e+07')
        self.assertEqual(f.formatDouble(55555555.123456, context), '5.556e+07')
        self.assertEqual(f.formatDouble(-5.5, context), '-5.5e+00')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-5.556e+07')
        f.setShowTrailingZeros(True)
        self.assertEqual(f.formatDouble(0, context), '0.000e+00')
        self.assertEqual(f.formatDouble(5, context), '5.000e+00')
        self.assertEqual(f.formatDouble(-5, context), '-5.000e+00')
        self.assertEqual(f.formatDouble(5.5, context), '5.500e+00')
        self.assertEqual(f.formatDouble(55555555.5, context), '5.556e+07')
        self.assertEqual(f.formatDouble(55555555.123456, context), '5.556e+07')
        self.assertEqual(f.formatDouble(-5.5, context), '-5.500e+00')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-5.556e+07')
        f.setShowPlusSign(True)
        self.assertEqual(f.formatDouble(0, context), '0.000e+00')
        self.assertEqual(f.formatDouble(5, context), '+5.000e+00')
        self.assertEqual(f.formatDouble(-5, context), '-5.000e+00')
        self.assertEqual(f.formatDouble(5.5, context), '+5.500e+00')
        self.assertEqual(f.formatDouble(55555555.5, context), '+5.556e+07')
        self.assertEqual(f.formatDouble(55555555.123456, context), '+5.556e+07')
        self.assertEqual(f.formatDouble(-5.5, context), '-5.500e+00')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-5.556e+07')

        f2 = f.clone()
        self.assertIsInstance(f2, QgsScientificNumericFormat)

        self.assertEqual(f2.showTrailingZeros(), f.showTrailingZeros())
        self.assertEqual(f2.showPlusSign(), f.showPlusSign())
        self.assertEqual(f2.numberDecimalPlaces(), f.numberDecimalPlaces())
        self.assertEqual(f2.showThousandsSeparator(), f.showThousandsSeparator())

        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        f2.writeXml(elem, doc, QgsReadWriteContext())

        f3 = QgsNumericFormatRegistry().createFromXml(elem, QgsReadWriteContext())
        self.assertIsInstance(f3, QgsScientificNumericFormat)

        self.assertEqual(f3.showTrailingZeros(), f.showTrailingZeros())
        self.assertEqual(f3.showPlusSign(), f.showPlusSign())
        self.assertEqual(f3.numberDecimalPlaces(), f.numberDecimalPlaces())
        self.assertEqual(f3.showThousandsSeparator(), f.showThousandsSeparator())

    def testDoubleToFraction(self):
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(1), (True, 1, 1, 1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(2), (True, 2, 1, 1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(-1), (True, 1, 1, -1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(-2), (True, 2, 1, -1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(0), (True, 0, 1, 1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(1000000), (True, 1000000, 1, 1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(-1000000), (True, 1000000, 1, -1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(0.5), (True, 1, 2, 1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(0.25), (True, 1, 4, 1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(0.75), (True, 3, 4, 1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(-0.5), (True, 1, 2, -1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(-0.25), (True, 1, 4, -1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(-0.75), (True, 3, 4, -1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(1.5), (True, 3, 2, 1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(1.25), (True, 5, 4, 1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(1.75), (True, 7, 4, 1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(-0.5), (True, 1, 2, -1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(-0.25), (True, 1, 4, -1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(-0.1), (True, 1, 10, -1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(-1.5), (True, 3, 2, -1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(-1.25), (True, 5, 4, -1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(-1.75), (True, 7, 4, -1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(0.3333333333333333333333), (True, 1, 3, 1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(0.333333333), (True, 333333355, 1000000066, 1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(0.333333333, 0.0000000001),
                         (True, 333333355, 1000000066, 1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(0.333333333, 0.000000001), (True, 1, 3, 1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(0.333333333, 0.1), (True, 1, 3, 1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(-0.3333333333333333333333), (True, 1, 3, -1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(-0.333333333),
                         (True, 333333355, 1000000066, -1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(-0.333333333, 0.0000000001),
                         (True, 333333355, 1000000066, -1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(-0.333333333, 0.000000001), (True, 1, 3, -1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(-0.333333333, 0.1), (True, 1, 3, -1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(0.000000123123), (True, 1, 8121959, 1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(3.14159265358979), (True, 312689, 99532, 1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(3.14159265358979, 0.0000001),
                         (True, 103993, 33102, 1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(3.14159265358979, 0.00001),
                         (True, 355, 113, 1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(3.14159265358979, 0.001), (True, 333, 106, 1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(3.14159265358979, 0.1), (True, 22, 7, 1))
        self.assertEqual(QgsFractionNumericFormat.doubleToVulgarFraction(3.14159265358979, 1), (True, 3, 1, 1))

    def testToUnicodeSuperscript(self):
        self.assertEqual(QgsFractionNumericFormat.toUnicodeSuperscript(''), '')
        self.assertEqual(QgsFractionNumericFormat.toUnicodeSuperscript('asd'), 'asd')
        self.assertEqual(QgsFractionNumericFormat.toUnicodeSuperscript('1234567890'), '¹²³⁴⁵⁶⁷⁸⁹⁰')
        self.assertEqual(QgsFractionNumericFormat.toUnicodeSuperscript('aa112233bbcc'), 'aa¹¹²²³³bbcc')

    def testToUnicodeSubcript(self):
        self.assertEqual(QgsFractionNumericFormat.toUnicodeSubscript(''), '')
        self.assertEqual(QgsFractionNumericFormat.toUnicodeSubscript('asd'), 'asd')
        self.assertEqual(QgsFractionNumericFormat.toUnicodeSubscript('1234567890'), '₁₂₃₄₅₆₇₈₉₀')
        self.assertEqual(QgsFractionNumericFormat.toUnicodeSubscript('aa112233bbcc'), 'aa₁₁₂₂₃₃bbcc')

    def testFractionFormat(self):
        """ test fraction formatter """
        f = QgsFractionNumericFormat()
        f.setUseUnicodeSuperSubscript(False)
        context = QgsNumericFormatContext()
        self.assertEqual(f.formatDouble(0, context), '0')
        self.assertEqual(f.formatDouble(5, context), '5')
        self.assertEqual(f.formatDouble(5.5, context), '5 1/2')
        self.assertEqual(f.formatDouble(-5, context), '-5')
        self.assertEqual(f.formatDouble(-5.5, context), '-5 1/2')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-55,555,555 1/2')
        context.setThousandsSeparator('⚡')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-55⚡555⚡555 1/2')
        f.setShowThousandsSeparator(False)
        self.assertEqual(f.formatDouble(-55555555.5, context), '-55555555 1/2')
        f.setShowPlusSign(True)
        self.assertEqual(f.formatDouble(0, context), '0')
        self.assertEqual(f.formatDouble(5, context), '+5')
        self.assertEqual(f.formatDouble(-5, context), '-5')
        self.assertEqual(f.formatDouble(5.5, context), '+5 1/2')
        self.assertEqual(f.formatDouble(-5.5, context), '-5 1/2')
        self.assertEqual(f.formatDouble(55555555.5, context), '+55555555 1/2')
        self.assertEqual(f.formatDouble(55555555.123456, context), '+55555555 5797/46956')
        self.assertEqual(f.formatDouble(-5.5, context), '-5 1/2')
        self.assertEqual(f.formatDouble(-55555555.5, context), '-55555555 1/2')
        context.setPositiveSign('w')
        self.assertEqual(f.formatDouble(5, context), 'w5')
        self.assertEqual(f.formatDouble(-5, context), '-5')
        self.assertEqual(f.formatDouble(5.5, context), 'w5 1/2')

        f.setShowPlusSign(False)
        f.setUseDedicatedUnicodeCharacters(True)
        self.assertEqual(f.formatDouble(0, context), '0')
        self.assertEqual(f.formatDouble(5, context), '5')
        self.assertEqual(f.formatDouble(5.5, context), '5 ½')
        self.assertEqual(f.formatDouble(-5, context), '-5')
        self.assertEqual(f.formatDouble(-5.5, context), '-5 ½')
        self.assertEqual(f.formatDouble(5.333333333333333333333333333, context), '5 ⅓')
        self.assertEqual(f.formatDouble(5.666666666666666666666666666, context), '5 ⅔')
        self.assertEqual(f.formatDouble(5.25, context), '5 ¼')
        self.assertEqual(f.formatDouble(5.75, context), '5 ¾')
        self.assertEqual(f.formatDouble(5.2, context), '5 ⅕')
        self.assertEqual(f.formatDouble(5.4, context), '5 ⅖')
        self.assertEqual(f.formatDouble(5.6, context), '5 ⅗')
        self.assertEqual(f.formatDouble(5.8, context), '5 ⅘')
        self.assertEqual(f.formatDouble(5.1666666666666666666666666666666666, context), '5 ⅙')
        self.assertEqual(f.formatDouble(5.8333333333333333333333333333333333, context), '5 ⅚')
        self.assertEqual(f.formatDouble(5.14285714285714285, context), '5 ⅐')
        self.assertEqual(f.formatDouble(5.125, context), '5 ⅛')
        self.assertEqual(f.formatDouble(5.375, context), '5 ⅜')
        self.assertEqual(f.formatDouble(5.625, context), '5 ⅝')
        self.assertEqual(f.formatDouble(5.875, context), '5 ⅞')
        self.assertEqual(f.formatDouble(5.1111111111111111, context), '5 ⅑')
        self.assertEqual(f.formatDouble(5.1, context), '5 ⅒')
        self.assertEqual(f.formatDouble(5.13131313133, context), '5 13/99')

        f.setUseUnicodeSuperSubscript(True)
        self.assertEqual(f.formatDouble(0, context), '0')
        self.assertEqual(f.formatDouble(5, context), '5')
        self.assertEqual(f.formatDouble(5.5, context), '5 ½')
        self.assertEqual(f.formatDouble(-5, context), '-5')
        self.assertEqual(f.formatDouble(-5.5, context), '-5 ½')
        self.assertEqual(f.formatDouble(5.55555555, context), '5 ¹¹¹¹¹¹¹¹/₂₀₀₀₀₀₀₀')
        self.assertEqual(f.formatDouble(-5.55555555, context), '-5 ¹¹¹¹¹¹¹¹/₂₀₀₀₀₀₀₀')
        self.assertEqual(f.formatDouble(0.555, context), '¹¹¹/₂₀₀')

        f.setShowPlusSign(True)
        f.setUseUnicodeSuperSubscript(False)

        f2 = f.clone()
        self.assertIsInstance(f2, QgsFractionNumericFormat)

        self.assertEqual(f2.showPlusSign(), f.showPlusSign())
        self.assertEqual(f2.showThousandsSeparator(), f.showThousandsSeparator())
        self.assertEqual(f2.thousandsSeparator(), f.thousandsSeparator())
        self.assertEqual(f2.useDedicatedUnicodeCharacters(), f.useDedicatedUnicodeCharacters())
        self.assertEqual(f2.useUnicodeSuperSubscript(), f.useUnicodeSuperSubscript())

        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        f2.writeXml(elem, doc, QgsReadWriteContext())

        f3 = QgsNumericFormatRegistry().createFromXml(elem, QgsReadWriteContext())
        self.assertIsInstance(f3, QgsFractionNumericFormat)

        self.assertEqual(f3.showPlusSign(), f.showPlusSign())
        self.assertEqual(f3.showThousandsSeparator(), f.showThousandsSeparator())
        self.assertEqual(f3.thousandsSeparator(), f.thousandsSeparator())
        self.assertEqual(f3.useDedicatedUnicodeCharacters(), f.useDedicatedUnicodeCharacters())
        self.assertEqual(f3.useUnicodeSuperSubscript(), f.useUnicodeSuperSubscript())

    def testGeographicFormat(self):
        """ test geographic formatter """
        f = QgsGeographicCoordinateNumericFormat()
        f.setNumberDecimalPlaces(3)
        f.setShowDirectionalSuffix(True)
        f.setAngleFormat(QgsGeographicCoordinateNumericFormat.AngleFormat.DegreesMinutes)
        f.setShowLeadingZeros(True)
        f.setShowDegreeLeadingZeros(True)

        f2 = f.clone()
        self.assertIsInstance(f2, QgsGeographicCoordinateNumericFormat)

        self.assertEqual(f2.numberDecimalPlaces(), f.numberDecimalPlaces())
        self.assertEqual(f2.showDirectionalSuffix(), f.showDirectionalSuffix())
        self.assertEqual(f2.angleFormat(), f.angleFormat())
        self.assertEqual(f2.showLeadingZeros(), f.showLeadingZeros())
        self.assertEqual(f2.showDegreeLeadingZeros(), f.showDegreeLeadingZeros())

        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        f2.writeXml(elem, doc, QgsReadWriteContext())

        f3 = QgsNumericFormatRegistry().createFromXml(elem, QgsReadWriteContext())
        self.assertIsInstance(f3, QgsGeographicCoordinateNumericFormat)

        self.assertEqual(f3.numberDecimalPlaces(), f.numberDecimalPlaces())
        self.assertEqual(f3.showDirectionalSuffix(), f.showDirectionalSuffix())
        self.assertEqual(f3.angleFormat(), f.angleFormat())
        self.assertEqual(f3.showLeadingZeros(), f.showLeadingZeros())
        self.assertEqual(f3.showDegreeLeadingZeros(), f.showDegreeLeadingZeros())

    def testGeographicCoordinateFormatLongitudeDms(self):
        """Test formatting longitude as DMS"""

        f = QgsGeographicCoordinateNumericFormat()
        context = QgsNumericFormatContext()
        context.setInterpretation(QgsNumericFormatContext.Interpretation.Longitude)

        f.setAngleFormat(QgsGeographicCoordinateNumericFormat.AngleFormat.DegreesMinutesSeconds)
        f.setNumberDecimalPlaces(2)
        f.setShowDirectionalSuffix(True)
        f.setShowTrailingZeros(True)

        self.assertEqual(f.formatDouble(80, context), "80°0′0.00″E")

        # check precision
        f.setNumberDecimalPlaces(4)
        self.assertEqual(f.formatDouble(80, context), "80°0′0.0000″E")
        self.assertEqual(f.formatDouble(80.12345678, context), "80°7′24.4444″E")
        f.setNumberDecimalPlaces(0)
        self.assertEqual(f.formatDouble(80.12345678, context), "80°7′24″E")

        # check if longitudes > 180 or <-180 wrap around
        f.setNumberDecimalPlaces(2)
        self.assertEqual(f.formatDouble(370, context),
                         "10°0′0.00″E")
        self.assertEqual(f.formatDouble(-370, context),
                         "10°0′0.00″W")
        self.assertEqual(f.formatDouble(181, context),
                         "179°0′0.00″W")
        self.assertEqual(f.formatDouble(-181, context),
                         "179°0′0.00″E")
        self.assertEqual(f.formatDouble(359, context),
                         "1°0′0.00″W")
        self.assertEqual(f.formatDouble(-359, context),
                         "1°0′0.00″E")

        # should be no directional suffixes for 0 degree coordinates
        self.assertEqual(f.formatDouble(0, context),
                         "0°0′0.00″")
        # should also be no directional suffix for 0 degree coordinates within specified precision
        self.assertEqual(f.formatDouble(-0.000001, context),
                         "0°0′0.00″")
        f.setNumberDecimalPlaces(5)
        self.assertEqual(
            f.formatDouble(-0.000001, context),
            "0°0′0.00360″W")
        f.setNumberDecimalPlaces(2)
        self.assertEqual(
            f.formatDouble(0.000001, context),
            "0°0′0.00″")
        f.setNumberDecimalPlaces(5)
        self.assertEqual(
            f.formatDouble(0.000001, context),
            "0°0′0.00360″E")

        # should be no directional suffixes for 180 degree longitudes
        f.setNumberDecimalPlaces(2)
        self.assertEqual(f.formatDouble(180, context),
                         "180°0′0.00″")
        self.assertEqual(
            f.formatDouble(179.999999, context),
            "180°0′0.00″")
        f.setNumberDecimalPlaces(5)
        self.assertEqual(
            f.formatDouble(179.999999, context),
            "179°59′59.99640″E")
        f.setNumberDecimalPlaces(2)
        self.assertEqual(
            f.formatDouble(180.000001, context),
            "180°0′0.00″")
        f.setNumberDecimalPlaces(5)
        self.assertEqual(
            f.formatDouble(180.000001, context),
            "179°59′59.99640″W")

        # test rounding does not create seconds >= 60
        f.setNumberDecimalPlaces(2)
        self.assertEqual(
            f.formatDouble(99.999999, context),
            "100°0′0.00″E")
        self.assertEqual(
            f.formatDouble(89.999999, context),
            "90°0′0.00″E")

        # test without direction suffix
        f.setShowDirectionalSuffix(False)
        self.assertEqual(f.formatDouble(80, context), "80°0′0.00″")

        # test 0 longitude
        f.setNumberDecimalPlaces(2)
        self.assertEqual(f.formatDouble(0, context), "0°0′0.00″")
        # test near zero longitude
        self.assertEqual(f.formatDouble(0.000001, context), "0°0′0.00″")
        # should be no "-" prefix for near-zero longitude when rounding to 2 decimal places
        self.assertEqual(
            f.formatDouble(-0.000001, context), "0°0′0.00″")
        f.setNumberDecimalPlaces(5)
        self.assertEqual(f.formatDouble(0.000001, context), "0°0′0.00360″")
        self.assertEqual(
            f.formatDouble(-0.000001, context), "-0°0′0.00360″")

        # test with padding
        f.setShowLeadingZeros(True)
        f.setShowDirectionalSuffix(True)
        f.setNumberDecimalPlaces(2)
        self.assertEqual(f.formatDouble(80, context), "80°00′00.00″E")
        self.assertEqual(f.formatDouble(85.44, context), "85°26′24.00″E")
        self.assertEqual(f.formatDouble(0, context), "0°00′00.00″")
        self.assertEqual(
            f.formatDouble(-0.000001, context), "0°00′00.00″")
        self.assertEqual(f.formatDouble(0.000001, context), "0°00′00.00″")
        f.setNumberDecimalPlaces(5)
        self.assertEqual(
            f.formatDouble(-0.000001, context), "0°00′00.00360″W")
        self.assertEqual(f.formatDouble(0.000001, context), "0°00′00.00360″E")

        # with degree padding
        f.setShowDegreeLeadingZeros(True)
        f.setNumberDecimalPlaces(2)
        self.assertEqual(f.formatDouble(100, context), "100°00′00.00″E")
        self.assertEqual(f.formatDouble(-100, context), "100°00′00.00″W")
        self.assertEqual(f.formatDouble(80, context), "080°00′00.00″E")
        self.assertEqual(f.formatDouble(-80, context), "080°00′00.00″W")
        self.assertEqual(f.formatDouble(5.44, context), "005°26′24.00″E")
        self.assertEqual(f.formatDouble(-5.44, context), "005°26′24.00″W")
        f.setShowDirectionalSuffix(False)
        self.assertEqual(f.formatDouble(100, context), "100°00′00.00″")
        self.assertEqual(f.formatDouble(-100, context), "-100°00′00.00″")
        self.assertEqual(f.formatDouble(80, context), "080°00′00.00″")
        self.assertEqual(f.formatDouble(-80, context), "-080°00′00.00″")
        self.assertEqual(f.formatDouble(5.44, context), "005°26′24.00″")
        self.assertEqual(f.formatDouble(-5.44, context), "-005°26′24.00″")

        f.setShowTrailingZeros(False)
        f.setShowDirectionalSuffix(True)
        f.setShowDegreeLeadingZeros(False)
        self.assertEqual(f.formatDouble(5.44, context), "5°26′24″E")
        self.assertEqual(f.formatDouble(-5.44, context), "5°26′24″W")
        self.assertEqual(f.formatDouble(-5.44101, context), "5°26′27.64″W")

        context.setDecimalSeparator('☕')
        self.assertEqual(f.formatDouble(5.44, context), "5°26′24″E")
        self.assertEqual(f.formatDouble(-5.44, context), "5°26′24″W")
        self.assertEqual(f.formatDouble(-5.44101, context), "5°26′27☕64″W")

    def testGeographicCoordinateFormatLatitudeDms(self):
        """Test formatting latitude as DMS"""

        f = QgsGeographicCoordinateNumericFormat()
        context = QgsNumericFormatContext()
        context.setInterpretation(QgsNumericFormatContext.Interpretation.Latitude)

        f.setAngleFormat(QgsGeographicCoordinateNumericFormat.AngleFormat.DegreesMinutesSeconds)
        f.setNumberDecimalPlaces(2)
        f.setShowDirectionalSuffix(True)
        f.setShowTrailingZeros(True)

        self.assertEqual(f.formatDouble(20, context), "20°0′0.00″N")

        # check precision
        f.setNumberDecimalPlaces(4)
        self.assertEqual(f.formatDouble(20, context), "20°0′0.0000″N")
        self.assertEqual(f.formatDouble(20.12345678, context), "20°7′24.4444″N")
        f.setNumberDecimalPlaces(0)
        self.assertEqual(f.formatDouble(20.12345678, context), "20°7′24″N")

        # check if latitudes > 90 or <-90 wrap around
        f.setNumberDecimalPlaces(2)
        self.assertEqual(f.formatDouble(190, context), "10°0′0.00″N")
        self.assertEqual(f.formatDouble(-190, context), "10°0′0.00″S")
        self.assertEqual(f.formatDouble(91, context), "89°0′0.00″S")
        self.assertEqual(f.formatDouble(-91, context), "89°0′0.00″N")
        self.assertEqual(f.formatDouble(179, context), "1°0′0.00″S")
        self.assertEqual(f.formatDouble(-179, context), "1°0′0.00″N")

        # should be no directional suffixes for 0 degree coordinates
        self.assertEqual(f.formatDouble(0, context), "0°0′0.00″")
        # should also be no directional suffix for 0 degree coordinates within specified precision
        self.assertEqual(f.formatDouble(0.000001, context), "0°0′0.00″")
        f.setNumberDecimalPlaces(5)
        self.assertEqual(f.formatDouble(0.000001, context), "0°0′0.00360″N")
        f.setNumberDecimalPlaces(2)
        self.assertEqual(f.formatDouble(-0.000001, context), "0°0′0.00″")
        f.setNumberDecimalPlaces(5)
        self.assertEqual(f.formatDouble(-0.000001, context), "0°0′0.00360″S")

        # test rounding does not create seconds >= 60
        f.setNumberDecimalPlaces(2)
        self.assertEqual(f.formatDouble(89.999999, context), "90°0′0.00″N")

        # test without direction suffix
        f.setShowDirectionalSuffix(False)
        self.assertEqual(f.formatDouble(20, context), "20°0′0.00″")

        # test 0 latitude
        self.assertEqual(f.formatDouble(0, context), "0°0′0.00″")
        # test near zero lat/long
        self.assertEqual(f.formatDouble(0.000001, context), "0°0′0.00″")
        # should be no "-" prefix for near-zero latitude when rounding to 2 decimal places
        self.assertEqual(f.formatDouble(-0.000001, context), "0°0′0.00″")
        f.setNumberDecimalPlaces(5)
        self.assertEqual(f.formatDouble(0.000001, context), "0°0′0.00360″")
        self.assertEqual(f.formatDouble(-0.000001, context), "-0°0′0.00360″")

        # test with padding
        f.setNumberDecimalPlaces(2)
        f.setShowLeadingZeros(True)
        f.setShowDirectionalSuffix(True)
        self.assertEqual(f.formatDouble(20, context), "20°00′00.00″N")
        self.assertEqual(f.formatDouble(85.44, context), "85°26′24.00″N")
        self.assertEqual(f.formatDouble(0, context), "0°00′00.00″")
        self.assertEqual(f.formatDouble(-0.000001, context), "0°00′00.00″")
        self.assertEqual(f.formatDouble(0.000001, context), "0°00′00.00″")
        f.setNumberDecimalPlaces(5)
        self.assertEqual(f.formatDouble(-0.000001, context), "0°00′00.00360″S")
        self.assertEqual(f.formatDouble(0.000001, context), "0°00′00.00360″N")

        # with degree padding
        f.setShowDegreeLeadingZeros(True)
        f.setNumberDecimalPlaces(2)
        self.assertEqual(f.formatDouble(80, context), "80°00′00.00″N")
        self.assertEqual(f.formatDouble(-80, context), "80°00′00.00″S")
        self.assertEqual(f.formatDouble(5.44, context), "05°26′24.00″N")
        self.assertEqual(f.formatDouble(-5.44, context), "05°26′24.00″S")
        f.setShowDirectionalSuffix(False)
        self.assertEqual(f.formatDouble(80, context), "80°00′00.00″")
        self.assertEqual(f.formatDouble(-80, context), "-80°00′00.00″")
        self.assertEqual(f.formatDouble(5.44, context), "05°26′24.00″")
        self.assertEqual(f.formatDouble(-5.44, context), "-05°26′24.00″")

        f.setShowTrailingZeros(False)
        f.setShowDirectionalSuffix(True)
        f.setShowDegreeLeadingZeros(False)
        self.assertEqual(f.formatDouble(5.44, context), "5°26′24″N")
        self.assertEqual(f.formatDouble(-5.44, context), "5°26′24″S")
        self.assertEqual(f.formatDouble(-5.44101, context), "5°26′27.64″S")

        context.setDecimalSeparator('☕')
        self.assertEqual(f.formatDouble(5.44, context), "5°26′24″N")
        self.assertEqual(f.formatDouble(-5.44, context), "5°26′24″S")
        self.assertEqual(f.formatDouble(-5.44101, context), "5°26′27☕64″S")

    def testGeographicCoordinateFormatLongitudeMinutes(self):
        """Test formatting longitude as DM"""

        f = QgsGeographicCoordinateNumericFormat()
        context = QgsNumericFormatContext()
        context.setInterpretation(QgsNumericFormatContext.Interpretation.Longitude)

        f.setAngleFormat(QgsGeographicCoordinateNumericFormat.AngleFormat.DegreesMinutes)
        f.setNumberDecimalPlaces(2)
        f.setShowDirectionalSuffix(True)
        f.setShowTrailingZeros(True)

        self.assertEqual(f.formatDouble(80, context), "80°0.00′E")

        # check precision
        f.setNumberDecimalPlaces(4)
        self.assertEqual(f.formatDouble(80, context), "80°0.0000′E")
        self.assertEqual(f.formatDouble(80.12345678, context), "80°7.4074′E")
        f.setNumberDecimalPlaces(0)
        self.assertEqual(f.formatDouble(80.12345678, context), "80°7′E")

        # check if longitudes > 180 or <-180 wrap around
        f.setNumberDecimalPlaces(2)
        self.assertEqual(f.formatDouble(370, context), "10°0.00′E")
        self.assertEqual(f.formatDouble(-370, context), "10°0.00′W")
        self.assertEqual(f.formatDouble(181, context), "179°0.00′W")
        self.assertEqual(f.formatDouble(-181, context), "179°0.00′E")
        self.assertEqual(f.formatDouble(359, context), "1°0.00′W")
        self.assertEqual(f.formatDouble(-359, context), "1°0.00′E")

        # should be no directional suffixes for 0 degree coordinates
        self.assertEqual(f.formatDouble(0, context), "0°0.00′")
        # should also be no directional suffix for 0 degree coordinates within specified precision
        self.assertEqual(f.formatDouble(-0.000001, context), "0°0.00′")
        self.assertEqual(f.formatDouble(0.000001, context), "0°0.00′")
        f.setNumberDecimalPlaces(5)
        self.assertEqual(f.formatDouble(-0.000001, context), "0°0.00006′W")
        self.assertEqual(f.formatDouble(0.000001, context), "0°0.00006′E")

        # test rounding does not create minutes >= 60
        f.setNumberDecimalPlaces(2)
        self.assertEqual(f.formatDouble(99.999999, context), "100°0.00′E")

        # should be no directional suffixes for 180 degree longitudes
        self.assertEqual(f.formatDouble(180, context), "180°0.00′")

        # should also be no directional suffix for 180 degree longitudes within specified precision
        self.assertEqual(f.formatDouble(180.000001, context), "180°0.00′")
        self.assertEqual(f.formatDouble(179.999999, context), "180°0.00′")
        f.setNumberDecimalPlaces(5)
        self.assertEqual(f.formatDouble(180.000001, context), "179°59.99994′W")
        self.assertEqual(f.formatDouble(179.999999, context), "179°59.99994′E")

        # test without direction suffix
        f.setNumberDecimalPlaces(2)
        f.setShowDirectionalSuffix(False)
        self.assertEqual(f.formatDouble(80, context), "80°0.00′")
        # test 0 longitude
        self.assertEqual(f.formatDouble(0, context), "0°0.00′")
        # test near zero longitude
        self.assertEqual(f.formatDouble(0.000001, context), "0°0.00′")
        # should be no "-" prefix for near-zero longitude when rounding to 2 decimal places
        self.assertEqual(f.formatDouble(-0.000001, context), "0°0.00′")
        f.setNumberDecimalPlaces(5)
        self.assertEqual(f.formatDouble(0.000001, context), "0°0.00006′")
        self.assertEqual(f.formatDouble(-0.000001, context), "-0°0.00006′")

        # test with padding
        f.setNumberDecimalPlaces(2)
        f.setShowLeadingZeros(True)
        f.setShowDirectionalSuffix(True)
        self.assertEqual(f.formatDouble(80, context), "80°00.00′E")
        self.assertEqual(f.formatDouble(0, context), "0°00.00′")
        self.assertEqual(f.formatDouble(-0.000001, context), "0°00.00′")
        self.assertEqual(f.formatDouble(0.000001, context), "0°00.00′")
        f.setNumberDecimalPlaces(5)
        self.assertEqual(f.formatDouble(-0.000001, context), "0°00.00006′W")
        self.assertEqual(f.formatDouble(0.000001, context), "0°00.00006′E")

        # with degree padding
        f.setShowDegreeLeadingZeros(True)
        f.setNumberDecimalPlaces(2)
        self.assertEqual(f.formatDouble(100, context), "100°00.00′E")
        self.assertEqual(f.formatDouble(-100, context), "100°00.00′W")
        self.assertEqual(f.formatDouble(80, context), "080°00.00′E")
        self.assertEqual(f.formatDouble(-80, context), "080°00.00′W")
        self.assertEqual(f.formatDouble(5.44, context), "005°26.40′E")
        self.assertEqual(f.formatDouble(-5.44, context), "005°26.40′W")
        f.setShowDirectionalSuffix(False)
        self.assertEqual(f.formatDouble(100, context), "100°00.00′")
        self.assertEqual(f.formatDouble(-100, context), "-100°00.00′")
        self.assertEqual(f.formatDouble(80, context), "080°00.00′")
        self.assertEqual(f.formatDouble(-80, context), "-080°00.00′")
        self.assertEqual(f.formatDouble(5.44, context), "005°26.40′")
        self.assertEqual(f.formatDouble(-5.44, context), "-005°26.40′")

        f.setShowTrailingZeros(False)
        f.setShowDirectionalSuffix(True)
        f.setShowDegreeLeadingZeros(False)
        self.assertEqual(f.formatDouble(5.44, context), "5°26.4′E")
        self.assertEqual(f.formatDouble(-5.44, context), "5°26.4′W")
        self.assertEqual(f.formatDouble(-5.44101, context), "5°26.46′W")

        context.setDecimalSeparator('☕')
        self.assertEqual(f.formatDouble(5.44, context), "5°26☕4′E")
        self.assertEqual(f.formatDouble(-5.44, context), "5°26☕4′W")
        self.assertEqual(f.formatDouble(-5.44101, context), "5°26☕46′W")

    def testGeographicCoordinateFormatLatitudeMinutes(self):
        """Test formatting latitude as DM"""

        f = QgsGeographicCoordinateNumericFormat()
        context = QgsNumericFormatContext()
        context.setInterpretation(QgsNumericFormatContext.Interpretation.Latitude)

        f.setAngleFormat(QgsGeographicCoordinateNumericFormat.AngleFormat.DegreesMinutes)
        f.setNumberDecimalPlaces(2)
        f.setShowDirectionalSuffix(True)
        f.setShowTrailingZeros(True)

        self.assertEqual(f.formatDouble(20, context), "20°0.00′N")

        # check precision
        f.setNumberDecimalPlaces(4)
        self.assertEqual(f.formatDouble(20, context), "20°0.0000′N")
        self.assertEqual(f.formatDouble(20.12345678, context), "20°7.4074′N")
        f.setNumberDecimalPlaces(0)
        self.assertEqual(f.formatDouble(20.12345678, context), "20°7′N")

        # check if latitudes > 90 or <-90 wrap around
        f.setNumberDecimalPlaces(2)
        self.assertEqual(f.formatDouble(190, context), "10°0.00′N")
        self.assertEqual(f.formatDouble(-190, context), "10°0.00′S")
        self.assertEqual(f.formatDouble(91, context), "89°0.00′S")
        self.assertEqual(f.formatDouble(-91, context), "89°0.00′N")
        self.assertEqual(f.formatDouble(179, context), "1°0.00′S")
        self.assertEqual(f.formatDouble(-179, context), "1°0.00′N")

        # should be no directional suffixes for 0 degree coordinates
        self.assertEqual(f.formatDouble(0, context), "0°0.00′")
        # should also be no directional suffix for 0 degree coordinates within specified precision
        self.assertEqual(f.formatDouble(-0.000001, context), "0°0.00′")
        self.assertEqual(f.formatDouble(0.000001, context), "0°0.00′")
        f.setNumberDecimalPlaces(5)
        self.assertEqual(f.formatDouble(-0.000001, context), "0°0.00006′S")
        self.assertEqual(f.formatDouble(0.000001, context), "0°0.00006′N")

        # test rounding does not create minutes >= 60
        f.setNumberDecimalPlaces(2)
        self.assertEqual(f.formatDouble(79.999999, context), "80°0.00′N")

        # test without direction suffix
        f.setShowDirectionalSuffix(False)
        self.assertEqual(f.formatDouble(20, context), "20°0.00′")
        # test 0 latitude
        self.assertEqual(f.formatDouble(0, context), "0°0.00′")
        # test near zero latitude
        self.assertEqual(f.formatDouble(0.000001, context), "0°0.00′")
        # should be no "-" prefix for near-zero latitude when rounding to 2 decimal places
        self.assertEqual(f.formatDouble(-0.000001, context), "0°0.00′")
        f.setNumberDecimalPlaces(5)
        self.assertEqual(f.formatDouble(0.000001, context), "0°0.00006′")
        self.assertEqual(f.formatDouble(-0.000001, context), "-0°0.00006′")

        # test with padding
        f.setNumberDecimalPlaces(2)
        f.setShowLeadingZeros(True)
        f.setShowDirectionalSuffix(True)
        self.assertEqual(f.formatDouble(20, context), "20°00.00′N")
        self.assertEqual(f.formatDouble(0, context), "0°00.00′")
        self.assertEqual(f.formatDouble(-0.000001, context), "0°00.00′")
        self.assertEqual(f.formatDouble(0.000001, context), "0°00.00′")
        f.setNumberDecimalPlaces(5)
        self.assertEqual(f.formatDouble(-0.000001, context), "0°00.00006′S")
        self.assertEqual(f.formatDouble(0.000001, context), "0°00.00006′N")

        # with degree padding
        f.setShowDegreeLeadingZeros(True)
        f.setNumberDecimalPlaces(2)
        self.assertEqual(f.formatDouble(80, context), "80°00.00′N")
        self.assertEqual(f.formatDouble(-80, context), "80°00.00′S")
        self.assertEqual(f.formatDouble(5.44, context), "05°26.40′N")
        self.assertEqual(f.formatDouble(-5.44, context), "05°26.40′S")
        f.setShowDirectionalSuffix(False)
        self.assertEqual(f.formatDouble(80, context), "80°00.00′")
        self.assertEqual(f.formatDouble(-80, context), "-80°00.00′")
        self.assertEqual(f.formatDouble(5.44, context), "05°26.40′")
        self.assertEqual(f.formatDouble(-5.44, context), "-05°26.40′")

        f.setShowTrailingZeros(False)
        f.setShowDirectionalSuffix(True)
        f.setShowDegreeLeadingZeros(False)
        self.assertEqual(f.formatDouble(5.44, context), "5°26.4′N")
        self.assertEqual(f.formatDouble(-5.44, context), "5°26.4′S")
        self.assertEqual(f.formatDouble(-5.44101, context), "5°26.46′S")

        context.setDecimalSeparator('☕')
        self.assertEqual(f.formatDouble(5.44, context), "5°26☕4′N")
        self.assertEqual(f.formatDouble(-5.44, context), "5°26☕4′S")
        self.assertEqual(f.formatDouble(-5.44101, context), "5°26☕46′S")

    def testGeographicCoordinateFormatLongitudeDegrees(self):
        """Test formatting longitude as decimal degrees"""

        f = QgsGeographicCoordinateNumericFormat()
        context = QgsNumericFormatContext()
        context.setInterpretation(QgsNumericFormatContext.Interpretation.Longitude)

        f.setAngleFormat(QgsGeographicCoordinateNumericFormat.AngleFormat.DecimalDegrees)
        f.setNumberDecimalPlaces(2)
        f.setShowDirectionalSuffix(True)
        f.setShowTrailingZeros(True)

        self.assertEqual(f.formatDouble(80, context), "80.00°E")

        # check precision
        f.setNumberDecimalPlaces(4)
        self.assertEqual(f.formatDouble(80, context), "80.0000°E")
        self.assertEqual(f.formatDouble(80.12345678, context), "80.1235°E")
        f.setNumberDecimalPlaces(0)
        self.assertEqual(f.formatDouble(80.12345678, context), "80°E")

        # check if longitudes > 180 or <-180 wrap around
        f.setNumberDecimalPlaces(2)
        self.assertEqual(f.formatDouble(370, context), "10.00°E")
        self.assertEqual(f.formatDouble(-370, context), "10.00°W")
        self.assertEqual(f.formatDouble(181, context), "179.00°W")
        self.assertEqual(f.formatDouble(-181, context), "179.00°E")
        self.assertEqual(f.formatDouble(359, context), "1.00°W")
        self.assertEqual(f.formatDouble(-359, context), "1.00°E")

        # should be no directional suffixes for 0 degree coordinates
        self.assertEqual(f.formatDouble(0, context), "0.00°")
        # should also be no directional suffix for 0 degree coordinates within specified precision
        self.assertEqual(f.formatDouble(-0.00001, context), "0.00°")
        self.assertEqual(f.formatDouble(0.00001, context), "0.00°")
        f.setNumberDecimalPlaces(5)
        self.assertEqual(f.formatDouble(-0.00001, context), "0.00001°W")
        self.assertEqual(f.formatDouble(0.00001, context), "0.00001°E")

        # should be no directional suffixes for 180 degree longitudes
        f.setNumberDecimalPlaces(2)
        self.assertEqual(f.formatDouble(180, context), "180.00°")

        # should also be no directional suffix for 180 degree longitudes within specified precision
        self.assertEqual(f.formatDouble(180.000001, context), "180.00°")
        self.assertEqual(f.formatDouble(179.999999, context), "180.00°")
        f.setNumberDecimalPlaces(6)
        self.assertEqual(f.formatDouble(180.000001, context), "179.999999°W")
        self.assertEqual(f.formatDouble(179.999999, context), "179.999999°E")

        # test without direction suffix
        f.setShowDirectionalSuffix(False)
        f.setNumberDecimalPlaces(2)
        self.assertEqual(f.formatDouble(80, context), "80.00°")
        # test 0 longitude
        self.assertEqual(f.formatDouble(0, context), "0.00°")
        # test near zero longitude
        self.assertEqual(f.formatDouble(0.000001, context), "0.00°")
        # should be no "-" prefix for near-zero longitude when rounding to 2 decimal places
        self.assertEqual(f.formatDouble(-0.000001, context), "0.00°")
        f.setNumberDecimalPlaces(6)
        self.assertEqual(f.formatDouble(0.000001, context), "0.000001°")
        self.assertEqual(f.formatDouble(-0.000001, context), "-0.000001°")

        # with degree padding
        f.setShowDegreeLeadingZeros(True)
        f.setNumberDecimalPlaces(2)
        f.setShowDirectionalSuffix(True)
        self.assertEqual(f.formatDouble(100, context), "100.00°E")
        self.assertEqual(f.formatDouble(-100, context), "100.00°W")
        self.assertEqual(f.formatDouble(80, context), "080.00°E")
        self.assertEqual(f.formatDouble(-80, context), "080.00°W")
        self.assertEqual(f.formatDouble(5.44, context), "005.44°E")
        self.assertEqual(f.formatDouble(-5.44, context), "005.44°W")
        f.setShowDirectionalSuffix(False)
        self.assertEqual(f.formatDouble(100, context), "100.00°")
        self.assertEqual(f.formatDouble(-100, context), "-100.00°")
        self.assertEqual(f.formatDouble(80, context), "080.00°")
        self.assertEqual(f.formatDouble(-80, context), "-080.00°")
        self.assertEqual(f.formatDouble(5.44, context), "005.44°")
        self.assertEqual(f.formatDouble(-5.44, context), "-005.44°")

        f.setShowTrailingZeros(False)
        f.setShowDirectionalSuffix(True)
        f.setShowDegreeLeadingZeros(False)
        self.assertEqual(f.formatDouble(5.44, context), "5.44°E")
        self.assertEqual(f.formatDouble(-5.44, context), "5.44°W")
        self.assertEqual(f.formatDouble(-5.44101, context), "5.44°W")

        context.setDecimalSeparator('☕')
        self.assertEqual(f.formatDouble(5.44, context), "5☕44°E")
        self.assertEqual(f.formatDouble(-5.44, context), "5☕44°W")
        self.assertEqual(f.formatDouble(-5.44101, context), "5☕44°W")

    def testGeographicCoordinateFormatLatitudeDegrees(self):
        """Test formatting latitude as decimal degrees"""

        f = QgsGeographicCoordinateNumericFormat()
        context = QgsNumericFormatContext()
        context.setInterpretation(QgsNumericFormatContext.Interpretation.Latitude)

        f.setAngleFormat(QgsGeographicCoordinateNumericFormat.AngleFormat.DecimalDegrees)
        f.setNumberDecimalPlaces(2)
        f.setShowDirectionalSuffix(True)
        f.setShowTrailingZeros(True)

        self.assertEqual(f.formatDouble(20, context), "20.00°N")

        # check precision
        f.setNumberDecimalPlaces(4)
        self.assertEqual(f.formatDouble(20, context), "20.0000°N")
        self.assertEqual(f.formatDouble(20.12345678, context), "20.1235°N")
        f.setNumberDecimalPlaces(0)
        self.assertEqual(f.formatDouble(20.12345678, context), "20°N")

        # check if latitudes > 90 or <-90 wrap around
        f.setNumberDecimalPlaces(2)
        self.assertEqual(f.formatDouble(190, context), "10.00°N")
        self.assertEqual(f.formatDouble(-190, context), "10.00°S")
        self.assertEqual(f.formatDouble(91, context), "89.00°S")
        self.assertEqual(f.formatDouble(-91, context), "89.00°N")
        self.assertEqual(f.formatDouble(179, context), "1.00°S")
        self.assertEqual(f.formatDouble(-179, context), "1.00°N")

        # should be no directional suffixes for 0 degree coordinates
        self.assertEqual(f.formatDouble(0, context), "0.00°")
        # should also be no directional suffix for 0 degree coordinates within specified precision
        self.assertEqual(f.formatDouble(-0.00001, context), "0.00°")
        self.assertEqual(f.formatDouble(0.00001, context), "0.00°")
        f.setNumberDecimalPlaces(5)
        self.assertEqual(f.formatDouble(-0.00001, context), "0.00001°S")
        self.assertEqual(f.formatDouble(0.00001, context), "0.00001°N")

        # test without direction suffix
        f.setShowDirectionalSuffix(False)
        f.setNumberDecimalPlaces(2)
        self.assertEqual(f.formatDouble(80, context), "80.00°")
        # test 0 longitude
        self.assertEqual(f.formatDouble(0, context), "0.00°")
        # test near zero latitude
        self.assertEqual(f.formatDouble(0.000001, context), "0.00°")
        # should be no "-" prefix for near-zero latitude when rounding to 2 decimal places
        self.assertEqual(f.formatDouble(-0.000001, context), "0.00°")
        f.setNumberDecimalPlaces(6)
        self.assertEqual(f.formatDouble(0.000001, context), "0.000001°")
        self.assertEqual(f.formatDouble(-0.000001, context), "-0.000001°")

        # with degree padding
        f.setShowDegreeLeadingZeros(True)
        f.setNumberDecimalPlaces(2)
        f.setShowDirectionalSuffix(True)
        self.assertEqual(f.formatDouble(80, context), "80.00°N")
        self.assertEqual(f.formatDouble(-80, context), "80.00°S")
        self.assertEqual(f.formatDouble(5.44, context), "05.44°N")
        self.assertEqual(f.formatDouble(-5.44, context), "05.44°S")
        f.setShowDirectionalSuffix(False)
        self.assertEqual(f.formatDouble(80, context), "80.00°")
        self.assertEqual(f.formatDouble(-80, context), "-80.00°")
        self.assertEqual(f.formatDouble(5.44, context), "05.44°")
        self.assertEqual(f.formatDouble(-5.44, context), "-05.44°")

        f.setShowTrailingZeros(False)
        f.setShowDirectionalSuffix(True)
        f.setShowDegreeLeadingZeros(False)
        self.assertEqual(f.formatDouble(5.44, context), "5.44°N")
        self.assertEqual(f.formatDouble(-5.44, context), "5.44°S")
        self.assertEqual(f.formatDouble(-5.44101, context), "5.44°S")

        context.setDecimalSeparator('☕')
        self.assertEqual(f.formatDouble(5.44, context), "5☕44°N")
        self.assertEqual(f.formatDouble(-5.44, context), "5☕44°S")
        self.assertEqual(f.formatDouble(-5.44101, context), "5☕44°S")

    def testRegistry(self):
        registry = QgsNumericFormatRegistry()
        self.assertTrue(registry.formats())
        for f in registry.formats():
            self.assertEqual(registry.format(f).id(), f)

        self.assertIn('default', registry.formats())
        registry.addFormat(TestFormat())
        self.assertIn('test', registry.formats())
        self.assertTrue(isinstance(registry.format('test'), TestFormat))
        self.assertTrue(isinstance(registry.create('test', {}, QgsReadWriteContext()), TestFormat))

        registry.removeFormat('test')

        self.assertNotIn('test', registry.formats())
        self.assertTrue(isinstance(registry.format('test'), QgsFallbackNumericFormat))
        self.assertTrue(isinstance(registry.create('test', {}, QgsReadWriteContext()), QgsFallbackNumericFormat))

        self.assertTrue(isinstance(registry.fallbackFormat(), QgsFallbackNumericFormat))

        self.assertEqual(registry.visibleName('default'), 'General')
        self.assertEqual(registry.visibleName('basic'), 'Number')

        self.assertEqual(registry.sortKey('default'), 0)
        self.assertEqual(registry.sortKey('basic'), 1)
        self.assertEqual(registry.sortKey('currency'), 100)


if __name__ == '__main__':
    unittest.main()
