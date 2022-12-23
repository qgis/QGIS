# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsNumericFormat GUI components

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
                       QgsGeographicCoordinateNumericFormat,
                       QgsNumericFormatRegistry,
                       QgsNumericFormat,
                       QgsApplication)

from qgis.gui import (QgsNumericFormatSelectorWidget,
                      QgsNumericFormatGuiRegistry,
                      QgsNumericFormatConfigurationWidgetFactory,
                      QgsNumericFormatWidget,
                      QgsGui)

from qgis.testing import start_app, unittest
from qgis.PyQt.QtTest import QSignalSpy

start_app()


class TestFormat(QgsNumericFormat):

    def __init__(self):
        super().__init__()
        self.xx = 1

    def id(self):
        return 'test'

    def formatDouble(self, value, context):
        return 'xxx' + str(value)

    def visibleName(self):
        return 'Test'

    def clone(self):
        res = TestFormat()
        res.xx = self.xx
        return res

    def create(self, configuration, context):
        res = TestFormat()
        res.xx = configuration['xx']
        return res

    def configuration(self, context):
        return {'xx': self.xx}


class TestFormatWidget(QgsNumericFormatWidget):

    def __init__(self, parent=None):
        super().__init__(parent)
        self.f = None

    def setFormat(self, format):
        self.f = format.clone()

    def format(self):
        return self.f.clone()


class TestWidgetFactory(QgsNumericFormatConfigurationWidgetFactory):

    def create(self, format):
        w = TestFormatWidget()
        w.setFormat(format)
        return w


class TestQgsNumericFormatGui(unittest.TestCase):

    def testRegistry(self):
        """
        Test the GUI registry for numeric formats
        """
        reg = QgsNumericFormatGuiRegistry()
        self.assertFalse(reg.formatConfigurationWidget(None))
        self.assertFalse(reg.formatConfigurationWidget(TestFormat()))

        reg.addFormatConfigurationWidgetFactory('test', TestWidgetFactory())
        original = TestFormat()
        original.xx = 55
        w = reg.formatConfigurationWidget(original)
        self.assertTrue(w)

        self.assertIsInstance(w.format(), TestFormat)
        self.assertEqual(w.format().xx, 55)

        reg.removeFormatConfigurationWidgetFactory('test')
        self.assertFalse(reg.formatConfigurationWidget(TestFormat()))
        reg.removeFormatConfigurationWidgetFactory('test')

    def testSelectorWidget(self):
        w = QgsNumericFormatSelectorWidget()
        spy = QSignalSpy(w.changed)
        # should default to a default format
        self.assertIsInstance(w.format(), QgsFallbackNumericFormat)

        w.setFormat(QgsBearingNumericFormat())
        self.assertIsInstance(w.format(), QgsBearingNumericFormat)
        self.assertEqual(len(spy), 1)
        w.setFormat(QgsBearingNumericFormat())
        self.assertEqual(len(spy), 2)
        w.setFormat(QgsCurrencyNumericFormat())
        self.assertIsInstance(w.format(), QgsCurrencyNumericFormat)
        self.assertEqual(len(spy), 3)

        QgsApplication.numericFormatRegistry().addFormat(TestFormat())
        QgsGui.numericFormatGuiRegistry().addFormatConfigurationWidgetFactory('test', TestWidgetFactory())
        original = TestFormat()
        original.xx = 55

        w = QgsNumericFormatSelectorWidget()
        w.setFormat(original)
        new = w.format()
        self.assertEqual(new.xx, 55)

    def testBasicFormat(self):
        w = QgsNumericFormatSelectorWidget()

        original = QgsBasicNumericFormat()
        original.setShowPlusSign(True)
        original.setShowThousandsSeparator(False)
        original.setNumberDecimalPlaces(4)
        original.setShowTrailingZeros(True)

        w.setFormat(original)
        new = w.format()

        self.assertIsInstance(new, QgsBasicNumericFormat)
        self.assertEqual(new.showPlusSign(), original.showPlusSign())
        self.assertEqual(new.showThousandsSeparator(), original.showThousandsSeparator())
        self.assertEqual(new.numberDecimalPlaces(), original.numberDecimalPlaces())
        self.assertEqual(new.showTrailingZeros(), original.showTrailingZeros())

    def testCurrencyFormat(self):
        w = QgsNumericFormatSelectorWidget()

        original = QgsCurrencyNumericFormat()
        original.setShowPlusSign(True)
        original.setShowThousandsSeparator(False)
        original.setNumberDecimalPlaces(4)
        original.setShowTrailingZeros(True)
        original.setPrefix('$$')
        original.setSuffix('AUD')

        w.setFormat(original)
        new = w.format()

        self.assertIsInstance(new, QgsCurrencyNumericFormat)
        self.assertEqual(new.showPlusSign(), original.showPlusSign())
        self.assertEqual(new.showThousandsSeparator(), original.showThousandsSeparator())
        self.assertEqual(new.numberDecimalPlaces(), original.numberDecimalPlaces())
        self.assertEqual(new.showTrailingZeros(), original.showTrailingZeros())
        self.assertEqual(new.prefix(), original.prefix())
        self.assertEqual(new.suffix(), original.suffix())

    def testBearingFormat(self):
        w = QgsNumericFormatSelectorWidget()

        original = QgsBearingNumericFormat()
        original.setNumberDecimalPlaces(4)
        original.setShowTrailingZeros(True)
        original.setDirectionFormat(QgsBearingNumericFormat.UseRange0To360)

        w.setFormat(original)
        new = w.format()

        self.assertIsInstance(new, QgsBearingNumericFormat)
        self.assertEqual(new.numberDecimalPlaces(), original.numberDecimalPlaces())
        self.assertEqual(new.showTrailingZeros(), original.showTrailingZeros())
        self.assertEqual(new.directionFormat(), original.directionFormat())

    def testGeographicCoordinateFormat(self):
        w = QgsNumericFormatSelectorWidget()

        original = QgsGeographicCoordinateNumericFormat()
        original.setNumberDecimalPlaces(4)
        original.setShowTrailingZeros(True)
        original.setAngleFormat(QgsGeographicCoordinateNumericFormat.AngleFormat.DegreesMinutes)
        original.setShowDirectionalSuffix(True)
        original.setShowLeadingZeros(True)
        original.setShowDegreeLeadingZeros(True)

        w.setFormat(original)
        new = w.format()

        self.assertIsInstance(new, QgsGeographicCoordinateNumericFormat)
        self.assertEqual(new.numberDecimalPlaces(), original.numberDecimalPlaces())
        self.assertEqual(new.showTrailingZeros(), original.showTrailingZeros())
        self.assertEqual(new.showDirectionalSuffix(), original.showDirectionalSuffix())
        self.assertEqual(new.angleFormat(), original.angleFormat())
        self.assertEqual(new.showLeadingZeros(), original.showLeadingZeros())
        self.assertEqual(new.showDegreeLeadingZeros(), original.showDegreeLeadingZeros())

    def testPercentageFormat(self):
        w = QgsNumericFormatSelectorWidget()

        original = QgsPercentageNumericFormat()
        original.setNumberDecimalPlaces(4)
        original.setShowTrailingZeros(True)
        original.setInputValues(QgsPercentageNumericFormat.ValuesAreFractions)

        w.setFormat(original)
        new = w.format()

        self.assertIsInstance(new, QgsPercentageNumericFormat)
        self.assertEqual(new.numberDecimalPlaces(), original.numberDecimalPlaces())
        self.assertEqual(new.showTrailingZeros(), original.showTrailingZeros())
        self.assertEqual(new.inputValues(), original.inputValues())

    def testScientificFormat(self):
        w = QgsNumericFormatSelectorWidget()

        original = QgsScientificNumericFormat()
        original.setShowPlusSign(True)
        original.setNumberDecimalPlaces(4)
        original.setShowTrailingZeros(True)

        w.setFormat(original)
        new = w.format()

        self.assertIsInstance(new, QgsScientificNumericFormat)
        self.assertEqual(new.showPlusSign(), original.showPlusSign())
        self.assertEqual(new.numberDecimalPlaces(), original.numberDecimalPlaces())
        self.assertEqual(new.showTrailingZeros(), original.showTrailingZeros())

    def testDefaultFormat(self):
        w = QgsNumericFormatSelectorWidget()

        original = QgsFallbackNumericFormat()

        w.setFormat(original)
        new = w.format()

        self.assertIsInstance(new, QgsFallbackNumericFormat)


if __name__ == '__main__':
    unittest.main()
