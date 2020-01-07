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


if __name__ == '__main__':
    unittest.main()
