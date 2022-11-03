# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsProjectGpsSettings.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '03/11/2022'
__copyright__ = 'Copyright 2022, The QGIS Project'

import qgis  # NOQA

from qgis.core import (QgsProjectGpsSettings,
                       QgsReadWriteContext,
                       QgsBearingNumericFormat,
                       QgsGeographicCoordinateNumericFormat,
                       QgsSettings,
                       QgsLocalDefaultSettings,
                       QgsUnitTypes,
                       QgsCoordinateReferenceSystem,
                       Qgis)

from qgis.PyQt.QtCore import QCoreApplication

from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument
from qgis.testing import start_app, unittest
from utilities import (unitTestDataPath)

app = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsProjectGpsSettings(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("TestPyQgsProjectGpsSettings.com")
        QCoreApplication.setApplicationName("TestPyQgsProjectGpsSettings")
        QgsSettings().clear()
        start_app()

    def testSettings(self):
        p = QgsProjectGpsSettings()
        self.assertFalse(p.automaticallyCommitFeatures())
        self.assertFalse(p.automaticallyAddTrackPoints())

        spy_add_track = QSignalSpy(p.automaticallyAddTrackPointsChanged)
        spy_auto_commit = QSignalSpy(p.automaticallyCommitFeaturesChanged)

        p.setAutomaticallyAddTrackPoints(True)
        self.assertEqual(len(spy_add_track), 1)
        self.assertTrue(spy_add_track[-1][0])

        p.setAutomaticallyAddTrackPoints(True)
        self.assertEqual(len(spy_add_track), 1)

        self.assertTrue(p.automaticallyAddTrackPoints())
        p.setAutomaticallyAddTrackPoints(False)
        self.assertEqual(len(spy_add_track), 2)
        self.assertFalse(spy_add_track[-1][0])

        p.setAutomaticallyCommitFeatures(True)
        self.assertEqual(len(spy_auto_commit), 1)
        self.assertTrue(spy_auto_commit[-1][0])

        p.setAutomaticallyCommitFeatures(True)
        self.assertEqual(len(spy_auto_commit), 1)

        self.assertTrue(p.automaticallyCommitFeatures())
        p.setAutomaticallyCommitFeatures(False)
        self.assertEqual(len(spy_auto_commit), 2)
        self.assertFalse(spy_auto_commit[-1][0])

    def testReset(self):
        """
        Test that resetting inherits local default settings
        """
        p = QgsProjectGpsSettings()
        self.assertFalse(p.automaticallyCommitFeatures())
        self.assertFalse(p.automaticallyAddTrackPoints())

        p.setAutomaticallyCommitFeatures(True)
        p.setAutomaticallyAddTrackPoints(True)
        spy_add_track = QSignalSpy(p.automaticallyAddTrackPointsChanged)
        spy_auto_commit = QSignalSpy(p.automaticallyCommitFeaturesChanged)

        p.reset()
        self.assertFalse(p.automaticallyAddTrackPoints())
        self.assertFalse(p.automaticallyCommitFeatures())

        self.assertEqual(len(spy_add_track), 1)
        self.assertFalse(spy_auto_commit[-1][0])
        self.assertEqual(len(spy_auto_commit), 1)
        self.assertFalse(spy_auto_commit[-1][0])

    def testReadWrite(self):
        p = QgsProjectGpsSettings()

        p.setAutomaticallyCommitFeatures(True)
        p.setAutomaticallyAddTrackPoints(True)

        doc = QDomDocument("testdoc")
        elem = p.writeXml(doc, QgsReadWriteContext())

        p2 = QgsProjectGpsSettings()
        spy = QSignalSpy(p2.automaticallyAddTrackPointsChanged)
        spy2 = QSignalSpy(p2.automaticallyCommitFeaturesChanged)
        self.assertTrue(p2.readXml(elem, QgsReadWriteContext()))
        self.assertEqual(len(spy), 1)
        self.assertEqual(len(spy2), 1)
        self.assertTrue(p.automaticallyCommitFeatures())
        self.assertTrue(p.automaticallyAddTrackPoints())


if __name__ == '__main__':
    unittest.main()
