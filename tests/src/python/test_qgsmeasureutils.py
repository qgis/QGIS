"""QGIS Unit tests for QgsMeasureUtils

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import QgsMeasureUtils, QgsProject, Qgis, QgsSettings
import unittest
from qgis.testing import start_app, QgisTestCase


class TestQgsMeasureUtils(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestQgsMeasureUtils.com")
        QCoreApplication.setApplicationName("QGIS_TestQgsMeasureUtils")
        QgsSettings().clear()
        start_app()

    def test_format_distance(self):
        QgsSettings().setValue("qgis/measure/keepbaseunit", True)

        p = QgsProject()
        p.setDistanceUnits(Qgis.DistanceUnit.Feet)

        self.assertEqual(
            QgsMeasureUtils.formatDistanceForProject(
                p, 145.6, Qgis.DistanceUnit.Meters
            ),
            "477.690 ft",
        )
        self.assertEqual(
            QgsMeasureUtils.formatDistanceForProject(
                p, 145.6123, Qgis.DistanceUnit.Meters
            ),
            "477.731 ft",
        )
        self.assertEqual(
            QgsMeasureUtils.formatDistanceForProject(
                p, 145.6123, Qgis.DistanceUnit.Kilometers
            ),
            "477,730.643 ft",
        )

        p.setDistanceUnits(Qgis.DistanceUnit.Meters)
        self.assertEqual(
            QgsMeasureUtils.formatDistanceForProject(
                p, 145.6, Qgis.DistanceUnit.Meters
            ),
            "145.600 m",
        )
        self.assertEqual(
            QgsMeasureUtils.formatDistanceForProject(
                p, 145.6, Qgis.DistanceUnit.Kilometers
            ),
            "145,600.000 m",
        )
        self.assertEqual(
            QgsMeasureUtils.formatDistanceForProject(p, 145.6, Qgis.DistanceUnit.Feet),
            "44.379 m",
        )

        p.setDistanceUnits(Qgis.DistanceUnit.Degrees)
        self.assertEqual(
            QgsMeasureUtils.formatDistanceForProject(
                p, 145.6, Qgis.DistanceUnit.Meters
            ),
            "0.001308 deg",
        )
        self.assertEqual(
            QgsMeasureUtils.formatDistanceForProject(
                p, 145.6, Qgis.DistanceUnit.Kilometers
            ),
            "1.307947 deg",
        )

    def test_format_distance_no_keep_base_unit(self):
        QgsSettings().setValue("qgis/measure/keepbaseunit", False)

        p = QgsProject()
        p.setDistanceUnits(Qgis.DistanceUnit.Feet)

        self.assertEqual(
            QgsMeasureUtils.formatDistanceForProject(
                p, 145.6, Qgis.DistanceUnit.Meters
            ),
            "477.690 ft",
        )
        self.assertEqual(
            QgsMeasureUtils.formatDistanceForProject(
                p, 145.6123, Qgis.DistanceUnit.Meters
            ),
            "477.731 ft",
        )
        self.assertEqual(
            QgsMeasureUtils.formatDistanceForProject(
                p, 145.6123, Qgis.DistanceUnit.Kilometers
            ),
            "90.479 mi",
        )

        p.setDistanceUnits(Qgis.DistanceUnit.Meters)
        self.assertEqual(
            QgsMeasureUtils.formatDistanceForProject(
                p, 145.6, Qgis.DistanceUnit.Meters
            ),
            "145.600 m",
        )
        self.assertEqual(
            QgsMeasureUtils.formatDistanceForProject(
                p, 145.6, Qgis.DistanceUnit.Kilometers
            ),
            "145.600 km",
        )
        self.assertEqual(
            QgsMeasureUtils.formatDistanceForProject(p, 145.6, Qgis.DistanceUnit.Feet),
            "44.379 m",
        )

        p.setDistanceUnits(Qgis.DistanceUnit.Degrees)
        self.assertEqual(
            QgsMeasureUtils.formatDistanceForProject(
                p, 145.6, Qgis.DistanceUnit.Meters
            ),
            "0.001308 deg",
        )
        self.assertEqual(
            QgsMeasureUtils.formatDistanceForProject(
                p, 145.6, Qgis.DistanceUnit.Kilometers
            ),
            "1.307947 deg",
        )

    def test_format_area(self):
        QgsSettings().setValue("qgis/measure/keepbaseunit", True)

        p = QgsProject()
        p.setAreaUnits(Qgis.AreaUnit.SquareFeet)

        self.assertEqual(
            QgsMeasureUtils.formatAreaForProject(p, 145.6, Qgis.AreaUnit.SquareMeters),
            "1,567.225 ft²",
        )
        self.assertEqual(
            QgsMeasureUtils.formatAreaForProject(
                p, 145.6123, Qgis.AreaUnit.SquareMeters
            ),
            "1,567.358 ft²",
        )
        self.assertEqual(
            QgsMeasureUtils.formatAreaForProject(
                p, 145.6123, Qgis.AreaUnit.SquareKilometers
            ),
            "1,567,357,752.771 ft²",
        )

        p.setAreaUnits(Qgis.AreaUnit.SquareMeters)
        self.assertEqual(
            QgsMeasureUtils.formatAreaForProject(p, 145.6, Qgis.AreaUnit.SquareMeters),
            "145.600 m²",
        )
        self.assertEqual(
            QgsMeasureUtils.formatAreaForProject(
                p, 145.6, Qgis.AreaUnit.SquareKilometers
            ),
            "145,600,000.000 m²",
        )
        self.assertEqual(
            QgsMeasureUtils.formatAreaForProject(p, 145.6, Qgis.AreaUnit.SquareFeet),
            "13.527 m²",
        )

        p.setAreaUnits(Qgis.AreaUnit.SquareDegrees)
        self.assertEqual(
            QgsMeasureUtils.formatAreaForProject(p, 145.6, Qgis.AreaUnit.SquareMeters),
            "1.174949e-08 deg²",
        )
        self.assertEqual(
            QgsMeasureUtils.formatAreaForProject(
                p, 145.6, Qgis.AreaUnit.SquareKilometers
            ),
            "0.011749 deg²",
        )

    def test_format_area_no_keep_base_unit(self):
        QgsSettings().setValue("qgis/measure/keepbaseunit", False)

        p = QgsProject()
        p.setAreaUnits(Qgis.AreaUnit.SquareFeet)

        self.assertEqual(
            QgsMeasureUtils.formatAreaForProject(p, 145.6, Qgis.AreaUnit.SquareMeters),
            "1,567.225 ft²",
        )
        self.assertEqual(
            QgsMeasureUtils.formatAreaForProject(
                p, 145.6123, Qgis.AreaUnit.SquareMeters
            ),
            "1,567.358 ft²",
        )
        self.assertEqual(
            QgsMeasureUtils.formatAreaForProject(
                p, 145.6123, Qgis.AreaUnit.SquareKilometers
            ),
            "56.221 mi²",
        )

        p.setAreaUnits(Qgis.AreaUnit.SquareMeters)
        self.assertEqual(
            QgsMeasureUtils.formatAreaForProject(p, 145.6, Qgis.AreaUnit.SquareMeters),
            "145.600 m²",
        )
        self.assertEqual(
            QgsMeasureUtils.formatAreaForProject(
                p, 145.6, Qgis.AreaUnit.SquareKilometers
            ),
            "145.600 km²",
        )
        self.assertEqual(
            QgsMeasureUtils.formatAreaForProject(p, 145.6, Qgis.AreaUnit.SquareFeet),
            "13.527 m²",
        )

        p.setAreaUnits(Qgis.AreaUnit.SquareDegrees)
        self.assertEqual(
            QgsMeasureUtils.formatAreaForProject(p, 145.6, Qgis.AreaUnit.SquareMeters),
            "1.174949e-08 deg²",
        )
        self.assertEqual(
            QgsMeasureUtils.formatAreaForProject(
                p, 145.6, Qgis.AreaUnit.SquareKilometers
            ),
            "0.011749 deg²",
        )


if __name__ == "__main__":
    unittest.main()
