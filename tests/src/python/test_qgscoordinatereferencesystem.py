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

        # vertical crs
        crs = QgsCoordinateReferenceSystem("EPSG:5703").toGeocentricCrs()
        self.assertFalse(crs.isValid())

        # compound crs
        crs = QgsCoordinateReferenceSystem("EPSG:5500").toGeocentricCrs()
        self.assertTrue(crs.isValid())
        self.assertEqual(crs.type(), Qgis.CrsType.Geocentric)
        self.assertEqual(
            crs.toProj(),
            "+proj=geocent +ellps=GRS80 +units=m +no_defs +type=crs",
        )
        self.assertEqual(crs.ellipsoidAcronym(), "EPSG:7019")
        self.assertEqual(crs.celestialBodyName(), "Earth")

        # bound crs
        bound_crs = QgsCoordinateReferenceSystem()
        bound_crs.createFromWkt(
            """BOUNDCRS[SOURCECRS[PROJCRS["MGI / Austria Lambert",BASEGEOGCRS["MGI",DATUM["Militar-Geographische Institut",ELLIPSOID["Bessel 1841",6377397.155,299.1528128,LENGTHUNIT["metre",1]]],PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433]]],CONVERSION["unnamed",METHOD["Lambert Conic Conformal (2SP)",ID["EPSG",9802]],PARAMETER["Longitude of false origin",13.3333333333333,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8822]],PARAMETER["Latitude of false origin",47.5,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8821]],PARAMETER["Latitude of 1st standard parallel",49,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8823]],PARAMETER["Easting at false origin",400000,LENGTHUNIT["m",1],ID["EPSG",8826]],PARAMETER["Northing at false origin",400000,LENGTHUNIT["m",1],ID["EPSG",8827]],PARAMETER["scale_factor",1,SCALEUNIT["unity",1]],PARAMETER["Latitude of 2nd standard parallel",46,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8824]]],CS[Cartesian,2],AXIS["easting",east,ORDER[1],LENGTHUNIT["m",1]],AXIS["northing",north,ORDER[2],LENGTHUNIT["m",1]],ID["EPSG",31287]]],TARGETCRS[GEOGCRS["WGS 84",DATUM["World Geodetic System 1984",ELLIPSOID["WGS 84",6378137,298.257223563,LENGTHUNIT["metre",1]]],PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433]],CS[ellipsoidal,2],AXIS["latitude",north,ORDER[1],ANGLEUNIT["degree",0.0174532925199433]],AXIS["longitude",east,ORDER[2],ANGLEUNIT["degree",0.0174532925199433]],ID["EPSG",4326]]],ABRIDGEDTRANSFORMATION["Transformation from MGI to WGS84",METHOD["Position Vector transformation (geog2D domain)",ID["EPSG",9606]],PARAMETER["X-axis translation",601.705,ID["EPSG",8605]],PARAMETER["Y-axis translation",84.263,ID["EPSG",8606]],PARAMETER["Z-axis translation",485.227,ID["EPSG",8607]],PARAMETER["X-axis rotation",4.7354,ID["EPSG",8608]],PARAMETER["Y-axis rotation",-1.3145,ID["EPSG",8609]],PARAMETER["Z-axis rotation",-5.393,ID["EPSG",8610]],PARAMETER["Scale difference",0.9999976113,ID["EPSG",8611]]]]"""
        )
        self.assertTrue(bound_crs.isValid())
        self.assertEqual(bound_crs.type(), Qgis.CrsType.Bound)
        crs = bound_crs.toGeocentricCrs()
        self.assertTrue(crs.isValid())
        self.assertEqual(crs.type(), Qgis.CrsType.Geocentric)
        self.assertEqual(
            crs.toProj(),
            "+proj=geocent +ellps=bessel +units=m +no_defs +type=crs",
        )
        self.assertAlmostEqual(
            float(crs.ellipsoidAcronym().split(":")[1]), 6377397.155, -1
        )
        self.assertAlmostEqual(
            float(crs.ellipsoidAcronym().split(":")[2]), 6356078.962818189, -1
        )
        self.assertEqual(crs.celestialBodyName(), "Earth")


if __name__ == "__main__":
    unittest.main()
