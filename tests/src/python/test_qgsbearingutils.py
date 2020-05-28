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

import qgis  # NOQA switch sip api

from qgis.core import (QgsBearingUtils,
                       QgsCoordinateReferenceSystem,
                       QgsCoordinateTransformContext,
                       QgsPointXY
                       )

from qgis.testing import start_app, unittest


start_app()


class TestQgsBearingUtils(unittest.TestCase):

    def testTrueNorth(self):
        """ test calculating bearing to true north"""

        # short circuit - already a geographic crs
        crs = QgsCoordinateReferenceSystem.fromEpsgId(4326)
        transformContext = QgsCoordinateTransformContext()

        self.assertEqual(QgsBearingUtils.bearingTrueNorth(crs, transformContext, QgsPointXY(0, 0)), 0)
        self.assertEqual(QgsBearingUtils.bearingTrueNorth(crs, transformContext, QgsPointXY(44, 0)), 0)
        self.assertEqual(QgsBearingUtils.bearingTrueNorth(crs, transformContext, QgsPointXY(44, -43)), 0)
        self.assertEqual(QgsBearingUtils.bearingTrueNorth(crs, transformContext, QgsPointXY(44, 43)), 0)

        self.assertEqual(QgsBearingUtils.bearingTrueNorth(crs, transformContext, QgsPointXY(44, 200)), 0)
        self.assertEqual(QgsBearingUtils.bearingTrueNorth(crs, transformContext, QgsPointXY(44, -200)), 0)

        # no short circuit
        crs = QgsCoordinateReferenceSystem.fromEpsgId(3111)
        self.assertAlmostEqual(QgsBearingUtils.bearingTrueNorth(crs, transformContext, QgsPointXY(2508807, 2423425)), 0.06, 2)

        # try a south-up crs
        crs = QgsCoordinateReferenceSystem.fromEpsgId(2053)
        self.assertAlmostEqual(QgsBearingUtils.bearingTrueNorth(crs, transformContext, QgsPointXY(29, -27.55)), -180.0, 1)

        # try a north pole crs
        crs = QgsCoordinateReferenceSystem.fromEpsgId(3575)
        self.assertAlmostEqual(QgsBearingUtils.bearingTrueNorth(crs, transformContext, QgsPointXY(-780770, 652329)), 129.9, 1)
        self.assertAlmostEqual(QgsBearingUtils.bearingTrueNorth(crs, transformContext, QgsPointXY(513480, 873173)), -149.5, 1)


if __name__ == '__main__':
    unittest.main()
