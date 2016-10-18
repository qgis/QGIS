# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsBearingUtils.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '18/10/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis # switch sip api

from qgis.core import (QgsBearingUtils,
                       QgsCoordinateReferenceSystem,
                       QgsPoint
                       )

from qgis.testing import (start_app,
                          unittest
                          )


start_app()


class TestQgsBearingUtils(unittest.TestCase):

    def testTrueNorth(self):
        """ test calculating bearing to true north"""

        # short circuit - already a geographic crs
        crs = QgsCoordinateReferenceSystem()
        crs.createFromOgcWmsCrs('EPSG:4326')
        self.assertEqual(QgsBearingUtils.bearingTrueNorth(crs, QgsPoint(0, 0)), 0)
        self.assertEqual(QgsBearingUtils.bearingTrueNorth(crs, QgsPoint(44, 0)), 0)
        self.assertEqual(QgsBearingUtils.bearingTrueNorth(crs, QgsPoint(44, -43)), 0)
        self.assertEqual(QgsBearingUtils.bearingTrueNorth(crs, QgsPoint(44, 43)), 0)

        self.assertEqual(QgsBearingUtils.bearingTrueNorth(crs, QgsPoint(44, 200)), 0)
        self.assertEqual(QgsBearingUtils.bearingTrueNorth(crs, QgsPoint(44, -200)), 0)

        # no short circuit
        crs.createFromOgcWmsCrs('EPSG:3111')
        self.assertAlmostEqual(QgsBearingUtils.bearingTrueNorth(crs, QgsPoint(2508807, 2423425)), 0.06, 2)

        # try a south-up crs
        crs.createFromOgcWmsCrs('EPSG:2053')
        self.assertAlmostEqual(QgsBearingUtils.bearingTrueNorth(crs, QgsPoint(29, -27.55)), -180.0, 1)

        # try a north pole crs
        crs.createFromOgcWmsCrs('EPSG:3575')
        self.assertAlmostEqual(QgsBearingUtils.bearingTrueNorth(crs, QgsPoint(-780770, 652329)), 129.9, 1)
        self.assertAlmostEqual(QgsBearingUtils.bearingTrueNorth(crs, QgsPoint(513480, 873173)), -149.5, 1)

if __name__ == '__main__':
    unittest.main()
