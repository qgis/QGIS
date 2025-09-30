"""QGIS Unit tests for QgsLocaleUtils

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.PyQt.QtCore import QLocale
from qgis.core import (
    QgsLocaleUtils,
    QgsRectangle,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsLocaleUtils(QgisTestCase):

    def test_country_bounds(self):
        self.assertTrue(QgsLocaleUtils.countryBounds("xxxx").isNull())
        self.assertAlmostEqual(
            QgsLocaleUtils.countryBounds("Australia").xMinimum(), 112.91944420, 4
        )
        self.assertAlmostEqual(
            QgsLocaleUtils.countryBounds("Australia").xMaximum(), 159.106455, 4
        )
        self.assertAlmostEqual(
            QgsLocaleUtils.countryBounds("Australia").yMinimum(), -54.750420, 4
        )
        self.assertAlmostEqual(
            QgsLocaleUtils.countryBounds("Australia").yMaximum(), -9.24016689, 4
        )
        self.assertAlmostEqual(
            QgsLocaleUtils.countryBounds("New Zealand").xMinimum(), -177.95799715, 4
        )
        self.assertAlmostEqual(
            QgsLocaleUtils.countryBounds("New Zealand").xMaximum(), 178.83904046, 4
        )
        self.assertAlmostEqual(
            QgsLocaleUtils.countryBounds("New Zealand").yMinimum(), -52.6003132, 4
        )
        self.assertAlmostEqual(
            QgsLocaleUtils.countryBounds("New Zealand").yMaximum(), -29.22275147, 4
        )

    def test_territory_bounds(self):
        self.assertAlmostEqual(
            QgsLocaleUtils.territoryBounds(QLocale("EN-AU")).xMinimum(), 112.91944420, 4
        )
        self.assertAlmostEqual(
            QgsLocaleUtils.territoryBounds(QLocale("EN-AU")).xMaximum(), 159.106455, 4
        )
        self.assertAlmostEqual(
            QgsLocaleUtils.territoryBounds(QLocale("EN-AU")).yMinimum(), -54.750420, 4
        )
        self.assertAlmostEqual(
            QgsLocaleUtils.territoryBounds(QLocale("EN-AU")).yMaximum(), -9.24016689, 4
        )


if __name__ == "__main__":
    unittest.main()
