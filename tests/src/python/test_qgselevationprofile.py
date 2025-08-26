"""QGIS Unit tests for QgsElevationProfile.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    Qgis,
    QgsElevationProfile,
    QgsProject,
    QgsReadWriteContext,
    QgsCoordinateReferenceSystem,
    QgsLineString,
    QgsLineSymbol,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

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

        profile.setLayers([layer1, layer2])
        self.assertEqual(profile.layers(), [layer1, layer2])

        # TODO LAYERS

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

        # restoration of layers requires resolving references
        self.assertFalse(profile2.layers())
        profile2.resolveReferences(project)
        self.assertEqual(profile2.layers(), [layer1, layer2])


if __name__ == "__main__":
    unittest.main()
