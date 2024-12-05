"""QGIS Unit tests for QgsScaleCalculator

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Mathieu Pellerin"
__date__ = "30/12/2021"
__copyright__ = "Copyright 2021, The QGIS Project"


from qgis.core import Qgis, QgsRectangle, QgsScaleCalculator
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsScaleCalculator(QgisTestCase):

    def testCalculate(self):
        calculator = QgsScaleCalculator()

        calculator.setDpi(96)
        extent = QgsRectangle(336609, 1162304, 354942, 1168151)

        calculator.setMapUnits(Qgis.DistanceUnit.Meters)
        scale = calculator.calculate(extent, 65000)
        self.assertAlmostEqual(scale, 1066.001, 3)

        calculator.setMapUnits(Qgis.DistanceUnit.Feet)
        scale = calculator.calculate(extent, 65000)
        self.assertAlmostEqual(scale, 324.9171, 3)

        calculator.setMapUnits(Qgis.DistanceUnit.Miles)
        scale = calculator.calculate(extent, 65000)
        self.assertAlmostEqual(scale, 1715562.6535, 3)

        calculator.setMapUnits(Qgis.DistanceUnit.NauticalMiles)
        scale = calculator.calculate(extent, 65000)
        self.assertAlmostEqual(scale, 1974234.24348, 3)

        calculator.setMapUnits(Qgis.DistanceUnit.Kilometers)
        scale = calculator.calculate(extent, 65000)
        self.assertAlmostEqual(scale, 1066001.21138, 3)

        calculator.setMapUnits(Qgis.DistanceUnit.Yards)
        scale = calculator.calculate(extent, 65000)
        self.assertAlmostEqual(scale, 974.7515076, 3)

        calculator.setMapUnits(Qgis.DistanceUnit.Millimeters)
        scale = calculator.calculate(extent, 65000)
        self.assertAlmostEqual(scale, 1.066001211, 3)

        calculator.setMapUnits(Qgis.DistanceUnit.Centimeters)
        scale = calculator.calculate(extent, 65000)
        self.assertAlmostEqual(scale, 10.66001211, 3)

    def testCalculateImageSize(self):
        calculator = QgsScaleCalculator()

        calculator.setDpi(96)
        calculator.setMapUnits(Qgis.DistanceUnit.Meters)

        extent = QgsRectangle(336609, 1162304, 354942, 1168151)
        image_size = calculator.calculateImageSize(extent, 65000)
        self.assertAlmostEqual(image_size.width(), 1066.001, 3)
        self.assertAlmostEqual(image_size.height(), 339.983, 3)


if __name__ == "__main__":
    unittest.main()
