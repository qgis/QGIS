# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsCoordinateReferenceSystem.

Note that most of the tests for this class are in the c++ test file!

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
                       Qgis)
from qgis.testing import start_app, unittest

start_app()


class TestQgsCoordinateReferenceSystem(unittest.TestCase):

    def test_axis_order(self):
        """
        Test QgsCoordinateReferenceSystem.axisOrdering() (including the Python MethodCode associated with this)
        """
        self.assertEqual(QgsCoordinateReferenceSystem().axisOrdering(), [])
        self.assertEqual(QgsCoordinateReferenceSystem('EPSG:4326').axisOrdering(), [Qgis.CrsAxisDirection.North, Qgis.CrsAxisDirection.East])
        self.assertEqual(QgsCoordinateReferenceSystem('EPSG:3111').axisOrdering(), [Qgis.CrsAxisDirection.East, Qgis.CrsAxisDirection.North])


if __name__ == '__main__':
    unittest.main()
