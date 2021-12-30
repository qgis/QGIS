# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsScaleCalculator

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Mathieu Pellerin'
__date__ = '30/12/2021'
__copyright__ = 'Copyright 2021, The QGIS Project'

import qgis  # NOQA
import math
from qgis.PyQt.QtCore import Qt, QSizeF
from qgis.PyQt.QtTest import QSignalSpy

from qgis.core import QgsScaleCalculator, QgsRectangle, QgsUnitTypes
from qgis.testing import start_app, unittest

start_app()


class TestQgsScaleCalculator(unittest.TestCase):

    def testCalculateImageSize(self):
        calculator = QgsScaleCalculator()

        calculator.setDpi(96)
        calculator.setMapUnits(QgsUnitTypes.DistanceMeters)

        extent = QgsRectangle(336609, 1162304, 354942, 1168151)
        image_size = calculator.calculateImageSize(extent, 65000)
        self.assertAlmostEqual(image_size.width(), 1066.001, 3)
        self.assertAlmostEqual(image_size.height(), 339.983, 3)


if __name__ == '__main__':
    unittest.main()
