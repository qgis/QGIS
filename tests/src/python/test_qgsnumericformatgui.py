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
                       QgsReadWriteContext)

from qgis.gui import QgsNumericFormatSelectorWidget

from qgis.testing import start_app, unittest
from qgis.PyQt.QtTest import QSignalSpy

start_app()


class TestQgsNumericFormatGui(unittest.TestCase):

    def testSelectorWidget(self):
        w = QgsNumericFormatSelectorWidget()
        spy = QSignalSpy(w.changed)
        # should default to a default format
        self.assertIsInstance(w.format(), QgsFallbackNumericFormat)

        w.setFormat(QgsBearingNumericFormat())
        self.assertIsInstance(w.format(), QgsBearingNumericFormat)
        self.assertEqual(len(spy), 1)
        w.setFormat(QgsBearingNumericFormat())
        self.assertEqual(len(spy), 1)
        w.setFormat(QgsCurrencyNumericFormat())
        self.assertIsInstance(w.format(), QgsCurrencyNumericFormat)
        self.assertEqual(len(spy), 2)


if __name__ == '__main__':
    unittest.main()
