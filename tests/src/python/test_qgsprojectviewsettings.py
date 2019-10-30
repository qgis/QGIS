# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsProjectViewSettings.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '30/10/2019'
__copyright__ = 'Copyright 2019, The QGIS Project'

import qgis  # NOQA

from qgis.core import (QgsProjectViewSettings,
                       QgsReadWriteContext,
                       QgsReferencedRectangle,
                       QgsRectangle,
                       QgsCoordinateReferenceSystem)

from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument, QDomElement
from qgis.testing import start_app, unittest
from utilities import (unitTestDataPath)

app = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsProjectViewSettings(unittest.TestCase):

    def testMapScales(self):
        p = QgsProjectViewSettings()
        self.assertFalse(p.mapScales())
        self.assertFalse(p.useProjectScales())

        spy = QSignalSpy(p.mapScalesChanged)
        p.setMapScales([])
        self.assertEqual(len(spy), 0)
        p.setUseProjectScales(False)
        self.assertEqual(len(spy), 0)

        p.setMapScales([5000, 6000, 3000, 4000])
        # scales must be sorted
        self.assertEqual(p.mapScales(), [6000.0, 5000.0, 4000.0, 3000.0])
        self.assertEqual(len(spy), 1)
        p.setMapScales([5000, 6000, 3000, 4000])
        self.assertEqual(len(spy), 1)
        self.assertEqual(p.mapScales(), [6000.0, 5000.0, 4000.0, 3000.0])
        p.setMapScales([5000, 6000, 3000, 4000, 1000])
        self.assertEqual(len(spy), 2)
        self.assertEqual(p.mapScales(), [6000.0, 5000.0, 4000.0, 3000.0, 1000.0])

        p.setUseProjectScales(True)
        self.assertEqual(len(spy), 3)
        p.setUseProjectScales(True)
        self.assertEqual(len(spy), 3)
        p.setUseProjectScales(False)
        self.assertEqual(len(spy), 4)

        p.setUseProjectScales(True)
        p.setMapScales([5000, 6000, 3000, 4000])
        self.assertEqual(len(spy), 6)

        p.reset()
        self.assertEqual(len(spy), 7)
        self.assertFalse(p.mapScales())
        self.assertFalse(p.useProjectScales())

    def testDefaultViewExtent(self):
        p = QgsProjectViewSettings()
        self.assertTrue(p.defaultViewExtent().isNull())

        p.setDefaultViewExtent(QgsReferencedRectangle(QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem("EPSG:3857")))
        self.assertEqual(p.defaultViewExtent(), QgsReferencedRectangle(QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem("EPSG:3857")))

        p.setDefaultViewExtent(QgsReferencedRectangle())
        self.assertTrue(p.defaultViewExtent().isNull())

        p.setDefaultViewExtent(QgsReferencedRectangle(QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem("EPSG:3857")))
        p.reset()
        self.assertTrue(p.defaultViewExtent().isNull())

    def testReadWrite(self):
        p = QgsProjectViewSettings()
        self.assertFalse(p.mapScales())
        self.assertFalse(p.useProjectScales())
        doc = QDomDocument("testdoc")
        elem = p.writeXml(doc, QgsReadWriteContext())

        p2 = QgsProjectViewSettings()
        spy = QSignalSpy(p2.mapScalesChanged)
        self.assertTrue(p2.readXml(elem, QgsReadWriteContext()))
        self.assertFalse(p2.mapScales())
        self.assertFalse(p2.useProjectScales())
        self.assertEqual(len(spy), 0)
        self.assertTrue(p2.defaultViewExtent().isNull())

        p.setUseProjectScales(True)
        p.setMapScales([56, 78, 99])
        p.setDefaultViewExtent(QgsReferencedRectangle(QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem("EPSG:3857")))
        elem = p.writeXml(doc, QgsReadWriteContext())

        p2 = QgsProjectViewSettings()
        spy = QSignalSpy(p2.mapScalesChanged)
        self.assertTrue(p2.readXml(elem, QgsReadWriteContext()))
        self.assertEqual(p2.mapScales(), [99.0, 78.0, 56.0])
        self.assertTrue(p2.useProjectScales())
        self.assertEqual(len(spy), 1)
        self.assertEqual(p2.defaultViewExtent(), QgsReferencedRectangle(QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem("EPSG:3857")))


if __name__ == '__main__':
    unittest.main()
