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
import os

from qgis.core import (QgsProjectGpsSettings,
                       QgsReadWriteContext,
                       QgsBearingNumericFormat,
                       QgsGeographicCoordinateNumericFormat,
                       QgsSettings,
                       QgsLocalDefaultSettings,
                       QgsUnitTypes,
                       QgsCoordinateReferenceSystem,
                       Qgis,
                       QgsVectorLayer,
                       QgsProject)

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
        self.assertFalse(p.automaticallyAddTrackVertices())
        self.assertTrue(p.destinationFollowsActiveLayer())

        spy_add_track = QSignalSpy(p.automaticallyAddTrackVerticesChanged)
        spy_auto_commit = QSignalSpy(p.automaticallyCommitFeaturesChanged)
        spy_destination_follows_active = QSignalSpy(p.destinationFollowsActiveLayerChanged)

        p.setAutomaticallyAddTrackVertices(True)
        self.assertEqual(len(spy_add_track), 1)
        self.assertTrue(spy_add_track[-1][0])

        p.setAutomaticallyAddTrackVertices(True)
        self.assertEqual(len(spy_add_track), 1)

        self.assertTrue(p.automaticallyAddTrackVertices())
        p.setAutomaticallyAddTrackVertices(False)
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

        p.setDestinationFollowsActiveLayer(False)
        self.assertEqual(len(spy_destination_follows_active), 1)
        self.assertFalse(spy_destination_follows_active[-1][0])

        p.setDestinationFollowsActiveLayer(False)
        self.assertEqual(len(spy_destination_follows_active), 1)

        self.assertFalse(p.destinationFollowsActiveLayer())
        p.setDestinationFollowsActiveLayer(True)
        self.assertEqual(len(spy_destination_follows_active), 2)
        self.assertTrue(spy_destination_follows_active[-1][0])

        layer1 = QgsVectorLayer(os.path.join(unitTestDataPath(), 'lines.shp'), 'layer1')
        self.assertTrue(layer1.isValid())
        layer2 = QgsVectorLayer(os.path.join(unitTestDataPath(), 'points.shp'), 'layer2')
        self.assertTrue(layer2.isValid())

        self.assertFalse(p.destinationLayer())
        spy = QSignalSpy(p.destinationLayerChanged)
        p.setDestinationLayer(layer1)
        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[0][0], layer1)
        self.assertEqual(p.destinationLayer(), layer1)

        p.setDestinationLayer(layer1)
        self.assertEqual(len(spy), 1)

        p.setDestinationLayer(layer2)
        self.assertEqual(len(spy), 2)
        self.assertEqual(spy[1][0], layer2)
        self.assertEqual(p.destinationLayer(), layer2)

    def testReset(self):
        """
        Test that resetting inherits local default settings
        """
        p = QgsProjectGpsSettings()
        self.assertFalse(p.automaticallyCommitFeatures())
        self.assertFalse(p.automaticallyAddTrackVertices())
        self.assertTrue(p.destinationFollowsActiveLayer())

        p.setAutomaticallyCommitFeatures(True)
        p.setAutomaticallyAddTrackVertices(True)
        p.setDestinationFollowsActiveLayer(False)

        layer1 = QgsVectorLayer(os.path.join(unitTestDataPath(), 'lines.shp'), 'layer1')
        self.assertTrue(layer1.isValid())
        p.setDestinationLayer(layer1)

        spy_add_track = QSignalSpy(p.automaticallyAddTrackVerticesChanged)
        spy_auto_commit = QSignalSpy(p.automaticallyCommitFeaturesChanged)
        spy_dest_layer_changed = QSignalSpy(p.destinationLayerChanged)
        spy_destination_follows_active = QSignalSpy(p.destinationFollowsActiveLayerChanged)

        p.reset()
        self.assertFalse(p.automaticallyAddTrackVertices())
        self.assertFalse(p.automaticallyCommitFeatures())
        self.assertFalse(p.destinationLayer())
        self.assertTrue(p.destinationFollowsActiveLayer())

        self.assertEqual(len(spy_add_track), 1)
        self.assertFalse(spy_auto_commit[-1][0])
        self.assertEqual(len(spy_auto_commit), 1)
        self.assertFalse(spy_auto_commit[-1][0])
        self.assertEqual(len(spy_dest_layer_changed), 1)
        self.assertEqual(len(spy_destination_follows_active), 1)
        self.assertTrue(spy_destination_follows_active[-1][0])

    def testReadWrite(self):
        p = QgsProjectGpsSettings()

        p.setAutomaticallyCommitFeatures(True)
        p.setAutomaticallyAddTrackVertices(True)
        p.setDestinationFollowsActiveLayer(False)

        layer1 = QgsVectorLayer(os.path.join(unitTestDataPath(), 'lines.shp'), 'layer1')
        self.assertTrue(layer1.isValid())
        p.setDestinationLayer(layer1)

        project = QgsProject()
        project.addMapLayer(layer1)

        doc = QDomDocument("testdoc")
        elem = p.writeXml(doc, QgsReadWriteContext())

        p2 = QgsProjectGpsSettings()
        spy = QSignalSpy(p2.automaticallyAddTrackVerticesChanged)
        spy2 = QSignalSpy(p2.automaticallyCommitFeaturesChanged)
        spy_dest_layer_changed = QSignalSpy(p2.destinationLayerChanged)
        spy_destination_follows_active = QSignalSpy(p2.destinationFollowsActiveLayerChanged)

        self.assertTrue(p2.readXml(elem, QgsReadWriteContext()))
        self.assertEqual(len(spy), 1)
        self.assertEqual(len(spy2), 1)
        self.assertEqual(len(spy_dest_layer_changed), 1)
        self.assertEqual(len(spy_destination_follows_active), 1)
        self.assertTrue(p2.automaticallyCommitFeatures())
        self.assertTrue(p2.automaticallyAddTrackVertices())
        self.assertFalse(p2.destinationFollowsActiveLayer())
        # needs to be resolved first
        self.assertFalse(p2.destinationLayer())

        p2.resolveReferences(project)
        self.assertEqual(len(spy_dest_layer_changed), 2)
        self.assertEqual(p2.destinationLayer(), layer1)


if __name__ == '__main__':
    unittest.main()
