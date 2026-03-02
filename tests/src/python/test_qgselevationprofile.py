"""QGIS Unit tests for QgsElevationProfile.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import unittest

from qgis.core import (
    Qgis,
    QgsCoordinateReferenceSystem,
    QgsElevationProfile,
    QgsLineString,
    QgsLineSymbol,
    QgsProject,
    QgsReadWriteContext,
    QgsVectorLayer,
)
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument
from qgis.testing import QgisTestCase, start_app
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsElevationProfile(QgisTestCase):
    def testProfile(self):
        project = QgsProject()
        profile = QgsElevationProfile(project)

        layer1 = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer", "l1", "memory"
        )
        layer2 = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer", "l2", "memory"
        )
        project.addMapLayers([layer1, layer2])

        profile.setName("test profile")
        self.assertEqual(profile.name(), "test profile")

        profile.setCrs(QgsCoordinateReferenceSystem("EPSG:3111"))
        self.assertEqual(profile.crs(), QgsCoordinateReferenceSystem("EPSG:3111"))

        profile.setProfileCurve(QgsLineString([[0, 0], [10, 2]]))
        self.assertEqual(profile.profileCurve().asWkt(), "LineString (0 0, 10 2)")

        profile.setTolerance(5)
        self.assertEqual(profile.tolerance(), 5)

        profile.setLockAxisScales(True)
        self.assertTrue(profile.lockAxisScales())

        profile.setDistanceUnit(Qgis.DistanceUnit.Feet)
        self.assertEqual(profile.distanceUnit(), Qgis.DistanceUnit.Feet)

        line_symbol = QgsLineSymbol.createSimple({"color": "red"})
        profile.setSubsectionsSymbol(line_symbol)
        self.assertEqual(profile.subsectionsSymbol().color().name(), "#ff0000")

        profile.layerTree().addLayer(layer1)
        group = profile.layerTree().addGroup("my group")
        group.addLayer(layer2)

        # save to xml
        context = QgsReadWriteContext()
        doc = QDomDocument("testdoc")
        elem = profile.writeXml(doc, context)
        doc.appendChild(elem)

        # restore from xml
        profile2 = QgsElevationProfile(project)
        self.assertTrue(profile2.readXml(elem, doc, context))
        self.assertEqual(profile2.name(), "test profile")
        self.assertEqual(profile2.crs(), QgsCoordinateReferenceSystem("EPSG:3111"))
        self.assertEqual(profile2.profileCurve().asWkt(), "LineString (0 0, 10 2)")
        self.assertEqual(profile2.tolerance(), 5)
        self.assertTrue(profile2.lockAxisScales())
        self.assertEqual(profile2.distanceUnit(), Qgis.DistanceUnit.Feet)
        self.assertEqual(profile2.subsectionsSymbol().color().name(), "#ff0000")
        self.assertFalse(profile2.useProjectLayerTree())

        # restoration of layers requires resolving references
        self.assertEqual(
            [l.name() for l in profile2.layerTree().findLayers()], ["l1", "l2"]
        )
        self.assertEqual(
            [l.layer() for l in profile2.layerTree().findLayers()], [None, None]
        )
        profile2.resolveReferences(project)
        self.assertEqual(
            [l.layer() for l in profile2.layerTree().findLayers()], [layer1, layer2]
        )
        # make sure group structure was restored
        self.assertEqual(
            [n.name() for n in profile2.layerTree().children()], ["l1", "my group"]
        )
        group = profile2.layerTree().findGroup("my group")
        self.assertEqual([l.name() for l in group.children()], ["l2"])

    def test_sync_project_layer_tree(self):
        project = QgsProject()
        profile = QgsElevationProfile(project)

        layer1 = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer", "l1", "memory"
        )
        layer2 = QgsVectorLayer(
            "Point?field=fldtxt:string&field=fldint:integer", "l2", "memory"
        )
        project.addMapLayers([layer1, layer2])

        profile.setName("test profile")
        self.assertFalse(profile.useProjectLayerTree())

        spy = QSignalSpy(profile.useProjectLayerTreeChanged)
        profile.setUseProjectLayerTree(True)
        self.assertEqual(len(spy), 1)
        self.assertIsNone(profile.layerTree())

        profile.setUseProjectLayerTree(True)
        self.assertEqual(len(spy), 1)

        profile.setUseProjectLayerTree(False)
        self.assertFalse(profile.useProjectLayerTree())
        self.assertEqual(len(spy), 2)
        profile.setUseProjectLayerTree(True)

        # save to xml
        context = QgsReadWriteContext()
        doc = QDomDocument("testdoc")
        elem = profile.writeXml(doc, context)
        doc.appendChild(elem)

        # restore from xml
        profile2 = QgsElevationProfile(project)
        self.assertTrue(profile2.readXml(elem, doc, context))
        self.assertEqual(profile2.name(), "test profile")
        self.assertTrue(profile2.useProjectLayerTree())
        # no action, no crash
        profile2.resolveReferences(project)
        self.assertIsNone(profile2.layerTree())


if __name__ == "__main__":
    unittest.main()
