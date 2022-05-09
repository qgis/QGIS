# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsProjectStyleSettings.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Mathieu Pellerin'
__date__ = '09/05/2022'
__copyright__ = 'Copyright 2019, The QGIS Project'

import qgis  # NOQA
import os

from qgis.core import (QgsProject,
                       QgsProjectStyleSettings,
                       QgsReadWriteContext,
                       QgsSymbol,
                       QgsWkbTypes,
                       QgsColorRamp,
                       QgsGradientColorRamp,
                       QgsTextFormat,
                       Qgis)

from qgis.PyQt.QtCore import QTemporaryDir
from qgis.PyQt.QtGui import QFont, QColor

from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument, QDomElement
from qgis.testing import start_app, unittest
from utilities import (unitTestDataPath)

app = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsProjectViewSettings(unittest.TestCase):

    def testDefaultSymbol(self):
        p = QgsProjectStyleSettings()
        self.assertFalse(p.defaultSymbol(Qgis.SymbolType.Marker))
        self.assertFalse(p.defaultSymbol(Qgis.SymbolType.Line))
        self.assertFalse(p.defaultSymbol(Qgis.SymbolType.Fill))

        marker = QgsSymbol.defaultSymbol(QgsWkbTypes.PointGeometry)
        p.setDefaultSymbol(Qgis.SymbolType.Marker, marker)
        self.assertTrue(p.defaultSymbol(Qgis.SymbolType.Marker))

        line = QgsSymbol.defaultSymbol(QgsWkbTypes.LineGeometry)
        p.setDefaultSymbol(Qgis.SymbolType.Line, line)
        self.assertTrue(p.defaultSymbol(Qgis.SymbolType.Line))

        fill = QgsSymbol.defaultSymbol(QgsWkbTypes.PolygonGeometry)
        p.setDefaultSymbol(Qgis.SymbolType.Fill, fill)
        self.assertTrue(p.defaultSymbol(Qgis.SymbolType.Fill))

    def testDefaultColorRamp(self):
        p = QgsProjectStyleSettings()
        self.assertFalse(p.defaultColorRamp())

        ramp = QgsGradientColorRamp(QColor(255, 255, 255), QColor(255, 0, 0))
        p.setDefaultColorRamp(ramp)
        self.assertTrue(p.defaultColorRamp())

    def testDefaultTextFormat(self):
        p = QgsProjectStyleSettings()
        self.assertFalse(p.defaultTextFormat().isValid())

        textFormat = QgsTextFormat()
        textFormat.setFont(QFont())
        p.setDefaultTextFormat(textFormat)
        self.assertTrue(p.defaultTextFormat().isValid())

    def testRandomizeDefaultSymbolColor(self):
        p = QgsProjectStyleSettings()
        self.assertTrue(p.randomizeDefaultSymbolColor())
        p.setRandomizeDefaultSymbolColor(False)
        self.assertFalse(p.randomizeDefaultSymbolColor())

    def testDefaultSymbolOpacity(self):
        p = QgsProjectStyleSettings()
        self.assertEqual(p.defaultSymbolOpacity(), 1.0)
        p.setDefaultSymbolOpacity(0.25)
        self.assertEqual(p.defaultSymbolOpacity(), 0.25)

    def testReadWrite(self):
        p = QgsProjectStyleSettings()

        line = QgsSymbol.defaultSymbol(QgsWkbTypes.LineGeometry)
        p.setDefaultSymbol(Qgis.SymbolType.Line, line)

        ramp = QgsGradientColorRamp(QColor(255, 255, 255), QColor(255, 0, 0))
        p.setDefaultColorRamp(ramp)

        textFormat = QgsTextFormat()
        textFormat.setFont(QFont())
        p.setDefaultTextFormat(textFormat)

        p.setRandomizeDefaultSymbolColor(False)
        p.setDefaultSymbolOpacity(0.25)

        doc = QDomDocument("testdoc")
        elem = p.writeXml(doc, QgsReadWriteContext())

        p2 = QgsProjectStyleSettings()
        self.assertTrue(p2.readXml(elem, QgsReadWriteContext()))

        self.assertFalse(p2.defaultSymbol(Qgis.SymbolType.Marker))
        self.assertTrue(p2.defaultSymbol(Qgis.SymbolType.Line))
        self.assertFalse(p2.defaultSymbol(Qgis.SymbolType.Fill))
        self.assertTrue(p2.defaultColorRamp())
        self.assertTrue(p2.defaultTextFormat().isValid())
        self.assertFalse(p2.randomizeDefaultSymbolColor())
        self.assertEqual(p2.defaultSymbolOpacity(), 0.25)


if __name__ == '__main__':
    unittest.main()
