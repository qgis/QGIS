# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsCoordinateReferenceSystemUtils.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2022 by Nyall Dawson'
__date__ = '06/04/2022'
__copyright__ = 'Copyright 2022, The QGIS Project'

import qgis  # NOQA

from qgis.core import (QgsCoordinateReferenceSystem,
                       QgsCoordinateReferenceSystemUtils,
                       Qgis)
from qgis.testing import start_app, unittest

start_app()


class TestQgsCoordinateReferenceSystemUtils(unittest.TestCase):

    def test_axis_order(self):
        """
        Test QgsCoordinateReferenceSystem.axisOrdering() (including the Python MethodCode associated with this)
        """
        self.assertEqual(QgsCoordinateReferenceSystemUtils.defaultCoordinateOrderForCrs(QgsCoordinateReferenceSystem()), Qgis.CoordinateOrder.XY)
        self.assertEqual(QgsCoordinateReferenceSystemUtils.defaultCoordinateOrderForCrs(QgsCoordinateReferenceSystem('EPSG:3111')), Qgis.CoordinateOrder.XY)
        self.assertEqual(QgsCoordinateReferenceSystemUtils.defaultCoordinateOrderForCrs(QgsCoordinateReferenceSystem('EPSG:4326')), Qgis.CoordinateOrder.YX)

    def test_axis_direction_to_abbreviation(self):
        """
        Test QgsCoordinateReferenceSystem.axisDirectionToAbbreviatedString()
        """
        self.assertEqual(QgsCoordinateReferenceSystemUtils.axisDirectionToAbbreviatedString(Qgis.CrsAxisDirection.North), 'N')
        self.assertEqual(QgsCoordinateReferenceSystemUtils.axisDirectionToAbbreviatedString(Qgis.CrsAxisDirection.East), 'E')
        self.assertEqual(QgsCoordinateReferenceSystemUtils.axisDirectionToAbbreviatedString(Qgis.CrsAxisDirection.CounterClockwise), 'CCW')


if __name__ == '__main__':
    unittest.main()
