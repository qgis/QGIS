# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsProjectDisplaySettings.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '09/01/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

from qgis.core import (QgsProject,
                       QgsProjectDisplaySettings,
                       QgsReadWriteContext,
                       QgsBearingNumericFormat)

from qgis.PyQt.QtCore import QTemporaryDir

from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument, QDomElement
from qgis.testing import start_app, unittest
from utilities import (unitTestDataPath)

app = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsProjectDisplaySettings(unittest.TestCase):

    def testBearingFormat(self):
        p = QgsProjectDisplaySettings()

        format = QgsBearingNumericFormat()
        format.setNumberDecimalPlaces(9)
        format.setDirectionFormat(QgsBearingNumericFormat.UseRange0To360)

        spy = QSignalSpy(p.bearingFormatChanged)
        p.setBearingFormat(format)
        self.assertEqual(len(spy), 1)
        self.assertEqual(p.bearingFormat().numberDecimalPlaces(), 9)

        format = QgsBearingNumericFormat()
        format.setNumberDecimalPlaces(3)
        format.setDirectionFormat(QgsBearingNumericFormat.UseRange0To360)
        p.setBearingFormat(format)
        self.assertEqual(len(spy), 2)
        self.assertEqual(p.bearingFormat().numberDecimalPlaces(), 3)

        p.reset()
        self.assertEqual(len(spy), 3)
        self.assertEqual(p.bearingFormat().numberDecimalPlaces(), 6)

    def testReadWrite(self):
        p = QgsProjectDisplaySettings()

        format = QgsBearingNumericFormat()
        format.setNumberDecimalPlaces(9)
        format.setDirectionFormat(QgsBearingNumericFormat.UseRange0To360)
        p.setBearingFormat(format)

        doc = QDomDocument("testdoc")
        elem = p.writeXml(doc, QgsReadWriteContext())

        p2 = QgsProjectDisplaySettings()
        spy = QSignalSpy(p2.bearingFormatChanged)
        self.assertTrue(p2.readXml(elem, QgsReadWriteContext()))
        self.assertEqual(len(spy), 1)
        self.assertEqual(p2.bearingFormat().numberDecimalPlaces(), 9)
        self.assertEqual(p2.bearingFormat().directionFormat(), QgsBearingNumericFormat.UseRange0To360)


if __name__ == '__main__':
    unittest.main()
