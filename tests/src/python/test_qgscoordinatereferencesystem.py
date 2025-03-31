"""QGIS Unit tests for QgsCoordinateReferenceSystem.

Note that most of the tests for this class are in the c++ test file!

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2022 by Nyall Dawson"
__date__ = "06/04/2022"
__copyright__ = "Copyright 2022, The QGIS Project"


from qgis.core import Qgis, QgsCoordinateReferenceSystem
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsCoordinateReferenceSystem(QgisTestCase):

    def test_axis_order(self):
        """
        Test QgsCoordinateReferenceSystem.axisOrdering() (including the Python MethodCode associated with this)
        """
        self.assertEqual(QgsCoordinateReferenceSystem().axisOrdering(), [])
        self.assertEqual(
            QgsCoordinateReferenceSystem("EPSG:4326").axisOrdering(),
            [Qgis.CrsAxisDirection.North, Qgis.CrsAxisDirection.East],
        )
        self.assertEqual(
            QgsCoordinateReferenceSystem("EPSG:3111").axisOrdering(),
            [Qgis.CrsAxisDirection.East, Qgis.CrsAxisDirection.North],
        )

    def test_geocentric_to_geographic_crs(self):
        res = QgsCoordinateReferenceSystem("EPSG:4978").toGeographicCrs()
        self.assertTrue(res.isValid())
        self.assertEqual(res.type(), Qgis.CrsType.Geographic2d)
        self.assertEqual(res.mapUnits(), Qgis.DistanceUnit.Degrees)
        self.assertEqual(res.ellipsoidAcronym(), "EPSG:7030")
        self.assertEqual(
            res.axisOrdering(),
            [Qgis.CrsAxisDirection.East, Qgis.CrsAxisDirection.North],
        )

        res = QgsCoordinateReferenceSystem("IGNF:ATI").toGeographicCrs()
        self.assertTrue(res.isValid())
        self.assertEqual(res.type(), Qgis.CrsType.Geographic2d)
        self.assertEqual(res.mapUnits(), Qgis.DistanceUnit.Degrees)
        self.assertEqual(res.ellipsoidAcronym(), "EPSG:7027")
        self.assertEqual(
            res.axisOrdering(),
            [Qgis.CrsAxisDirection.East, Qgis.CrsAxisDirection.North],
        )

    def test_create_geocentric_crs(self):
        self.assertFalse(QgsCoordinateReferenceSystem.createGeocentricCrs("").isValid())
        self.assertFalse(
            QgsCoordinateReferenceSystem.createGeocentricCrs("xxxxxx").isValid()
        )

        # using ellipsoid code
        crs = QgsCoordinateReferenceSystem.createGeocentricCrs("EPSG:7030")
        self.assertTrue(crs.isValid())
        self.assertEqual(crs.type(), Qgis.CrsType.Geocentric)
        self.assertEqual(
            crs.toProj(), "+proj=geocent +ellps=WGS84 +units=m +no_defs +type=crs"
        )
        self.assertEqual(
            crs.ellipsoidAcronym(), "PARAMETER:6378137:6356752.31424517929553986"
        )
        self.assertEqual(crs.celestialBodyName(), "Earth")

        # non-earth ellipsoid
        crs = QgsCoordinateReferenceSystem.createGeocentricCrs("IAU_2015:49901")
        self.assertTrue(crs.isValid())
        self.assertEqual(crs.type(), Qgis.CrsType.Geocentric)
        self.assertEqual(
            crs.toProj(),
            "+proj=geocent +a=3396190 +rf=169.894447223612 +units=m +no_defs +type=crs",
        )
        self.assertEqual(crs.ellipsoidAcronym(), "PARAMETER:3396190:3376200")
        self.assertEqual(crs.celestialBodyName(), "Mars")

        # custom ellipsoid
        crs = QgsCoordinateReferenceSystem.createGeocentricCrs("PARAMETER:1200:800")
        self.assertTrue(crs.isValid())
        self.assertEqual(crs.type(), Qgis.CrsType.Geocentric)
        self.assertEqual(
            crs.toProj(), "+proj=geocent +a=1200 +rf=3 +units=m +no_defs +type=crs"
        )
        self.assertEqual(crs.ellipsoidAcronym()[:18], "PARAMETER:1200:800")
        self.assertEqual(crs.celestialBodyName(), "Non-Earth body")

    def test_to_geocentric_crs(self):
        self.assertFalse(QgsCoordinateReferenceSystem().toGeocentricCrs().isValid())

        # using WGS84 datum
        crs = QgsCoordinateReferenceSystem("EPSG:4326").toGeocentricCrs()
        self.assertTrue(crs.isValid())
        self.assertEqual(crs.type(), Qgis.CrsType.Geocentric)
        self.assertEqual(
            crs.toProj(), "+proj=geocent +datum=WGS84 +units=m +no_defs +type=crs"
        )
        self.assertEqual(crs.ellipsoidAcronym(), "EPSG:7030")
        self.assertEqual(crs.celestialBodyName(), "Earth")

        # non WGS84 datum
        crs = QgsCoordinateReferenceSystem("EPSG:28356").toGeocentricCrs()
        self.assertTrue(crs.isValid())
        self.assertEqual(crs.type(), Qgis.CrsType.Geocentric)
        self.assertEqual(
            crs.toProj(), "+proj=geocent +ellps=GRS80 +units=m +no_defs +type=crs"
        )
        self.assertEqual(crs.ellipsoidAcronym(), "EPSG:7019")
        self.assertEqual(crs.celestialBodyName(), "Earth")

        # non-earth ellipsoid, spherical
        crs = QgsCoordinateReferenceSystem("ESRI:103883").toGeocentricCrs()
        self.assertTrue(crs.isValid())
        self.assertEqual(crs.type(), Qgis.CrsType.Geocentric)
        self.assertEqual(
            crs.toProj(),
            "+proj=geocent +R=3396190 +units=m +no_defs +type=crs",
        )
        self.assertEqual(crs.ellipsoidAcronym(), "PARAMETER:3396190:3396190")
        self.assertEqual(crs.celestialBodyName(), "Mars")

        # non-earth ellipsoid, ellipsoidal
        crs = QgsCoordinateReferenceSystem("IAU_2015:49936").toGeocentricCrs()
        self.assertTrue(crs.isValid())
        self.assertEqual(crs.type(), Qgis.CrsType.Geocentric)
        self.assertEqual(
            crs.toProj(),
            "+proj=geocent +a=3396190 +b=3376200 +units=m +no_defs +type=crs",
        )
        self.assertEqual(crs.ellipsoidAcronym(), "IAU_2015:49901")
        self.assertEqual(crs.celestialBodyName(), "Mars")


if __name__ == "__main__":
    unittest.main()
